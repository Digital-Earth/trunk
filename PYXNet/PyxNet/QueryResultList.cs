using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet
{
    public class QueryResultList : Pyxis.Utilities.DynamicList<QueryResult>
    {
        #region Fields and properties

        /// <summary>
        /// Used to do the leg work of the query.
        /// </summary>
        private Querier m_querier;

        /// <summary>
        /// the stack to use for communications.
        /// </summary>
        private Stack m_stack;

        /// <summary>
        /// What to search for.
        /// </summary>
        private string m_contents;
        
        private bool m_started = false;

        /// <summary>
        /// Used to determine if the list has been started (is in an active state).
        /// </summary>
        public bool Started
        {
            get { return m_started; }
        }

        /// <summary>
        /// Used to make sure that the start logic is only called once in a mutlithreaded
        /// application.
        /// </summary>
        private readonly object m_startLock = new object();

        #endregion

        #region Constructors

        /// <summary>
        /// Constructor.  After construction call the start method or WaitForResults to 
        /// go live with the query and start gathering results.
        /// </summary>
        /// <param name="stack">The stack to act upon.</param>
        /// <param name="contents">The string contents of the query.</param>
        public QueryResultList(Stack stack, string contents) 
        {
            if (null == stack)
            {
                throw new System.ArgumentNullException("stack");
            }

            if (null == contents || contents.Length == 0)
            {
                throw new System.ArgumentNullException("contents");
            }

            m_contents = contents;
            m_stack = stack;
        }

        #endregion

        #region Handlers

        /// <summary>
        /// Handler for hooking into the querier when new results arrive and adding them to the
        /// list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleResult(object sender, Querier.ResultEventArgs args)
        {
            this.Add(args.QueryResult);
        }

        #endregion

        #region Methods

        /// <summary>
        /// Activate the query and start gathering resulsts.
        /// </summary>
        public void Start()
        {
            // TODO:  what should we do with results we have already???
            lock (m_startLock)
            {
                if (!m_started)
                {
                    m_querier = new Querier(m_stack, m_contents, 2000);

                    // TODO: Start logic
                    m_querier.Result += HandleResult;
                    m_querier.Start();
                    m_started = true;
                }
            }
        }

        /// <summary>
        /// A blocking call that will wait for a minimum number of results to be available
        /// in the list but only for a given amount of time.  Will start the query if it needs it.
        /// </summary>
        /// <param name="minimumResultCount">The minimim number of results that need to be in the list.</param>
        /// <param name="timeOutSeconds">The maximum time to wait in seconds.</param>
        /// <returns>True if we got the minimum number of results.</returns>
        public bool WaitForResults(int minimumResultCount, int timeOutSeconds)
        {
            Start();
            DateTime startTime = DateTime.Now;
            while ((this.Count < minimumResultCount) && (DateTime.Now - startTime).TotalSeconds < timeOutSeconds)
            {
                // Hang out for a moment.
                System.Windows.Forms.Application.DoEvents();
                
                System.Threading.Thread.Sleep(200);
            }
            return (this.Count >= minimumResultCount);
        }

        /// <summary>
        /// Unhook the handlers and stop adding new results to the list.
        /// </summary>
        public void Stop()
        {
            lock (m_startLock)
            {
                if (m_started)
                {
                    // TODO: Stop logic
                    m_querier.Result -= HandleResult;
                    m_querier.Stop();
                    m_started = false;
                }
            }
        }

        #endregion
    }
}
