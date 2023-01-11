/******************************************************************************
JobList.cs

begin		: November 19, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// This class translates the functionality provided by the JobList in 
    /// pyxlib to C#.  Detailed documentation for the JobList can be found 
    /// in joblist.h.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class JobList<T> : IEnumerable<T> where T : class
    {
        class InternalList<U> : DynamicList<U> where U : class
        {
            /// <summary>
            /// Removes the first item, and returns it.
            /// </summary>
            /// <returns>The (former) first item in the list.</returns>
            public U RemoveFirst()
            {
                U result = default(U);

                lock (m_locker)
                {
                    if (m_list.Count > 0)
                    {
                        result = m_list[0];
                        m_list.RemoveAt(0);
                    }
                }

                if (result != null)
                {
                    OnCountChanged();
                }

                return result;
            }
        }

        #region Member Variables

        /// <summary>
        /// The jobs that are to be processed.
        /// </summary>
        private InternalList<T> m_jobs = new InternalList<T>();

        /// <summary>
        /// The number of jobs that have been handed out.
        /// </summary>
        int m_jobsHandedOut = 0;

        /// <summary>
        /// The number of jobs to hand out before we stop handing out jobs.
        /// -1 will indicate no job limit.
        /// </summary>
        int m_jobsToDo = -1;

        /// <summary>
        /// Set to true if the last job has been handed out.
        /// </summary>
        bool m_isFinished = false;

        /// <summary>
        /// Used to wait until a job is available.
        /// </summary>
        private System.Threading.Semaphore m_semaphore =
            new System.Threading.Semaphore(0, 2000000000);

        #endregion Member Variables

        /// <summary>
        /// Initializes a new instance of the <see cref="JobList&lt;T&gt;"/> 
        /// class with no set end limit.
        /// </summary>
        public JobList()
        {
        }
        
        /// <summary>
        /// Initializes a new instance of the <see cref="JobList&lt;T&gt;"/> 
        /// class with a given maximum number of jobs to hand out.
        /// </summary>
        /// <param name="jobsToDo">The jobs to do.</param>
        public JobList(int jobsToDo)
        {
            m_jobsToDo = jobsToDo;
        }

        /// <summary>
        /// Adds the specified job.
        /// </summary>
        /// <param name="job">The job.</param>
        public void Add(T job)
        {
            m_jobs.Add(job);            
            m_semaphore.Release();
        }

        /// <summary>
        /// Gets a job from the list.
        /// </summary>
        /// <returns>T, or null, meaning there are no more jobs available
        /// and no more jobs will be coming.</returns>
        public T Get()
        {
            T job = null;            

            while (job == null)
            {
                m_semaphore.WaitOne();                

                if (m_isFinished)
                {
                    m_semaphore.Release();
                    return job;
                }

                job = m_jobs.RemoveFirst();                

                if (job != null)
                {
                    m_jobsHandedOut += 1;

                    if (m_jobsToDo != -1 && m_jobsHandedOut >= m_jobsToDo)
                    {
                        m_isFinished = true;
                        m_semaphore.Release();
                    }
                }
            }

            return job;
        }

        /// <summary>
        /// Clear all pending jobs.
        /// </summary>
        public void Clear()
        {
            m_jobs.Clear();
        }

        /// <summary>
        /// removes the given job.
        /// </summary>
        public void Remove(T job)
        {
            m_jobs.Remove(job);
        }

        /// <summary>
        /// Clear all pending jobs and set the finished flag so that 
	    /// threads can complete after they ask for the next job.
        /// </summary>
        public void Stop()
        {
            m_isFinished = true;
            Clear();

            m_semaphore.Release();
        }

        public IEnumerator<T> GetEnumerator()
        {
            return m_jobs.AsReadOnly().GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public int Count
        {
            get
            {
                return m_jobs.Count;
            }
        }
    }
}
