using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.Import;

namespace Pyxis.WorldView.Studio
{
    /// <summary>
    /// Imports a series of data sets one at a time from a directory and generates a log file named
    /// TestLocalDataSources YYYY-MM-DDTHH-MM-SS.txt so log files from subsequent runs can be compared
    /// with a tool like WinDiff. The log file it output to the root of the test directory.
    /// </summary>
    public class ImportTest
    {
        /// <summary>
        /// The engine
        /// </summary>
        private Engine m_engine;

        /// <summary>
        /// Separator line used in the log file
        /// </summary>
        private static string s_separator =
            "--------------------------------------------------------------------------------";

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="engine">The engine</param>
        public ImportTest(Engine engine)
        {
            m_engine = engine;
        }

        /// <summary>
        /// Perform a test of all local data sources in the specified path.
        /// </summary>
        /// <param name="path">The path to the root folder</param>
        /// <param name="clearCache">true if the cache was cleared when the engine was created.</param>
        public void TestLocalDataSources(string path, bool clearCache)
        {
            if (m_engine == null)
            {
                return;
            }

            var dateTime = DateTime.Now.ToString("yyyy'-'MM'-'dd'T'HH'-'mm'-'ss");

            // create log file
            var sw = new StreamWriter(path +  "\\" + "TestLocalDataSources " + dateTime + ".txt");

            sw.WriteLine("START LOCAL DATA SOURCE TEST");
            sw.WriteLine("Path: " + path);
            sw.WriteLine("DateTime: " + dateTime);
            sw.WriteLine("Cache state: " + (clearCache ? "cleared" : "not cleared"));
            sw.Flush();

            // start a timer so we can time the operations
            var stopwatch = Stopwatch.StartNew();

            // find all data sets in the root directory
            int nDataSetsTested = 0;
            DataSetCatalog catalog = m_engine.BuildCatalog(path);
            if (catalog != null)
            {
                nDataSetsTested = TestCatalog(sw, catalog);
                sw.WriteLine(s_separator);
            }
            else
            {
                sw.Write("ERROR: No data sets found in path");
            }

            sw.WriteLine("Number of data sets tested: " + nDataSetsTested);

            stopwatch.Stop();
            sw.WriteLine("Total time for test (ms): " + stopwatch.ElapsedMilliseconds);

            sw.WriteLine("END LOCAL DATA SOURCE TEST");
            sw.Close();
        }

        /// <summary>
        /// Test all data sets in the specified catalog.
        /// </summary>
        /// <param name="sw">StreamWriter for logging</param>
        /// <param name="catalog">The catalog</param>
        /// <returns>The number of data sets tested</returns>
        private int TestCatalog(StreamWriter sw, DataSetCatalog catalog)
        {
            int nDataSetsTested = 0;

            // test each data set in the catalog
            foreach (var dataSet in catalog.DataSets)
            {
                TestDataSet(sw, dataSet);
                nDataSetsTested++;
            }

            // test each subcatalog in the catalog
            foreach (var subCatalog in catalog.SubCatalogs)
            {
                nDataSetsTested += TestCatalog(sw, subCatalog);
            }

            return nDataSetsTested;
        }

        /// <summary>
        /// Test the specified data set.
        /// </summary>
        /// <param name="sw">StreamWriter for logging</param>
        /// <param name="dataSet">The data set</param>
        private void TestDataSet(StreamWriter sw, DataSet dataSet)
        {
            sw.WriteLine(s_separator);

            // log some header information
            sw.WriteLine("DATA SET INFO");
            sw.WriteLine("URI: " + dataSet.Uri);
            sw.WriteLine("Layer: " + dataSet.Layer);
            sw.WriteLine("Name: " + dataSet.Metadata.Name);
            sw.WriteLine("Description: " + dataSet.Metadata.Description);

            // start a timer so we can time the operations
            var stopwatch = Stopwatch.StartNew();

            bool success = false;
            try
            {
                // import the geosource
                var geosource = m_engine.BeginImport(dataSet).Task.Result;
                if (geosource != null)
                {
                    var process = m_engine.GetProcess(geosource);
                    if (process != null && process.isNotNull())
                    {
                        var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
                        var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                        if (coverage != null && coverage.isNotNull())
                        {
                            sw.WriteLine("Type: coverage");
                        }
                        else if (featureCollection != null && featureCollection.isNotNull())
                        {
                            sw.WriteLine("Type: feature collection");

                            var featureDefinition = featureCollection.getFeatureDefinition();
                            sw.WriteLine("- Start of Feature Definition -");
                            TestTableDefinition(sw, featureDefinition);
                            sw.WriteLine("- End of Feature Definition -");

                            // convert the feature collection into a coverage to mimic how the Studio sees it
                            sw.WriteLine("Converting feature collection to a coverage");
                            process = CoverageFromFeatureCollection(process);
                            coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
                            if (coverage == null || coverage.isNull())
                            {
                                sw.WriteLine("ERROR: Unable to create coverage from feature collection");
                            }
                        }
                        else
                        {
                            sw.WriteLine("ERROR: Unsupported output type");
                        }

                        if (coverage != null && coverage.isNotNull())
                        {
                            success = true;

                            // record the time it took to initialize the geosource
                            stopwatch.Stop();
                            sw.WriteLine("Initialization time (ms): " + stopwatch.ElapsedMilliseconds);
                            stopwatch.Restart();

                            TestCoverage(sw, coverage);

                            // record the time it took to test the data in the geosource
                            stopwatch.Stop();
                            sw.WriteLine("Data read time (ms): " + stopwatch.ElapsedMilliseconds);
                        }
                    }       
                }
            }
            catch (Exception)
            {
                // fall through
            }

            if (!success)
            {
                sw.WriteLine("ERROR: Failed to build pipeline");                
            }

            sw.Flush();
        }

        /// <summary>
        /// Test the specified coverage.
        /// </summary>
        /// <param name="sw">StreamWriter for logging</param>
        /// <param name="coverage">The coverage</param>
        private void TestCoverage(StreamWriter sw, ICoverage_SPtr coverage)
        {
            // test the geometry
            PYXGeometry_SPtr geom = coverage.getGeometry();
            if (geom == null || geom.isNull())
            {
                sw.WriteLine("ERROR: Geometry is null");
                return;
            }
            sw.WriteLine("Resolution: " + geom.getCellResolution());

            var coverageDefinition = coverage.getCoverageDefinition();
            sw.WriteLine("- Start of Coverage Definition -");
            TestTableDefinition(sw, coverageDefinition);
            sw.WriteLine("- End of Coverage Definition -");

            // sample the coverage at a lower (overview) resolution
            int samplingResolution = Math.Max(PYXTile.knDefaultTileDepth, geom.getCellResolution() - PYXTile.knDefaultTileDepth);
            sw.WriteLine("Sampling resolution: " + samplingResolution);

            PYXTileCollection collection = new PYXTileCollection();
            geom.copyTo(collection, samplingResolution);
            
            sw.WriteLine("- Start of Tile Info -");

            // sample n tiles from the collection
            const int numTilesToSample = 10;
            int interval = collection.getGeometryCount() / (numTilesToSample - 1);

            int i = 0;
            for (var it = collection.getTileIterator(); !it.end(); it.next(), ++i)
            {
                if (i % interval == 0)
                {
                    PYXIcosIndex idx = it.getTile().getRootIndex();
                    idx.setResolution(Math.Max(PYXIcosIndex.knMinSubRes, samplingResolution - PYXTile.knDefaultTileDepth));
                    var valueTile = coverage.getCoverageTile(PYXTile.create(idx, samplingResolution).get());
                    if (!valueTile.isValueTileCompatible(coverageDefinition))
                    {
                        sw.WriteLine(   "ERROR: Value tile for tile " + valueTile.getTile().getRootIndex().toString() +
                                        " at resolution " + geom.getCellResolution() +
                                        " is not compatible with the coverage definition"   );
                        return;
                    }

                    TestValueTile(sw, valueTile);                   
                }
            }

            sw.WriteLine("- End of Tile Info -");
        }

        /// <summary>
        /// Convert a feature collection to a coverage.
        /// </summary>
        /// <param name="featureCollection">The feature collection.</param>
        /// <param name="style">The style to use (optional)</param>
        /// <returns>The corresponding coverage</returns>
        private static IProcess_SPtr CoverageFromFeatureCollection(IProcess_SPtr featureCollection, Style style = null)
        {
            if (style == null)
            {
                style = new Style
                {
                    Fill = FieldStyle.FromColor(Color.White),
                    Line = FieldStyle.FromColor(Color.Gray)
                };
            }

            var styleProcess = style.ApplyStyle(featureCollection);

            var rasterizer =
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.StyledFeaturesRasterizer)
                    .AddInput(0, styleProcess)
                    .AddAttribute("UseAlpha", "1");
            var rasterizerProcess = PYXCOMFactory.CreateProcess(rasterizer);

            var styledFeatures =
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageCache)
                    .AddInput(0, rasterizerProcess);
            var process = PYXCOMFactory.CreateProcess(styledFeatures);

            process.initProc(true);

            return process;
        }

        /// <summary>
        /// Test a tile from a coverage.
        /// </summary>
        /// <param name="sw">StreamWriter for logging</param>
        /// <param name="valueTile">The value tile</param>
        private static void TestValueTile(StreamWriter sw, PYXValueTile_SPtr valueTile)
        {
            // calculate statistics
            var stats = valueTile.calcStatistics();

            // log the statistics
            PYXIcosIndex idx = valueTile.getTile().getRootIndex();
            sw.WriteLine(   "Tile " + idx.toString() + ":" +
                            " #cells = " + stats.nCells +
                            " #values = " + stats.nValues +
                            " min = " + stats.minValue.getString() +
                            " max = " + stats.maxValue.getString() +
                            " avg = " + stats.avgValue.getString()  );
        }

        /// <summary>
        /// Test a table definition from a data set.
        /// </summary>
        /// <param name="sw">StreamWriter for logging</param>
        /// <param name="tableDefinition">The table definition</param>
        private static void TestTableDefinition(StreamWriter sw, PYXTableDefinition_CSPtr tableDefinition)
        {
            if (tableDefinition == null || tableDefinition.isNull())
            {
                sw.WriteLine("ERROR: Table definition is null");
                return;
            }

            // log information from the table definition
            int numFields = tableDefinition.getFieldCount();
            sw.WriteLine("# Fields: " + numFields);
            foreach (var fd in tableDefinition.FieldDefinitions)
            {
                sw.WriteLine(   "Field " + fd.getName() + ":" +
                                " context = " + fd.getContext() +
                                " numeric = " + fd.isNumeric() +
                                " count = " + fd.getCount());
            }  
        }
    }
}
