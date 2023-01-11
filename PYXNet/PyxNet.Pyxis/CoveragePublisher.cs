using Pyxis.Utilities;
using PyxNet.DataHandling;
using global::PyxNet.Pyxis.GeoPackets;
using PyxNet.Service;
/******************************************************************************
CoveragePublisher.cs

begin      : 30/03/2007 2:35:25 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;

namespace PyxNet.Pyxis
{
    /// <summary>
    ///
    /// </summary>
    public class CoveragePublisher
    {

        //A flag to control uploading tiles to storage
        public static bool UploadToStorage = false;

        // A TraceTool that keeps a log of where we got all our pieces from.
        private NumberedTraceTool<CoveragePublisher> m_tracer
            = new NumberedTraceTool<CoveragePublisher>(TraceTool.GlobalTraceLogEnabled);

        // TODO: made public for testing code to access,  evaluate this decision.
        public class PublishedCoverageInfo : Publishing.Publisher.IPublishedItemInfo, IDisposable
        {
            #region Fields And Properties

            private DataInfo m_dataInfo = new DataInfo();

            public DataInfo DataInfo
            {
                get { return m_dataInfo; }
                set { m_dataInfo = value; }
            }

            private IProcess_SPtr m_spProcess;

            /// <summary>
            /// The process we are publishing.
            /// </summary>
            public IProcess_SPtr Process
            {
                get { return m_spProcess; }
            }

            private ICoverage_SPtr m_spCoverage;

            /// <summary>
            /// The process we are publishing as a coverage.
            /// </summary>
            public ICoverage_SPtr Coverage
            {
                get { return m_spCoverage; }
            }

            private ICache_SPtr m_spCache;

            /// <summary>
            /// The process we are publishing as a cache.
            /// </summary>
            public ICache_SPtr Cache
            {
                get { return m_spCache; }
            }

            /// <summary>
            /// Storage for the version of the process that we are publishing.
            /// </summary>
            private int m_processVersion;

            /// <summary>
            /// The version of the process that we are publishing.
            /// </summary>
            public int ProcessVersion
            {
                get { return m_processVersion; }
                set
                {
                    m_processVersion = value;
                    m_coverageInfo = null;
                }
            }

            /// <summary>
            /// Storage for the cell resolution of the process that we are publishing.
            /// </summary>
            private int m_cellResolution;

            /// <summary>
            /// The cell resolution of the process that we are publishing.
            /// </summary>
            public int CellResolution
            {
                get { return m_cellResolution; }
                set
                {
                    m_cellResolution = value;
                    m_coverageInfo = null;
                }
            }

            /// <summary>
            /// Storage for the tile resolution of the process that we are publishing.
            /// </summary>
            private int m_tileDepth;

            /// <summary>
            /// The tile resolution of the process that we are publishing.
            /// TODO:  I think that this can go away -- caches are multi resolutional now.
            /// </summary>
            public int TileDepth
            {
                get { return m_tileDepth; }
                set { m_tileDepth = value; }
            }

            private string m_description;

            private string m_lowerCaseDescription;

            private List<string> m_keywords = null;

            /// <summary>
            /// Gets the keywords (a list of words that match this item).
            /// </summary>
            /// <value>The keywords.</value>
            private List<string> Keywords
            {
                get
                {
                    if (m_keywords == null)
                    {
                        // We could lock on this, but it doesn't matter that much.
                        List<string> keywords = new List<string>();
                        keywords.AddRange(m_lowerCaseDescription.Split(null));
                        if (m_spProcess != null)
                        {
                            keywords.AddRange(m_spProcess.getProcName().ToLower().Split(null));
                        }
                        m_keywords = keywords;
                    }
                    return m_keywords;
                }
            }

            private string m_procGuidString;

            public string Description
            {
                get { return m_description; }
                set
                {
                    m_description = value;
                    m_lowerCaseDescription = m_description.ToLower();
                    m_keywords = null;
                }
            }

            /// <summary>
            /// Gets this pipeline's proc ref.
            /// </summary>
            /// <value>The proc ref.</value>
            public ProcRef ProcRef
            {
                get
                {
                    return new ProcRef(pyxlib.strToGuid(m_procGuidString), m_processVersion);
                }
            }

            private PyxNet.Service.Certificate m_certificate;

            public PyxNet.Service.Certificate Certificate
            {
                get { return m_certificate; }
                set { m_certificate = value; }
            }

            #endregion Fields And Properties

            /// <summary>
            /// Constructor.
            /// </summary>
            /// <param name="spProc">A handle to the process.</param>
            public PublishedCoverageInfo(IProcess_SPtr spProc)
            {
                if (null == spProc)
                {
                    throw new ArgumentNullException("spProc");
                }

                m_spCoverage = pyxlib.QueryInterface_ICoverage(spProc.getOutput());
                if (m_spCoverage.get() == null)
                {
                    // we did not get a coverage, so throw.
                    throw new ArgumentException("The process is not a coverage.", "spProc");
                }

                m_spProcess = spProc;
                m_spCache = pyxlib.QueryInterface_ICache(spProc.getOutput());
                if (m_spCache.get() == null)
                {
                    // we don't have a cache at the top, so add a cache to the chain
                    m_spProcess = ApplicationUtility.PYXCOMFactory.CreateProcess(
                        new ApplicationUtility.PYXCOMProcessCreateInfo(ApplicationUtility.PYXCOMFactory.WellKnownProcesses.CoverageCache)
                        .BorrowNameAndDescription(spProc)
                        .AddInput(0, spProc));

                    m_spProcess.initProc();

                    m_spCache = pyxlib.QueryInterface_ICache(m_spProcess.getOutput());
                    m_spCoverage = pyxlib.QueryInterface_ICoverage(m_spProcess.getOutput());
                }

                ProcessVersion = m_spProcess.getProcVersion();
                m_procGuidString = pyxlib.guidToStr(m_spProcess.getProcID());
                Description = m_spProcess.getProcDescription();

                DataInfo dataInfo = new DataInfo();
                // set the data id guid to be the guid of the process...
                dataInfo.DataSetID.Guid = new Guid(pyxlib.guidToStr(m_spProcess.getProcID()));
                dataInfo.DataChunkSize = 65535;
                dataInfo.DataLength = 0;
                dataInfo.AllAvailable = true;
                dataInfo.UseEncryption = false;
                dataInfo.UseSigning = false;
                dataInfo.UsesHashCodes = false;

                DataInfo = dataInfo;
                ProcessVersion = m_spProcess.getProcVersion();
                CellResolution = m_spCache.getCacheCellResolution();
                TileDepth = m_spCache.getCacheTileDepth();
            }

            #region Equality

            public override bool Equals(object obj)
            {
                return Equals(obj as PublishedCoverageInfo);
            }

            public bool Equals(PublishedCoverageInfo info)
            {
                if (null == info)
                {
                    return false;
                }

                if (info.DataInfo.DataSetID == DataInfo.DataSetID &&
                    info.ProcessVersion == ProcessVersion)
                {
                    return true;
                }

                return false;
            }

            public override int GetHashCode()
            {
                throw new MissingMethodException("Method not implemented.");
            }

            #endregion Equality

            /// <summary>
            /// Cache the CoverageInfo for performance.
            /// </summary>
            private CoverageRequestMessage m_coverageInfo = null;

            /// <summary>
            /// Helper property to get a CoverageRequestMessage that is initialized to
            /// have all the information about the published coverage.
            /// </summary>
            /// <returns></returns>
            public CoverageRequestMessage CoverageInfo
            {
                get
                {
                    if (m_coverageInfo == null)
                    {
                        m_coverageInfo = new CoverageRequestMessage();
                        m_coverageInfo.ProcessVersion = ProcessVersion;
                        m_coverageInfo.CellResolution = CellResolution;
                        m_coverageInfo.PipelineDefinition = PipeManager.writePipelineToNewString(Process);
                        m_coverageInfo.Geometry = Coverage.getGeometry();
                    }
                    return m_coverageInfo;
                }
            }

            /// <summary>
            /// Check to see if a query string matches the published coverage in any way.
            /// </summary>
            /// <param name="testAgainst">The string to test.</param>
            /// <returns>True if the string matches.</returns>
            public bool Matches(string testAgainst)
            {
                string lowerCaseTestAgainst = testAgainst.ToLower();
                string[] wordsToMatch = lowerCaseTestAgainst.Split(null);

                bool matched = true;
                foreach (string word in wordsToMatch)
                {
                    if (!Keywords.Contains(word))
                    {
                        matched = false;
                        break;
                    }
                }
                if (matched)
                {
                    return true;
                }

                // check to see if the Guid matches in string format.
                return m_procGuidString.Equals(testAgainst);
            }

            #region IPublishedItemInfo Members

            IEnumerable<string> PyxNet.Publishing.Publisher.IPublishedItemInfo.Keywords
            {
                get
                {
                    // add in the guid of the process.
                    yield return m_procGuidString;

                    // add in all the parts of the description
                    foreach (string keyword in this.Keywords)
                    {
                        yield return keyword;
                    }

                    // TODO: scan for meta data that could be used to search on
                    // and add it to the local query hash table.
                }
            }

            public QueryResult Matches(Query query, Stack stack)
            {
                if (Matches(query.Contents))
                {
                    // we found a match
                    NodeInfo connectedNodeInfo = null;
                    foreach (NodeInfo nodeInfo in stack.KnownHubList.ConnectedHubs)
                    {
                        connectedNodeInfo = nodeInfo;
                        break;
                    }
                    QueryResult queryResult =
                        new QueryResult(query.Guid, query.OriginNode, stack.NodeInfo, connectedNodeInfo);
                    queryResult.MatchingContents = Process.getProcName();
                    queryResult.MatchingDescription = Description;
                    queryResult.MatchingDataSetID = DataInfo.DataSetID;

                    // add in extra info here about version number and resolution
                    queryResult.ExtraInfo = CoverageInfo.ToMessage();
                    return queryResult;
                }

                return null;
            }

            #endregion IPublishedItemInfo Members

            public void Dispose()
            {
                if (m_spCache != null)
                {
                    m_spCache.Dispose();
                    m_spCache = null;
                }
                if (m_spCoverage != null)
                {
                    m_spCoverage.Dispose();
                    m_spCoverage = null;
                }
                if (m_spProcess != null)
                {
                    m_spProcess.Dispose();
                    m_spProcess = null;
                }
            }
        }

        #region Fields and Properties

        /// <summary>
        /// This list of coverages that we are publishing.
        /// </summary>
        private readonly DynamicList<PublishedCoverageInfo> m_publishedCoverages =
            new DynamicList<PublishedCoverageInfo>();

        /// <summary>
        /// The stack that we are using to publish/monitor.
        /// </summary>
        private readonly Stack m_stack;

        #endregion Fields and Properties

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="stack">The stack that you want to publish on.</param>
        public CoveragePublisher(Stack stack)
        {
            m_stack = stack;

            // register the messages that we want to monitor.
            m_stack.RegisterHandler(DataChunkRequest.MessageID, HandleChunkRequest);
            m_stack.RegisterHandler(DataInfoRequest.MessageID, HandleInfoRequest);
        }

        #region DisplayUri Event

        /// <summary> Event handler for DisplayUri. </summary>
        public event EventHandler<DisplayUriEventArgs> DisplayUri
        {
            add
            {
                m_DisplayUri.Add(value);
            }
            remove
            {
                m_DisplayUri.Remove(value);
            }
        }

        private EventHelper<DisplayUriEventArgs> m_DisplayUri = new EventHelper<DisplayUriEventArgs>();

        /// <summary>
        /// Raises the DisplayUri event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theUri"></param>
        public void OnDisplayUri(object sender, Uri theUri)
        {
            m_DisplayUri.Invoke(sender, new DisplayUriEventArgs(theUri));
        }

        #endregion DisplayUri Event

        #region Published Event

        private EventHelper<PublishedEventArgs> m_Published =
            new EventHelper<PublishedEventArgs>();

        /// <summary> Event handler for Published. </summary>
        public event EventHandler<PublishedEventArgs> Published
        {
            add
            {
                m_Published.Add(value);
            }
            remove
            {
                m_Published.Remove(value);
            }
        }

        /// <summary>
        /// Raises the Published event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theProcess"></param>
        public void OnPublished(object sender, IProcess_SPtr theProcess)
        {
            m_Published.Invoke(sender, new PublishedEventArgs(theProcess));
        }

        /// <summary>
        /// Raises the Published event, using the default sender (this).
        /// </summary>
        /// <param name="theProcess"></param>
        public void OnPublished(IProcess_SPtr theProcess)
        {
            OnPublished(this, theProcess);
        }

        #endregion Published Event

        #region Message Handlers

        /// <summary>
        /// Handle DataChunkRequest messages.  Look for any data that we are publishing that
        /// matches the chunk request and return the chunk of data if we have it.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public void HandleChunkRequest(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            m_tracer.DebugWriteLine("Handling Chunk request from {0}", args.Context.Sender);

            DataChunkRequest request = new DataChunkRequest(args.Message);

            if (!request.ExtraInfo.StartsWith(CoverageRequestMessage.MessageID))
            {
                // it must have an ExtraInfo that is a CoverageRequestMessage to make sense of this request.
                return;
            }

            CoverageRequestMessage coverageRequest = new CoverageRequestMessage(request.ExtraInfo);

            var info = m_publishedCoverages.FirstOrDefault(x => x.DataInfo.DataSetID.Equals(request.DataSetID)
                                                             && x.ProcessVersion == coverageRequest.ProcessVersion);
            if (!IsValidRequest(coverageRequest, info.ProcRef))
            {
                return;
            }

            if (info != null)
            {
                DataChunk chunk = null;

                switch (coverageRequest.Mode)
                {
                    case CoverageRequestMessage.TransferMode.Tile:
                        {
                            // TODO: make a map <rootIndex, FileInfo> so that we don't
                            // have to keep making the FileInfo's.
                            PYXIcosIndex rootIndex = new PYXIcosIndex(coverageRequest.TileIndex);
                            PYXTile_SPtr transferTilePtr = PYXTile.create(rootIndex,
                                rootIndex.getResolution() +
                                coverageRequest.CacheTileDepth);
                            string transferFilename = info.Cache.toFileName(transferTilePtr.get());
                            System.IO.FileInfo tileInfo = new System.IO.FileInfo(transferFilename);

                            var coverageCacheClient = CoverageCacheClient.FromProcess(info.Process);
                            var isFromStorage = false;
                            if (!tileInfo.Exists)
                            {
                                isFromStorage = coverageCacheClient.TryDownloadCacheFromStorage(transferTilePtr.get());
                                if (isFromStorage)
                                {
                                    tileInfo.Refresh();
                                }
                            }

                            if (!tileInfo.Exists)
                            {
                                info.Cache.forceCoverageTile(transferTilePtr.get());
                                tileInfo.Refresh();
                            }
                            if (tileInfo.Exists)
                            {
                                coverageCacheClient.StoreToBlobAsync(transferTilePtr.get());

                                chunk = new DataChunk(tileInfo, request.Offset, request.ChunkSize);
                                chunk.Data.UseCompression = false;

                                if (UploadToStorage && !isFromStorage
                                    && !coverageCacheClient.TileExistsOnStorage(transferTilePtr.get()))
                                {
                                    coverageCacheClient.AddTileToStorageAsync(transferTilePtr.get());
                                }
                            }
                        }
                        break;

                    case CoverageRequestMessage.TransferMode.DataSourceDefinition:
                    case CoverageRequestMessage.TransferMode.DataSourceValues:
                    case CoverageRequestMessage.TransferMode.CoverageDefinition:
                    case CoverageRequestMessage.TransferMode.Geometry:
                        {
                            System.IO.FileInfo fileInfo = new System.IO.FileInfo(
                                info.Cache.getCacheDir() + "/" +
                                CoverageRequestHelper.FileName(coverageRequest.Mode));
                            if (fileInfo.Exists)
                            {
                                chunk = new DataChunk(fileInfo, request.Offset, request.ChunkSize);
                            }
                        }
                        break;
                }

                if (null != chunk)
                {
                    chunk.DataSetID = request.DataSetID;
                    chunk.ExtraInfo = request.ExtraInfo;

                    args.Context.Sender.SendMessage(chunk.ToMessage());
                    GlobalPerformanceCounters.Counters.CoverageBytesUploaded.IncrementBy(
                        chunk.ChunkSize);

                    Stack stack = (Stack)sender;

                    UsageReports.CoverageBytesUploaded(stack.NodeInfo.FriendlyName,
                                                       stack.NodeInfo.NodeGUID,
                                                       args.Context.Sender.RemoteNodeInfo.FriendlyName,
                                                       args.Context.Sender.RemoteNodeInfo.NodeGUID,
                                                       chunk.DataSetID,
                                                       chunk.ChunkSize
                                                       );
                }
            }
        }

        private bool IsValidRequest(CoverageRequestMessage request, ProcRef procRef)
        {
#if ENFORCE_CERTIFICATE
            if (request.UsageCertificate == null)
            {
                return false;
            }
            
            if (!request.UsageCertificate.Valid)
            {
                return false;
            }

            if (!m_stack.CertificateValidator.IsCertificateValid(request.UsageCertificate))
            {
                return false;
            }

            var fact = request.UsageCertificate.FindFirstFact<GeoSourcePermissionFact>();
            if (fact == null)
            {
                return false;
            }

            var factProcRef = pyxlib.strToProcRef(fact.ProcRef);

            if (procRef != factProcRef)
            {
                return false;
            }
#endif
            return true;
        }

        /// <summary>
        /// Handle InfoRequest messages.  Look for any data we are publishing and
        /// return the info that is pertinent to that data.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public void HandleInfoRequest(object sender,
            PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            m_tracer.DebugWriteLine("Handling Info request from {0}", args.Context.Sender);

            DataInfoRequest request = new DataInfoRequest(args.Message);

            {
                foreach (PublishedCoverageInfo info in m_publishedCoverages)
                {
                    if (info.DataInfo.DataSetID == request.DataSetID)
                    {
                        if (request.ExtraInfo.Length == 0)
                        {
                            // we are looking for general info about the Coverage.
                            // send back specifics in the ExtraInfo
                            DataInfo returnInfo = new DataInfo(info.DataInfo);
                            returnInfo.ExtraInfo = info.CoverageInfo.ToMessage();
                            args.Context.Sender.SendMessage(returnInfo.ToMessage());
                            return;
                        }

                        CoverageRequestMessage coverageRequest = new CoverageRequestMessage(request.ExtraInfo);

                        // it must be the right version of the process.
                        if (info.ProcessVersion == coverageRequest.ProcessVersion)
                        {
                            DataInfo returnInfo = new DataInfo(info.DataInfo);
                            returnInfo.ExtraInfo = request.ExtraInfo;
                            System.IO.FileInfo fileInfo = null;
                            switch (coverageRequest.Mode)
                            {
                                case CoverageRequestMessage.TransferMode.Tile:
                                    {
                                        // TODO: make a map <rootIndex, FileInfo> so that we don't
                                        // have to keep making the FileInfo's.
                                        PYXIcosIndex rootIndex = new PYXIcosIndex(coverageRequest.TileIndex);
                                        PYXTile_SPtr infoTilePtr = PYXTile.create(rootIndex,
                                            coverageRequest.CacheTileDepth + rootIndex.getResolution());
                                        fileInfo = new System.IO.FileInfo(
                                            info.Cache.toFileName(infoTilePtr.get()));

                                        if (fileInfo.Exists)
                                        {
                                            bool failedToLoad = false;
                                            try
                                            {
                                                if (PYXValueTile.createFromFile(fileInfo.FullName).isNull())
                                                {
                                                    failedToLoad = true;
                                                }
                                            }
                                            catch (Exception)
                                            {
                                                failedToLoad = true;
                                            }

                                            if (failedToLoad)
                                            {
                                                //the file was invalid - remove it from disk.
                                                fileInfo.Delete();
                                                m_tracer.WriteLine("Removing corrupted file {0}", fileInfo.FullName);
                                            }
                                        }

                                        if (!fileInfo.Exists)
                                        {
                                            info.Cache.forceCoverageTile(infoTilePtr.get());
                                            fileInfo.Refresh();
                                        }
                                    }
                                    break;

                                case CoverageRequestMessage.TransferMode.DataSourceDefinition:
                                case CoverageRequestMessage.TransferMode.DataSourceValues:
                                case CoverageRequestMessage.TransferMode.CoverageDefinition:
                                case CoverageRequestMessage.TransferMode.Geometry:
                                    {
                                        fileInfo = new System.IO.FileInfo(
                                            info.Cache.getCacheDir() + "/" + CoverageRequestHelper.FileName(coverageRequest.Mode));
                                    }
                                    break;
                            }

                            if (null == fileInfo || !fileInfo.Exists)
                            {
                                // we don't have that part of the cache yet.
                                return;
                            }

                            returnInfo.DataLength = fileInfo.Length;
                            m_tracer.DebugWriteLine("Responding to Info request from {0}", args.Context.Sender);
                            args.Context.Sender.SendMessage(returnInfo.ToMessage());
                            return;
                        }
                    }
                }
            }
        }

        #endregion Message Handlers

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="processHandle">The process which implements the coverage to publish.</param>
        public IProcess_SPtr Publish(ProcRef processHandle, PyxNet.Service.ServiceInstance licenseServer)
        {
            if (null == processHandle)
            {
                return new IProcess_SPtr();
            }

            // Get the process through the process resolver.
            IProcess_SPtr spProcess = PipeManager.getProcess(processHandle);

            return Publish(spProcess, licenseServer);
        }

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="spProc">The process which implements the coverage to publish.</param>
        public IProcess_SPtr Publish(IProcess_SPtr spProc, PyxNet.Service.ServiceInstance licenseServer)
        {
            if (null == spProc)
            {
                return new IProcess_SPtr();
            }

            PublishedCoverageInfo pub;
            try
            {
                pub = new PublishedCoverageInfo(spProc);
            }
            catch (ArgumentException)
            {
                // It's not a coverage process.
                return new IProcess_SPtr();
            }

            return Publish(pub, licenseServer);
        }

        /// <summary>
        /// Publish a coverage over PyxNet making it available to remote nodes.
        /// This is a blocking call, and can take a while.
        /// </summary>
        /// <param name="pub">The info about the coverage to publish.</param>
        /// <param name="licenseServer">The license server to use to negotiate a license.
        /// Can be null if no negotiation is wanted.</param>
        /// <returns>The process smart pointer, which points to null if unsuccessful.</returns>
        private IProcess_SPtr Publish(
            PublishedCoverageInfo pub,
            PyxNet.Service.ServiceInstance licenseServer)
        {
            // Return if it's already published.
            if (m_publishedCoverages.Contains(pub))
            {
                return pub.Process;
            }

            // If we made it here, we are trying publishing a new coverage (to us).
            if (GetExistingPublicationCertificate(pub) ||
                (RequestPublicationCertificate(pub, licenseServer)))
            {
                // we found a local certificate to publish, so we are OK to publish.
                if (m_publishedCoverages.Add(pub, false))
                {
                    m_stack.Publisher.PublishItem(pub);
                    OnPublished(pub.Process);
                }

                // TODO: verify the return value here.
                return pub.Process;
            }

            return new IProcess_SPtr();
        }

        /// <summary>
        /// Unpublish a coverage over PyxNet making it no longer available to remote nodes.
        /// </summary>
        /// <param name="coverageInfo">The info about the coverage to unpublish.</param>
        public bool Unpublish(ProcRef procRef)
        {
            var coverageInfo = m_publishedCoverages.FirstOrDefault(x => x.ProcRef == procRef);
            return Unpublish(coverageInfo);
        }

        private bool Unpublish(PublishedCoverageInfo coverageInfo)
        {
            bool canUnpublish = false;
            lock (m_publishedCoverages)
            {
                if (coverageInfo != null && m_publishedCoverages.Remove(coverageInfo))
                {
                    canUnpublish = true;
                }
            }
            if (canUnpublish)
            {
                m_stack.Publisher.UnpublishItem(coverageInfo);
            }
            return canUnpublish;
        }

        /// <summary>
        /// This function will look for the publication certificate, first in the local
        /// stack's repository, then search for it on PyxNet.
        /// </summary>
        /// <param name="pub"></param>
        /// <returns></returns>
        private bool GetExistingPublicationCertificate(
            PublishedCoverageInfo pub)
        {
#if LICENSE_SERVER
            Stack stack = m_stack;

            // Build the fact that we want to find.
            PyxNet.Service.ResourceId coverageResourceId =
                new PyxNet.Service.ResourceId(pub.DataInfo.DataSetID.Guid);
            PyxNet.Service.ResourceDefinitionFact newFact =
                new PyxNet.Service.ResourceDefinitionFact(
                    pub.CoverageInfo.PipelineDefinition);
            newFact.ResourceId = coverageResourceId;

            // Find it.
            PyxNet.Service.CertificateFinder finder =
                new PyxNet.Service.CertificateFinder(stack, newFact);
            PyxNet.Service.ResourceDefinitionFact resourceDefinitionFact =
                finder.Find() as PyxNet.Service.ResourceDefinitionFact;

            // Return true if the certificate is not null.
            if (resourceDefinitionFact != null)
            {
                pub.Certificate = GetExistingResourcePermisson(resourceDefinitionFact.ResourceId);
            }
            return pub.Certificate != null;
#else
            return true;
#endif
        }

        private PyxNet.Service.Certificate GetExistingResourcePermisson(
            PyxNet.Service.ResourceId resourceId)
        {
            Stack stack = m_stack;

            // TODO: Change to PipelinePermissionFact
            // Build the fact that we want to find.
            PyxNet.Service.ResourcePermissionFact newFact =
                new PyxNet.Service.ResourcePermissionFact(resourceId,
                    stack.NodeInfo.NodeId);

            // Find it.
            PyxNet.Service.CertificateFinder finder =
                new PyxNet.Service.CertificateFinder(stack, newFact);
            PyxNet.Service.ICertifiableFact resourcePermissionFact =
                finder.Find();

            // Return it.
            if (resourcePermissionFact != null)
            {
                return resourcePermissionFact.Certificate;
            }
            return null;
        }

        /// <summary>
        /// This function will request the publication certificate from a server.
        /// It is a blocking call.
        /// </summary>
        /// <param name="pub"></param>
        /// <returns></returns>
        private bool RequestPublicationCertificate(
            PublishedCoverageInfo pub,
            PyxNet.Service.ServiceInstance licenseServer)
        {
            // TODO: Deal with equivalency.

            Stack stack = m_stack;

            PyxNet.Service.ResourceId coverageResourceId =
                new PyxNet.Service.ResourceId(pub.DataInfo.DataSetID.Guid);

            PyxNet.Service.ResourceDefinitionFact newDefinitionFact =
                new PyxNet.Service.ResourceDefinitionFact(pub.CoverageInfo.PipelineDefinition);
            newDefinitionFact.ResourceId = coverageResourceId;

            // TODO: Switch to PipelinePermissionFact
            PyxNet.Service.ResourcePermissionFact newPermissionFact =
                new PyxNet.Service.ResourcePermissionFact(coverageResourceId,
                    stack.NodeInfo.NodeId);

            // Search for a certificate locally.
            foreach (PyxNet.Service.ResourceDefinitionFact fact in
                m_stack.CertificateRepository.GetMatchingFacts(
                    newDefinitionFact.UniqueKeyword, newDefinitionFact.GetType()))
            {
                System.Diagnostics.Debug.Assert(fact != null);
                pub.Certificate = GetExistingResourcePermisson(fact.ResourceId);
                return true;
            }

            SynchronizationEvent permissionGrantedTimer =
                new SynchronizationEvent();

            // If here, we don't have a certificate.  Request one.
            PyxNet.Service.CertificateRequester requester =
                new PyxNet.Service.CertificateRequester(stack,
                    newDefinitionFact, newPermissionFact);
            requester.DisplayUri +=
                delegate(object sender, PyxNet.DisplayUriEventArgs a)
                {
                    OnDisplayUri(this, a.Uri);
                };
            requester.CertificateReceived +=
                delegate(object sender, PyxNet.Service.CertificateRequester.CertificateReceivedEventArgs c)
                {
                    // Get the certificate.
                    PyxNet.Service.Certificate certificate = c.Certificate;
                    System.Diagnostics.Debug.Assert(certificate != null);

                    // Add it to the stack's certificate repository.
                    stack.CertificateRepository.Add(certificate);

                    // Set it in the published coverage info.
                    pub.Certificate = certificate;
                };
            requester.PermissionGranted +=
                delegate(object sender, PyxNet.Service.CertificateRequester.ResponseReceivedEventArgs e)
                {
                    permissionGrantedTimer.Pulse();
                };
            requester.Start(licenseServer, TimeSpan.FromSeconds(30));

            // Wait until we get one.
            permissionGrantedTimer.Wait();

            return true;
        }

        /// <summary>
        /// Stop publishing all coverages, and stop responding to messages.
        /// </summary>
        public void StopPublishing()
        {
            // unregister the messages that we want to monitor.
            m_stack.UnregisterHandler(DataChunkRequest.MessageID, HandleChunkRequest);
            m_stack.UnregisterHandler(DataInfoRequest.MessageID, HandleInfoRequest);

            //unpublish all published coverage one by one
            m_publishedCoverages.ToList().ForEach(x => Unpublish(x));
        }

        internal bool IsPublished(IProcess_SPtr spProc)
        {
            foreach (var item in m_publishedCoverages)
            {
                if (pyxlib.isEqualProcRef(item.ProcRef, new ProcRef(spProc)))
                    return true;
            }
            return false;
        }
    }
}

// see PyxNet.Pyxis.Text.cs for unit tests.