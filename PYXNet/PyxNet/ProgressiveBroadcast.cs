/******************************************************************************
ProgressiveBroadcast.cs

begin      : April 16, 2007
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// A progressive broadcast sends a message to candidate hubs, one by one, until the broadcast is stopped.
    /// Each hub that receives a message sends a ProgressiveBroadcastAcknowledgement back, 
    /// containing information about ther results and additional candidates for the broadcast.
    /// </summary>
    public abstract class ProgressiveBroadcast
    {
        #region Fields and properties

        #region Tracer

        /// <summary>
        /// This is the trace tool that one should use for all things to do with this object.
        /// </summary>
        private Pyxis.Utilities.NumberedTraceTool<ProgressiveBroadcast> m_tracer 
            = new Pyxis.Utilities.NumberedTraceTool<ProgressiveBroadcast>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        public Pyxis.Utilities.NumberedTraceTool<ProgressiveBroadcast> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        #endregion

        #region Stack

        /// <summary>
        /// The stack that will process the broadcast.
        /// </summary>
        private readonly Stack m_stack;

        /// <summary>
        /// The stack that will process the broadcast.
        /// </summary>
        public Stack Stack
        {
            get
            {
                return m_stack;
            }
        }

        #endregion

        #region Visited and candidate hubs

        // TODO: Why is this a Guid and not a NodeInfo???
        /// <summary>
        /// A list of hub guids that this broadcast has visited.
        /// </summary>
        private readonly List<Guid> m_visitedHubs = new List<Guid>();

        /// <summary>
        /// A list of hub NodeInfo objects that represent candidates for broadcast.
        /// These have not yet been hit.
        /// </summary>
        private readonly List<NodeInfo> m_candidateHubs = new List<NodeInfo>();

        /// <summary>
        /// A list of hubs to which we were unable to connect.   
        /// </summary>
        private readonly List<NodeInfo> m_unreachableHubs = new List<NodeInfo>();

        /// <summary>
        /// The number of candidate demotions that have occurred.
        /// </summary>
        private int m_candidateDemotionCount = 0;

        #endregion

        #region Temporary Connections

        /// <summary>
        /// A list of temporary connections.
        /// </summary>
        private readonly IList<StackConnection> m_temporaryConnections =
            new Pyxis.Utilities.DynamicList<StackConnection>();

        #endregion

        #region Timer

        /// <summary>
        /// The timer that triggers sending to the next candidate.
        /// </summary>
        private System.Timers.Timer m_timer = null;

        /// <summary>
        /// The time to wait for an acknowledgement (milliseconds)
        /// before the timer elapses and the next candidate is tried.
        /// </summary>
        private readonly int m_timeOut;

        /// <summary>
        /// A lock for operations on the timer.
        /// </summary>
        private object m_lockTimer = new object();

        #endregion

        #region Are Handlers Connected

        /// <summary>
        /// This is true if the stack's relevant event handlers are connected.
        /// </summary>
        private bool m_areHandlersConnected = false;

        /// <summary>
        /// This is true if the stack's relevant event handlers are connected.
        /// </summary>
        protected bool AreHandlersConnected
        {
            get
            {
                return m_areHandlersConnected;
            }
        }

        #endregion

        #endregion

        #region Constructor

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="stack">The stack to act upon.</param>
        /// <param name="timeOut">The time to wait for an acknowledgement, in milliseconds.</param>
        protected ProgressiveBroadcast(Stack stack, int timeOut)
        {
            if (null == stack)
            {
                throw new System.ArgumentNullException("stack");
            }

            m_stack = stack;
            m_timeOut = timeOut;
        }

        #endregion

        #region Methods

        private bool m_started = false;

        public bool Started
        {
            get { return m_started; }
            set
            {
                if (value)
                {
                    this.Start();
                }
                else
                {
                    this.Stop();
                }
            }
        }

        /// <summary>
        /// Start a broadcast.
        /// </summary>
        public virtual void Start()
        {
            m_started = true;
            Tracer.DebugWriteLine("Starting.");

            // TODO: evaluate support for calling Start() more than once.
            lock (m_candidateHubs)
            {
                // Add StackConnection hubs to candidate list.
                m_stack.GetHubs(m_candidateHubs);
            }

            // Diagnostic helper.
            if (m_candidateHubs.Count == 0)
            {
                System.Diagnostics.Trace.WriteLine( 
                    "Not connected to any hubs.  PyxNet operation may not succeed.");
            }

            // Connect to stack.
            ConnectHandlers();

            // Send to the stack.
            Send(null);
        }

        /// <summary>
        /// Stop the broadcast.
        /// </summary>
        public void Stop()
        {
            Tracer.DebugWriteLine("Stopping (if it's not already stopped).");

            OnStopped(StoppedEventArgs.Empty);

            m_started = false;

            // Stop timer.
            StopTimer();

            // Disconnect from stack.
            DisconnectHandlers();

            // Clear the lists.
            // (Why do we do this?)
            lock (m_candidateHubs)
            {
                m_candidateHubs.Clear();
                m_visitedHubs.Clear();
                m_unreachableHubs.Clear();
            }
            m_temporaryConnections.Clear();

            // Reset the candidate demotion count.
            m_candidateDemotionCount = 0;

            Tracer.DebugWriteLine("Definitely stopped.");
        }

        /// <summary>
        /// Connect the handlers to the stack events.
        /// </summary>
        protected virtual void ConnectHandlers()
        {
            lock (m_stack)
            {
                if (!m_areHandlersConnected)
                {
                    // Handle stack's closed connection event.
                    m_stack.OnClosedConnection += HandleClosedStackConnection;

                    m_areHandlersConnected = true;
                }
            }
        }

        /// <summary>
        /// Disconnect the handlers from the stack events.
        /// </summary>
        protected virtual void DisconnectHandlers()
        {
            lock (m_stack)
            {
                // Unhook from stack's closed connection event.
                m_stack.OnClosedConnection -= HandleClosedStackConnection;

                m_areHandlersConnected = false;
            }
        }

        /// <summary>
        /// Send to the connection.
        /// </summary>
        /// <param name="connection">The connection to send to.  If null, send to the stack.</param>
        /// <returns>True if successful.</returns>
        protected abstract bool Send(StackConnection connection);

        /// <summary>
        /// Send to the next viable hub.
        /// Assumes that the candidate list is ordered from best to worst choice,
        /// and reorders it as such.
        /// </summary>
        private bool SendToNextCandidateHub(bool timed)
	    {
            bool sent = false;

            Tracer.DebugWriteLine("About to send to a candidate, if any.");

            // Send to the first candidate in the list.
            // If this is a no-go, move it to the end and try sending to the new first one.
            for (; ; )
            {
                if (!AreHandlersConnected)
                {
                    // TODO: Consider adding a call to Stop here?
                    return sent;
                }

                // Get the first candidate, and the candidate count.
                NodeInfo candidate = null;
                int candidateCount = 0;
                lock (m_candidateHubs)
                {
                    candidateCount = m_candidateHubs.Count;
                    foreach (NodeInfo candidateHub in m_candidateHubs)
                    {
                        if (!m_unreachableHubs.Contains(candidateHub))
                        {
                            candidate = candidateHub;
                            break;
                        }
                    }
                }
                if (null == candidate)
                {
                    // There aren't any candidates left.
                    return false;
                }
                System.Diagnostics.Debug.Assert(0 < candidateCount, 
                    "The candidate count is 0, but somehow a candidate was found.  The locking must be faulty.");

                Tracer.DebugWriteLine("Considering sending to candidate '{0}'.", candidate.FriendlyName);

                // Move the candidate to the back of the list.
                if (!DemoteCandidateHub(candidate))
                {
                    Tracer.DebugWriteLine(
                        "Could not send to candidate '{0}' because the stack is disconnected.",
                        candidate.FriendlyName);

                    // Abort.
                    System.Diagnostics.Debug.Assert(!sent, 
                        "At this point in the code, 'sent' shouldn't be true.");
                    return false;
                }

                // Get the next connection.
                // If this is the first time through the candidate list, only use persistent connections.
                // After that, use whatever we can get.
                // We can use the demotion count to determine this, because we demote a candidate upon visiting it
                // (in the "DemoteCandidateHub" call above).
                StackConnection connection = null;
                if (m_candidateDemotionCount <= candidateCount)
                {
                    connection = m_stack.FindConnection(candidate, true);
                }
                else
                {
					connection = m_stack.GetConnection(candidate, false, TimeSpan.Zero);
                    if (connection == null)
                    {
                        SetHubAsBad(candidate);
                        m_unreachableHubs.Add(candidate);
                    }
                }
                if (null != connection)
                {
                    // Send to the connection, and start the timer.
                    if (Send(connection))
                    {
                        Tracer.DebugWriteLine(
                            "Sent from '{0}' to candidate hub '{1}'.",
                            m_stack.NodeInfo.FriendlyName, candidate.FriendlyName);

                        sent = true;
                        break;
                    }
                }

                // Didn't connect.
                Tracer.DebugWriteLine(
                    "Didn't connect '{0}' to candidate hub '{1}'.",
                    m_stack.NodeInfo.FriendlyName, candidate.FriendlyName);
            }

            if (timed)
            {
                // Wait until attempting to send again.
                // If there are no candidates now, there might be by then.
                StartTimer();
            }

            return sent;
	    }

        /// <summary>
        /// Add a candidate hub to the list.
        /// </summary>
        /// <param name="candidate">The candidate to add.</param>
        /// <returns>True if the stack was connected, allowing the candidate to be added.</returns>
        private bool AddCandidateHub(NodeInfo candidate)
        {
            // If the stack event handlers are still connected, add the candidate.
            if (m_areHandlersConnected)
            {
                lock (m_candidateHubs)
                {
                    Tracer.DebugWriteLine("Adding '{0}' to candidates list.",
                        candidate.FriendlyName);

                    m_candidateHubs.Add(candidate);
                }
                return true;
            }
            return false;
        }

        /// <summary>
        /// Move the candidate to the bottom of the candidate hub list.
        /// </summary>
        /// <param name="candidate">The candidate to demote.</param>
        /// <returns>True if the stack was connected, allowing the candidate to be re-added to the end.</returns>
        private bool DemoteCandidateHub(NodeInfo candidate)
        {
            lock (m_candidateHubs)
            {
                // Move the candidate to the back of the list, to try it again later,
                // and continue.
                if (m_candidateHubs.Remove(candidate))
                {
                    ++m_candidateDemotionCount;
                    return AddCandidateHub(candidate);
                }
            }
            return false;
        }

        /// <summary>
        /// Start the timer.
        /// </summary>
        private void StartTimer()
        {
            if (m_areHandlersConnected)
            {
                lock (m_lockTimer)
                {
                    StopTimer();

                    m_timer = new System.Timers.Timer();
                    m_timer.Enabled = false;
                    m_timer.Interval = m_timeOut;
                    m_timer.AutoReset = false;
                    m_timer.Elapsed += HandleTimerElapsed;
                    m_timer.Start();
                }
            }
        }

        /// <summary>
        /// Stop the timer.
        /// </summary>
        private void StopTimer()
        {
            lock (m_lockTimer)
            {
                if (null != m_timer)
                {
                    m_timer.Elapsed -= HandleTimerElapsed;
                    m_timer.Stop();
                    m_timer.Interval = m_timeOut;
                    m_timer.Dispose();
                    m_timer = null;
                }
            }
        }

        #endregion

        #region Events

        #region Stopped Event
        public class StoppedEventArgs : EventArgs
        {
            public static readonly new StoppedEventArgs Empty = new StoppedEventArgs();
        }

        /// <summary>
        /// This event is fired whenever the broadcast operation is stopped.
        /// </summary>
        public event EventHandler<StoppedEventArgs> Stopped
        {
            add
            {
                m_Stopped.Add(value);
            }
            remove
            {
                m_Stopped.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<StoppedEventArgs> m_Stopped = new Pyxis.Utilities.EventHelper<StoppedEventArgs>();

        /// <summary>
        /// Raises the <see cref="E:Stopped"/> event.
        /// </summary>
        /// <param name="e">The <see cref="PyxNet.Querier.StoppedEventArgs"/> instance containing the event data.</param>
        protected virtual void OnStopped(StoppedEventArgs e)
        {
            m_Stopped.Invoke(this, e);
        }
        #endregion Stopped Event

        #endregion Events

        #region Handlers

        /// <summary>
        /// Handle the timer elapsed event.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The elapsed event arguments.</param>
        void HandleTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            Tracer.DebugWriteLine("Timer elapsed; sending to next connected hub.");

            // Didn't get acknowledgement back in time.  
            // Forget about this hub, and send to the next one.
            SendToNextCandidateHub(true);
        }

        /// <summary>
        /// Determines whether or not this sender can handle the acknowledgement.
        /// </summary>
        /// <param name="acknowledgement">The acknowledgement to handle.</param>
        /// <returns>True if this sender can handle the acknowledgement.</returns>
        protected abstract bool CanHandleAcknowledgement(ProgressiveBroadcastAcknowledgement acknowledgement);

        /// <summary>
        /// Handle an acknowledgement event coming from one of the connections in the stack.
        /// </summary>
        /// <param name="connectionIndex">The index of the stack connection.</param>
        /// <param name="acknowledgement">The acknowledgement.</param>
        protected virtual void HandleAcknowledgement(StackConnection connection, ProgressiveBroadcastAcknowledgement acknowledgement)
        {
            if (null != acknowledgement)
            {
                if (!CanHandleAcknowledgement(acknowledgement))
                {
                    Tracer.DebugWriteLine("Could not handle acknowledgement in '{0}' from '{1}'.",
                        m_stack.NodeInfo.FriendlyName, connection.RemoteNodeInfo.FriendlyName);
                    return;
                }

                // Kill the timer.
                StopTimer();

                Tracer.DebugWriteLine("Handling acknowledgement in '{0}'.",
                    m_stack.NodeInfo.FriendlyName);

                // If target not hit, and this was our temporary connection to a candidate,
                // kill the connection.
                if (acknowledgement.IsDeadEnd)
                {
                    Tracer.DebugWriteLine("Dead end reached.");

                    if (null != connection)
                    {
                        m_temporaryConnections.Remove(connection);
                    }
                }

                if (!AreHandlersConnected)
                {
                    return;
                }

                Tracer.DebugWriteLine(
                    "About to update from acknowledgement from {0}.",
                    (null == connection ? "contained stack" : connection.ToString()));

                lock (m_candidateHubs)
                {
                    // Add each in acknowledgement.CandidateHubs to m_candidateHubs.
                    acknowledgement.CandidateHubs.ForEach(delegate(NodeInfo candidateHub)
                    {
                        if (!m_visitedHubs.Contains(candidateHub.NodeGUID) &&
                            !m_candidateHubs.Contains(candidateHub) &&
                            !IsBadHub(candidateHub) )
                        {
                            AddCandidateHub(candidateHub);
                        }
                    });

                    // Add each in acknowledgement.VisitedHubs to m_visitedHubs,
                    // and remove each from m_candidateHubs.
                    acknowledgement.VisitedHubs.ForEach(delegate(NodeInfo visitedHub)
                    {
                        if (!m_visitedHubs.Contains(visitedHub.NodeGUID))
                        {
                            Tracer.DebugWriteLine(
                                "Adding '{0}' to visited list.",
                                visitedHub.FriendlyName);

                            m_visitedHubs.Add(visitedHub.NodeGUID);
                        }

                        Tracer.DebugWriteLine(
                            "Removing '{0}' from the candidates list, if there.",
                            visitedHub.FriendlyName);

                        m_candidateHubs.Remove(visitedHub);
                    });
                }
            }

            // Send to the next hub.
            SendToNextCandidateHub(true);
        }

        private static object s_notConnectedHubsLockObject = new object();
        private static Dictionary<NodeInfo, DateTime> s_notConnectedHubs = new Dictionary<NodeInfo, DateTime>();

        private bool IsBadHub(NodeInfo candidateHub)
        {
            lock (s_notConnectedHubsLockObject)
            {                
                if (!s_notConnectedHubs.ContainsKey(candidateHub))
                {
                    //hub not in list
                    return false;
                }

                if ((DateTime.Now - s_notConnectedHubs[candidateHub]) < Properties.Settings.Default.RetryConnectingHubsTimeout)
                {
                    //failed to connect to hub is still fresh
                    return true;
                }

                //try to reconnect after 5 min
                s_notConnectedHubs.Remove(candidateHub);
                return false;
            }
        }

        private void SetHubAsBad(NodeInfo hub)
        {
            lock (s_notConnectedHubsLockObject)
            {
                s_notConnectedHubs[hub] = DateTime.Now;
            }
        }

        /// <summary>
        /// Handle a closed stack connection.
        /// </summary>
        /// <param name="stack">The stack with the closed connection.</param>
        /// <param name="connection">The connection that has been closed.</param>
        void HandleClosedStackConnection(Stack stack, StackConnection connection)
        {
            m_temporaryConnections.Remove(connection);
        }

        #endregion
    }
}
