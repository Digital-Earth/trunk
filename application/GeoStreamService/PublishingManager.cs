/******************************************************************************
PublishingManager.cs

begin		: Auguest 20, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using GeoStreamService.Jobs;
using Microsoft.Practices.Unity;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.GeoWebStreamService;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Protocol;
using Pyxis.Publishing.Protocol.ContractObligations;
using Pyxis.Services.PipelineLibrary.Repositories;
using Pyxis.Utilities.Shell;
using PyxNet;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Timers;

namespace GeoStreamService
{
    internal class PublishingManager
    {
        private JobManager<CleanUpJob> m_cleanUpJobManager;
        private Timer m_cleanUpTimer;
        private UnityContainer m_container;

        private JobManager<DownloadJob> m_downloadJobManager;
        private JobManager<ImportJob> m_importJobManager;
        private JobsJournal m_jobsJournal;
        private System.Collections.Concurrent.ConcurrentDictionary<ProcRef, ProcessJobFactory> m_processJobFactory;
        private JobManager<ProcessJob> m_processJobManager;
        private JobManager<PublishJob> m_publishJobManager;
        private JobManager<ReportStatusJob> m_reportStatusJobManager;
        private Timer m_restartJobsTimer;
        private Timer m_sendReportTimer;

        private IEnumerable<IJobManager> jobManagers
        {
            get
            {
                yield return m_importJobManager;
                yield return m_downloadJobManager;
                yield return m_publishJobManager;
                yield return m_processJobManager;
                yield return m_cleanUpJobManager;
                yield return m_reportStatusJobManager;
            }
        }

        public PublishingManager(string licenseServerUrl)
        {
            InitializeDependencies(licenseServerUrl);

            m_sendReportTimer = new Timer(Properties.Settings.Default.SendReportInverval.TotalMilliseconds);
            m_sendReportTimer.Elapsed += delegate(object sender, ElapsedEventArgs e) { AddReportJob(); };
            m_sendReportTimer.Start();

            m_restartJobsTimer = new Timer(Properties.Settings.Default.RestartJobsInterval.TotalMilliseconds);
            m_restartJobsTimer.Elapsed += delegate(object sender, ElapsedEventArgs e) { RestartIncompleteJobs(); };
            m_restartJobsTimer.Start();

            m_cleanUpTimer = new Timer(Properties.Settings.Default.CleanUpInverval.TotalMilliseconds);
            m_cleanUpTimer.Elapsed += delegate(object sender, ElapsedEventArgs e) { AddCleanUpJob(); };
            m_cleanUpTimer.AutoReset = false;
        }

        [ShellAction(Name = "testgallery", Description = "Import and test all the pipelines in the gallery")]
        public void TestGallery(string testLevelString, int start)
        {
            GalleryTester.TestLevel testLevel;
            if (Enum.TryParse<GalleryTester.TestLevel>(testLevelString, out testLevel))
            {
                Pause();
                m_container.Resolve<GalleryTester>().TestGallery(testLevel, start);
            }
            else
            {
                GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, "Invalid gallery test argument : {0} use one of the followings {1}", testLevelString,
                    Enum.GetNames(typeof(GalleryTester.TestLevel)).Aggregate((x, y) => x + "," + y));
            }
        }

        [ShellAction(Name = "testpipeline", Description = "Import and test the pipeline of the given procRef")]
        public void TestPipeline(string profRefStr)
        {
            Pause();
            m_container.Resolve<GalleryTester>().TestPipeline(GalleryTester.TestLevel.Full, profRefStr);
        }

        public IGwssStatus CurrentStatus()
        {
            var status = new GwssStatus();

            status.NodeId = StackSingleton.Stack.NodeInfo.NodeGUID.ToString();

            status.Name = StackSingleton.Stack.NodeInfo.FriendlyName.ToString();

            status.ServerType = Properties.Settings.Default.ServerType;

            status.ServerStatus = ServerStatus.CurrentStatus();

            status.OperationsStatuses = m_jobsJournal.CurrentStatus;

            //Pipelines Status
            status.AddPipelineStatus(PipelineRepository.Instance.GetAllPublishedPipelines().Select(x => pyxlib.procRefToStr(x.ProcRef)).ToList(), PipelineStatusCode.Published);
            status.AddPipelineStatus(PipelineRepository.Instance.GetNotPublishedPipelines().Select(x => pyxlib.procRefToStr(x)).ToList(), PipelineStatusCode.Publishing);
            status.AddPipelineStatus(PipelineRepository.Instance.GetNotDownloadedPipelines().Select(x => pyxlib.procRefToStr(x)).ToList(), PipelineStatusCode.Downloading);

            return status;
        }

        [ShellAction(Name = "pause", Description = "Pause all the Job managers")]
        public void Pause()
        {
            var tasks = new List<Task>();
            foreach (var jobManager in jobManagers)
            {
                tasks.Add(Task.Factory.StartNew(() =>
                    {
                        jobManager.Pause();
                    }));
            }
            Task.WaitAll(tasks.ToArray());
        }

        [ShellAction(Name = "resume", Description = "Resume all the Job managers")]
        public void Resume()
        {
            var tasks = new List<Task>();
            foreach (var jobManager in jobManagers)
            {
                tasks.Add(Task.Factory.StartNew(() => jobManager.Resume()));
            }
            Task.WaitAll(tasks.ToArray());
        }

        public void Stop()
        {
            var tasks = new List<Task>();
            foreach (var jobManager in jobManagers)
            {
                tasks.Add(Task.Factory.StartNew(() => jobManager.Stop()));
            }
            Task.WaitAll(tasks.ToArray());
        }

        private void CancelJobs(ProcRef procRef)
        {
            var hint = new ProcRefJobCancellationHint(procRef);
            m_importJobManager.Cancel(hint);
            m_downloadJobManager.Cancel(hint);
            m_processJobManager.Cancel(hint);
            m_publishJobManager.Cancel(hint);
        }

        [ShellAction(Name = "p|publish", Description = "Publish the given procRef")]
        public void Publish(string procRefStr)
        {
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Import, "Requesting to import " + procRefStr);
            AddImportJobIfNeeded(pyxlib.strToProcRef(procRefStr));
        }

        [ShellAction(Name = "u|unpublish", Description = "Unpublish the given procRef")]
        public void Unpublish(string procRefStr)
        {
            var procRef = pyxlib.strToProcRef(procRefStr);

            CancelJobs(procRef);

            var pipeline = PipelineRepository.Instance.GetByProcRef(procRef);
            if (pipeline != null && pipeline.IsPublished)
            {
                PyxNet.Pyxis.PublisherSingleton.Publisher.Unpublish(procRef);
            }

            PipelineRepository.Instance.SetIsImported(procRef, false);
            PipelineRepository.Instance.TryRemovePipeline(procRef);
            PipelineRepository.Instance.CheckPoint();
        }

        [ShellAction(Name = "c|cleanup", Description = "Removes unnecessary downloaded files and cache")]
        public void AddCleanUpJob()
        {
            if (m_cleanUpJobManager.IsIdle() && m_importJobManager.IsIdle())
            {
                try
                {
                    Pause();
                    var cleanUpJob = m_container.Resolve<CleanUpJob>();
                    cleanUpJob.JobCompleted += (object sender, EventArgs e) => { Resume(); };
                    cleanUpJob.JobFailed += delegate(object sender, EventArgs e) { m_cleanUpTimer.Start(); Resume(); };
                    m_jobsJournal.Track(cleanUpJob);
                    m_cleanUpJobManager.Add(cleanUpJob);
                    m_cleanUpJobManager.Resume();
                    m_reportStatusJobManager.Resume();
                }
                catch
                {
                    Resume();
                }
            }
            else
            {
                m_cleanUpTimer.Start();
            }
        }

        private void AddReportJob()
        {
            if (GeoStreamService.ServerType == GeoStreamService.ServerTypes.Test)
            {
                return;
            }

            if (m_reportStatusJobManager.IsIdle())
            {
                var reportJob = m_container.Resolve<ReportStatusJob>(new ParameterOverride("getReport", new Func<IGwssStatus>(CurrentStatus)));
                reportJob.JobCompleted += OnReportCompleted;
                reportJob.ResponseReceived += ProcessResponse;
                m_reportStatusJobManager.Add(reportJob);
            }
        }

        private void AddImportJobIfNeeded(ProcRef procRef)
        {
            var pipeline = PipelineRepository.Instance.GetByProcRef(procRef);
            if (pipeline != null && pipeline.IsImported && !pipeline.IsTemporary)
            {
                GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Import, procRef.ToString() + "Is Already Imported");
                return;
            }

            var importJob = m_container.Resolve<ImportJob>(
                new ParameterOverrides
                    {
                        {"procRef", procRef},
                        {"pipelineClient", m_container.Resolve<IPipelineClient>()}
                    });
            importJob.JobCompleted += OnImportCompleted;
            m_jobsJournal.Track(importJob);
            m_importJobManager.Add(importJob);
        }

        private void AddDownloadJob(IProcess_SPtr process)
        {
            var downloadJob = m_container.Resolve<DownloadJob>(
                new ParameterOverrides
                    {
                        {"process", process},
                        { "name", process.getProcName()}
                    });
            downloadJob.JobCompleted += OnDownloadCompleted;
            m_jobsJournal.Track(downloadJob);

            m_downloadJobManager.Add(downloadJob);
        }

        private void AddPublishJob(IProcess_SPtr process)
        {
            var publishJob = m_container.Resolve<PublishJob>(
            new ParameterOverrides
                    {
                        {"process", process},
                        { "name", process.getProcName()}
                    });
            publishJob.JobCompleted += OnPublishCompleted;
            m_jobsJournal.Track(publishJob);
            m_publishJobManager.Add(publishJob);
        }

        private void AddProcessJob(IProcess_SPtr process)
        {
            if (GeoStreamService.ServerType == GeoStreamService.ServerTypes.Publisher)
            {
                return;
            }

            //checking for disk space
            long requiredDiskSpace = 10 * (long)1073741824; // 10 GB - should be replaced with an actual estimation of the process cache.
            var freeDiskSpace = new System.IO.DriveInfo(AppServices.getCacheDir("ProcessCache").Substring(0, 1)).AvailableFreeSpace;
            if (requiredDiskSpace > freeDiskSpace)
            {
                return;
            }

            var procRef = new ProcRef(process);
            if (!m_processJobFactory.ContainsKey(procRef))
            {
                int lastProcessedResolution = PipelineRepository.Instance.GetByProcRef(procRef).MaxProcessedResolution;
                m_processJobFactory[procRef] = ProcessJobFactory.CreateFactory(process, lastProcessedResolution);
            }

            var factory = m_processJobFactory[procRef];

            var processJob = factory.Emit(m_container);

            if (processJob == null)
            {
                m_processJobFactory.TryRemove(procRef, out factory);
                return;
            }

            processJob.JobCompleted += OnProcessCompleted;
            m_jobsJournal.Track(processJob);
            m_processJobManager.Add(processJob);
        }

        private void InitializeDependencies(string licenseServerUrl)
        {
            // initialize all jobs threads
            m_reportStatusJobManager = new JobManager<ReportStatusJob>("GWSS Report Status Job Manager");
            m_cleanUpJobManager = new JobManager<CleanUpJob>("GWSS Clean up Job Manager");
            m_importJobManager = new JobManager<ImportJob>("GWSS Import Job Manager");
            m_downloadJobManager = new JobManager<DownloadJob>("GWSS Download Job Manager");
            m_publishJobManager = new JobManager<PublishJob>("GWSS Publish Job Manager");
            m_processJobManager = new JobManager<ProcessJob>("GWSS Processing Job Manager");

            m_jobsJournal = new JobsJournal();

            m_processJobFactory = new System.Collections.Concurrent.ConcurrentDictionary<ProcRef, ProcessJobFactory>();

            // Initialize container
            m_container = new UnityContainer();
            m_container.RegisterInstance<JobManager<ImportJob>>(m_importJobManager);
            m_container.RegisterInstance<JobManager<DownloadJob>>(m_downloadJobManager);
            m_container.RegisterInstance<JobManager<ProcessJob>>(m_processJobManager);
            m_container.RegisterInstance<JobManager<PublishJob>>(m_publishJobManager);
            m_container.RegisterInstance<UnityContainer>(m_container);

            m_container.RegisterType<DownloadJob>();
            m_container.RegisterType<ImportJob>();
            m_container.RegisterType<PublishJob>();
            m_container.RegisterType<ProcessJob>();
            m_container.RegisterType<ReportStatusJob>();
            m_container.RegisterType<CleanUpJob>();

            m_container.RegisterType<GalleryTester>();

            m_container.RegisterType<IGwssClient, GwssClient>(new InjectionConstructor(licenseServerUrl));
            m_container.RegisterType<IPipelineClient, PipelineClient>(new InjectionConstructor(licenseServerUrl));
        }

        private void OnReportCompleted(object sender, EventArgs e)
        {
            var lastJob = sender as ReportStatusJob;

            if (lastJob.Response != null)
            {
                m_jobsJournal.RemoveCompletedJobsFromJurnal(lastJob.GwssReport.OperationsStatuses);
            }
            }

        private void OnImportCompleted(object sender, EventArgs e)
        {
            var lastJob = sender as ImportJob;
            AddDownloadJob(lastJob.Process);
            AddReportJob();
        }

        private void OnDownloadCompleted(object sender, EventArgs e)
        {
            var lastJob = sender as DownloadJob;
            AddPublishJob(lastJob.Process);
            AddReportJob();
        }

        private void OnPublishCompleted(object sender, EventArgs e)
        {
            var lastJob = sender as PublishJob;
            PyxNet.StackSingleton.Stack.ForceQueryHashTableUpdate();
            AddProcessJob(lastJob.Process);
            AddReportJob();
        }

        private void OnProcessCompleted(object sender, EventArgs e)
        {
            var lastJob = sender as ProcessJob;
            AddReportJob();
            PipelineRepository.Instance.SetProcessedResolution(lastJob.ProcRef, lastJob.ProcessingResolution);
            PipelineRepository.Instance.CheckPoint();
            RestartIncompleteJobs();
        }

        private object m_processLSResponceLock = new object();
        private void ProcessResponse(ILsStatus response)
        {
            lock (m_processLSResponceLock)
            {
                GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Report, "Processing LS response");
            foreach (var request in response.PipelinesRequests)
            {
                try
                {
                    if (request.OperationType == OperationType.Import || request.OperationType == OperationType.Publish)
                    {
                        Publish(request.Parameters["ProcRef"]);
                    }
                    else if (request.OperationType == OperationType.Remove)
                    {
                        Unpublish(request.Parameters["ProcRef"]);
                    }
                }
                catch (Exception e)
                {
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, "Following command failed " + request.OperationType + " on " + request.Parameters["ProcRef"] + ". " + e.Message + e.StackTrace);
                }
            }
        }
        }

        [ShellAction(Name = "restart", Description = "Restart Incomplete Jobs")]
        private void RestartIncompleteJobs()
        {
            try
            {
                m_restartJobsTimer.Enabled = false;
                var tasks = new List<Task>();
                //Process
                if (m_processJobManager.IsIdle())
                {
                    tasks.AddRange(PipelineRepository.Instance.GetNotProcessedPipelines().TakeRandom(5).Select(
                        procRef => Task.Factory.StartNew(() => { AddProcessJob(PipeManager.getProcess(procRef, false)); })
                            .ContinueWith((t) => { HandleBrokenProcess(t.Exception, procRef); }, TaskContinuationOptions.OnlyOnFaulted)));
                }

                //Publish
                if (m_publishJobManager.IsIdle())
                {
                    tasks.AddRange(PipelineRepository.Instance.GetNotPublishedPipelines().TakeRandom(5).Select(
                        procRef => Task.Factory.StartNew(() => { AddPublishJob(PipeManager.getProcess(procRef, false)); })
                            .ContinueWith((t) => { HandleBrokenProcess(t.Exception, procRef); }, TaskContinuationOptions.OnlyOnFaulted)));
                }

                //Download
                if (m_downloadJobManager.IsIdle())
                {
                    tasks.AddRange(PipelineRepository.Instance.GetNotDownloadedPipelines().TakeRandom(5).Select(
                        procRef => Task.Factory.StartNew(() => { AddDownloadJob(PipeManager.getProcess(procRef, false)); })
                            .ContinueWith((t) => { HandleBrokenProcess(t.Exception, procRef); }, TaskContinuationOptions.OnlyOnFaulted)));
                }

                if (!Task.WaitAll(tasks.ToArray(), TimeSpan.FromMinutes(10)))
                {
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, "Restarting incomplete jobs timed out!");
                }
            }
            catch (AggregateException ae)
            {
                foreach (var exception in ae.InnerExceptions.Where(e => !(e is TaskCanceledException)))
                {
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, exception.Message + "\n" + exception.StackTrace);
                }
            }
            finally
            {
                m_restartJobsTimer.Enabled = true;
            }
        }

        private void HandleBrokenProcess(Exception e, ProcRef procRef)
        {
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, "Removing ProcRef : " + procRef.ToString() + " due to " + e.Message + e.InnerException + e.StackTrace);
            Unpublish(procRef.ToString());
        }
    }

    static class IListExtension
    {
        // Returns a number of randomly chosen elements in the source. This method might repeat an element.
        public static IEnumerable<T> TakeRandom<T>(this IList<T> source, int count)
        {
            var rnd = new Random();
            int to = Math.Min(count, source.Count);
            for (int i = 0; i < to; i++)
            {
                yield return source.ElementAt(rnd.Next(source.Count()));
            }
        }
    }
}