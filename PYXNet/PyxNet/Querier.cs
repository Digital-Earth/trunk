/******************************************************************************
Querier.cs

begin      : April 5, 2007
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;

namespace PyxNet
{
    /// <summary>
    /// An agent class that can perform a query over PyxNet.  It knows how and whom to 
    /// contact and how to continue getting results.  The simple usage is to constuct a 
    /// Querier and then call GetResults() to get a result set back from the query.
    /// The more advanced usage is to hook into the OnMoreResults event before calling
    /// GetResults(), and then the query will be left running after the initial result set
    /// is returned, and as more results come in for this query, the OnMoreResults event will
    /// fire.
    /// </summary>
    public class Querier : ProgressiveBroadcast
    {
        #region Fields and properties

        #region Query

        /// <summary>
        /// The query to be sent.
        /// </summary>
        private readonly Query m_query;

        /// <summary>
        /// The query to be sent.
        /// </summary>
        public Query Query
        {
            get
            {
                return m_query;
            }
        }

        #endregion /* Query */

        #region Timeout
        private TimeSpan m_queryTimeout = TimeSpan.Zero;

        /// <summary>
        /// Gets or sets the query timeout.  If no query results are returned 
        /// before this time elapses, then QueryExpired will be raised.
        /// </summary>
        /// <value>The query timeout.</value>
        public TimeSpan QueryTimeout
        {
            get { return m_queryTimeout; }
            set { m_queryTimeout = value; }
        }

        /// <summary>
        /// Holds onto our timer if we have a query timeout.
        /// </summary>
        private System.Timers.Timer m_queryTimeoutTimer = null;
        #endregion Timeout

        #endregion /* Fields and properties */

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="Querier"/> class.
        /// </summary>
        /// <param name="stack">The stack to act upon.</param>
        /// <param name="query">The query.</param>
        /// <param name="timeOut">The time to wait for an acknowledgement, in milliseconds.</param>
        public Querier(Stack stack, Query query, int timeOut)
            :
            base(stack, timeOut)
        {
            if (null == stack)
            {
                throw new System.ArgumentNullException("stack");
            }
            m_query = query;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="stack">The stack to act upon.</param>
        /// <param name="contents">The string contents of the query.</param>
        /// <param name="timeOut">The time to wait for an acknowledgement, in milliseconds.</param>
        public Querier(Stack stack, string contents, int timeOut) :
            this(stack, new Query(stack.NodeInfo, contents), timeOut)
        {
        }

        #endregion

        #region Methods

        /// <summary>
        /// Send to the connection.
        /// </summary>
        /// <param name="connection">The connection to send to.  If null, send to the stack.</param>
        /// <returns>True if successful.</returns>
        protected override bool Send(StackConnection connection)
        {
            if (null == connection)
            {
                // Pass the query to the stack, and handle the acknowledgement.
                QueryAcknowledgement acknowledgement = Stack.ProcessQuery(m_query);
                HandleAcknowledgement(null, acknowledgement);
                return true;
            }

            Logging.Categories.Query.Log("Sending.Query(" + m_query.Guid + ").RemoteNode", connection.RemoteNodeInfo.ToString());
            // Send to the connection.
            return connection.SendMessage(m_query.ToMessage());
        }

        /// <summary>
        /// Determines whether or not this sender can handle the acknowledgement.
        /// </summary>
        /// <param name="acknowledgement">The acknowledgement to handle.</param>
        /// <returns>True if this sender can handle the acknowledgement.</returns>
        protected override bool CanHandleAcknowledgement(ProgressiveBroadcastAcknowledgement acknowledgement)
        {
            return CanHandleAcknowledgement(acknowledgement as QueryAcknowledgement);
        }

        /// <summary>
        /// Determines whether or not this sender can handle the acknowledgement.
        /// </summary>
        /// <param name="acknowledgement">The acknowledgement to handle.</param>
        /// <returns>True if this sender can handle the acknowledgement.</returns>
        private bool CanHandleAcknowledgement(QueryAcknowledgement acknowledgement)
        {
            return (null != acknowledgement) && (m_query.Guid.Equals(acknowledgement.QueryGuid));
        }

        /// <summary>
        /// Connect the handlers to the stack events.
        /// </summary>
        protected override void ConnectHandlers()
        {
            lock (Stack)
            {
                if (!AreHandlersConnected)
                {
                    // Handle stack's query acknowledgement event.
                    Stack.OnQueryAcknowledgement += HandleAcknowledgement;

                    base.ConnectHandlers();
                }
            }
        }

        /// <summary>
        /// Disconnect the handlers from the stack events.
        /// </summary>
        protected override void DisconnectHandlers()
        {
            lock (Stack)
            {
                // Unhook handler from stack's query acknowledgement event.
                Stack.OnQueryAcknowledgement -= HandleAcknowledgement;

                base.DisconnectHandlers();
            }
        }

        /// <summary>
        /// Start a broadcast.
        /// </summary>
        public override void Start()
        {
            if (QueryTimeout != TimeSpan.Zero)
            {
                m_queryTimeoutTimer = new System.Timers.Timer(QueryTimeout.TotalMilliseconds);
                m_queryTimeoutTimer.AutoReset = false;
                m_queryTimeoutTimer.Elapsed +=
                    delegate(object o, System.Timers.ElapsedEventArgs args)
                    {
                        OnQueryExpired();
                    };
                m_queryTimeoutTimer.Enabled = true;
            }
            Logging.Categories.Query.Log("Starting.Query(" + m_query.Guid + ").Contents", m_query.Contents.Substring(0,Math.Min(m_query.Contents.Length,500)));
            base.Start();
        }
        #endregion

        #region Handlers

        /// <summary>
        /// This handler is used to watch for query results coming in from PyxNet
        /// that are relevant to this query, and then to fire the OnResult event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleResult(object sender, Stack.QueryResultEventArgs args)
        {
            QueryResult result = args.Result;
            if (result.QueryGuid.Equals(Query.Guid))
            {
                System.Timers.Timer timer = m_queryTimeoutTimer;
                if (timer != null)
                {
                    timer.Enabled = false;
                }
                Stack stack = sender as Stack;
                stack.Tracer.WriteLine("Query result received.  Handling.");
                OnResult(result);
            }
        }

        #endregion /* Handlers */

        #region Events

        #region Result Event

        /// <summary>
        /// Class which wraps a QueryResult object, and will be passed as the second argument to a OnResultHandler.
        /// </summary>
        public class ResultEventArgs : EventArgs
        {
            private QueryResult m_queryResult;

            public QueryResult QueryResult
            {
                get { return m_queryResult; }
                set { m_queryResult = value; }
            }

            public ResultEventArgs(QueryResult queryResult)
            {
                m_queryResult = queryResult;
            }
        }

        public delegate void ResultHandler(object sender, ResultEventArgs args);

        /// <summary>
        /// Storage for the event handler.  Note that we don't use an EventHelper<> 
        /// here, because we add some extra wiring in the add/remove.
        /// </summary>
        private event ResultHandler m_onResult;
        private object m_resultEventLock = new object();

        /// <summary>
        /// Event which is fired when a result comes in.
        /// </summary>
        public event ResultHandler Result
        {
            add
            {
                lock (m_resultEventLock)
                {
                    if (m_onResult == null)
                    {
                        Stack.OnQueryResult += HandleResult;
                    }
                    m_onResult += value;
                }
            }
            remove
            {
                lock (m_resultEventLock)
                {
                    m_onResult -= value;
                    if (m_onResult == null)
                    {
                        Stack.OnQueryResult -= HandleResult;
                    }
                }
            }
        }

        /// <summary>
        /// Method to safely raise event OnResult.
        /// </summary>
        protected void OnResult(QueryResult qr)
        {
            try
            {
                ResultHandler onResult = m_onResult;
                if (null != onResult)
                {
                    onResult(this, new ResultEventArgs(qr));
                }
            }
            catch (Exception ex)
            {
                Stack.Tracer.ForcedWriteLine(
                    "Ignoring exception in Querier.OnResult: {0}", ex.ToString());
            }
        }

        #endregion Result Event

        #region QueryExpired Event

        /// <summary> EventArgs for a QueryExpired event. </summary>    
        public class QueryExpiredEventArgs : EventArgs
        {
            private Querier m_Querier;

            /// <summary>The Querier.</summary>
            public Querier Querier
            {
                get { return m_Querier; }
                set { m_Querier = value; }
            }

            internal QueryExpiredEventArgs(Querier theQuerier)
            {
                m_Querier = theQuerier;
            }
        }

        /// <summary> Event handler for QueryExpired. </summary>
        public event EventHandler<QueryExpiredEventArgs> QueryExpired
        {
            add
            {
                m_QueryExpired.Add(value);
            }
            remove
            {
                m_QueryExpired.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<QueryExpiredEventArgs> m_QueryExpired = new Pyxis.Utilities.EventHelper<QueryExpiredEventArgs>();

        /// <summary>
        /// Raises the QueryExpired event.
        /// </summary>
        public void OnQueryExpired()
        {
            m_QueryExpired.Invoke( this, new QueryExpiredEventArgs(this));
        }

        #endregion QueryExpired Event
	
        #endregion
    }
}