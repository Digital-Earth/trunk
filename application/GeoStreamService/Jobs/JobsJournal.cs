using System;
using System.Collections.Generic;
using System.Linq;
using Pyxis.Contract.Operations;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// Provides an Archive of all Jobs status that where completed as well a list of all active jobs.
    ///
    /// To use this class simply use Track(Job) so this class can track the progress of the job.
    /// </summary>
    internal class JobsJournal
    {
        private Object m_lockObject = new Object();

        private List<IOperationStatus> ActiveJobs { get; set; }

        private List<IOperationStatus> FinishedJob { get; set; }

        public JobsJournal()
        {
            ActiveJobs = new List<IOperationStatus>();
            FinishedJob = new List<IOperationStatus>();
        }

        public void Track(Job job)
        {
            job.JobStarted += HandleJobStarted;
            job.JobCompleted += HandleJobCompleted;
            job.JobFailed += HandleJobFailed;
        }

        private void HandleJobStarted(object sender, EventArgs e)
        {
            var job = (sender as Job);
            if (job != null && job.Status != null)
            {
                lock (m_lockObject)
                {
                    ActiveJobs.Add(job.Status);
                }
            }
        }

        private void HandleJobCompleted(object sender, EventArgs e)
        {
            MoveJobToFinished(sender as Job);
        }

        private void HandleJobFailed(object sender, EventArgs e)
        {
            MoveJobToFinished(sender as Job);
        }

        private void MoveJobToFinished(Job job)
        {
            if (job != null && job.Status != null)
            {
                lock (m_lockObject)
                {
                    ActiveJobs.Remove(job.Status);
                    FinishedJob.Add(job.Status);
                }
            }

            job.JobStarted -= HandleJobStarted;
            job.JobCompleted -= HandleJobCompleted;
            job.JobFailed -= HandleJobFailed;
        }

        public List<IOperationStatus> CurrentStatus
        {
            get
            {
                lock (m_lockObject)
                {
                    return ActiveJobs.Concat(FinishedJob).OrderBy(x => x.StartTime).ToList();
                }
            }
        }

        public void RemoveCompletedJobsFromJurnal(IEnumerable<IOperationStatus> jobs)
        {
            lock (m_lockObject)
            {
                foreach (var job in jobs)
                {
                    FinishedJob.Remove(job);
                }
            }
        }
    }
}