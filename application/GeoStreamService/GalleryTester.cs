using GeoStreamService.Jobs;
using Microsoft.Practices.Unity;
using Pyxis.Publishing.Protocol;
using System;
using System.Linq;

namespace GeoStreamService
{
    internal class GalleryTester
    {
        [Dependency]
        public UnityContainer Container { get; set; }
        [Dependency]
        public IPipelineClient Client { get; set; }

        public enum TestLevel
        {
            Import,
            Initialize,
            Geometry,
            Definition,
            GetValue,
            Full
        }

        public void TestGallery(TestLevel testLevel, int start)
        {
            try
            {
                var allPipelines = Client.GetAllPipelines();
                var pipepineCount = allPipelines.Count();
                int counter = start;

                foreach (var pipeline in allPipelines.Skip(start))
                {
                    counter++;
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.GalleryTest, String.Format(" pipeline.ProcRef, {0} of {1} Test started ", counter, pipepineCount));
                    TestPipeline(testLevel, pipeline.ProcRef);
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.GalleryTest, String.Format("pipeline.ProcRef, {0} of {1} Test finished ", counter, pipepineCount));
                    GeoStreamService.WriteLine("==============================");
                }
            }
            catch (Exception e)
            {
                Log(GeoStreamService.TraceCategory.Error, "Exception in testing the gallery", ExceptionToString(e));
            }
            finally
            {
                Pyxis.Utilities.Logging.LogRepository.Flush();
            }
        }

        public void TestPipeline(TestLevel testLevel, string procRefStr)
        {
            try
            {
                //import
                var process = Import(procRefStr);
                Log(procRefStr, "Imported successfully");

                if (testLevel > TestLevel.Import)
                {

                    //Initialize
                    Initialize(process);
                    Log(procRefStr, "Initialized successfully");

                    if (testLevel > TestLevel.Initialize)
                    {
                        var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
                        var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                        if (coverage.isNotNull())
                        {
                            // If Output is a coverage..
                            TestCoverage(coverage, testLevel);
                        }
                        else if (featureCollection.isNotNull())
                        {
                            // If Output is a feature..
                            TestFeature(featureCollection, testLevel);
                        }
                        else
                        {
                            throw new Exception("Pipeline type is unknown");
                        }
                    }
                }
                Log(GeoStreamService.TraceCategory.GalleryTest, procRefStr, "Available");
            }
            catch (Exception e)
            {
                Log(GeoStreamService.TraceCategory.Error, procRefStr, "Offline");
                Log(GeoStreamService.TraceCategory.Error, procRefStr, ExceptionToString(e));
            }
        }

        private static void Initialize(IProcess_SPtr process)
        {
            process.initProc(true);
            if (process.getInitState() != IProcess.eInitStatus.knInitialized)
            {
                throw new Exception(process.getInitError().getError());
            }
        }

        private string ExceptionToString(Exception exception)
        {
            string result = "";
            var innerException = exception.InnerException;
            if (innerException != null)
            {
                result += ExceptionToString(innerException) + "\n";
            }
            result += "Exception Type:\n" + exception.GetType().Name;
            result += "Exception Message:\n" + exception.Message;
            result += "Stack Trace:\n" + exception.StackTrace;
            return result;
        }

        private IProcess_SPtr Import(string procRefStr)
        {
            var procRef = pyxlib.strToProcRef(procRefStr);
            var importJob = Container.Resolve<ImportJob>(new ParameterOverrides {
                            { "procRef", procRef },
                            { "pipelineClient", Container.Resolve<IPipelineClient>() }});
            importJob.JobFailed += delegate(object sender, EventArgs e) { Log(procRefStr, importJob.Exception.ToString()); };
            importJob.Execute();
            return importJob.Process;
        }

        private void Log(string key, string value)
        {
            Logging.Categories.GalleryTest.Log(key, value);
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.GalleryTest, "{0} : {1}", key, value);
        }
        private void Log(GeoStreamService.TraceCategory category, string key, string value)
        {
            Logging.Categories.GalleryTest.Log(key, value);
            GeoStreamService.WriteLine(category, "{0} : {1}", key, value);
        }

        private void TestCoverage(ICoverage_SPtr coverage, TestLevel testLevel)
        {
            // Testing the geometry
            PYXGeometry_SPtr geom = null;
            if (testLevel >= TestLevel.Geometry)
            {
                geom = coverage.getGeometry();
                if (geom.isNull())
                {
                    throw new Exception("Coverage geometry is null");
                }
            }

            PYXTableDefinition_CSPtr coverageDefinition = coverage.getCoverageDefinition();
            if (coverageDefinition.isNull())
            {
                throw new Exception("Coverage definition is null");
            }

            // Testing the coverage values
            if (testLevel >= TestLevel.GetValue)
            {
                //Create a possibly lower resolution collection
                PYXTileCollection collection = new PYXTileCollection();
                geom.copyTo(collection, Math.Min(11, geom.getCellResolution() - 11));

                int i = 10; // get value for the first i indices
                for (var it = collection.getIterator(); !it.end() && i > 0; it.next(), i--)
                {
                    var value = coverage.getCoverageTile(PYXTile.create(it.getIndex(), geom.getCellResolution()).get());
                    if (!value.isValueTileCompatible(coverageDefinition))
                    {
                        throw new Exception("Value tile for tile " + it.getIndex().toString() + " at resolution " + geom.getCellResolution() + " is not compatible with the coverage definition");
                    }
                }
            }
        }

        private void TestFeature(IFeatureCollection_SPtr featureCollection, TestLevel testLevel)
        {
            // Testing the geometry
            if (testLevel >= TestLevel.Geometry)
            {
                var geom = featureCollection.getGeometry();
                if (geom.isNull())
                {
                    throw new Exception("Feature collection geometry is null");
                }
            }

            // Testing the features
            if (testLevel >= TestLevel.Definition)
            {
                var featureEnum = featureCollection.GetFeaturesEnumerator();
                var collectionDefinition = featureCollection.getFeatureDefinition();
                if (collectionDefinition.isNull())
                {
                    throw new Exception("Feature Collection definition is Null");
                }
                foreach (var feature in featureEnum.Take(1000)) // test the first 1000 features
                {
                    if (feature.getGeometry().isNull())
                    {
                        throw new Exception("Feature geometry is Null");
                    }

                    var definition = feature.getDefinition();
                    if (definition.isNull())
                    {
                        throw new Exception("Feature definition is Null");
                    }
                    if (definition.getFieldCount() != collectionDefinition.getFieldCount())
                    {
                        throw new Exception("Feature definition is different from parent feature collection");
                    }
                }
            }
        }
    }
}