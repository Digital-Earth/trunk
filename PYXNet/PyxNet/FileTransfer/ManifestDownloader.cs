/******************************************************************************
ManifestDownloader.cs

begin      : May 23, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace PyxNet.FileTransfer
{
    /// <summary>
    /// This utility class downloads the contents of a Manifest.  It is
    /// assumed that the Manifest itself (as an XML file) has already been
    /// downloaded and read into a Manifest object.
    /// </summary>
    public class ManifestDownloader
    {
        private readonly Pyxis.Utilities.Manifest m_manifest;

        private readonly Stack m_stack;

        private Pyxis.Utilities.DynamicList<ManifestEntryDownloader> m_activeDownloaders =
            new Pyxis.Utilities.DynamicList<ManifestEntryDownloader>();

        private SynchronizationEvent m_allDownloadsFinished = new SynchronizationEvent();

        private PyxNet.Service.Certificate m_certificate;

        private DownloadContext m_downloadContext;

        private ManifestEntry m_manifestEntry;

        private TimeSpan m_noActivityTimeout = TimeSpan.FromSeconds(300);

        private Pyxis.Utilities.DynamicList<ManifestEntryDownloader> m_waitingDownloaders =
           new Pyxis.Utilities.DynamicList<ManifestEntryDownloader>();

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestDownloader"/> class.
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="manifest">The manifest.</param>
        /// <param name="certificate">The certificate.</param>
        /// <param name="manifestDirectory">The manifest directory.</param>
        public ManifestDownloader(Stack stack, Pyxis.Utilities.Manifest manifest,
            PyxNet.Service.Certificate certificate, string manifestDirectory)
        {
            m_stack = stack;
            m_manifest = manifest;
            m_certificate = certificate;

            string outputDirectory =
                System.IO.Path.Combine(OutputDirectory, manifestDirectory);
            m_downloadContext = new DownloadContext(outputDirectory);

            DataProgress = new Pyxis.Utilities.ProgressData
            {
                Title = "Downloading manifest " + manifest.GetHashCode().ToString(),
                Units = "Bytes",
                FinalValue = 0
            };
            foreach (var entry in m_manifest.Entries)
            {
                DataProgress.IncrementFinalValue((int)entry.FileSize);
            }

            FileProgress = new Pyxis.Utilities.ProgressData
            {
                Title = "Downloading manifest " + manifest.GetHashCode().ToString(),
                Units = "Files",
                FinalValue = m_manifest.Entries.Count
            };
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestDownloader"/> class.
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="manifest">The manifest.</param>
        /// <param name="certificate">The certificate.</param>
        public ManifestDownloader(Stack stack, Pyxis.Utilities.Manifest manifest,
            PyxNet.Service.Certificate certificate)
            : this(stack, manifest, certificate, manifest.InstallationDirectory)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestDownloader"/> class.
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="manifest">The manifest.</param>
        /// <param name="certificate">The certificate.</param>
        public ManifestDownloader(Stack stack, Pyxis.Utilities.ManifestEntry manifestEntry,
            PyxNet.Service.Certificate certificate, DownloadContext downloadContext)
        {
            m_stack = stack;
            m_manifestEntry = manifestEntry;
            m_certificate = certificate;

            string outputDirectory =
                System.IO.Path.Combine(OutputDirectory, downloadContext.TempDirectory);
            m_downloadContext = new DownloadContext(outputDirectory);

            DataProgress = new Pyxis.Utilities.ProgressData
            {
                Title = "Downloading manifest entry" + manifestEntry.GetHashCode().ToString(),
                Units = "Bytes",
                FinalValue = 0
            };

            DataProgress.IncrementFinalValue((int)manifestEntry.FileSize);

            FileProgress = new Pyxis.Utilities.ProgressData
            {
                Title = "Downloading manifest entry " + manifestEntry.GetHashCode().ToString(),
                Units = "Files",
                FinalValue = m_manifestEntry == null ? m_manifest.Entries.Count : 1
            };
        }

        public enum DownloadStatusState
        {
            Initializing,
            Searching,
            Downloading,
            Downloaded,
            NotFound,
            CantDownload,
            Cancelled
        }

        /// <summary>
        /// Gets the base directory.
        /// </summary>
        /// <value>The base directory.</value>
        public static string BaseDirectory
        {
            get
            {
                string baseDirectory = System.IO.Path.Combine(
                    Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                    "PYXIS");
                return baseDirectory;
            }
        }

        /// <summary>
        /// Gets the cache directory.
        /// </summary>
        /// <value>The cache directory.</value>
        public static string CacheDirectory
        {
            get
            {
                string cacheDirectory = System.IO.Path.Combine(
                    BaseDirectory,
                    "DownloadCache");
                return cacheDirectory;
            }
        }

        /// <summary>
        /// Gets the output directory.
        /// </summary>
        /// <value>The output directory.</value>
        public static string OutputDirectory
        {
            get
            {
                string outputDirectory = System.IO.Path.Combine(
                    BaseDirectory,
                    "Downloads");
                return outputDirectory;
            }
        }

        public CancellationToken CancellationToken
        {
            get;
            set;
        }

        public PyxNet.Service.Certificate Certificate
        {
            get { return m_certificate; }
            set { m_certificate = value; }
        }

        /// <summary>
        /// Gets or sets the data progress, measured in total bytes downloaded.
        /// </summary>
        /// <value>The data progress.</value>
        public Pyxis.Utilities.ProgressData DataProgress { get; private set; }

        /// <summary>
        /// Gets or sets the download context.
        /// </summary>
        /// <value>The download context.</value>
        public DownloadContext DownloadContext
        {
            get { return m_downloadContext; }
            set { m_downloadContext = value; }
        }

        public IEnumerable<ManifestEntry> Entries
        {
            get
            {
                if (m_manifestEntry != null)
                {
                    yield return m_manifestEntry;
                }
                else
                {
                    foreach (var entry in m_manifest.Entries)
                    {
                        yield return entry;
                    }
                }
            }
        }

        /// <summary>
        /// Gets or sets the file progress (number of files downloaded).
        /// </summary>
        /// <value>The file progress.</value>
        public Pyxis.Utilities.ProgressData FileProgress { get; private set; }

        public Pyxis.Utilities.Manifest Manifest
        {
            get { return m_manifest; }
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

        public Stack Stack
        {
            get { return m_stack; }
        }

        private bool ShouldCancel
        {
            get
            {
                return CancellationToken != null && CancellationToken.IsCancellationRequested;
            }
        }

        /// <summary>
        /// Deploys the specified manifest, by copying it to the output directory.
        /// </summary>
        /// <param name="manifest">The manifest.</param>
        /// <returns>True, iff the manifest contents were deployed.</returns>
        public static bool Deploy(Pyxis.Utilities.Manifest manifest)
        {
            string outputDirectory =
                System.IO.Path.Combine(OutputDirectory, manifest.InstallationDirectory);
            DownloadContext downloadContext = new DownloadContext(outputDirectory);
            DownloadContext.DownloadedManifest downloadedManifest = new DownloadContext.DownloadedManifest(
                downloadContext, manifest);
            try
            {
                if (downloadedManifest.Valid)
                {
                    downloadedManifest.Update();
                    return true;
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine("Unable to update manifest. " + ex.Message);
            }
            return false;
        }

        /// <summary>
        /// Deploys the specified manifest, by copying it to the output directory.
        /// </summary>
        /// <param name="manifestFact">The manifest fact.</param>
        /// <returns>True, iff the manifest contents were deployed.</returns>
        public static bool Deploy(PyxNet.Service.ResourceInstanceFact manifestFact)
        {
            Pyxis.Utilities.Manifest manifest = ReadExistingManifest(manifestFact);
            if (manifest != null)
            {
                return Deploy(manifest);
            }
            return false;
        }

        /// <summary>
        /// Reads an existing manifest.
        /// </summary>
        /// <param name="manifestFact">The manifest fact.</param>
        /// <returns></returns>
        public static Pyxis.Utilities.Manifest ReadExistingManifest(PyxNet.Service.ResourceInstanceFact manifestFact)
        {
            string directoryName = System.IO.Path.Combine(
                CacheDirectory, manifestFact.ManifestEntry.FileStampAsDirectoryName);
            string fileName = System.IO.Path.Combine(directoryName,
                manifestFact.ManifestEntry.FileName);
            Pyxis.Utilities.Manifest manifest = Pyxis.Utilities.Manifest.ReadFromFile(fileName);
            return manifest;
        }

        public bool Download()
        {
            foreach (var groupedEntries in Entries.GroupBy(entry => entry.GetIdentity()))
            {
                if (ShouldCancel)
                {
                    RaiseDownloadFailed(null, DownloadStatusState.Cancelled);
                    RaiseDownloadFinished();
                    return false;
                }
                var entries = groupedEntries.ToList();

                if (entries.Count > 1)
                {
                    System.Diagnostics.Trace.WriteLine("Multiple entries have same identity: " + String.Join(", ", entries.Select(x => x.FileName)));
                }

                m_waitingDownloaders.Add(PrepareDownloader(entries[0], entries.Count()));
            }

            for (int i = 0; i < MaximumParallelDownloads && !ShouldCancel; i++)
            {
                TryStartingNewDownload();
            }

            m_allDownloadsFinished.Wait();

            RaiseDownloadFinished();

            return !Failed;
        }

        public void Start()
        {
            var thread = new System.Threading.Thread(delegate() { Download(); });
            thread.Name = "Manifest downloader thread";
            thread.IsBackground = true;
            thread.Start();
        }

        /// <summary>
        /// prepares a single downloader
        /// </summary>
        /// <param name="entry">The entry.</param>
        /// <param name="numberOfEntries">how many entires have the same file Identity.</param>
        private ManifestEntryDownloader PrepareDownloader(Pyxis.Utilities.ManifestEntry entry, int numberOfEntries)
        {
            var downloader = new ManifestEntryDownloader(m_stack, entry, m_certificate, DownloadContext);
            downloader.NoActivityTimeout = NoActivityTimeout;

            RaiseDownloadStarting(this, ref downloader);

            if (downloader.DownloadStatus.Value == DownloadStatusState.Downloaded)
            {
                FileProgress.IncrementCurrentValue(numberOfEntries);
                return downloader;
            }

            downloader.Progress.CurrentValue.Changed += delegate(object o, Pyxis.Utilities.ChangedEventArgs<int> args)
                {
                    DataProgress.IncrementCurrentValue((args.NewValue - args.OldValue) * numberOfEntries);
                };

            downloader.DownloadStatus.Changed += delegate(object sender, ChangedEventArgs<ManifestDownloader.DownloadStatusState> e)
                {
                    m_downloadStatusChanged.Invoke(this, new DownloadStateChangedEvetArgs(entry.FileName, e.NewValue, FileProgress.CompletedAmount.Value));
                };

            downloader.FileDownloadComplete += delegate(object sender, ManifestEntryDownloader.FileDownloadCompleteEventArgs args)
                {
                    m_stack.Tracer.WriteLine("Succesful download of {0}.",
                        args.FileDataDownloader.ManifestEntry.FileName);
                    System.Diagnostics.Debug.Assert(downloader == args.FileDataDownloader);
                    m_activeDownloaders.Remove(downloader);
                    FileProgress.IncrementCurrentValue(numberOfEntries);
                    TryStartingNewDownload();
                };

            downloader.DownloadFailed += delegate(object sender, ManifestEntryDownloader.DownloadFailedEventArgs args)
                {
                    if (downloader.DownloadStatus.Value != DownloadStatusState.Downloaded)
                    {
                        m_stack.Tracer.WriteLine("(UpdateFailed) Failed to download {0}. retrying...",
                            args.FileDataDownloader.ManifestEntry.FileName);

                        m_activeDownloaders.Remove(downloader);
                        downloader.Stop();
                        RaiseDownloadFailed(entry, downloader.DownloadStatus.Value);
                    }
                };

            downloader.CancellationToken = CancellationToken;

            return downloader;
        }

        private void TryStartingNewDownload()
        {
            ManifestEntryDownloader nextDownloader = null;
            lock (m_waitingDownloaders)
            {
                if (Failed || m_activeDownloaders.Count > MaximumParallelDownloads)
                {
                    return;
                }

                if (ShouldCancel)
                {
                    RaiseDownloadFailed(null, DownloadStatusState.Cancelled);
                    m_allDownloadsFinished.Pulse();
                }

                nextDownloader = m_waitingDownloaders.FirstOrDefault();
                if (nextDownloader != null)
                {
                    m_activeDownloaders.Add(nextDownloader);
                    m_waitingDownloaders.Remove(nextDownloader);
                }
                else if (m_activeDownloaders.Count == 0)
                {
                    m_allDownloadsFinished.Pulse();
                }
            }
            if (nextDownloader != null)
            {
                nextDownloader.Start();
                m_stack.Tracer.WriteLine("Starting download of {0}.", nextDownloader.ManifestEntry.FileName);
            }
        }

        #region Finished Event

        private Pyxis.Utilities.EventHelper<DownloadFinishedEventArgs> m_finished = new Pyxis.Utilities.EventHelper<DownloadFinishedEventArgs>();

        /// <summary>
        /// Event handler for Finished.
        /// </summary>
        public event EventHandler<DownloadFinishedEventArgs> DownloadFinished
        {
            add
            {
                m_finished.Add(value);
            }
            remove
            {
                m_finished.Remove(value);
            }
        }

        /// <summary>
        /// Raises the Finished event.
        /// </summary>
        private void RaiseDownloadFinished()
        {
            var args = new DownloadFinishedEventArgs
            {
                ManifestDownloader = this,
                DownloadedManifest = m_manifestEntry != null ? null : new DownloadContext.DownloadedManifest(DownloadContext, Manifest)
            };
            m_finished.Invoke(this, args);
        }

        /// <summary> EventArgs for a Finished event. </summary>
        public class DownloadFinishedEventArgs : EventArgs
        {
            internal DownloadFinishedEventArgs()
            {
            }

            /// <summary>
            /// The downloaded manifest.
            /// </summary>
            /// <value>The downloaded manifest.</value>
            public DownloadContext.DownloadedManifest DownloadedManifest { get; set; }

            /// <summary>The ManifestDownloader.</summary>
            public ManifestDownloader ManifestDownloader { get; set; }
        }

        #endregion Finished Event

        #region Failure Event

        private Pyxis.Utilities.EventHelper<FailureEventArgs> m_failure = new Pyxis.Utilities.EventHelper<FailureEventArgs>();

        /// <summary> Event handler for Failure. </summary>
        public event EventHandler<FailureEventArgs> DownloadFailed
        {
            add
            {
                m_failure.Add(value);
            }
            remove
            {
                m_failure.Remove(value);
            }
        }

        public bool Failed { get; private set; }

        /// <summary>
        /// Raises the Failure event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theManifestEntry"></param>
        public void RaiseDownloadFailed(Pyxis.Utilities.ManifestEntry theManifestEntry, DownloadStatusState status)
        {
            // TODO: Check to see if the download worked (from the DownloadContext)
            foreach (var downloader in m_activeDownloaders)
            {
                downloader.Stop();
            }
            Failed = true;
            FailureEventArgs args = new FailureEventArgs(theManifestEntry, status);
            m_allDownloadsFinished.Pulse();
            m_failure.Invoke(this, args);
        }

        /// <summary> EventArgs for a Failure event. </summary>
        public class FailureEventArgs : EventArgs
        {
            internal FailureEventArgs(Pyxis.Utilities.ManifestEntry theManifestEntry, DownloadStatusState status)
            {
                ManifestEntry = theManifestEntry;
                DownloadStatus = status;
            }

            public DownloadStatusState DownloadStatus { get; private set; }

            public Pyxis.Utilities.ManifestEntry ManifestEntry { get; private set; }
        }

        #endregion Failure Event

        #region DownloadStartingEvent

        private Pyxis.Utilities.EventHelper<DownloadStartingEventArgs> m_DownloadStarting = new Pyxis.Utilities.EventHelper<DownloadStartingEventArgs>();

        private Pyxis.Utilities.EventHelper<DownloadStateChangedEvetArgs> m_downloadStatusChanged = new Pyxis.Utilities.EventHelper<DownloadStateChangedEvetArgs>();
        private int m_maximumParallelDownloads = 10;

        /// <summary> Event handler for DownloadStarting. </summary>
        public event EventHandler<DownloadStartingEventArgs> DownloadStarting
        {
            add
            {
                m_DownloadStarting.Add(value);
            }
            remove
            {
                m_DownloadStarting.Remove(value);
            }
        }

        /// <summary> Event handler for Download Status changed. </summary>
        public event EventHandler<DownloadStateChangedEvetArgs> DownloadStatusChanged
        {
            add
            {
                m_downloadStatusChanged.Add(value);
            }
            remove
            {
                m_downloadStatusChanged.Remove(value);
            }
        }

        public int MaximumParallelDownloads
        {
            get { return m_maximumParallelDownloads; }
            set { m_maximumParallelDownloads = value; }
        }

        /// <summary>
        /// Raises the DownloadStarting event.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="theManifestEntryDownloader">The data downloader with querier.</param>
        /// <param name="downloadNeeded">if set to <c>true</c> [download needed].</param>
        private void RaiseDownloadStarting(object sender, ref ManifestEntryDownloader theManifestEntryDownloader)
        {
            DownloadStartingEventArgs args = new DownloadStartingEventArgs(theManifestEntryDownloader);
            m_DownloadStarting.Invoke(sender, args);
            theManifestEntryDownloader = args.ManifestEntryDownloader;
        }

        /// <summary> EventArgs for a DownloadStarting event. </summary>
        public class DownloadStartingEventArgs : EventArgs
        {
            private ManifestEntryDownloader m_manifestEntryDownloader;

            internal DownloadStartingEventArgs(ManifestEntryDownloader theManifestEntryDownloader)
            {
                m_manifestEntryDownloader = theManifestEntryDownloader;
            }

            /// <summary>The DataHandling.ManifestEntryDownloader .</summary>
            public ManifestEntryDownloader ManifestEntryDownloader
            {
                get { return m_manifestEntryDownloader; }
                set { m_manifestEntryDownloader = value; }
            }
        }

        #endregion DownloadStartingEvent

        public class DownloadStateChangedEvetArgs : EventArgs
        {
            internal DownloadStateChangedEvetArgs(string fileName, DownloadStatusState state, double progress)
            {
                FileName = fileName;
                State = state;
                Progress = progress;
            }

            public string FileName { get; private set; }

            public double Progress { get; private set; }

            public DownloadStatusState State { get; private set; }
        }
    }

    public class FileTransferLog
    {
        public static void WriteLine(string key, string value)
        {
            Console.WriteLine(key + " : " + value);
            Logging.Categories.Downloading.Log(key, value);
        }
    }
}