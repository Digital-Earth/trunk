/******************************************************************************
ProcessJob.cs

begin		: February 9, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Microsoft.Practices.Unity;
using Pyxis.Contract.Operations;
using Pyxis.Services.PipelineLibrary.Repositories;
using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Storage;
using Pyxis.Storage.BlobProviders;
using System.IO;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// Responsible for processing the pipeline, i.e. caching all tiles.  When a pipeline is published is to the GWSS,
    /// the following occurs:
    /// 1. The pipeline is processed at its native resolution - 11.
    /// 2. A new ProcessJob is then added to the job queue for a resolution equal to PYXIcosIndex.knMinSubRes, or
    /// (PYXIcosIndex.knMinSubRes + 1) if the native resolution - 11 was PYXIcosIndex.knMinSubRes.
    /// 3. When the above job completes, it adds another ProcessJob to the queue for (resolution + 1).
    /// 4. The above is repeated until all resolutions upto the native resolution have been processed.
    ///   The pipeline is then marked as IsProcessed.
    /// </summary>
    internal class ProcessJob : JobOnProcess
    {
        private string m_pipelineName;
        private int m_resolution = -1;
        private int m_tileDepth;
        private ICache_SPtr m_spCache;

        public int ProcessingResolution { get; private set; }

        public ProcessJob(IProcess_SPtr process, string name, int resolution)
            : base()
        {
            m_resolution = resolution;
            ProcRef = new ProcRef(process);
            Process = process;
            m_pipelineName = name;
            Status = new ObservableOperationStatus();
            Status.Operation.OperationType = Pyxis.Contract.Operations.OperationType.Process;
            Status.Operation.Parameters.Add("ProcRef", pyxlib.procRefToStr(ProcRef));
            Status.Operation.Parameters.Add("Resolution", m_resolution.ToString());
            Status.Description = String.Format("Process pipeline '{0}={1}'  at resolution {2}", m_pipelineName, ProcRef, m_resolution);
        }

        /// <summary>
        /// Gets or sets the tiles processed.
        /// </summary>
        /// <value>The tiles processed.</value>
        private Pyxis.Utilities.ProgressData TilesProcessed { get; set; }

        public static bool operator !=(ProcessJob a, ProcessJob b)
        {
            return !(a == b);
        }

        public static bool operator ==(ProcessJob a, ProcessJob b)
        {
            if (object.ReferenceEquals(a, null))
            {
                return object.ReferenceEquals(b, null);
            }
            return a.Equals(b);
        }

        public override bool Equals(object obj)
        {
            if (obj is ProcessJob &&
                ((ProcessJob)(obj)).m_pipelineName == m_pipelineName &&
                ((ProcessJob)(obj)).ProcRef == ProcRef &&
                ((ProcessJob)(obj)).ProcessingResolution == ProcessingResolution)
            {
                return true;
            }
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            if (ProcRef != null)
            {
                return m_pipelineName.GetHashCode() ^ ProcRef.GetHashCode() ^ ProcessingResolution.GetHashCode();
            }
            else
            {
                return 0;
            }
        }

        protected override void DoExecute()
        {
            try
            {
                //we only need to process coverage pipelines at the moment
                if (!Process.ProvidesOutputType(ICoverage.iid))
                {
                    //all other pipelines we just marked as processed
                    PipelineRepository.Instance.SetIsProcessed(ProcRef, true);
                    Status.StatusCode = OperationStatusCode.Completed;
                    PipelineRepository.Instance.CheckPoint();
                    return;
                }

                m_spCache = pyxlib.QueryInterface_ICache(Process.getOutput());
                if (m_spCache.get() == null)
                {
                    throw new ArgumentException("Supplied process does not have a cache.");
                }
                m_spCache.initCacheDir();

                // Initialize the number of tiles needed.
                TilesProcessed = new Pyxis.Utilities.ProgressData
                {
                    Units = "tiles"
                };
                // Make sure pipeline is local
                if (Process.SupportingFiles().All(x => x.Exists))
                {
                    ProcessPipeline(Process);
                }
                else
                {
                    throw new Exception("Can not locate supporting files for this pipeline");
                }
            }
            catch (System.Runtime.InteropServices.ExternalException ex)
            {
                throw new Exception(
                   string.Format(
                       "\nAn unexpected error occurred when attempting to process {0}={1}; " +
                       "the exception info is printed below:\nError code = {2}\n{3}\n{4}",
                       m_pipelineName, pyxlib.procRefToStr(ProcRef), ex.ErrorCode, (ex.InnerException != null) ? ex.InnerException.ToString() : "No Inner Exception", ex.ToString()), ex);
            }
            catch (Exception ex)
            {
                throw new Exception(
                    string.Format(
                        "\nAn unexpected error occurred when attempting to process {0}={1}; " +
                        "the exception info is printed below:\n{2}",
                        m_pipelineName, pyxlib.procRefToStr(ProcRef), ex.ToString()), ex);
            }
        }

        /// <summary>
        /// Processes the tiles for the given process.
        /// </summary>
        /// <param name="process">The process.</param>
        /// <summary>
        /// Does the actual processing.
        /// </summary>
        /// <param name="process">The process.</param>
        /// <returns>The number of tiles in the dataset.</returns>

        private int CountTiles(PYXGeometry_SPtr geometry)
        {
            int totalTiles = 0;

            for (PYXIterator_SPtr it = geometry.getIterator(); !it.end() && !ExitManager.ShouldExit; it.next())
            {
                ++totalTiles;
            }
            return totalTiles;
        }

        private void ProcessGeometry(ICoverage_SPtr coverage, PYXGeometry_SPtr geometry)
        {
            // Tell the coverage that it should get ready to read all of this data.
            coverage.geometryHint(geometry);

            var coverageCacheClient = PyxNet.Pyxis.GeoPackets.CoverageCacheClient.FromProcess(Process);
            int previousPercent = 0;
            for (PYXIterator_SPtr it = geometry.getIterator(); !it.end() && !ExitManager.ShouldExit; it.next())
            {
                using (PYXTile_SPtr tilePtr = PYXTile.create(it.getIndex(), ProcessingResolution))
                {
                    coverageCacheClient.StoreToBlob(tilePtr.get());
                   
                    TilesProcessed.IncrementCurrentValue(1);

                    Status.Progress = (TilesProcessed.CurrentValue.Value * 100) / TilesProcessed.FinalValue;
                    int percent = (int)Status.Progress;

                    // only display progress in increments of 1% or 100 tiles
                    if (previousPercent != percent || TilesProcessed.CurrentValue.Value % 100 == 0)
                    {
                        previousPercent = percent;
                        GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Process, TilesProcessed.ToString());
                    }
                    CancellationToken.ThrowIfCancellationRequested();
                }
            }
            // Tell the coverage that we are done with this area.
            coverage.endGeometryHint(geometry);
        }



        private void ProcessPipeline(IProcess_SPtr process)
        {


            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
            var geometry = SetupGeometry(coverage);

            if (ExitManager.ShouldExit)
            {
                return;
            }

            CancellationToken.ThrowIfCancellationRequested();

            if (m_resolution == -1)
            {
                throw new ArgumentOutOfRangeException("m_resolution");
            }
            else
            {
                TilesProcessed.Title = string.Format("Processing '{0}={1}' at resolution '{2}'...",
                    process.getProcName(), pyxlib.procRefToStr(ProcRef), ProcessingResolution.ToString());

                TilesProcessed.FinalValue = CountTiles(geometry);

                //checking for disk space
                long requiredDiskSpace = 10 * (long)1073741824; // 10 GB - should be replaced with an actual estimation of the process cache.
                var freeDiskSpace = new System.IO.DriveInfo(AppServices.getCacheDir("ProcessCache").Substring(0, 1)).AvailableFreeSpace;
                if (requiredDiskSpace > freeDiskSpace)
                {
                    throw new Exception("Not enough free disk space, required: " + ApplicationUtility.FileUtility.SizeToString(requiredDiskSpace));
                }

                ProcessGeometry(coverage, geometry);

                Trace.time(string.Format("Beginning processing '{0}={1}' at resolution '{2}'...",
                    process.getProcName(), pyxlib.procRefToStr(ProcRef), ProcessingResolution.ToString()));
            }
        }

        private PYXGeometry_SPtr SetupGeometry(ICoverage_SPtr coverage)
        {
            m_tileDepth = PYXTile.knDefaultTileDepth;

            PYXGeometry_SPtr geometry = coverage.getGeometry();

            if (geometry == null || geometry.isNull())
            {
                Logging.Categories.Jobs.Error("The geometry of the pipeline " + m_pipelineName + " is null");
            }

            int resolution = m_resolution;
            // determine the tile depth for tile fetches
            if ((resolution - m_tileDepth) < PYXIcosIndex.knMinSubRes)
            {
                m_tileDepth = resolution - PYXIcosIndex.knMinSubRes;
            }
            geometry.setCellResolution(resolution - m_tileDepth);
            ProcessingResolution = resolution;

            return geometry;
        }
    }

    internal class ProcessJobFactory
    {
        public IProcess_SPtr Process { get; private set; }

        private List<int> NeededResolutions { get; set; }

        private ProcessJobFactory(IProcess_SPtr process, List<int> neededResolutions)
        {
            Process = process;
            NeededResolutions = neededResolutions;
        }

        public static ProcessJobFactory CreateFactory(IProcess_SPtr process, int lastProcessedResolution)
        {
            //we only need to process coverage pipelines at the moment
            if (!process.ProvidesOutputType(ICoverage.iid))
            {
                return new ProcessJobFactory(process, new List<int>());
            }

            if (process.getInitState() != IProcess.eInitStatus.knInitialized)
            {
                process.initProc(true);
            }

            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
            var maxResolution = coverage.getGeometry().getCellResolution() + 2;

            var resolutionToProcess = AllNeededResolutions().Where(resolution => lastProcessedResolution < resolution && resolution <= maxResolution).ToList();

            return new ProcessJobFactory(process, resolutionToProcess);
        }

        public ProcessJob Emit(UnityContainer container)
        {
            if (NeededResolutions.Count == 0)
            {
                //all other pipelines we just marked as processed
                PipelineRepository.Instance.SetIsProcessed(new ProcRef(Process), true);
                PipelineRepository.Instance.CheckPoint();

                return null;
            }
            else
            {
                var processJob = container.Resolve<ProcessJob>(
                  new ParameterOverrides
                        {
                            {"process", Process},
                            {"name", Process.getProcName()},
                            {"resolution", NeededResolutions[0]}
                        });
                //the job was completed succuefully - mark that we don't need to process this resolution any more.
                processJob.JobCompleted += (s, e) => { NeededResolutions.RemoveAt(0); };
                return processJob;
            }
        }

        private static IEnumerable<int> AllNeededResolutions()
        {
            yield return 4;
            yield return 5;
            yield return 6;
            yield return 8;
            yield return 9;
            foreach (var resolution in Enumerable.Range(11, PYXMath.knMaxAbsResolution - 1))
            {
                yield return resolution;
            }
        }
    }
}