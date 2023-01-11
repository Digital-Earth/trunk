/******************************************************************************
DownloadJob.cs

begin		: February 9, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using Pyxis.Services.PipelineLibrary.Repositories;
using Pyxis.Utilities;
using ApplicationUtility;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// Responsible for downlading the data file(s) that make up the pipeline.
    /// </summary>
    internal class DownloadJob : JobOnProcess
    {
        private long m_downloadedBytes;

        private object m_downloadedBytesLockObject = new Object();

        public DownloadJob(IProcess_SPtr process, string name)
        {
            ProcRef = new ProcRef(process);
            Process = process;
            PipelineName = name;

            Status = new ObservableOperationStatus();
            Status.Operation.OperationType = Pyxis.Contract.Operations.OperationType.Download;
            Status.Operation.Parameters.Add("ProcRef", pyxlib.procRefToStr(ProcRef));
            Status.Description = String.Format("Download Source files for '{0}={1}'", PipelineName, ProcRef);
        }

        public static string OutputDirectory
        {
            get { return "UserDownloads"; }
        }

        public long DownloadedBytes
        {
            get
            {
                lock (m_downloadedBytesLockObject)
                {
                    return m_downloadedBytes;
                }
            }
            private set
            {
                lock (m_downloadedBytesLockObject)
                {
                    m_downloadedBytes = value;
                    Status.Progress = 100.0f * m_downloadedBytes / TotalBytes;
                }
            }
        }

        public string PipelineName { get; private set; }

        public long TotalBytes { get; private set; }

        public static bool operator !=(DownloadJob a, DownloadJob b)
        {
            return !(a == b);
        }

        public static bool operator ==(DownloadJob a, DownloadJob b)
        {
            if (object.ReferenceEquals(a, null))
            {
                return object.ReferenceEquals(b, null);
            }
            return a.Equals(b);
        }

        public override bool Equals(object obj)
        {
            if (obj != null && obj is DownloadJob && ProcRef == ((DownloadJob)obj).ProcRef)
            {
                return true;
            }
            return base.Equals(obj);
        }

        public override int GetHashCode()
        {
            if (ProcRef != null)
            {
                return ProcRef.GetHashCode();
            }
            else
            {
                return 0;
            }
        }

        public void IncrementDownloadedBytes(long amount)
        {
            lock (m_downloadedBytesLockObject)
            {
                DownloadedBytes += amount;
            }
        }

        protected override void DoExecute()
        {

            bool downloadSuccessfully = Download();

            if (downloadSuccessfully)
            {
                Pyxis.Utilities.ChecksumSingleton.Checksummer.WriteChecksumCache();
                GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Download, "Reinitializing '{0}={1}'", PipelineName, pyxlib.procRefToStr(ProcRef));
                Process.reinitProc(true);
                GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Download, "Reinitialize Completed '{0}={1}'", PipelineName, pyxlib.procRefToStr(ProcRef));

                PipelineRepository.Instance.SetIsDownloaded(ProcRef, true);
                PipelineRepository.Instance.CheckPoint();
            }
            else
            {
                throw new Exception("Failed to download file(s) for :" + PipelineName);
            }
        }

        private bool Download()
        {
            if (Process.getInitState() != IProcess.eInitStatus.knInitialized)
            {
                Process.initProc(true);
            }

            var manifests = new List<Manifest>(Process.ExtractManifests());

            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Download, "Downloading the following data file(s) for '{0}={1}':\n=======",
                    PipelineName, pyxlib.procRefToStr(ProcRef));

            bool allFilesDownloadedSuccessfully = true;

            TotalBytes = manifests.Sum(x => x.TotalSize);

            foreach (var manifest in manifests)
            {
                var helper = new DownloadHelper(manifest, this);
                allFilesDownloadedSuccessfully &= helper.Download(CancellationToken);
                if (ExitManager.ShouldExit || CancellationToken.IsCancellationRequested)
                {
                    throw new System.Threading.Tasks.TaskCanceledException();
                }
            }

            return allFilesDownloadedSuccessfully;
        }

        private class DownloadHelper
        {
            public List<string> DownloadedFiles;
            private DownloadJob m_downloadJob;
            private List<string> m_failedFiles;
            private SynchronizationEvent m_finished;
            private Manifest m_manifest;

            public DownloadHelper(Manifest manifest, DownloadJob downloadJob)
            {
                m_manifest = manifest;
                m_downloadJob = downloadJob;
                DownloadedFiles = new List<string>();
                m_failedFiles = new List<string>();
                // TODO[kabiraman]: What is this directory for?
                m_manifest.InstallationDirectory = DownloadJob.OutputDirectory;
            }

            public bool Download(CancellationToken ct)
            {
                var downloader = new PyxNet.FileTransfer.ManifestDownloader(PyxNet.StackSingleton.Stack, m_manifest, null);
                downloader.CancellationToken = ct;
                downloader.DownloadStarting += OnDownloadStarted;
                downloader.DownloadFinished += OnDownloadFinished;
                downloader.DownloadFailed += OnDownloadFailed;
                downloader.DownloadStatusChanged += DownloadStatusChanged;

                m_finished = new SynchronizationEvent();
                downloader.Start();
                m_finished.Wait();

                return (m_failedFiles.Count == 0);
            }

            private void DownloadStatusChanged(object sender, PyxNet.FileTransfer.ManifestDownloader.DownloadStateChangedEvetArgs e)
            {
                GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Download, "Download status of data file(s) for {0}  : {1}", m_downloadJob.PipelineName, e.State);
            }

            private void OnDownloadFailed(object sender, PyxNet.FileTransfer.ManifestDownloader.FailureEventArgs args)
            {

                if (args.ManifestEntry == null)
                {
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Download, "Download failed for {0}={1} Status:{2}",
                        m_downloadJob.PipelineName, pyxlib.procRefToStr(m_downloadJob.ProcRef), args.DownloadStatus);

                    foreach (var entry in m_manifest.Entries)
                    {
                        m_failedFiles.Add(
                            string.Format(
                        "File {0} with checksum {1}", entry.FileName, entry.FileStamp));
                    }
                }
                else
                {
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Download, "Download failed for {0} (checksum {1}) Status {2}.",
                        args.ManifestEntry.FileName, args.ManifestEntry.FileStamp, args.DownloadStatus);

                    m_failedFiles.Add(string.Format(
                        "File {0} with checksum {1}", args.ManifestEntry.FileName, args.ManifestEntry.FileStamp));
                }

                m_finished.Pulse();
            }

            private void OnDownloadFinished(object sender, PyxNet.FileTransfer.ManifestDownloader.DownloadFinishedEventArgs args)
            {
                if (args.ManifestDownloader.Failed)
                {
                    return;
                }
                try
                {
                    if (!args.DownloadedManifest.Valid)
                    {
                        throw new Exception("Invalid Download manifest");
                    }

                    foreach (var entry in args.DownloadedManifest.Entries)
                    {
                        if (!DownloadedFiles.Contains(entry.ManifestEntry.FileStamp))
                        {
                            DownloadedFiles.Add(entry.ManifestEntry.FileStamp);
                        }
                    }
                }
                catch (Exception ex)
                {
                    // TODO[kabiraman]: What to do here?
                    Trace.error(string.Format(
                        "An error occurred while attempting to update a download manifest for '{0}={1}':\n{2}",
                        m_downloadJob.PipelineName, pyxlib.procRefToStr(m_downloadJob.ProcRef), ex.Message));
                    Console.WriteLine(string.Format(
                        "\nAn unexpected error occurred after the downloading of certain data file(s) completed for '{0}={1} :\n{2} ; \nMoving on to next file(s), if any.\n",
                        m_downloadJob.PipelineName, pyxlib.procRefToStr(m_downloadJob.ProcRef), ex.Message));

                    foreach (var entry in m_manifest.Entries)
                    {
                        m_failedFiles.Add(
                            string.Format(
                        "File {0} with checksum {1}", entry.FileName, entry.FileStamp));
                    }
                }
                finally
                {
                    m_finished.Pulse();
                }
            }

            private void OnDownloadStarted(object s, PyxNet.FileTransfer.ManifestDownloader.DownloadStartingEventArgs args)
            {
                int progress = 0;

                args.ManifestEntryDownloader.Progress.CurrentValue.Changed +=
                    delegate(object o, Pyxis.Utilities.ChangedEventArgs<int> change)
                    {
                        int percent = (int)(args.ManifestEntryDownloader.Progress.CompletedAmount.Value * 100);
                        var entryDownloader = args.ManifestEntryDownloader;
                        // don't trace out progress more than in increments of 1%.
                        if (progress != percent)
                        {
                            progress = percent;

                            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Download,
                                "Downloading '{0} with checksum {1}' - currently {2} of {3} bytes ({4}%)",
                                entryDownloader.ManifestEntry.FileName,
                                entryDownloader.ManifestEntry.FileStamp,
                                entryDownloader.Progress.CurrentValue.Value,
                                entryDownloader.Progress.FinalValue,
                                progress.ToString());

                            if (ExitManager.ShouldExit)
                            {
                                m_finished.Pulse();
                            }
                        }

                        m_downloadJob.IncrementDownloadedBytes(change.NewValue - change.OldValue);
                    };
            }
        }
    }
}