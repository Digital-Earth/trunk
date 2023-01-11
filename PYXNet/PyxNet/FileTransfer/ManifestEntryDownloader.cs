using Pyxis.Storage;
using Pyxis.Storage.BlobProviders;
using Pyxis.Utilities;
using PyxNet.DataHandling;
using System;
using System.Collections.Generic;
using System.IO;

namespace PyxNet.FileTransfer
{
    public class ManifestEntryDownloader
    {
        private readonly Pyxis.Utilities.ManifestEntry m_manifestEntry;

        /// <summary>
        /// The stack we are working with.
        /// </summary>
        private readonly PyxNet.Stack m_stack;

        private PyxNet.Service.Certificate m_certificate;

        private Pyxis.Utilities.EventHelper<PyxNet.DataHandling.DataDownloader.DownloadDataReceivedEventArgs> m_DataReceived =
                    new Pyxis.Utilities.EventHelper<DataDownloader.DownloadDataReceivedEventArgs>();

        private FileTransfer.DownloadContext m_downloadContext;

        private DataDownloader m_downloader;

        private Pyxis.Utilities.EventHelper<FileDownloadCompleteEventArgs> m_FileDownloadComplete = new Pyxis.Utilities.EventHelper<FileDownloadCompleteEventArgs>();

        private TimeSpan m_holdTime = TimeSpan.FromSeconds(10);

        /// <summary>
        /// Permits self-synchronization.
        /// </summary>
        private object m_internalLock = new object();

        private object m_localMutex = new object();

        private int m_maxRetries = 3;
        private TimeSpan m_noActivityTimeout = TimeSpan.FromMinutes(10);
        private Pyxis.Utilities.DeadManTimer m_noActivityTimer;
        private PyxNet.Querier m_querier;
        private int m_retryCounts;
        private object m_statusLockObject = new object();
        private Pyxis.Utilities.EventHelper<DownloadFailedEventArgs> m_UpdateFailed = new Pyxis.Utilities.EventHelper<DownloadFailedEventArgs>();

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestEntryDownloader"/> class.
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="manifestEntry">The manifest entry.</param>
        /// <param name="certificate">The certificate.</param>
        /// <param name="downloadContext">The download context.</param>
        public ManifestEntryDownloader(
            Stack stack,
            Pyxis.Utilities.ManifestEntry manifestEntry,
            PyxNet.Service.Certificate certificate,
            DownloadContext downloadContext)
        {
            DownloadStatus = new ObservableObject<ManifestDownloader.DownloadStatusState>(ManifestDownloader.DownloadStatusState.Initializing);
            m_stack = stack;
            m_manifestEntry = manifestEntry;
            m_certificate = certificate;
            m_downloadContext = downloadContext;
            Progress = new Pyxis.Utilities.ProgressData
            {
                Title = "Downloading " + manifestEntry.FileName,
                Units = "Bytes",
                FinalValue = (int)manifestEntry.FileSize
            };

            Validator = new ManifestEntryMessageValidator(ManifestEntry);
        }

        /// <summary>
        /// DataReceivedEvent:  Data has been received.
        /// </summary>
        public event EventHandler<PyxNet.DataHandling.DataDownloader.DownloadDataReceivedEventArgs> DataReceived
        {
            add
            {
                m_DataReceived.Add(value);
            }
            remove
            {
                m_DataReceived.Remove(value);
            }
        }

        /// <summary>
        /// Event which is fired when a data set fails to download.
        /// </summary>
        public event EventHandler<DownloadFailedEventArgs> DownloadFailed
        {
            add
            {
                m_UpdateFailed.Add(value);
            }
            remove
            {
                m_UpdateFailed.Remove(value);
            }
        }

        // The event handler for when the search and download process for a file completes.
        /// <summary>
        /// Event which is fired when a data set is completely downloaded.
        /// </summary>
        public event EventHandler<FileDownloadCompleteEventArgs> FileDownloadComplete
        {
            add
            {
                m_FileDownloadComplete.Add(value);
            }
            remove
            {
                m_FileDownloadComplete.Remove(value);
            }
        }

        public System.Threading.CancellationToken CancellationToken { get; set; }

        public PyxNet.Service.Certificate Certificate
        {
            get { return m_certificate; }
            set { m_certificate = value; }
        }

        /// <summary>
        /// Gets or sets the download context.
        /// </summary>
        /// <value>The download context.</value>
        public FileTransfer.DownloadContext DownloadContext
        {
            get
            {
                return m_downloadContext;
            }
            set { m_downloadContext = value; }
        }

        public Pyxis.Utilities.ObservableObject<ManifestDownloader.DownloadStatusState> DownloadStatus { get; private set; }

        /// <summary>
        /// Gets the manifest entry.  This is the "filespec" that we are downloading.
        /// </summary>
        /// <value>The manifest entry.</value>
        public Pyxis.Utilities.ManifestEntry ManifestEntry
        {
            get { return m_manifestEntry; }
        }

        public int MaxRetries
        {
            get { return m_maxRetries; }
            set { m_maxRetries = value; }
        }

        /// <summary>
        /// Gets or sets the download timeout.  If no query results are returned
        /// or no data is downloaded before this time elapses,
        /// then DownloadFailed will be raised.
        /// </summary>
        /// <value>The query timeout.</value>
        public TimeSpan NoActivityTimeout
        {
            get { return m_noActivityTimeout; }
            set { m_noActivityTimeout = value; }
        }

        public Pyxis.Utilities.ProgressData Progress { get; set; }

        private bool ShouldCancel
        {
            get
            {
                return CancellationToken != null && CancellationToken.IsCancellationRequested;
            }
        }

        private ManifestEntryMessageValidator Validator { get; set; }

        /// <summary>
        /// Starts the updating process for the given file.
        /// </summary>
        public void Start()
        {
            lock (m_statusLockObject)
            {
                if (this.DownloadStatus.Value == ManifestDownloader.DownloadStatusState.Downloaded)
                {
                    OnFileDownloadComplete();
                    return;
                }
                else if (DownloadStatus.Value == ManifestDownloader.DownloadStatusState.Initializing)
                {
                    DownloadStatus.Value = ManifestDownloader.DownloadStatusState.Searching;
                }
                else
                {
                    return; // we are downloading this file
                }
            }
            //Try to find the file locally or from the storage server
            if (DownloadContext.VerifyTemp(ManifestEntry) || TryDownloadFromFileStorage())
            {
                DownloadStatus.Value = ManifestDownloader.DownloadStatusState.Downloaded;
                OnFileDownloadComplete();
                return;
            }

            // Search for the file
            m_querier = new PyxNet.Querier(m_stack,
                ManifestEntry.FileStamp,
                1000);

            // Timeout logic...
            m_noActivityTimer = new Pyxis.Utilities.DeadManTimer(NoActivityTimeout);
            m_noActivityTimer.Elapsed += HandleDeadManTimerExpired;

            // Event handler for when the file is found
            m_querier.Result += HandleQuerierResult;

            FileTransferLog.WriteLine("Searching for ", ManifestEntry.FileName + " (file stamp:" + ManifestEntry.FileStamp + ")");
            // Run the querier
            m_querier.Start();
        }

        private bool TryDownloadFromFileStorage()
        {
            //Try to download the file from the Storage
            try
            {
                var blobClient = new CachedBlobProvider();
                var fileClient = new Pyxis.Storage.FileSystemStorage.FileSystemStorage(blobClient);
                var location = this.DownloadContext.ConstructTempPath(this.ManifestEntry);
                var progressTracker = fileClient.DownloadFileAsync(ManifestEntry.FileStamp, location);
                Progress.FinalValue = (int)progressTracker.Max;
                progressTracker.ProgressMade += (t) => { Progress.CurrentValue.Value = (int)progressTracker.Current; };
                progressTracker.Wait();
                // if we failed, undo changes to Progress 
                if (!progressTracker.Task.Result)
                {
                    Progress.FinalValue = (int)ManifestEntry.FileSize;
                }
                return progressTracker.Task.Result;
            }
            catch (Exception e)
            {
                FileTransferLog.WriteLine("Exception occured trying to download the files from storage server : ", e.Message + "\n" + e.StackTrace);
                return false;
            }
        }

        /// <summary>
        /// Starts the updating process for the given file.
        /// </summary>
        public void Start(TimeSpan timeout)
        {
            NoActivityTimeout = timeout;
            Start();
        }

        public void Stop()
        {
            Querier querier = m_querier;
            m_querier = null;
            if (querier != null)
            {
                querier.Stop();
                querier.Result -= this.HandleQuerierResult;
            }
            if (m_noActivityTimer != null)
            {
                m_noActivityTimer.Elapsed -= HandleDeadManTimerExpired;
                m_noActivityTimer = null;
            }
            if (m_downloader != null)
            {
                m_downloader.Stop();
            }
        }

        /// <summary>
        /// Method to safely raise event DataReceived.
        /// </summary>
        protected void OnDataReceived(PyxNet.DataHandling.DataDownloader.DownloadDataReceivedEventArgs arg)
        {
            if (ShouldCancel)
            {
                CancelDownload();
            }
            m_DataReceived.Invoke(this, arg);
            m_noActivityTimer.KeepAlive();
        }

        /// <summary>
        /// Method to safely raise event OnDownloadComplete.
        /// </summary>
        protected void OnFileDownloadComplete()
        {
            m_FileDownloadComplete.Invoke(this, new FileDownloadCompleteEventArgs(this));
        }

        /// <summary>
        /// Method to safely raise event OnDownloadComplete.
        /// </summary>
        protected void OnUpdateFailed()
        {
            m_UpdateFailed.Invoke(this, new DownloadFailedEventArgs(this));
            Stop();
        }

        private void CancelDownload()
        {
            Stop();
            FileTransferLog.WriteLine("Download Cancelled for ", ManifestEntry.FileName);
            DownloadStatus.Value = ManifestDownloader.DownloadStatusState.Cancelled;
            OnUpdateFailed();
        }

        /// <summary>
        /// Handles the OnDataReceived event of the Downloader control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="args">The <see cref="PyxNet.DataHandling.DataDownloader.DataReceivedEventArgs"/> instance containing the event data.</param>
        private void HandleDataReceived(object sender, DataDownloader.DownloadDataReceivedEventArgs args)
        {
            if ((args != null) && (args.Chunk != null))
            {
                Progress.IncrementCurrentValue(args.Chunk.ChunkSize);
            }
            OnDataReceived(args);
        }

        private void HandleDeadManTimerExpired(object o, System.Timers.ElapsedEventArgs a)
        {
            if (ShouldCancel)
            {
                CancelDownload();
            }

            if (m_downloader == null)
            {
                DownloadStatus.Value = ManifestDownloader.DownloadStatusState.NotFound;
            }
            else
            {
                DownloadStatus.Value = ManifestDownloader.DownloadStatusState.CantDownload;
            }

            if (m_retryCounts < MaxRetries)
            {
                m_retryCounts++;
                Stop();
                lock (m_statusLockObject)
                {
                    DownloadStatus.Value = ManifestDownloader.DownloadStatusState.Initializing;
                }
                Start();
            }
            else
            {
                FileTransferLog.WriteLine("Failed to find " + ManifestEntry.FileName + " (file stamp:" + ManifestEntry.FileStamp + ")", "SynchronizationEvent expired");
                OnUpdateFailed();
            }
        }

        /// <summary>
        /// The event handler for when a download completes.
        /// Write the downloaded file to disk.
        /// </summary>
        /// <param name="sender">The event's sender.</param>
        /// <param name="args">The arguments from the download completion.</param>
        private void HandleDownloadComplete(object sender, PyxNet.DataHandling.DataDownloader.DownloadCompleteEventArgs args)
        {
            DownloadStatus.Value = ManifestDownloader.DownloadStatusState.Downloaded;

            try
            {
                FileTransferLog.WriteLine(ManifestEntry.FileName + " (file stamp:" + ManifestEntry.FileStamp + ")", "Download to temp completed");

                Stop();

                lock (m_internalLock)
                {
                    FileInfo downloadedFile = new FileInfo(
                        this.DownloadContext.ConstructTempPath(this.ManifestEntry));

                    try
                    {
                        // Create target directory if it does not already exist
                        if (!Directory.Exists(downloadedFile.DirectoryName))
                        {
                            Directory.CreateDirectory(downloadedFile.DirectoryName);
                        }
                    }
                    catch (IOException)
                    {
                        // The directory probably existed already.
                    }

                    try
                    {
                        // Write downloaded file to disk.
                        // The file might already be in use, if this event gets called simultaneously
                        // on two threads.  This will throw.
                        DataDownloader.CopyStream(args.DataDownloader.DestFile, downloadedFile,
                            (int)args.DataDownloader.DestFile.Length);
                    }
                    catch (Exception ex)
                    {
                        System.Diagnostics.Trace.WriteLine("failed to copy file to download cache, this could happen if file was downloaded more then one in parallel :" + ex.Message);
                        //we can contine to run because if the file is not valid - the VerifyTemp will return fale and throw OnUpdateFailed.
                    }

                    // Verify that the downloaded file's MD5 matches what we are looking for.
                    // This would catch if the file didn't download properly.
                    if (ManifestEntry.FileStamp != null)
                    {
                        if (!this.DownloadContext.VerifyTemp(this.ManifestEntry))
                        {
                            FileTransferLog.WriteLine("Failed to find " + ManifestEntry.FileName + " (file stamp:" + ManifestEntry.FileStamp + ")", "Verify temp failed");
                            OnUpdateFailed();
                            return;
                        }
                    }

                    OnFileDownloadComplete();

                    FileTransferLog.WriteLine(ManifestEntry.FileName + " (file stamp:" + ManifestEntry.FileStamp + ")", " Download completed");
                }
            }
            catch (Exception ex)
            {
                FileTransferLog.WriteLine("ManifestEntryDownloader failed with expection: ", ex.Message);
                OnUpdateFailed();
            }
        }

        /// <summary>
        /// Event handler for a single result being returned from the query thread.
        /// </summary>
        /// <param name="sender">The event's sender.</param>
        /// <param name="args">The arguments from the result of the query.</param>
        private void HandleQuerierResult(object sender, Querier.ResultEventArgs args)
        {
            /*
             * If this object's querier returned a result, process the result.
             * ie: this querier found the file it was assigned to find.
             */
            if (m_querier.Query.Guid.Equals(args.QueryResult.QueryGuid))
            {
                PyxNet.QueryResult result = args.QueryResult;

                //if we answer to ourself. skip it
                if (result.ResultNode == result.QueryOriginNode)
                {
                    return;
                }

                lock (m_localMutex)
                {
                    if (m_downloader == null)
                    {
                        DownloadStatus.Value = ManifestDownloader.DownloadStatusState.Downloading;

                        // Download the file
                        List<NodeInfo> downloadFrom = new List<NodeInfo>();
                        downloadFrom.Add(result.ResultNode);

                        Validator.AddProvider(result.ResultNode, result.MatchingDataSetID);

                        m_downloader =
                            new PyxNet.DataHandling.DataDownloader(result.MatchingDataSetID, m_stack, downloadFrom)
                                {
                                    Validator = Validator,
                                    HoldTime = m_holdTime,
                                    Certificate = m_certificate,
                                };

                        m_downloader.OnDownloadComplete += HandleDownloadComplete;
                        m_downloader.OnDataReceived += HandleDataReceived;

                        m_stack.Tracer.WriteLine("Downloading '{0}' from {1}.",
                            ManifestEntry.FileName, result.ResultNode.FriendlyName);

                        m_downloader.Start();
                    }
                    else
                    {
                        m_stack.Tracer.WriteLine("Also downloading '{0}' from {1}.",
                            ManifestEntry.FileName, result.ResultNode.FriendlyName);

                        if (m_downloader.AddProvider(result.ResultNode, result.MatchingDataSetID))
                        {
                            Validator.AddProvider(result.ResultNode, result.MatchingDataSetID);
                        }
                    }

                    FileTransferLog.WriteLine(ManifestEntry.FileName + " (file stamp:" + ManifestEntry.FileStamp + ")", "Downloading from " + result.ResultNode.FriendlyName);

                    m_noActivityTimer.KeepAlive();
                }
            }
        }

        /// <summary>
        /// Class which will be passed as the second argument to a DownloadCompleteHandler which
        /// wraps a DataDownloader object.
        /// </summary>
        public class DownloadFailedEventArgs : EventArgs
        {
            public DownloadFailedEventArgs(ManifestEntryDownloader theFileDownloader)
            {
                FileDataDownloader = theFileDownloader;
            }

            public ManifestEntryDownloader FileDataDownloader { get; set; }
        }

        /// <summary>
        /// Class which will be passed as the second argument to a DownloadCompleteHandler which
        /// wraps a DataDownloader object.
        /// </summary>
        public class FileDownloadCompleteEventArgs : EventArgs
        {
            private ManifestEntryDownloader m_FileDownloader;

            public FileDownloadCompleteEventArgs(ManifestEntryDownloader theFileDownloader)
            {
                m_FileDownloader = theFileDownloader;
            }

            public ManifestEntryDownloader FileDataDownloader
            {
                get { return m_FileDownloader; }
                set { m_FileDownloader = value; }
            }
        }

        internal class ManifestEntryMessageValidator : DataDownloader.IMessagesValidator
        {
            public ManifestEntryMessageValidator(ManifestEntry entry)
            {
                ManifestEntry = entry;
                ProvidersDataSetID = new Dictionary<NodeId, Guid>();
            }

            private ManifestEntry ManifestEntry { get; set; }

            private Dictionary<NodeId, Guid> ProvidersDataSetID { get; set; }

            public void AddProvider(NodeInfo nodeInfo, Guid DatasetId)
            {
                ProvidersDataSetID[nodeInfo.NodeId] = DatasetId;
            }

            public bool IsMatch(NodeInfo fromNode, DataInfo dataInfo)
            {
                return dataInfo.DataLength == ManifestEntry.FileSize &&
                    ProvidersDataSetID.ContainsKey(fromNode.NodeId) &&
                    dataInfo.DataSetID == ProvidersDataSetID[fromNode.NodeId];
            }

            public bool IsMatch(NodeInfo fromNode, DataChunk dataChunk)
            {
                return ProvidersDataSetID.ContainsKey(fromNode.NodeId) &&
                    dataChunk.DataSetID == ProvidersDataSetID[fromNode.NodeId];
            }
        }
    }
}