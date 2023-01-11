using System;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Pyxis.Storage
{
    /// <summary>
    /// Used to track the completion of a lengthy task.
    /// </summary>
    /// <typeparam name="T">The return type of the task</typeparam>
    public class ProgressTracker<T>
    {
        /// <summary>
        /// The current progress of the task in caller-defined units from 0 to Max
        /// </summary>
        private long m_current;

        private object m_lockObj = new object();
        /// <summary>
        /// Manages the task.
        /// </summary>
        private TaskCompletionSource<T> m_tcs = new TaskCompletionSource<T>();

        /// <summary>
        /// The current progress of the task from 0 to Max.
        /// </summary>
        public long Current
        {
            get
            {
                return m_current;
            }

            // Since this class will be called by multiple threads, it is probably an
            // error to set Current directly. Use the Report() method instead.
            private set
            {
                // clamp value to valid range
                if (value < 0)
                {
                    value = 0;
                }

                if (value > Max)
                {
                    value = Max;
                }

                m_current = value;

                // recalculate Percent complete and call the ProgressMade handler if it has changed
                var newPercentage = (int)(m_current * 100 / Max);
                if (Percent != newPercentage)
                {
                    Percent = newPercentage;
                    if (ProgressMade != null)
                    {
                        ProgressMade.Invoke(this);
                    }
                }
            }
        }

        /// <summary>
        /// Report to the tracker that some progress has been made on the task.
        /// </summary>
        /// <param name="progress">The progress in caller-defined units.</param>
        public void Report(long progress)
        {
            lock (m_lockObj)
            {
                Current += progress;
            }
        }

        /// <summary>
        /// The maximum value for the task. When Current reaches Max the task is considered completed.
        /// </summary>
        public long Max { get; private set; }

        /// <summary>
        /// The progress of the task in percentage completed from 0 to 100.
        /// </summary>
        public int Percent { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="max">The expected length of the task in caller-defined units</param>
        public ProgressTracker(long max)
        {
            m_current = 0;
            Max = max;
        }

        /// <summary>
        /// Called when the percentage complete of the task changes.
        /// </summary>
        /// <param name="tracker">The progress tracker</param>
        public delegate void ProgressMadeHandler(ProgressTracker<T> tracker);

        public event ProgressMadeHandler ProgressMade;

        /// <summary>
        /// Wait for the task to complete.
        /// </summary>
        public void Wait()
        {
            m_tcs.Task.Wait();
        }

        /// <summary>
        /// Get the task.
        /// </summary>
        public Task<T> Task
        {
            get { return m_tcs.Task; }
        }

        /// <summary>
        /// Create a progress tracker for a function that returns type T.
        /// </summary>
        /// <param name="max">The expected length of the task in caller-defined units.</param>
        /// <param name="function">The function to be executed.</param>
        /// <returns>The progress tracker.</returns>

        public static ProgressTracker<T> FromFunction(long max, Func<ProgressTracker<T>, T> function)
        {
            return FromFunctionWithDelayedStart(max,function,System.Threading.Tasks.Task.FromResult(""));
        }

        /// <summary>
        /// Create a progress tracker for a function that returns type T.
        /// The given function will be called after the given task is completed.
        /// </summary>
        /// <param name="max">The expected length of the task in caller-defined units.</param>
        /// <param name="function">The function to be executed.</param>
        /// <param name="after">A Task to wait upon in order to start</param>
        /// <returns>The progress tracker.</returns>
        public static ProgressTracker<T> FromFunctionWithDelayedStart(long max, Func<ProgressTracker<T>, T> function, Task after)
        {
            var progressTracker = new ProgressTracker<T>(max);
            after.ContinueWith((task) => function(progressTracker))
                .ContinueWith((x) =>
                {
                    if (x.IsFaulted)
                    {
                        Trace.TraceError("FromFunction: " + x.Exception.InnerException.Message + "\n" + x.Exception.InnerException.StackTrace);
                        progressTracker.m_tcs.TrySetException(x.Exception.InnerException);
                    }
                    progressTracker.m_tcs.TrySetResult(x.Result);
                });
            return progressTracker;
        }
    }
}