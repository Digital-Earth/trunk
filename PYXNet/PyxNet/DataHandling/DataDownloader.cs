/******************************************************************************
DataDownloader.cs

begin      : 08/03/2007 10:09:11 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace PyxNet.DataHandling
{
    /// <summary>
    /// Class to download data from multiple sources keyed on a data GUID.  Need to look
    /// at some way to persist this to disk as a partially downloaded data set.
    /// </summary>
    public class DataDownloader
    {
        // A Pyxis.Utilities.TraceTool that keeps a log of where we got all our pieces from.
        private Pyxis.Utilities.NumberedTraceTool<DataDownloader> m_tracer =
            new Pyxis.Utilities.NumberedTraceTool<DataDownloader>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        /// <summary>
        /// Storage for HoldTime.
        /// </summary>
        private TimeSpan m_holdTime = TimeSpan.Zero;

        /// <summary>
        /// Time to hold a connection when finished with that connection.
        /// A time of zero will not hold the connection.
        /// </summary>
        public TimeSpan HoldTime
        {
            get { return m_holdTime; }
            set { m_holdTime = value; }
        }

        #region IMessagesValidator

        public interface IMessagesValidator
        {
            bool IsMatch(NodeInfo fromNode, DataInfo dataInfo);

            bool IsMatch(NodeInfo fromNode, DataChunk dataChunk);
        }

        public class DefaultMessageValidator : IMessagesValidator
        {
            public Guid DataSetID { get; set; }

            public Message ExtraInfo { get; set; }

            public DefaultMessageValidator(Guid dataSetId, Message extraInfo)
            {
                DataSetID = dataSetId;
                ExtraInfo = extraInfo;
            }

            #region IDataInfoValidator Members

            public bool IsMatch(NodeInfo fromNode, DataInfo dataInfo)
            {
                return dataInfo.DataSetID == DataSetID &&
                       ExtraInfo.Equals(dataInfo.ExtraInfo);
            }

            public bool IsMatch(NodeInfo fromNode, DataChunk dataChunk)
            {
                return dataChunk.DataSetID == DataSetID &&
                       ExtraInfo.Equals(dataChunk.ExtraInfo);
            }

            #endregion IDataInfoValidator Members
        }

        #endregion IMessagesValidator

        /// <summary>
        /// Helper class to keep track of the state of the various nodes that we are
        /// talking to to get data.
        /// </summary>
        private class DataProvider
        {
            /// <summary>
            /// The NodeInfo of the node so that we can contact it.
            /// </summary>
            public NodeInfo NodeInfo { get; private set; }

            /// <summary>
            /// The connection to the Node that we will use to send messages.
            /// </summary>
            public StackConnection Connection { get; set; }

            /// <summary>
            /// The last data request message that we sent to the Node.
            /// </summary>
            public DataChunkRequest CurrentRequest { get; set; }

            /// <summary>
            /// The time that we sent the last data request.
            /// </summary>
            public DateTime CurrentRequestTime { get; set; }

            /// <summary>
            /// This is set to false when GetNextChunk finds no chunk to ask for
            /// from this conection, and then examined by the timer to see if
            /// this connection can be used to speed up a slow connection.
            /// </summary>
            public bool IsActiveConnection { get; set; }

            /// <summary>
            /// The number of bytes we have downloaded from this connection.
            /// </summary>
            public long BytesDownloaded { get; set; }

            /// <summary>
            /// The time spent getting data from this connection.  Used in
            /// conjunction with the number of bytes downloaded to get a speed
            /// metric for this connection.
            /// </summary>
            public TimeSpan DownloadTime { get; set; }

            /// <summary>
            /// This value is true if we have asked another connection to get
            /// the same piece of data.
            /// </summary>
            public bool IsDoubleRequested { get; set; }

            /// <summary>
            /// The information about what parts of the data this node has available.
            /// </summary>
            public DataInfo DataInfo = null;

            /// <summary>
            /// Constructor.
            /// </summary>
            /// <param name="nodeInfo"></param>
            public DataProvider(NodeInfo nodeInfo)
            {
                NodeInfo = nodeInfo;
                CurrentRequestTime = DateTime.MinValue;
                IsDoubleRequested = false;
                DownloadTime = TimeSpan.Zero;
            }

            internal DataChunkRequest CreateDataChunkRequest(int chunkOffset, int chunkLength, Service.Certificate certificate)
            {
                return CreateDataChunkRequest(chunkOffset, chunkLength, certificate, null);
            }

            internal DataChunkRequest CreateDataChunkRequest(int chunkOffset, int chunkLength, Service.Certificate certificate, Message extraInfo)
            {
                var request = new DataChunkRequest(DataInfo.DataSetID,
                            chunkOffset, chunkLength,
                            DataInfo.UseEncryption, DataInfo.UseSigning,
                            certificate);

                if (extraInfo != null)
                {
                    request.ExtraInfo = extraInfo;
                }

                return request;
            }
        }

        public class DownloadStatus
        {
            public bool DownloadedCompleted { get; set; }

            public int DataInfoRequestsCount { get; set; }

            public int DataInfoRepliesCount { get; set; }

            public int DataChunkRequestsCount { get; set; }

            public int DataChunkRepliesCount { get; set; }

            public DateTime StartTime { get; set; }

            public DateTime FirstDataInfoRequestTime { get; set; }

            public DateTime FirstDataInfoReplayTime { get; set; }

            public DateTime FirstDataChunkRequestTime { get; set; }

            public DateTime FirstDataChunkReplayTime { get; set; }

            public DateTime LastDataChunkTime { get; set; }

            public DateTime CompletedTime { get; set; }

            public string Issues { get; set; }

            public TimeSpan WaitForRequestToStart
            {
                get
                {
                    if (FirstDataInfoRequestTime != DateTime.MinValue)
                    {
                        return FirstDataInfoRequestTime - StartTime;
                    }
                    return TimeSpan.Zero;
                }
            }

            public TimeSpan WaitForInfoTime
            {
                get
                {
                    if (FirstDataInfoReplayTime != DateTime.MinValue)
                    {
                        return FirstDataInfoReplayTime - FirstDataInfoRequestTime;
                    }
                    return TimeSpan.Zero;
                }
            }

            public TimeSpan DownloadChunksTime
            {
                get
                {
                    if (FirstDataChunkReplayTime != DateTime.MinValue)
                    {
                        return CompletedTime - FirstDataChunkReplayTime;
                    }
                    return TimeSpan.Zero;
                }
            }

            public TimeSpan TotalTime
            {
                get { return CompletedTime - StartTime; }
            }

            public int NumberOfProvidersFound { get; set; }

            public int NumberOfProvidersNotFound { get; set; }

            public override string ToString()
            {
                return
                    TotalTime.TotalSeconds > 1 ?
                    String.Format(
                        "DownloadCompleted = {0}: {9}\nDataInfo: Requests={1},Replies={2}\nDataChunks: Requests={3},Replies={4}\nTotalTime: {5} (WaitForStart={6:0.00}% WaitForInfo={7:0.00}% DownloadChunks={8:0.00}%)",
                        DownloadedCompleted, DataInfoRequestsCount, DataInfoRepliesCount, DataChunkRequestsCount, DataChunkRepliesCount,
                        TotalTime, 100 * WaitForRequestToStart.TotalSeconds / TotalTime.TotalSeconds, 100 * WaitForInfoTime.TotalSeconds / TotalTime.TotalSeconds, 100 * DownloadChunksTime.TotalSeconds / TotalTime.TotalSeconds, Issues) :
                    String.Format(
                        "DownloadCompleted = {0}: {6}\nDataInfo: Requests={1},Replies={2}\nDataChunks: Requests={3},Replies={4}\nTotalTime: {5})",
                        DownloadedCompleted, DataInfoRequestsCount, DataInfoRepliesCount, DataChunkRequestsCount, DataChunkRepliesCount,
                        TotalTime, Issues);
            }
        }

        public IMessagesValidator Validator { get; set; }

        #region Properties and Member Variables

        /// <summary>
        /// A random number source used to make the propagation of chunks through the
        /// network random, so that more different chunks will be available.
        /// </summary>
        private Random m_randomGenerator = new Random();

        /// <summary>
        /// Used to detect slow/dead connections and ask for the data again, but on a faster
        /// available connection.
        /// </summary>
        private System.Timers.Timer m_deadManTimer =
            new System.Timers.Timer(TimeSpan.FromSeconds(100).TotalMilliseconds);

        /// <summary>
        /// Storage for the identity of the data set that we are downloading.
        /// </summary>
        private DataGuid m_DataSetID;

        /// <summary>
        /// The identity of the data set that we are downloading.
        /// </summary>
        public DataGuid DataSetID
        {
            get { return m_DataSetID; }
            set { m_DataSetID = value; }
        }

        /// <summary>
        /// The stack that we will use to transfer the data.
        /// </summary>
        private Stack m_stack;

        /// <summary>
        /// The nodes on the network that have this data.
        /// </summary>
        private Pyxis.Utilities.DynamicList<DataProvider> m_providers;

        private List<NodeInfo> m_suggestedProviders;

        private Object m_providersLockObject = new Object();

        private List<DataProvider> GetProviders()
        {
            lock (m_providersLockObject)
            {
                return new List<DataProvider>(m_providers);
            }
        }

        private Pyxis.Utilities.TemporaryFile m_temporaryFile = null;

        /// <summary>
        /// Storage for the file that is being transferred.
        /// </summary>
        private volatile Stream m_destFile = null;

        private readonly Object m_destFileLock = new Object();

        /// <summary>
        /// This is the file that is being transferred.
        /// </summary>
        public Stream DestFile
        {
            get
            {
                if (null == m_destFile)
                {
                    lock (m_destFileLock)
                    {
                        if (null == m_destFile)
                        {
                            if (m_downloadedDataInfo.DataLength > 50000)
                            {
                                m_temporaryFile = new Pyxis.Utilities.TemporaryFile();
                                m_destFile = new FileStream(m_temporaryFile.Name, FileMode.CreateNew);
                            }
                            else
                            {
                                m_destFile = new MemoryStream();
                            }
                        }
                    }
                }

                return m_destFile;
            }
        }

        /// <summary>
        /// All the information about the data we are getting.
        /// </summary>
        private DataInfo m_dataInfo;

        /// <summary>
        /// The information about what we have downloaded so far kept track of in the
        /// available bits.
        /// </summary>
        private DataInfo m_downloadedDataInfo;

        public DataInfo DownloadedDataInfo
        {
            get { return m_downloadedDataInfo; }
        }

        /// <summary>
        /// Storage for the data extra info
        /// </summary>
        private Message m_extraInfo;

        /// <summary>
        /// Extended information about what the data is to download.
        /// One use case would be the tile index of data to download when
        /// the downloaded data is a tile from a coverage.
        /// </summary>
        public Message ExtraInfo
        {
            get { return m_extraInfo; }
            set { m_extraInfo = value; }
        }

        public DownloadStatus Status { get; set; }

        /// <summary>
        /// The chunks that we have requested so far.
        /// </summary>
        private System.Collections.BitArray m_requestedChunks;

        /// <summary>
        /// Used to make the process of finding the next section of data to download
        /// thread safe.
        /// </summary>
        private object m_getNextChunkLock = new object();

        /// <summary>
        /// Used to make the process of writing data to the backing memory stream
        /// thread safe.
        /// </summary>
        private object m_writingLock = new object();

        /// <summary>
        /// Used to make the process of firing the DownloadComplete event
        /// thread safe.
        /// </summary>
        private object m_completeLock = new object();

        /// <summary>
        /// Will be set to true when the downlaod is complete, and is used to ensure
        /// that the DownloadComplete event is only fired once.
        /// </summary>
        private bool m_isDownloadComplete = false;

        /// <summary>
        /// The number of bytes that we have downloaded so far.
        /// </summary>
        private int m_bytesReceived = 0;

        private object startLock = new Object();

        private bool isStarted = false;

        private Service.Certificate m_certificate;

        public Service.Certificate Certificate
        {
            get
            {
                if (m_certificate == null)
                {
                    m_certificate = m_stack.CertificateRepository.RequestCertificate(
                        m_stack, GetServiceId());
                }
                return m_certificate;
            }
            set
            {
                m_certificate = value;
            }
        }

        #endregion Properties and Member Variables

        #region Conversion

        /// <summary>
        /// Display as the amount of data that we have transferred so far.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            if (null != m_dataInfo)
            {
                return String.Format("Bytes Transferred : {0} of {1}",
                    m_bytesReceived, m_dataInfo.DataLength);
            }
            else
            {
                return "Waiting for download information.";
            }
        }

        #endregion Conversion

        #region Constructors

        /// <summary>
        /// Constructs the downloader, and starts the downloading process.
        /// </summary>
        /// <param name="dataSetID">The ID of the data to download.</param>
        /// <param name="communicationStack">The stack that will be used to communicate.</param>
        /// <param name="providers">A list of nodes to get the data from.</param>
        public DataDownloader(DataGuid dataSetID, Stack communicationStack,
            List<NodeInfo> providers) :
            this(dataSetID, communicationStack, providers, new Message())
        {
        }

        /// <summary>
        /// Constructs the downloader, and starts the downloading process.
        /// </summary>
        /// <param name="dataSetID">The ID of the data to download.</param>
        /// <param name="communicationStack">The stack that will be used to communicate.</param>
        /// <param name="providers">A list of nodes to get the data from.</param>
        /// <param name="extraInfo">The extra data needed to specify the data to download.</param>
        public DataDownloader(DataGuid dataSetID, Stack communicationStack,
            List<NodeInfo> providers, Message extraInfo)
        {
            m_tracer.TracePrefix = string.Format("DataDownloader {0}:", dataSetID.ToString());

            if (null == providers)
            {
                throw new System.ArgumentNullException("providers", "No list of providers for a DataDownloader.");
            }
            if (null == communicationStack)
            {
                throw new System.ArgumentNullException("communicationStack", "No stack for a DataDownloader.");
            }

            m_stack = communicationStack;

            m_providers = new Pyxis.Utilities.DynamicList<DataProvider>();
            m_suggestedProviders = new List<NodeInfo>(providers);

            DataSetID = dataSetID;
            ExtraInfo = extraInfo;

            // hook the timer event
            m_deadManTimer.Enabled = false;
            m_deadManTimer.Elapsed += new System.Timers.ElapsedEventHandler(HandleDeadManTimerElapsed);

            Validator = new DefaultMessageValidator(DataSetID, ExtraInfo);
        }

        #endregion Constructors

        #region Message Handlers

        /// <summary>
        /// Handle a DataInfo message coming back from a possible provider.
        /// Use this DataInfo as the master DataInfo if it is the first one we
        /// have received.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public void HandleInfo(object sender,
            PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            DataInfo receivedInfo = new DataInfo(args.Message);

            //validate the datainfo metadata
            if (!Validator.IsMatch(args.Context.Sender.RemoteNodeInfo, receivedInfo))
            {
                return;
            }

            m_tracer.DebugWriteLine("Received info message from {0}", args.Context.Sender.ToString());

            DataProvider provider = GetProviders().FirstOrDefault(x => IsFromSameNode(x.Connection, args.Context.Sender));

            if (null == provider)
            {
                // we got info from a connection that wasn't in our list.
                return;
            }

            if (Status != null)
            {
                Status.DataInfoRepliesCount++;
                if (Status.FirstDataInfoReplayTime == DateTime.MinValue)
                {
                    Status.FirstDataInfoReplayTime = DateTime.Now;
                }
            }

            //inital guess...
            bool useThisProvider = true;

            lock (m_getNextChunkLock)
            {
                if (null == m_dataInfo)
                {
                    m_dataInfo = receivedInfo;
                    m_downloadedDataInfo = new DataInfo(receivedInfo);
                    int numberChunks = (int)(m_dataInfo.DataLength / m_dataInfo.DataChunkSize) + 1;
                    m_requestedChunks = new System.Collections.BitArray(numberChunks, false);
                    m_downloadedDataInfo.AllAvailable = false;
                    m_downloadedDataInfo.AvailableChunks = new System.Collections.BitArray(numberChunks, false);
                }
                else
                {
                    if (m_dataInfo.DataLength != receivedInfo.DataLength)
                    {
                        if (Status != null)
                        {
                            Status.Issues += " Different data size";
                        }
                        useThisProvider = false;
                        m_tracer.WriteLine("Not everyone had the same size for this data set ( datasetID = " + m_DataSetID + " )");
                        //OnDownloadFailedRaise();
                    }
                }
            }

            if (useThisProvider)
            {
                provider.DataInfo = receivedInfo;
                GetNextChunk(provider);
            }
        }

        /// <summary>
        /// Determines whether connection is the same connection (or from the same NodeID)
        /// as connection2.
        /// </summary>
        /// <param name="connection">The connection.</param>
        /// <param name="connection2">The other connection.</param>
        /// <returns>True if the connections are from the same node.</returns>
        private bool IsFromSameNode(StackConnection connection, StackConnection connection2)
        {
            // if we have no connecion to this provider yet, then this can't be the right one.
            if (connection == null || connection2 == null)
            {
                return false;
            }

            // if it came back on the same connection we sent it on, then it is OK.
            bool AreEqual = connection.Equals(connection2);

            // If it came from someone with the same NodeID, then it will be OK too.
            if (!AreEqual && connection.RemoteNodeInfo != null && connection2.RemoteNodeInfo != null)
            {
                AreEqual = connection.RemoteNodeInfo.Equals(connection2.RemoteNodeInfo);
            }

            return (AreEqual);
        }

        /// <summary>
        /// Deals with incoming chunks of data and writes them to the data stream.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        public void HandleChunk(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            DateTime receivedTime = DateTime.Now;

            DataChunk chunk = new DataChunk(args.Message);
            m_tracer.DebugWriteLine("Recieved data chunk for dataset {2} offset {1} from {0}",
                args.Context.Sender.ToString(), chunk.Offset, chunk.DataSetID);

            //validate the chunk metadata
            if (!Validator.IsMatch(args.Context.Sender.RemoteNodeInfo, chunk))
            {
                return;
            }

            if (m_isDownloadComplete)
            {
                return;
            }

            DataProvider provider = GetProviders().FirstOrDefault(x => IsFromSameNode(x.Connection, args.Context.Sender));

            if (null == provider)
            {
                // we got info from a connection that wasn't in our list.
                return;
            }

            if ((null == provider.DataInfo) ||
                (provider.DataInfo.DataChunkSize == 0))
            {
                // how can we handle data from an invalid dataInfo?
                m_tracer.DebugWriteLine("Chunk was ignored because it was not from a valid provider.");
                return;
            }

            Pyxis.Utilities.GlobalPerformanceCounters.Counters.DataBytesDownloaded.IncrementBy(
                chunk.ChunkSize);

            if (Status != null)
            {
                Status.DataChunkRepliesCount++;
                if (Status.FirstDataChunkReplayTime == DateTime.MinValue)
                {
                    Status.FirstDataChunkReplayTime = DateTime.Now;
                }
            }

            bool chunkWasUsed = false;

            lock (m_writingLock)
            {
                // for now chunk.ChunkSize == m_dataInfo.DataChunkSize
                int chunkNumber = chunk.Offset / provider.DataInfo.DataChunkSize;

                if (!m_downloadedDataInfo.AvailableChunks[chunkNumber])
                {
                    m_bytesReceived += chunk.ChunkSize;

                    chunk.WriteFileChunk(DestFile);

                    chunkWasUsed = true;

                    // TODO: validate the check sum here.
                    m_downloadedDataInfo.AvailableChunks[chunkNumber] = true;
                }
                else
                {
                    m_tracer.DebugWriteLine("Chunk was thrown away because it was a duplicate.");
                }
            }

            lock (provider)
            {
                //check if this chunk is what we requested
                if (provider.CurrentRequest != null && provider.CurrentRequest.Offset == chunk.Offset)
                {
                    // if we have a valid request time, then use this data to
                    // calculate the speed of the connection.
                    if (DateTime.MinValue != provider.CurrentRequestTime)
                    {
                        // download timing
                        provider.BytesDownloaded += chunk.ChunkSize;
                        provider.DownloadTime += receivedTime - provider.CurrentRequestTime;
                    }

                    //we are free to make a new request
                    provider.CurrentRequest = null;
                }
            }

            //if we used the chunk...
            if (chunkWasUsed)
            {
                // notify that we have more data.
                OnDownloadDataReceivedRaise(args.Context.Sender, chunk);
            }

            lock (m_completeLock)
            {
                if (!m_isDownloadComplete)
                {
                    bool gotAllData = true;
                    for (int index = 0; index < m_downloadedDataInfo.AvailableChunks.Length; ++index)
                    {
                        gotAllData = gotAllData && m_downloadedDataInfo.AvailableChunks[index];
                    }
                    if (gotAllData)
                    {
                        provider.IsActiveConnection = false;
                        m_downloadedDataInfo.AllAvailable = true;
                        m_isDownloadComplete = true;
                        OnDownloadCompleteRaise();
                        Stop();
                    }
                }
                else
                {
                    provider.IsActiveConnection = false;
                }
            }

            if (!m_isDownloadComplete)
            {
                GetNextChunk(provider);
            }
        }

        #endregion Message Handlers

        #region Service

        private static readonly Guid DataReaderServiceId = new Guid(
            "{C8D62DB8-F8A3-49f7-8392-B86DD1DEC8A4}");

        private PyxNet.Service.ServiceId GetServiceId()
        {
            PyxNet.Service.ServiceId result = new PyxNet.Service.ServiceId(
                DataReaderServiceId, this.m_DataSetID.Guid);
            return result;
        }

        #endregion Service

        #region Downloading

        /// <summary>
        /// Unregisters the handlers.
        /// </summary>
        private void UnregisterHandlers()
        {
            m_stack.UnregisterHandler(DataChunk.MessageID, HandleChunk);
            m_stack.UnregisterHandler(DataInfo.MessageID, HandleInfo);
        }

        /// <summary>
        /// Registers the handlers.
        /// </summary>
        private void RegisterHandlers()
        {
            m_stack.RegisterHandler(DataChunk.MessageID, HandleChunk);
            m_stack.RegisterHandler(DataInfo.MessageID, HandleInfo);
        }

        /// <summary>
        /// Start the download.  No messages will be sent or data downloaded until
        /// the start method is called.  This method starts a thread to do its work and
        /// returns quickly, as the connection process to the data providers can be
        /// long.
        /// </summary>
        public void Start()
        {
            lock (startLock)
            {
                if (isStarted)
                {
                    return;
                }
                isStarted = true;
            }
            if (Status == null)
            {
                Status = new DownloadStatus()
                             {
                                 StartTime = DateTime.Now,
                                 Issues = "",
                                 FirstDataChunkRequestTime = DateTime.MinValue,
                                 FirstDataChunkReplayTime = DateTime.MinValue,
                                 FirstDataInfoRequestTime = DateTime.MinValue,
                                 FirstDataInfoReplayTime = DateTime.MinValue,
                                 DownloadedCompleted = false,
                                 DataChunkRepliesCount = 0,
                                 DataChunkRequestsCount = 0,
                                 DataInfoRepliesCount = 0,
                                 DataInfoRequestsCount = 0,
                                 NumberOfProvidersFound = 0,
                                 NumberOfProvidersNotFound = 0,
                             };
            }

            // register the messages that we want to monitor.
            RegisterHandlers();

            DateTime now = DateTime.Now;
            // get the info about the data so we can start downloading it.
            System.Threading.ThreadPool.QueueUserWorkItem(
                delegate(Object unused)
                {
                    if (DateTime.Now - now > TimeSpan.FromSeconds(5))
                    {
                        System.Diagnostics.Trace.WriteLine("get info started " + (DateTime.Now - now) + " after Start was called");
                    }
                    GetInfo();
                });
        }

        public void Stop()
        {
            lock (startLock)
            {
                if (!isStarted)
                {
                    return;
                }
                isStarted = false;
            }
            ResetProviders();
            UnregisterHandlers();
        }

        /// <summary>
        /// Download the file from PyxNet and save it to disk.
        /// </summary>
        /// <param name="downloadFilename">The fully qualified filename for storing the downloaded file.</param>
        /// <returns>True if the file was downlaoded and saved.</returns>
        public bool DownloadFile(String downloadFilename)
        {
            return DownloadFile(new System.IO.FileInfo(downloadFilename));
        }

        /// <summary>
        /// Download the file from PyxNet and save it to disk.
        /// </summary>
        /// <param name="downloadFile">The FileInfo used for storing the downloaded file.</param>
        /// <returns>True if the file was downloaded and saved.</returns>
        public bool DownloadFile(FileInfo downloadFile)
        {
            // Time out after 10 seconds.
            return DownloadFile(downloadFile, 10);
        }

        /// <summary>
        /// Download the file from PyxNet and save it to disk.
        /// This can take a while.
        /// </summary>
        /// <param name="downloadFile">The FileInfo used for storing the downloaded file.</param>
        /// <param name="timeOutSeconds">The number of seconds to wait.</param>
        /// <returns>True if the file was downloaded and saved.</returns>
        public bool DownloadFile(FileInfo downloadFile, int timeOutSeconds)
        {
            //lete data downloader finish downloading....
            if (Download(timeOutSeconds))
            {
                //try copy to destination file
                try
                {
                    if (null != downloadFile)
                    {
                        // make sure the directory exists.
                        if (!downloadFile.Directory.Exists)
                        {
                            downloadFile.Directory.Create();
                        }
                        CopyStream(DestFile, downloadFile, (int)this.DownloadedDataInfo.DataLength);
                    }
                    return true;
                }
                catch (IOException)
                {
                    // could not save it...
                    m_tracer.DebugWriteLine("Download finished but could not save the file.");
                }
            }

            return false;
        }

        public bool Download(int timeOutSeconds)
        {
            Status = new DownloadStatus()
                         {
                             StartTime = DateTime.Now,
                             Issues = "",
                             FirstDataChunkRequestTime = DateTime.MinValue,
                             FirstDataChunkReplayTime = DateTime.MinValue,
                             FirstDataInfoRequestTime = DateTime.MinValue,
                             FirstDataInfoReplayTime = DateTime.MinValue,
                             DownloadedCompleted = false,
                             DataChunkRepliesCount = 0,
                             DataChunkRequestsCount = 0,
                             DataInfoRepliesCount = 0,
                             DataInfoRequestsCount = 0,
                             NumberOfProvidersFound = 0,
                             NumberOfProvidersNotFound = 0,
                         };

            // Check the certificate.
            if (this.Certificate == null || !this.Certificate.Valid)
            {
                return false;
            }

            bool completeDownload = false;
            bool downloadFailed = false;

            DateTime lastNewDataTime = DateTime.Now;

            EventHandler<DownloadCompleteEventArgs> handleComplete =
                delegate(object sender, DataDownloader.DownloadCompleteEventArgs args)
                {
                    m_tracer.DebugWriteLine("Download complete.");

                    completeDownload = true;
                    if (Status != null)
                    {
                        Status.CompletedTime = DateTime.Now;
                        Status.DownloadedCompleted = true;
                    }
                };

            EventHandler<DownloadDataReceivedEventArgs> handleReceive =
                delegate(object sender, DataDownloader.DownloadDataReceivedEventArgs args)
                {
                    m_tracer.DebugWriteLine("Data received.");

                    lastNewDataTime = DateTime.Now;
                };

            if (GetProviders().Count == 0)
            {
                if (Status != null)
                {
                    Status.Issues += " started with no providers,";
                }
            }

            try
            {
                OnDownloadComplete += handleComplete;
                OnDataReceived += handleReceive;

                Start();

                // make sure we get a complete download.
                while (!completeDownload)
                {
                    // hang out for a moment
                    System.Threading.Thread.Sleep(200);
                    TimeSpan elapsedTime = DateTime.Now - lastNewDataTime;

                    // Break out of the loop if it takes too long.
                    if (elapsedTime.TotalSeconds >= timeOutSeconds)
                    {
                        if (Status != null)
                        {
                            Status.Issues += "Timeout";
                            Status.DownloadedCompleted = false;
                            Status.CompletedTime = DateTime.Now;
                        }
                        break;
                    }
                }

                if (completeDownload && !downloadFailed)
                {
                    return true;
                }
            }
            finally
            {
                OnDownloadComplete -= handleComplete;
                OnDataReceived -= handleReceive;
                Stop();
                if (Status != null)
                {
                    LogStatusIfNeeded();
                }
            }

            return false;
        }

        private void LogStatusIfNeeded()
        {
            if (Status.DownloadedCompleted == false)
            {
                Logging.Categories.Downloading.Error("DataGuid(" + DataSetID + ") : " + Status.ToString());
            }
            else if (Status.TotalTime.TotalSeconds > 15)
            {
                Logging.Categories.Downloading.Warning("DataGuid(" + DataSetID + ") : " + Status.ToString());
            }
        }

        public static byte[] GetStreamAsBuffer(Stream input)
        {
            if (input is FileStream)
            {
                string name = string.Empty;
                long length = 0;
                using (FileStream inputStream = input as FileStream)
                {
                    name = inputStream.Name;
                    length = inputStream.Length;
                }
                var result = new byte[length];
                using (var stream = new FileStream(name, FileMode.Open))
                {
                    stream.Read(result, 0, (int)length);
                }
                return result;
            }
            else if (input is MemoryStream)
            {
                MemoryStream inputStream = input as MemoryStream;
                return inputStream.GetBuffer();
            }
            else
            {
                // NOT-TODO: At some point, we could do a copy from other stream types (not needed now.)
                throw new NotImplementedException("CopyStream does not support all stream types.");
            }
        }

        /// <summary>
        /// Copies the stream to the given output file.
        /// </summary>
        /// <param name="input">The input.</param>
        /// <param name="output">The output.</param>
        /// <param name="lengthInBytes">The size of the stream in bytes.</param>
        public static void CopyStream(Stream input, FileInfo output, int lengthInBytes)
        {
            if (input is FileStream)
            {
                string name = string.Empty;
                using (FileStream inputStream = input as FileStream)
                {
                    name = inputStream.Name;
                }

                if (output.Exists)
                {
                    if (output.Length > 0)
                    {
                        throw new System.IO.IOException(
                            string.Format("Unable to copy to {0}. File already exists.",
                            output.FullName));
                    }
                    // Note: We are over-writing an empty file.
                    // System.Diagnostics.Trace.TraceError("Overwriting empty file {0}", output.FullName);
                }
                System.IO.File.Copy(name, output.FullName, true);
            }
            else if (input is MemoryStream)
            {
                MemoryStream inputStream = input as MemoryStream;
                var tmpFile = new FileInfo(output.FullName + "." + DateTime.Now.Ticks + ".tmp");
                if (tmpFile.Exists)
                {
                    try
                    {
                        tmpFile.Delete();
                    }
                    catch (Exception)
                    {
                        System.Diagnostics.Trace.WriteLine("Unable to remove tmp file :" + tmpFile);
                    }
                }

                using (FileStream outputStream = tmpFile.OpenWrite())
                {
                    outputStream.Write(inputStream.GetBuffer(), 0, lengthInBytes);
                    outputStream.Flush();
                }
                tmpFile.MoveTo(output.FullName);
            }
            else
            {
                // NOT-TODO: At some point, we could do a copy from other stream types (not needed now.)
                throw new NotImplementedException("CopyStream does not support all stream types.");
            }
        }

        /// <summary>
        /// Connect and send an info Request Message to a data provider.
        /// </summary>
        /// <param name="provider">The data provider to request info from.</param>
        /// <returns>False if the provider is no good; otherwise true if it should be tried again.</returns>
        private bool GetInfo(DataProvider provider, DataGuid datasetId)
        {
            try
            {
                if (null == provider.DataInfo)
                {
                    if (null != provider.Connection)
                    {
                        if (!m_isDownloadComplete &&
                            null != provider.Connection)
                        {
                            if (Status != null)
                            {
                                Status.DataInfoRequestsCount++;
                                if (Status.FirstDataInfoRequestTime == DateTime.MinValue)
                                {
                                    Status.FirstDataInfoRequestTime = DateTime.Now;
                                }
                            }

                            if (!provider.Connection.SendMessage(new DataInfoRequest(datasetId, ExtraInfo).ToMessage()))
                            {
                                m_tracer.DebugWriteLine("Failed to request info from {0}", provider.Connection.ToString());
                                provider.Connection = null;
                            }
                            else
                            {
                                if (Status != null)
                                {
                                    Status.NumberOfProvidersFound++;
                                }
                                m_tracer.DebugWriteLine("Requesting info from {0}", provider.Connection.ToString());
                            }
                        }
                    }

                    while (null == provider.Connection)
                    {
                        m_tracer.DebugWriteLine("Getting a connection to {0}.", provider.NodeInfo);

                        provider.Connection = m_stack.GetConnection(provider.NodeInfo, false,
                            TimeSpan.FromSeconds(20));
                        if (provider.Connection == null)
                        {
                            if (Status != null)
                            {
                                Status.NumberOfProvidersNotFound++;
                            }
                            m_stack.Tracer.DebugWriteLine("Could not connect to {0}.",
                                provider.NodeInfo);

                            // Give up on this provider for now.
                            break;
                        }

                        m_tracer.DebugWriteLine("Got a connection to {0}.", provider.NodeInfo);

                        if (!m_isDownloadComplete &&
                            null != provider.Connection)
                        {
                            if (Status != null)
                            {
                                Status.DataInfoRequestsCount++;
                                if (Status.FirstDataInfoRequestTime == DateTime.MinValue)
                                {
                                    Status.FirstDataInfoRequestTime = DateTime.Now;
                                }
                            }

                            if (!provider.Connection.SendMessage(new DataInfoRequest(datasetId, ExtraInfo).ToMessage()))
                            {
                                m_tracer.DebugWriteLine("Failed to request info from {0}", provider.Connection.ToString());
                                provider.Connection = null;
                            }
                            else
                            {
                                if (Status != null)
                                {
                                    Status.NumberOfProvidersFound++;
                                }
                                m_tracer.DebugWriteLine("Requesting info from {0}", provider.Connection.ToString());
                            }
                        }
                    }
                }
                else
                {
                    if (Status != null)
                    {
                        Status.Issues += " didn't send info request,";
                    }
                }
            }
            catch (Exception ex)
            {
                if (Status != null)
                {
                    Status.Issues += " didn't send info request (" + ex.Message + "),";
                }
                throw;
            }

            return true;
        }

        /// <summary>
        /// Contacts all providers to get information about the data.
        /// </summary>
        private void GetInfo()
        {
            if (m_isDownloadComplete)
            {
                return;
            }
            foreach (var nodeinfo in m_suggestedProviders)
            {
                AddProvider(nodeinfo);
            }

            // TODO: we may want to retry going through the list and connecting to anyone that
            // we couldn't connect to.
        }

        /// <summary>
        /// Add in a new provider of data to an active downloader.
        /// </summary>
        /// <param name="newProvider">The PyxNet NodeInfo of the node that can provide data.</param>
        /// <returns>True if a provider was added, false otherwise</returns>
        public bool AddProvider(NodeInfo newProvider)
        {
            return AddProvider(newProvider, DataSetID);
        }

        /// <summary>
        /// Add in a new provider of data to an active downloader.
        /// </summary>
        /// <param name="newProvider">The PyxNet NodeInfo of the node that can provide data.</param>
        /// /// <param name="providerDatasetId">the DataSetId to use for this provider.</param>
        /// <returns>True if a provider was added, false otherwise</returns>
        public bool AddProvider(NodeInfo newProvider, DataGuid providerDatasetId)
        {
            DataProvider newDataProvider = new DataProvider(newProvider);

            if (GetInfo(newDataProvider, providerDatasetId))
            {
                lock (m_providersLockObject)
                {
                    if (!m_providers.Any(provider => provider.NodeInfo.NodeGUID == newProvider.NodeGUID))
                    {
                        m_providers.Add(newDataProvider);
                        return true;
                    }
                }
            }
            return false;
        }

        /// <summary>
        /// Looks for a chunk of data that the provider has, and we don't and then requests that
        /// data to be transferred to us.
        /// </summary>
        /// <param name="provider">The provider that we wish to get data from.</param>
        private void GetNextChunk(DataProvider provider)
        {
            lock (provider)
            {
                //provider is busy...
                if (provider.CurrentRequest != null)
                {
                    //we don't ask a provider twice...
                    return;
                }
            }

            lock (m_getNextChunkLock)
            {
                List<int> qualifiedChunks = new List<int>(m_requestedChunks.Count);
                var providers = GetProviders();
                int lowestAvailability = providers.Count;

                for (int chunkNumber = 0; chunkNumber < m_requestedChunks.Count; ++chunkNumber)
                {
                    if (!m_requestedChunks[chunkNumber] &&
                        null != provider.DataInfo &&
                        (provider.DataInfo.AllAvailable ||
                        provider.DataInfo.AvailableChunks[chunkNumber]))
                    {
                        // we have a possible Chunk Number to request from this connection
                        // because we haven't asked for it, and they said they have it, so
                        // we will see how available this chunk is.
                        int availability = providers
                            .Where(x => x.DataInfo != null)
                            .Where(x => x.DataInfo.AllAvailable || x.DataInfo.AvailableChunks[chunkNumber])
                            .Count();

                        if (availability < lowestAvailability)
                        {
                            lowestAvailability = availability;
                            qualifiedChunks.Clear();
                        }
                        if (availability == lowestAvailability)
                        {
                            qualifiedChunks.Add(chunkNumber);
                        }
                    }
                }

                if (qualifiedChunks.Count > 0)
                {
                    // randomly choose one out of the bunch.
                    int requestChunkNumber = qualifiedChunks[m_randomGenerator.Next(qualifiedChunks.Count)];

                    StackConnection sendConnection;
                    Message sendMessage;
                    lock (provider)
                    {
                        int chunkOffset = requestChunkNumber * provider.DataInfo.DataChunkSize;
                        int chunkLength = provider.DataInfo.DataChunkSize;
                        if ((chunkOffset + chunkLength) > provider.DataInfo.DataLength)
                        {
                            chunkLength = (int)(provider.DataInfo.DataLength - chunkOffset);
                        }
                        provider.CurrentRequestTime = DateTime.Now;
                        provider.CurrentRequest = provider.CreateDataChunkRequest(chunkOffset, chunkLength, Certificate, ExtraInfo);
                        provider.IsDoubleRequested = false;
                        provider.IsActiveConnection = true;
                        m_tracer.DebugWriteLine("Requesting data chunk for data set {2} offset {1} from {0}",
                            provider.Connection.ToString(), chunkOffset, provider.DataInfo.DataSetID);
                        sendConnection = provider.Connection;
                        sendMessage = provider.CurrentRequest.ToMessage();
                    }
                    sendConnection.SendMessage(sendMessage);
                    m_requestedChunks[requestChunkNumber] = true;

                    if (Status != null)
                    {
                        Status.DataChunkRequestsCount++;
                        if (Status.FirstDataChunkRequestTime == DateTime.MinValue)
                        {
                            Status.FirstDataChunkRequestTime = DateTime.Now;
                        }
                    }
                }
                else
                {
                    // if we can't reuse this provider right now...
                    if (!UseFasterConnection(provider))
                    {
                        provider.IsActiveConnection = false;
                        // enable the timer here that will keep checking to
                        // see if we can use a faster connection.
                        m_deadManTimer.Enabled = true;
                    }
                }
            }
        }

        /// <summary>
        /// Handler for the deadman timer for this download.  This timer is only enabled
        /// if we have inactive providers in our list, and is used to try and put the
        /// inactive providers back into play by requesting the same data as a slow connection
        /// in hopes that we can get the data back sooner.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void HandleDeadManTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            // stop the timer if we are finished downloading.
            if (m_isDownloadComplete)
            {
                m_deadManTimer.Enabled = false;
                return;
            }

            // find out if we need to stop the timer at the end of the loop
            bool closeTimer = true;
            var providers = GetProviders();
            // look for any providers that are marked as inactive and see if we can put
            // them into action.
            foreach (var provider in providers
                .Where(x => (!x.IsActiveConnection
                            && null != x.DataInfo
                            && DateTime.MinValue != x.CurrentRequestTime)
                ))
            {
                if (UseFasterConnection(provider))
                {
                    provider.IsActiveConnection = true;
                }
                else
                {
                    // we have an inactive connecction still so we need to come back
                    // later and see if we need to press it into service.
                    closeTimer = false;
                }
            }

            if (closeTimer)
            {
                m_deadManTimer.Enabled = false;
            }
        }

        /// <summary>
        /// This is the logic for putting inactive connections back into play to try and get
        /// data from a fast connection instead of a slow one.
        /// </summary>
        /// <param name="provider">The provider that we are considering putting back into action.</param>
        /// <returns>True if the provider has been put back in play (sent a request for data).</returns>
        private bool UseFasterConnection(DataProvider provider)
        {
            // find twice the average time to download a chunk.
            long thresholdTicks;
            long downloadTimeTicks;
            long bytesDownloaded;
            int dataChunkSize;
            lock (provider)
            {
                downloadTimeTicks = provider.DownloadTime.Ticks;
                bytesDownloaded = provider.BytesDownloaded;
                dataChunkSize = provider.DataInfo.DataChunkSize;
            }
            if ((dataChunkSize == 0) || ((bytesDownloaded / dataChunkSize) == 0))
            {
                // if it is taking longer than a second we will try on another connection
                // that we don't know the speed of yet.
                thresholdTicks = 1000;
            }
            else
            {
                thresholdTicks = 2 * downloadTimeTicks / (bytesDownloaded / dataChunkSize);
            }
            TimeSpan thresholdTime = new TimeSpan(thresholdTicks);

            // find the slowest connection
            TimeSpan slowestDownload = new TimeSpan(0);
            int slowestConnectionIndex = 0;
            bool foundSlowestConnection = false;
            var providers = GetProviders();
            for (int index = 0; index < providers.Count; ++index)
            {
                if (providers[index].IsActiveConnection &&
                    !providers[index].IsDoubleRequested &&
                     providers[index].CurrentRequest != null)
                {
                    TimeSpan activeTime = DateTime.Now - providers[index].CurrentRequestTime;
                    if (activeTime > slowestDownload)
                    {
                        slowestDownload = activeTime;
                        slowestConnectionIndex = index;
                        foundSlowestConnection = true;
                    }
                }
            }

            if (foundSlowestConnection && slowestDownload > thresholdTime)
            {
                StackConnection sendConnection;
                Message sendMessage;
                lock (provider)
                {
                    // we found a slow connection to speed up
                    provider.IsActiveConnection = true;
                    //recreate the reuqest for this provider (dataset/use encription can be changed)
                    var oldRequest = providers[slowestConnectionIndex].CurrentRequest;
                    provider.CurrentRequest = provider.CreateDataChunkRequest(oldRequest.Offset, oldRequest.ChunkSize, Certificate, oldRequest.ExtraInfo);
                    provider.CurrentRequestTime = DateTime.Now;
                    sendConnection = provider.Connection;
                    sendMessage = provider.CurrentRequest.ToMessage();
                }
                sendConnection.SendMessage(sendMessage);

                // mark the slow connection as double requested.
                providers[slowestConnectionIndex].IsDoubleRequested = true;
                return true;
            }

            return false;
        }

        #endregion Downloading

        public void ResetProviders()
        {
            lock (m_providersLockObject)
            {
                foreach (var provider in m_providers)
                {
                    lock (provider)
                    {
                        provider.Connection = null;
                    }
                }
                m_providers.Clear();
            }
        }

        public void ResetProviders(List<NodeInfo> providers)
        {
            ResetProviders();
            foreach (var provider in providers)
            {
                AddProvider(provider);
            }
        }

        #region DownloadComplete Event

        /// <summary>
        /// Class which will be passed as the second argument to a DownloadCompleteHandler which
        /// wraps a DataDownloader object.
        /// </summary>
        public class DownloadCompleteEventArgs : EventArgs
        {
            private DataDownloader m_DataDownloader;

            public DataDownloader DataDownloader
            {
                get { return m_DataDownloader; }
                set { m_DataDownloader = value; }
            }

            internal DownloadCompleteEventArgs(DataDownloader theDataDownloader)
            {
                m_DataDownloader = theDataDownloader;
            }
        }

        /// <summary>
        /// Event which is fired when a data set is completely downloaded.
        /// </summary>
        public event EventHandler<DownloadCompleteEventArgs> OnDownloadComplete
        {
            add
            {
                m_OnDownloadComplete.Add(value);
            }
            remove
            {
                m_OnDownloadComplete.Remove(value);
            }
        }

        private Pyxis.Utilities.EventHelper<DownloadCompleteEventArgs> m_OnDownloadComplete = new Pyxis.Utilities.EventHelper<DownloadCompleteEventArgs>();

        /// <summary>
        /// Method to safely raise event OnDownloadComplete.
        /// </summary>
        protected void OnDownloadCompleteRaise()
        {
            // hold on to all the connections that we used for a while if
            // we are set up to do that.
            if (m_holdTime != TimeSpan.Zero)
            {
                foreach (DataProvider dp in GetProviders())
                {
                    if (dp.Connection != null)
                    {
                        m_stack.ConnectionManager.ConnectionHolder.HoldConnection(dp.Connection, m_holdTime);
                    }
                }
            }

            m_OnDownloadComplete.Invoke(this, new DownloadCompleteEventArgs(this));
        }

        #endregion DownloadComplete Event

        #region DownloadDataReceived Event

        /// <summary>
        /// Class which will be passed as the second argument to a DataReceivedHandler which
        /// wraps a DataDownloader object.
        /// </summary>
        public class DownloadDataReceivedEventArgs : EventArgs
        {
            private DataDownloader m_DataDownloader;

            public DataDownloader DataDownloader
            {
                get { return m_DataDownloader; }
                set { m_DataDownloader = value; }
            }

            public DataChunk Chunk
            {
                get;
                set;
            }

            public StackConnection Sender
            {
                get;
                set;
            }

            internal DownloadDataReceivedEventArgs(DataDownloader theDataDownloader, StackConnection theSender, DataChunk theChunk)
            {
                m_DataDownloader = theDataDownloader;
                Sender = theSender;
                Chunk = theChunk;
            }
        }

        /// <summary>
        /// Event which is fired when some data has been recieved.
        /// </summary>
        public event EventHandler<DownloadDataReceivedEventArgs> OnDataReceived
        {
            add
            {
                m_OnDataReceived.Add(value);
            }
            remove
            {
                m_OnDataReceived.Remove(value);
            }
        }

        private Pyxis.Utilities.EventHelper<DownloadDataReceivedEventArgs> m_OnDataReceived = new Pyxis.Utilities.EventHelper<DownloadDataReceivedEventArgs>();

        /// <summary>
        /// Method to safely raise event OnDataReceived.
        /// </summary>
        protected void OnDownloadDataReceivedRaise(StackConnection sender, DataChunk chunk)
        {
            m_OnDataReceived.Invoke(this, new DownloadDataReceivedEventArgs(this, sender, chunk));
        }

        #endregion DownloadDataReceived Event
    }
}

// Testing for this class is done in conjunction with the FilePublisher class.
// see FilePublisher.cs