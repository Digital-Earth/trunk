/******************************************************************************
JobManager.cs

begin		: January 13, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Utilities;
using System;
using System.Diagnostics;
using System.Linq;
using System.Threading;

namespace GeoStreamService.Jobs
{
    internal interface IJobManager
    {
        Pyxis.Utilities.SynchronizationEvent Finished { get; }

        void Pause();

        void Resume();

        void Stop();
    }

    /// <summary>
    /// Manages a <see cref="Pyxis.Utilities.JobList"/> for the GeoWeb Stream Server,
    /// where jobs added to the job list are executed on a separate, foreground thread.
    /// The thread is foreground so that jobs can terminate gracefully.
    /// </summary>
    internal class JobManager<T> : IJobManager where T : Job
    {
        private Object m_lockObj = new Object();
        private T m_currentJob;
        private DeadManTimer m_currentJobTimer;

        private Pyxis.Utilities.JobList<T> m_jobList =
            new Pyxis.Utilities.JobList<T>();

        private TimeSpan m_jobTimeOut = TimeSpan.FromMinutes(15);
        private ManualResetEvent m_resumeEvent = new ManualResetEvent(true);
        private System.Threading.Thread m_thread;
        private object m_isPausedLock = new object();
        private object m_pauseWaitLock = new object();

        /// <summary>
        /// When job managers are done, they set Finished to true.  When all job managers are done,
        /// signals the thread that is waiting on the job managers to finish.
        /// </summary>
        public Pyxis.Utilities.SynchronizationEvent Finished { get; private set; }

        public bool IsPaused { get; private set; }

        public string Name { get; private set; }

        public JobManager(string name)
        {
            m_thread = new System.Threading.Thread(DoWork);
            Name = name;
            m_thread.Name = Name;
            m_thread.IsBackground = false;
            m_thread.Start();

            Finished = new Pyxis.Utilities.SynchronizationEvent();
            ExitManager.AddObjectToStop(this);
        }

        public void Add(T job)
        {
            lock (m_lockObj)
            {
                if (!m_jobList.Contains(job) && m_currentJob != job)
                {
                    job.Status.Guid = System.Guid.NewGuid().ToString();
                    m_jobList.Add(job);
                }
            }
        }

        public void Cancel(IJobCancellationHint hint)
        {
            lock (m_lockObj)
            {
                if (hint.ShouldCancel(m_currentJob))
                {
                    m_currentJob.Cancel();
                }

                foreach (var job in m_jobList.ToList())
                {
                    if (hint.ShouldCancel(job))
                    {
                        job.Cancel();
                        m_jobList.Remove(job);
                    }
                }
            }
        }

        public void Pause()
        {
            lock (m_isPausedLock)
            {
                if (!IsPaused)
                {
                    IsPaused = true;
                    GeoStreamService.WriteLine(Name + "trying to paused at:");
                    var location = GetStackTrace(m_thread).ToString();
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, location);
                    //wait on this lock until the current job is finished
                    lock (m_pauseWaitLock)
                    {
                        m_resumeEvent.Reset();
                        GeoStreamService.WriteLine(Name + " paused");
                    }
                }
            }
        }

        public void Resume()
        {
            lock (m_isPausedLock)
            {
                if (IsPaused)
                {
                    m_resumeEvent.Set();
                    IsPaused = false;
                    GeoStreamService.WriteLine(Name + " resumed");
                }
            }
        }

        public void Stop()
        {
            m_jobList.Stop();
        }

        internal bool IsIdle()
        {
            if (Monitor.TryEnter(m_lockObj, TimeSpan.FromSeconds(2)))
            {
                try
                {
                    return m_currentJob == null && m_jobList.Count == 0;
                }
                finally
                {
                    Monitor.Exit(m_lockObj);
                }
            }
            return false;
        }

        private void DoWork()
        {
            while (!ExitManager.ShouldExit)
            {
                try
                {
                    T job = m_jobList.Get();

                    if (job != null)
                    {
                        lock (m_lockObj)
                        {
                            m_currentJob = job;
                        }
                        job.Status.StatusChanged += JobStatusChanged;
                        lock (m_pauseWaitLock)
                        {
                            m_resumeEvent.WaitOne();
                            m_currentJobTimer = new DeadManTimer(m_jobTimeOut, JobTimedOut);
                            job.Execute();
                        }
                    }
                }
                catch (TimeoutException)
                {
                    lock (m_lockObj)
                    {
                        m_jobList.Add(m_currentJob);
                    }
                }
                catch (Exception ex)
                {
                    Trace.error("Job Execution failure: " + ex.ToString());
                    GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, ex.Message);
                }
                finally
                {
                    lock (m_lockObj)
                    {
                        if (m_currentJobTimer != null)
                        {
                            m_currentJobTimer.Stop();
                            m_currentJobTimer = null;
                        }
                        if (m_currentJob != null)
                        {
                            m_currentJob.Status.StatusChanged -= JobStatusChanged;
                        }
                        m_currentJob = null;
                    }
                }
            }

            Finished.Pulse();
        }

        private void JobStatusChanged(object sender, EventArgs e)
        {
            m_currentJobTimer.KeepAlive();
        }

        private void JobTimedOut(object sender, System.Timers.ElapsedEventArgs e)
        {
            GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, "Following job took more than {1} minutes :\n{0}", m_currentJob.Progress, m_jobTimeOut.TotalMinutes);
            //Uncomment line below to see a stack trace when a job takes longer to finish
            //GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, GetStackTrace(m_thread).ToString());
        }

        private StackTrace GetStackTrace(Thread targetThread)
        {
            StackTrace stackTrace = null;
            var ready = new ManualResetEventSlim();

            new Thread(() =>
            {
                // Backstop to release thread in case of deadlock:
                ready.Set();
                Thread.Sleep(200);
                try { targetThread.Resume(); }
                catch { }
            }).Start();

            ready.Wait();
            targetThread.Suspend();
            try { stackTrace = new StackTrace(targetThread, true); }
            catch { /* Deadlock */ }
            finally
            {
                try { targetThread.Resume(); }
                catch { stackTrace = null;  /* Deadlock */  }
            }

            return stackTrace;
        }
    }
}