using System.Collections.Generic;
using System.Linq;
using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Web;
using ApplicationUtility;
using PyxCrawler.Models;
using PyxCrawler.Utilities;
using Pyxis.Core;
using Pyxis.IO.Import;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;

namespace PyxCrawler.Import
{
    /// <summary>
    /// Responsible for verifying the usability of crawled data sets.
    /// A verified data set is, at the point of verification, assured to be able
    /// to import and be used by PYXIS and will not generate errors.
    /// Such a data set is marked as "Working".  Data sets that fail verification
    /// will have any error message attached to them for analysis.
    /// </summary>
    public static class DataSetVerifier
    {
        private static Engine s_engine;
        private static object s_controlLock = new Object();
        private static List<OnlineGeospatialDataSet> s_beingVerifiedDataSet = new List<OnlineGeospatialDataSet>();
        private static List<AnnotatedOnlineGeospatialService> s_beingVerifiedService = new List<AnnotatedOnlineGeospatialService>();
        private static object s_beingVerifiedLock = new object();
        private const int s_threadCount = 4;
        private static CancellationTokenSource s_cancelToken = new CancellationTokenSource();
        private static List<Task<bool>> s_threadPool = new List<Task<bool>>();
        private static int s_verifiedCount;
        private static readonly int s_writeBackThreshold = 1;
        private static bool s_dirtyCache;
        private static object s_verifiedCountLock = new object();

        public static bool Start(Engine engine)
        {
            lock (s_controlLock)
            {
                // If the engine is set already or is not provided, return a failure
                if (s_engine != null || engine == null)
                {
                    return false;
                }
                s_engine = engine;
                s_engine.WhenReady(() =>
                {
                    // Create the tasks
                    for (int i = 0; i < s_threadCount; ++i)
                    {
                        s_threadPool.Add(new Task<bool>(VerifyTask, s_cancelToken.Token));
                        s_threadPool.Last().Start();
                    }
                });
                return true;
            }
        }

        public static void Stop()
        {
            lock (s_controlLock)
            {
                // Check that verification is in progress
                if (s_engine == null)
                {
                    return;
                }
                s_engine = null;
                // Set the thread cancellation token
                try
                {
                    s_cancelToken.Cancel();
                    // Wait for all threads to exit
                    Task.WhenAll(s_threadPool.ToArray());
                    s_threadPool.Clear();
                    s_cancelToken = new CancellationTokenSource();
                    return;
                }
                catch (Exception)
                {
                    return;
                }
            }
        }

        private static bool VerifyTask()
        {
            while (!s_cancelToken.IsCancellationRequested)
            {
                // Ask for the next data set to verify
                OnlineGeospatialDataSet dataSet = null;
                var service = GetNextToVerify(out dataSet, new List<OnlineGeospatialServiceStatus> { OnlineGeospatialServiceStatus.NeedsVerifying });
                // If there's nothing in the queue, freeze for 10 seconds
                if (service == null)
                {
                    lock (s_verifiedCountLock)
                    {
                        if (s_dirtyCache)
                        {
                            OnlineGeospatialDatasetDb.Save();
                            s_dirtyCache = false;
                        }
                    }
                    Thread.Sleep(10000);
                }
                else
                {
                    // Verify the data set
                    Verify(dataSet, service);
                    // Remove the data set from the list of being verified
                    lock (s_beingVerifiedLock)
                    {
                        s_beingVerifiedDataSet.Remove(dataSet);
                        s_beingVerifiedService.Remove(service);
                    }
                    lock (s_verifiedCountLock)
                    {
                        // TODO make check thread-safe if multiple threads are used
                        if (s_dirtyCache && s_verifiedCount % s_writeBackThreshold == 0)
                        {
                            OnlineGeospatialDatasetDb.Save();
                            s_dirtyCache = false;
                        }
                    }
                }
            }
            return true;
        }

        private static AnnotatedOnlineGeospatialService GetNextToVerify(out OnlineGeospatialDataSet dataSet, List<OnlineGeospatialServiceStatus> statesToVerify = null)
        {
            lock (s_beingVerifiedLock)
            {
                // only verify a data set until one service is verified working
                foreach (var ds in OnlineGeospatialDatasetDb.Datasets.Where(d => !d.Services.Any(s => s.Status == OnlineGeospatialServiceStatus.Working) && !s_beingVerifiedDataSet.Contains(d)))
                {
                    foreach (var service in ds.Services
                        .Where(service => !s_beingVerifiedService.Contains(service))
                        .Where(service =>
                        {
                            if (statesToVerify == null || !statesToVerify.Any())
                            {
                                return true;
                            }
                            return statesToVerify.Contains(service.Status);
                        })
                        .OrderBy(s => s.Protocol, new ProtocolComparer()))
                    {
                        dataSet = ds;
                        s_beingVerifiedDataSet.Add(dataSet);
                        s_beingVerifiedService.Add(service);
                        return service;
                    }
                }
            }
            dataSet = null;
            return null;
        }

        private static void Verify(OnlineGeospatialDataSet dataSet)
        {
            foreach (var s in dataSet.Services.Where(s => s != null))
            {
                Verify(dataSet, s);
            }
        }

        private static void Verify(OnlineGeospatialDataSet dataSet, AnnotatedOnlineGeospatialService service)
        {
            if (dataSet == null  || service == null || !dataSet.Services.Contains(service))
            {
                return;
            }

            TryVerify(dataSet, service);
            
            lock (s_verifiedCountLock)
            {
                s_verifiedCount++;
                s_dirtyCache = true;
            }
        }

        internal static GeoSource TryVerify(OnlineGeospatialDataSet dataSet, AnnotatedOnlineGeospatialService service)
        {
            GeoSource verifiedGeoSource = null;
            // Try importing the data set
            try
            {
                var progress = s_engine.BeginImport(ToDataSet(dataSet, service));
                if (progress == null)
                {
                    service.Status = OnlineGeospatialServiceStatus.Unknown;
                    SetServiceNullValues(service);
                }
                // Check if we received a GeoSource (this is a blocking call)
                else if (progress.Task.Result != null)
                {
                    var geoSource = progress.Task.Result;
                    var process = s_engine.GetProcess(geoSource);

                    var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

                    if (coverage.isNotNull())
                    {
                        var passed = TestCoverage(Console.Out, coverage);
                        if (passed)
                        {
                            service.Status = OnlineGeospatialServiceStatus.Working;
                            SetServiceNullValues(service);
                            SetServiceGeoSourceValues(dataSet, service, geoSource);
                            verifiedGeoSource = geoSource;
                        }
                        else
                        {
                            service.Status = OnlineGeospatialServiceStatus.Broken;
                            SetServiceNullValues(service);
                            service.Error = "Failed PyxCrawler Coverage Test";
                        }
                    }
                    else
                    {
                        // no deep checking of vectors yet
                        service.Status = OnlineGeospatialServiceStatus.Working;
                        SetServiceNullValues(service);
                        SetServiceGeoSourceValues(dataSet, service, geoSource);
                        verifiedGeoSource = geoSource;
                    }
                }
                else
                {
                    service.Status = OnlineGeospatialServiceStatus.Broken;
                    SetServiceNullValues(service);
                }
            }
            catch (Exception e)
            {
                // TODO: think about the most appropriate status to set
                service.Status = OnlineGeospatialServiceStatus.WorkingButNotUsable;
                SetServiceNullValues(service);
                service.Error = e.Message;
                service.Trace = e.ToString();
                if (e.InnerException != null)
                {
                    service.Error += "\n---" + e.InnerException.Message;
                }
            }
            return verifiedGeoSource;
        }

        private static void SetServiceNullValues(AnnotatedOnlineGeospatialService service)
        {
            service.Error = null;
            service.Trace = null;
            service.Definition = null;
            service.ProcRef = null;
        }

        private static void SetServiceGeoSourceValues(OnlineGeospatialDataSet dataSet, AnnotatedOnlineGeospatialService service, GeoSource geoSource)
        {
            if (service != null && geoSource != null)
            {
                service.Definition = geoSource.Definition;
                service.ProcRef = geoSource.ProcRef;
                var process = PipeManager.getProcess(pyxlib.strToProcRef(geoSource.ProcRef));
                var systemTags = PipelineTagger.Tag(process);
                systemTags.Tags.Add("Crawled");
                dataSet.SystemTags = systemTags.Tags;
            }
        }

        private static DataSet ToDataSet(OnlineGeospatialDataSet dataSet, OnlineGeospatialService service)
        {
            if (dataSet == null || service == null)
            {
                return null;
            }
            return new DataSet
            {
                Uri = GetDataSetService(dataSet, service).Uri.AbsoluteUri,
                Metadata = new SimpleMetadata
                {
                    Name = dataSet.Name,
                    Description = dataSet.Description,
                    Tags = dataSet.Tags
                },
                Fields = dataSet.Fields,
                Domains = dataSet.Domains,
                Layer = dataSet.DatasetId
            };
        }

        private static AnnotatedOnlineGeospatialService GetDataSetService(OnlineGeospatialDataSet dataSet, OnlineGeospatialService service)
        {
            return dataSet.Services.First(s => s.Protocol == service.Protocol && s.Version == service.Version);
        }

        /// <summary>
        /// Test the specified coverage.
        /// </summary>
        /// <param name="sw">StreamWriter for logging</param>
        /// <param name="coverage">The coverage</param>
        private static bool TestCoverage(TextWriter sw, ICoverage_SPtr coverage)
        {
            // test the geometry
            PYXGeometry_SPtr geom = coverage.getGeometry();
            if (geom == null || geom.isNull())
            {
                sw.WriteLine("ERROR: Geometry is null");
                return false;
            }
            sw.WriteLine("Resolution: " + geom.getCellResolution());

            var coverageDefinition = coverage.getCoverageDefinition();
            sw.WriteLine("- Start of Coverage Definition -");
            if (!TestTableDefinition(sw, coverageDefinition))
            {
                return false;
            }
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
                        sw.WriteLine("ERROR: Value tile for tile " + valueTile.getTile().getRootIndex().toString() +
                                        " at resolution " + geom.getCellResolution() +
                                        " is not compatible with the coverage definition");
                        return false;
                    }

                    TestValueTile(sw, valueTile);
                }
            }

            sw.WriteLine("- End of Tile Info -");
            return true;
        }

        /// <summary>
        /// Test a tile from a coverage.
        /// </summary>
        /// <param name="sw">StreamWriter for logging</param>
        /// <param name="valueTile">The value tile</param>
        private static void TestValueTile(TextWriter sw, PYXValueTile_SPtr valueTile)
        {
            // calculate statistics
            var stats = valueTile.calcStatistics();

            // log the statistics
            PYXIcosIndex idx = valueTile.getTile().getRootIndex();
            sw.WriteLine("Tile " + idx.toString() + ":" +
                            " #cells = " + stats.nCells +
                            " #values = " + stats.nValues +
                            " min = " + stats.minValue.getString() +
                            " max = " + stats.maxValue.getString() +
                            " avg = " + stats.avgValue.getString());
        }

        /// <summary>
        /// Test a table definition from a data set.
        /// </summary>
        /// <param name="sw">StreamWriter for logging</param>
        /// <param name="tableDefinition">The table definition</param>
        private static bool TestTableDefinition(TextWriter sw, PYXTableDefinition_CSPtr tableDefinition)
        {
            if (tableDefinition == null || tableDefinition.isNull())
            {
                sw.WriteLine("ERROR: Table definition is null");
                return false;
            }

            // log information from the table definition
            int numFields = tableDefinition.getFieldCount();
            sw.WriteLine("# Fields: " + numFields);
            foreach (var fd in tableDefinition.FieldDefinitions)
            {
                sw.WriteLine("Field " + fd.getName() + ":" +
                                " context = " + fd.getContext() +
                                " numeric = " + fd.isNumeric() +
                                " count = " + fd.getCount());
            }

            return true;
        }
    }
}