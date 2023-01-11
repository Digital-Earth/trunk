/******************************************************************************
CoverageDownloader.cs

begin      : May 4, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#if !NO_LIBRARY
using Pyxis.Services.PipelineLibrary.Repositories;
#endif
using Pyxis.Utilities;
using PyxNet.DataHandling;
using PyxNet.Pyxis.GeoPackets;
using PyxNet.Service;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace PyxNet.Pyxis
{
    /// <summary>
    /// Class for passing DownloadFailedEventArgs used in the
    /// CoverageDownloader.DownloadFailed event.
    /// </summary>
    public class DownloadFailedEventArgs : System.EventArgs
    {
        private readonly IProcess_SPtr m_spCache;

        /// <summary>
        /// The process that was trying to download and couldn't.
        /// </summary>
        public IProcess_SPtr SpCache
        {
            get { return m_spCache; }
        }

        private readonly PYXTile_SPtr m_spTile;

        /// <summary>
        /// The tile that was being downloaded when the failure occured.
        /// </summary>
        public PYXTile_SPtr SpTile
        {
            get { return m_spTile; }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="Message">The error message to pass.</param>
        public DownloadFailedEventArgs(IProcess_SPtr SpProc, PYXTile_SPtr SpTile)
        {
            m_spCache = SpProc;
            m_spTile = SpTile;
        }
    }

    /// <summary>
    /// Responsible for transferring data from all publishers for a Pyxis Coverage.
    /// At start up it will transfer the table definitions if they are not already
    /// present on this machine, and then transfer tiles of data as they are requested.
    ///
    /// Entry point for open that searches for sources of data and then gets the three base files
    /// so that the local coverage proc is operational. (If the local files don't exist already.)
    /// Entry point to get a tile of data that searches for sources (if we need more) and then downloads the tile.
    /// The coverage downloader should keep a list of recent nodes that are publishing and then reuse that list.
    /// Have a method of running down the list of publishers and getting rid of dead ones.  Maybe
    /// we could hook in to the File Info response message and any node that didn't answer a file info
    /// could bew tossed from the list for next time.
    /// For the short term, just get one source of data and use it.
    /// </summary>
    public class CoverageDownloader
    {
        // A TraceTool that keeps a log of where we got all our pieces from.
        private NumberedTraceTool<CoverageDownloader> m_tracer;

        internal const int SecondsToHoldConnections = 20;

        private static int s_WaitForPublisherSeconds = 45;

        private static int s_RefreshPublisherList = 5 * 60;

        public static event EventHandler<DownloadFailedEventArgs> DownloadFailed;

        protected void OnDownloadFailed(DownloadFailedEventArgs e)
        {
            EventHandler<DownloadFailedEventArgs> handler = DownloadFailed;

            if (handler != null)
            {
                handler(this, e);
            }
        }

        #region Properties and Members

        /// <summary>
        /// The stack that we are using to communicate.
        /// </summary>
        private readonly Stack m_stack;

        /// <summary>
        /// The cache process that we are populating with data from across PyxNet.
        /// </summary>
        private IProcess_SPtr m_spCacheProc;

        /// <summary>
        /// CoverageCacheClient to download coverage from storage
        /// </summary>
        CoverageCacheClient m_coverageCacheClient;

        private ICache_SPtr m_spCacheCache;

        /// <summary>
        /// The search string that we will use to find this coverage.
        /// </summary>
        private readonly String m_coverageQueryString;

        /// <summary>
        /// Gets or sets the published certificate.
        /// </summary>
        /// <value>The published certificate.</value>
        public PyxNet.Service.Certificate PublishedCertificate { get; set; }

        /// <summary>
        /// Storage for the coverage dataset guid, which is the same as the process ID.
        /// </summary>
        private readonly DataGuid m_coverageDataGuid = new DataGuid();

        /// <summary>
        /// A list of PyxNet nodes that are publishing this data.
        /// </summary>
        private readonly List<NodeInfo> m_publishers = new List<NodeInfo>();

        /// <summary>
        /// A list of publishers that is populated from the QueryResultsList as
        /// it is accessed.
        /// </summary>
        public List<NodeInfo> Publishers
        {
            get
            {
                lock (m_publishers)
                {
                    while (m_publishers.Count < m_queryResults.Count)
                    {
                        m_publishers.Add(m_queryResults[m_publishers.Count].ResultNode);
                    }
                }
                return m_publishers;
            }
        }

        /// <summary>
        /// Thread safe access to the count of the publishers.
        /// </summary>
        private int PublisherCount
        {
            get
            {
                int returnValue;
                lock (m_publishers)
                {
                    returnValue = Publishers.Count;
                }
                return returnValue;
            }
        }



        /// <summary>
        /// The Pyxis data cell resolution that the publisher says the data is.
        /// </summary>
        private int m_cellResolution;

        /// <summary>
        /// The version of this process.
        /// </summary>
        private int m_procVersion;

        /// <summary>
        /// The PyxNet query agent that is finding places to get this coverage from.
        /// </summary>
        private readonly QueryResultList m_queryResults = null;

        /// <summary>
        /// The time that we started the last query.
        /// </summary>
        private DateTime m_queryStartTime;

        #endregion Properties and Members

        #region Static Helper Functions

        /// <summary>
        /// Helper function for readying a remote process to be properly resolved.
        /// </summary>
        /// <param name="queryResults">A list of query results for the coverage.</param>
        /// <returns>True if the remote process was imported and is ready for resolving.</returns>
        static public bool ImportRemoteProcess(List<QueryResult> queryResults)
        {
            if ((queryResults.Count == 0) ||
                !queryResults[0].ExtraInfo.StartsWith(CoverageRequestMessage.MessageID))
            {
                return false;
            }

            CoverageRequestMessage coverageRequest = new CoverageRequestMessage(queryResults[0].ExtraInfo);

            // get the .ppl from the coverageRequest and import the .ppl into the library.
            Vector_IProcess vecProcesses = PipeManager.importStr(coverageRequest.PipelineDefinition);

#if !NO_LIBRARY
            // set all the processes as not temporary
            PipelineRepository.Instance.SetIsTemporary(vecProcesses, false);

            // set the top of the process chain as NOT hidden
            ProcRef procRef = new ProcRef(pyxlib.strToGuid(
                queryResults[0].MatchingDataSetID.Guid.ToString()),
                coverageRequest.ProcessVersion);
            PipelineRepository.Instance.SetIsHidden(procRef, false);
            PipelineRepository.Instance.SetIsPublished(procRef, true);
#endif

            return true;
        }

        /// <summary>
        /// Call this helper function before creating any processes in the system
        /// that need to download coverage data over PyxNet.
        /// </summary>
        static public void InitializeCoverageDownloaderSupport()
        {
            CacheManager.getCacheCreatedNotifier().Event +=
                HandleCacheWithNoLocalDataNotification;
        }

        /// <summary>
        /// Call this helper function before closing down PyxNet.
        /// </summary>
        static public void StopCoverageDownloaderSupport()
        {
            CacheManager.getCacheCreatedNotifier().Event -=
                HandleCacheWithNoLocalDataNotification;
        }

        /// <summary>
        /// Handler for caches being initialized without a data supply event.  Tries to establish
        /// a data supply over Pyxnet.
        /// </summary>
        /// <param name="spEvent">A CacheWithProcessEvent event</param>
        static private void HandleCacheWithNoLocalDataNotification(NotifierEvent_SPtr spEvent)
        {
            DateTime startTime = DateTime.Now;
            CacheWithProcessEvent cacheEvent =
                CacheWithProcessEvent.dynamic_cast(spEvent.__deref__());
            if (cacheEvent != null)
            {
                ICache_SPtr spCache = cacheEvent.getCache();
                if (spCache.get() == null)
                {
                    throw new ArgumentException("Supplied cache is not valid.");
                }

                if (spCache.getNeedATileNotifier().getObserverCount() == 0)
                {
                    CoverageDownloader cd = new CoverageDownloader(
                        StackSingleton.Stack, cacheEvent.getProcess());

                    if (cd.Initialize())
                    {
                        PublisherSingleton.Publisher.StartCoveragePublish(cacheEvent.getProcess());
                    }
                    else
                    {
                        cd.Detach();
                        cacheEvent.setHandled(false);
                        StackSingleton.Stack.Tracer.DebugWriteLine("Failed to initialize the Coverage Downloader");
                        return;
                    }
                }
            }

            cacheEvent.setHandled(true);
            TimeSpan elapsedTime = DateTime.Now - startTime;
            StackSingleton.Stack.Tracer.DebugWriteLine(
                "Cache hook-up to pyxnet took {0} seconds.", elapsedTime.TotalSeconds);
        }

        #endregion Static Helper Functions

        /// <summary>
        /// Constructor - create the downloader and get ready to download. Will throw an ArgumentException
        /// if the supplied IProcess_SPtr is not a process that implements a cache.
        /// </summary>
        /// <param name="stack">The stack that we will use to communicate with PyxNet.</param>
        /// <param name="spProc">The cache that we will be populating with data.</param>
        public CoverageDownloader(Stack stack, IProcess_SPtr spProc)
        {
            m_stack = stack;
            m_spCacheProc = spProc;
            m_tracer = new NumberedTraceTool<CoverageDownloader>(stack.Tracer.Enabled);
            //Uncomment the next line for debugging.
            //m_tracer.Enabled = true;

            // TODO: check for the proc being initialized and fail if it isn't.
            m_spCacheCache = pyxlib.QueryInterface_ICache(spProc.getOutput());
            if (m_spCacheCache.get() == null)
            {
                throw new ArgumentException("Supplied process not a cache.");
            }

            m_coverageCacheClient = CoverageCacheClient.FromProcess(m_spCacheProc);

            m_spCacheCache.getNeedATileNotifier().Event += NeedToTransferATileEvent;

            m_coverageQueryString = pyxlib.guidToStr(spProc.getProcID());
            m_procVersion = spProc.getProcVersion();

            m_coverageDataGuid.Guid = new Guid(pyxlib.guidToStr(spProc.getProcID()));

            m_queryResults = new QueryResultList(m_stack, m_coverageQueryString);
            m_queryResults.AddingElement += HandleAddingResult;

            m_certificateRetainer = new CertificateRetainer(m_stack, new ProcRef(m_spCacheProc).ToString());

            // stamp in the directory for the cache
            m_spCacheCache.initCacheDir();

            // keep track of all downloaders
            CoverageDownloaderManager.Manager.Add(this);
        }

        /// <summary>
        /// This event is watching the cache that is supplying data, and when there is
        /// data missing, attempts to download it from PyxNet.
        /// </summary>
        /// <param name="spEvent">The event details.</param>
        private void NeedToTransferATileEvent(NotifierEvent_SPtr spEvent)
        {
            CacheNeedsTileEvent cacheEvent = CacheNeedsTileEvent.dynamic_cast(spEvent.__deref__());
            if (cacheEvent != null)
            {
                string tileFilename = m_spCacheCache.toFileName(cacheEvent.getTile().get());

                // Try to download the tile from storage

                if (m_coverageCacheClient.TryDownloadCacheFromStorage(cacheEvent.getTile().get()))
                {
                    m_spCacheCache.addTileFile(tileFilename,
                                                     cacheEvent.getTile(),
                                                     ProcessDataChangedEvent.ChangeTrigger.knNewTileDownloaded);

                    GlobalPerformanceCounters.Counters.CoverageTilesDownloaded.Increment();

                    return;
                }

                if (PublisherCount == 0 ||
                    (DateTime.Now - m_queryStartTime).TotalSeconds > s_RefreshPublisherList)
                {
                    InitializePublishersList();
                    WaitForPublishers();
                }

                // This flag is set if we get a damaged tile during download.
                bool panicMode = false;

                try
                {
                    // TODO: when we can't find anymore publishers, we should notify the user, and
                    // maybe start a background thread to watch for publishers and then send notification
                    // that the data set is available (when process notifications are available).
                    // QUESTION:  What is the strategy for user notification from this point?
                    // Probably notification of some sort, but as a part of what object? Or should it
                    // be a static (global) event?

                    if (PublisherCount > 0)
                    {
                        CoverageRequestMessage extraInfo = CreateCoverageRequestMessage(
                            CoverageRequestMessage.TransferMode.Tile);
                        extraInfo.CacheTileDepth = cacheEvent.getTile().getDepth();
                        extraInfo.TileIndex = cacheEvent.getTile().getRootIndex().toString();

                        DataDownloader downloader;
                        lock (m_publishers)
                        {
                            downloader = new DataDownloader(m_coverageDataGuid, m_stack, Publishers, extraInfo.ToMessage());
                        }

                        //--
                        //-- register event handler to log received data
                        //--
                        downloader.OnDataReceived += new EventHandler<DataDownloader.DownloadDataReceivedEventArgs>(OnDataReceived);

                        EventHandler<DynamicList<QueryResult>.ElementEventArgs> handleAddedElement =
                            delegate(object sender, DynamicList<QueryResult>.ElementEventArgs e)
                            {
                                downloader.AddProvider(e.Element.ResultNode);
                            };

                        // If we find new Providers during the download add them in.
                        m_queryResults.AddedElement += handleAddedElement;

                        downloader.HoldTime = TimeSpan.FromSeconds(SecondsToHoldConnections);

                        // use extended time out of 30 seconds to mitigate snow flakes.
                        if (downloader.DownloadFile(
                            new System.IO.FileInfo(tileFilename), 30))
                        {
                            // addTileFile will throw an exception if the data is bad.
                            m_spCacheCache.addTileFile(tileFilename,
                                                       cacheEvent.getTile(),
                                                       ProcessDataChangedEvent.ChangeTrigger.knNewTileDownloaded);

                            GlobalPerformanceCounters.Counters.CoverageTilesDownloaded.Increment();
                        }
                        else
                        {
                            cacheEvent.setDownloadFailed(true);
                            OnDownloadFailed(new DownloadFailedEventArgs(m_spCacheProc, cacheEvent.getTile()));
                        }

                        // Remove the handler.
                        m_queryResults.AddedElement -= handleAddedElement;
                    }
                }
                catch (Exception ex)
                {
                    m_tracer.WriteLine("Failure during download of {0}: {1}.",
                        tileFilename, ex.ToString());

                    panicMode = true;
                }

                if (panicMode)
                {
                    // Gene's hypothesis is that we are getting different data from
                    //  different providers.  Check each provider individually to
                    //  see if they have good data.

                    bool attemptDownloadFromEveryProvider = false;
                    //Enable this next line for debugging.
                    //attemptDownloadFromEveryProvider = true;

                    bool downloadWasCompleted = false;

                    foreach (NodeInfo publisher in Publishers)
                    {
                        CoverageRequestMessage extraInfo = CreateCoverageRequestMessage(
                            CoverageRequestMessage.TransferMode.Tile);
                        extraInfo.CacheTileDepth = cacheEvent.getTile().getDepth();
                        extraInfo.TileIndex = cacheEvent.getTile().getRootIndex().toString();

                        List<NodeInfo> singlePublisherList = new List<NodeInfo>();
                        singlePublisherList.Add(publisher);

                        DataDownloader downloader = new DataDownloader(m_coverageDataGuid, m_stack, singlePublisherList, extraInfo.ToMessage());
                        downloader.HoldTime = TimeSpan.FromSeconds(SecondsToHoldConnections);
                        bool dataWasReceived = false;
                        downloader.OnDataReceived += delegate(object sender,
                            DataDownloader.DownloadDataReceivedEventArgs args)
                            {
                                dataWasReceived = true;
                            };

                        string providerSpecificTileFilename = tileFilename + "." + publisher.FriendlyName;

                        // use extended time out of 30 seconds to mitigate snow flakes.
                        if (downloader.DownloadFile(
                            new System.IO.FileInfo(providerSpecificTileFilename), 30))
                        {
                            System.IO.FileInfo fileInfo = new System.IO.FileInfo(providerSpecificTileFilename);

                            try
                            {
                                System.IO.File.Copy(providerSpecificTileFilename, tileFilename, true);

                                m_spCacheCache.addTileFile(tileFilename,
                                                           cacheEvent.getTile(),
                                                           ProcessDataChangedEvent.ChangeTrigger.knNewTileDownloaded);
                                GlobalPerformanceCounters.Counters.CoverageTilesDownloaded.Increment();

                                m_tracer.WriteLine(
                                    "Succesfully downloaded {0} ({1} bytes).", providerSpecificTileFilename,
                                    fileInfo.Exists ? fileInfo.Length.ToString() : "n/a");

                                downloadWasCompleted = true;

                                if (attemptDownloadFromEveryProvider == false)
                                {
                                    break;
                                }
                            }
                            catch (Exception ex)
                            {
                                m_tracer.WriteLine(
                                    "Failed to download {0} ({1} bytes).  {2}", providerSpecificTileFilename,
                                    fileInfo.Exists ? fileInfo.Length.ToString() : "n/a",
                                    ex.ToString());
                            }
                        }
                        else
                        {
                            m_tracer.WriteLine("Failed to download {0} within 30 seconds.  {1} was received.",
                                providerSpecificTileFilename,
                                dataWasReceived ? "Data " : "No data ");
                        }
                    }

                    if (!downloadWasCompleted)
                    {
                        // We've tried it all, and come up short.
                        cacheEvent.setDownloadFailed(true);
                        OnDownloadFailed(new DownloadFailedEventArgs(m_spCacheProc, cacheEvent.getTile()));
                    }
                }
            }
        }

        /// <summary>
        /// Called when coverage data downloaded from pyxnet.
        /// Event contains information about the chunk and the sender that should be audited.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="PyxNet.DataHandling.DataDownloader.DownloadDataReceivedEventArgs"/> instance containing the event data.</param>
        private void OnDataReceived(object sender, DataDownloader.DownloadDataReceivedEventArgs e)
        {
            UsageReports.CoverageBytesDownloaded(e.Sender.RemoteNodeInfo.FriendlyName,
                                                  e.Sender.RemoteNodeInfo.NodeGUID,
                                                  m_stack.NodeInfo.FriendlyName,
                                                  m_stack.NodeInfo.NodeGUID,
                                                  e.Chunk.DataSetID,
                                                  e.Chunk.ChunkSize
                                                  );
        }

        /// <summary>
        /// Creates a coverage request message.
        /// </summary>
        /// <param name="mode">The mode.</param>
        /// <returns></returns>
        private CoverageRequestMessage CreateCoverageRequestMessage(
            CoverageRequestMessage.TransferMode mode)
        {
            CoverageRequestMessage extraInfo = new CoverageRequestMessage();
            extraInfo.Mode = mode;
            extraInfo.ProcessVersion = m_procVersion;
            extraInfo.UsageCertificate = m_certificateRetainer.GetCertificate();
            extraInfo.PublishedCertificate = this.PublishedCertificate;
            return extraInfo;
        }

        private CertificateRetainer m_certificateRetainer;

        /// <summary>
        /// This handler is used to make sure that the result being added to the list is one that we want.
        /// We leave the state of the AllowAddition flag alone unless we want to deny addition, so that we
        /// won't override someone else's denial.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleAddingResult(object sender, QueryResultList.AddingElementEventArgs args)
        {
            QueryResult qr = args.PotentialElement;

            if (qr.ResultNode == m_stack.NodeInfo)
            {
                //we don't add the local node as query result for a remote chanell end
                args.AllowAddition = false;
                return;
            }

            if (qr.MatchingDataSetID.Equals(m_coverageDataGuid))
            {
                // for this to be a valid result, the  must have an ExtraInfo that contains
                // a CoverageRequestMessage and the version must match.
                if (qr.ExtraInfo.StartsWith(CoverageRequestMessage.MessageID))
                {
                    try
                    {
                        CoverageRequestMessage coverageRequest = new CoverageRequestMessage(qr.ExtraInfo);

                        if (coverageRequest.ProcessVersion == m_procVersion)
                        {
                            m_cellResolution = coverageRequest.CellResolution;

                            // disallow duplicate node id's
                            for (int index = 0; index < m_queryResults.Count; index++)
                            {
                                if (m_queryResults[index].ResultNode == qr.ResultNode)
                                {
                                    args.AllowAddition = false;
                                    break;
                                }
                            }

                            return;
                        }
                    }
                    catch (Exception exception)
                    {
                        System.Diagnostics.Trace.WriteLine(
                            "The coverage request message could not be constructed from the extra info: " +
                            exception);
                    }
                }
            }

            // if we get to this point in the function then we don't allow addition.
            args.AllowAddition = false;
        }

        /// <summary>
        /// Call this function once to initiate a PyxNet query to find data sources for the coverage.
        /// This will hook up the handler to monitor for more results coming in for additional sources
        /// for the data.
        /// </summary>
        private void InitializePublishersList()
        {
            lock (m_publishers)
            {
                // the publishers list will be rebuilt from the query results
                // list, so its OK just to clear it.
                m_publishers.Clear();
            }

            if ((m_queryResults.Started) &&
                ((m_queryResults.Count == 0) || (DateTime.Now - m_queryStartTime).TotalSeconds > s_RefreshPublisherList))
            {
                // We should restart the query here by stopping it
                // before the call to start, but only if it has been
                // longer than the normal waiting for results period.
                if (m_queryStartTime != null)
                {
                    TimeSpan elapsed = DateTime.Now - m_queryStartTime;
                    if (elapsed.TotalSeconds <= s_WaitForPublisherSeconds)
                    {
                        // The query is running already and is still under the time
                        // that we want to wait for results, so we don't need to do
                        // any starting or stopping of the query.
                        return;
                    }
                }
                m_queryResults.Stop();
            }
            m_queryResults.Start();
            m_queryStartTime = DateTime.Now;
        }

        private void WaitForPublishers()
        {
            // make a query and look for query results
            m_queryResults.WaitForResults(1, s_WaitForPublisherSeconds);
        }

        /// <summary>
        /// Helper function to download part of the coverage definition.
        /// </summary>
        /// <param name="mode"></param>
        /// <returns></returns>
        private bool DownloadFileByMode(CoverageRequestMessage.TransferMode mode)
        {
            // create the extra info to explain what part of the coverage we want.
            CoverageRequestMessage extraInfo = CreateCoverageRequestMessage(mode);

            DataDownloader downloader;
            lock (m_publishers)
            {
                downloader = new DataDownloader(m_coverageDataGuid, m_stack, Publishers, extraInfo.ToMessage());
            }

            downloader.HoldTime = TimeSpan.FromSeconds(SecondsToHoldConnections);

            return downloader.DownloadFile(m_coverageCacheClient.FilePathFromMode(mode));
        }

       
        public static CoverageRequestMessage.TransferMode[] Modes =
                {
                    CoverageRequestMessage.TransferMode.DataSourceDefinition,
                     CoverageRequestMessage.TransferMode.DataSourceValues,
                     CoverageRequestMessage.TransferMode.CoverageDefinition,
                     CoverageRequestMessage.TransferMode.Geometry
                };

        /// <summary>
        /// Add a query result to the list of query results that are being used to find providers
        /// of data for this coverage.
        /// </summary>
        /// <param name="qr"></param>
        public void AddQueryResult(QueryResult qr)
        {
            lock (m_queryResults)
            {
                m_queryResults.Add(qr);
            }
        }

        /// <summary>
        /// Initialize the coverage downloader and transfer the minimum amount of information
        /// over PyxNet to be able to support data transfer.  This function must be called and must
        /// return true before this is a usable object.
        /// </summary>
        /// <returns>True if the coverage is live and ready to go, otherwise false.</returns>
        public bool Initialize()
        {
            bool downloaded = true;

               // make sure we have a place to put the files
                System.IO.DirectoryInfo coverageDir = new System.IO.DirectoryInfo(m_spCacheCache.getCacheDir());
                if (!coverageDir.Exists)
                {
                    // TODO: make sure this creates a whole directory chain, not just a leaf.
                    coverageDir.Create();
                }

            // Check if we need to download the definition files
            foreach (var item in Modes)
            {
                var filePath = m_coverageCacheClient.FilePathFromMode(item);
                if(!File.Exists(filePath))
                {
                   downloaded &= m_coverageCacheClient.TryDownloadDefinitionsFromStorage(item);
                }
            }

            if(!downloaded)
            {
                // we need publishers
                InitializePublishersList();
                WaitForPublishers();
                if (PublisherCount == 0)
                {
                    m_stack.Tracer.WriteLine("Coverage Downloader could not find any publishers for {0}",
                        m_spCacheProc.getProcID().ToString());
                    return false;
                }

                downloaded = true;
                foreach (var item in Modes)
                {
                    if (!File.Exists(m_coverageCacheClient.FilePathFromMode(item)))
                    {
                        downloaded &= DownloadFileByMode(item);
                    }
                }

                // if we have not yet transferred all the files, then we are not initialized.
                if (!downloaded)
                {
                    m_stack.Tracer.WriteLine("Coverage Downloader could not transfer base cache files for {0}",
                        m_spCacheProc.getProcID().ToString());
                    return false;
                }
            }

            // now see if we need to initialize the process before we use it.
            if ((m_spCacheProc.getInitState() != IProcess.eInitStatus.knInitialized) &&
                (m_spCacheProc.getInitState() != IProcess.eInitStatus.knInitializing))
            {
                return (m_spCacheProc.initProc() == IProcess.eInitStatus.knInitialized);
            }
            return true;
        }

        internal void Detach()
        {
            //deatch from cache
            m_spCacheCache.getNeedATileNotifier().Event -= NeedToTransferATileEvent;

            //stop quires
            m_queryResults.Stop();

            //remove all pyxlib refenreces...

            m_spCacheCache = null;
            m_spCacheProc = null;
        }
    }

    /// <summary>
    /// CoverageDownloaderManager responsible to manage all CoverageDownloader,
    /// to allow the ability to detach them all when needed
    /// </summary>
    public class CoverageDownloaderManager
    {
        #region Members

        /// <summary>
        /// Weak referencing all Downloaders
        /// </summary>
        private WeakReferenceList<CoverageDownloader> m_downloaders = new WeakReferenceList<CoverageDownloader>();

        #endregion Members

        #region Methods

        /// <summary>
        /// Add new CoverageDownloader to manager list
        /// </summary>
        /// <param name="downloader"> a CoverageDownloader </param>
        public void Add(CoverageDownloader downloader)
        {
            m_downloaders.Add(downloader);
        }

        /// <summary>
        /// Detach all Downloaders under the Manager supervise
        /// </summary>
        public void DetachAllDownloaders()
        {
            foreach (CoverageDownloader downloader in m_downloaders)
            {
                downloader.Detach();
            }
        }

        #endregion Methods

        #region Singleton

        private static CoverageDownloaderManager s_manager;

        /// <summary>
        /// Get the Manager.
        /// </summary>
        public static CoverageDownloaderManager Manager
        {
            get
            {
                if (s_manager == null)
                {
                    s_manager = new CoverageDownloaderManager();
                }
                return s_manager;
            }
        }

        #endregion Singleton
    }
}