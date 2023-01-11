/******************************************************************************
Job.cs

begin		: February 9, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Threading;
using Pyxis.Contract.Operations;

namespace GeoStreamService.Jobs
{
    /// <summary>
    /// The base class for all GeoWeb Stream Server jobs.
    /// </summary>
    internal abstract class Job : Pyxis.Utilities.Job
    {
        protected CancellationTokenSource m_cancellationTokenSource;

        public Job()
        {
            m_cancellationTokenSource = new CancellationTokenSource();
        }

        public event EventHandler JobCompleted;

        public event EventHandler JobFailed;

        public event EventHandler JobStarted;

        /// <summary>
        /// Provided in XML form, saying what it has completed and what is currently doing.
        /// </summary>
        public string Progress
        {
            get
            {
                return Status.Description;
            }
            set
            {
                Status.Description = value as string;
            }
        }

        public ObservableOperationStatus Status { get; set; }

        protected CancellationToken CancellationToken
        {
            get
            {
                return m_cancellationTokenSource.Token;
            }
        }

        public void Cancel()
        {
            Status.StatusCode = OperationStatusCode.Cancelled;
            m_cancellationTokenSource.Cancel();
        }

        public override void Execute()
        {
            try
            {
                if (Status != null)
                {
                    Status.StartTime = DateTime.UtcNow;
                    Status.StatusCode = OperationStatusCode.Running;
                }

                CancellationToken.ThrowIfCancellationRequested();

                RaiseJobStarted();

                base.Execute();
            }
            catch (Exception ex)
            {
                Exception = ex;
            }
            finally
            {
                if (Status != null)
                {
                    Status.EndTime = DateTime.UtcNow;
                    if (Exception is System.Threading.Tasks.TaskCanceledException)
                    {
                        Status.StatusCode = OperationStatusCode.Cancelled;
                    }
                    else if (Exception != null)
                    {
                        Status.StatusCode = OperationStatusCode.Failed;
                        RaiseJobFailed();
                        GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, Exception.Message);
                    }
                    else
                    {
                        Finished.Value = true;
                        Status.StatusCode = OperationStatusCode.Completed;
                        RaiseJobCompleted();
                    }
                }
            }
        }

        private void RaiseJobCompleted()
        {
            if (Status != null)
            {
                GeoStreamService.WriteLine("Job Completed: " + Status.Description + " at " + DateTime.Now + "\n");
            }
            Pyxis.Utilities.InvokeExtensions.SafeInvoke(JobCompleted, this);
        }

        private void RaiseJobFailed()
        {
            if (Status != null)
            {
                GeoStreamService.WriteLine("Job Failed: " + Status.Description + " at " + DateTime.Now + "\n--> Error: " + Exception.Message + "\n" + Exception.StackTrace);
            }
            Pyxis.Utilities.InvokeExtensions.SafeInvoke(JobFailed, this);
        }

        private void RaiseJobStarted()
        {
            if (Status != null)
            {
                GeoStreamService.WriteLine("Starting Job: " + Status.Description + " at " + DateTime.Now);
            }
            Pyxis.Utilities.InvokeExtensions.SafeInvoke(JobStarted, this);
        }
    }

    internal abstract class JobOnProcess : Job
    {
        public ProcRef ProcRef;
        public IProcess_SPtr Process;
    }
}