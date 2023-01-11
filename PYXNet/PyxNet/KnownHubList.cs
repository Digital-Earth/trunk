using System;
using System.Collections.Generic;
using Pyxis.Utilities;
using System.Text;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates the information that we send and recieve in a 
    /// Known Hub List message in PYXNet.  
    /// </summary>
    public class KnownHubList : ITransmissible
    {
        /// <summary>
        /// This is the trace tool that one should use for all things to do with propagating 
        /// Known Hub List messages and the handling of Known Hub List messages around the network.
        /// </summary>
        private Pyxis.Utilities.NumberedTraceTool<KnownHubList> m_tracer
            = new Pyxis.Utilities.NumberedTraceTool<KnownHubList>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        #region Properties

        /// <summary>
        /// The stack that the hubs are known to.
        /// If null, this represents a list of hubs for a remote stack.
        /// </summary>
        private readonly Stack m_stack = null;

        /// <summary>
        /// This lock ensures that the known and connected hub lists remain synchronized.
        /// </summary>
        private readonly object m_lockThis = new object();

        /// <summary>
        /// Storage for the list of hubs that are directly connected.
        /// </summary>
        private readonly DynamicList<NodeInfo> m_connectedHubs;

        /// <summary>
        /// This returns a read-only copy of the connected hub list.
        /// </summary>
        public IList<NodeInfo> ConnectedHubs
        {
            get
            {
                lock (m_lockThis)
                {
                    return m_connectedHubs.AsReadOnly();
                }
            }
        }

        /// <summary>
        /// Storage for the list of hubs that we know about that are not directly connected.
        /// </summary>
        private readonly DynamicList<NodeInfo> m_knownHubs;

        /// <summary>
        /// This returns a read-only copy of the list of
        /// hubs that we know about that are not directly connected.
        /// </summary>
        public IList<NodeInfo> KnownHubs
        {
            get
            {
                lock (m_lockThis)
                {
                    return m_knownHubs.AsReadOnly();
                }
            }
        }

        #endregion

        #region Conversion

        /// <summary>
        /// Constructs a string representing the hub list.
        /// </summary>
        /// <param name="hubs">The hub list.</param>
        /// <returns>A string representation of the hub list.</returns>
        private string ToString(IEnumerable<NodeInfo> hubs)
        {
            StringBuilder list = new StringBuilder();
            foreach (NodeInfo element in hubs)
            {
                if (list.Length != 0)
                {
                    list.Append(", ");
                }
                list.Append(element.ToString());
            }
            return list.ToString();
        }

        /// <summary>
        /// Display as a list of connected and known hubs.
        /// </summary>
        /// <returns>A list of hubs in clear text format.</returns>
        public override string ToString()
        {
            int connectedCount;
            string connectedString;
            int knownCount;
            string knownString;
            lock (m_lockThis)
            {
                connectedString = ToString(m_connectedHubs);
                connectedCount = m_connectedHubs.Count; // Count is O(1)
                knownString = ToString(m_knownHubs);
                knownCount = m_knownHubs.Count; // Count is O(1)
            }

            StringBuilder list = new StringBuilder(connectedCount.ToString());
            list.Append(" connected hubs");
            if (0 != connectedCount)
            {
                list.Append(" (");
                list.Append(connectedString);
                list.Append(")");
            }
            list.Append(", ");
            list.Append(knownCount.ToString());
            list.Append(" known hubs");
            if (0 != knownCount)
            {
                list.Append(" (");
                list.Append(knownString);
                list.Append(")");
            }

            return list.ToString();
        }

        #endregion

        #region Events

        #region Hub Connected Event

        /// <summary>
        /// The event fired when the first hub is added to the "connected" list.
        /// </summary>
        public event EventHandler<EventArgs> HubConnected
        {
            add
            {
                m_HubConnected.Add(value);
            }
            remove
            {
                m_HubConnected.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<EventArgs> m_HubConnected = new Pyxis.Utilities.EventHelper<EventArgs>();

        /// <summary>
        /// Raises the HubConnected event.
        /// </summary>
        /// <param name="sender"></param>
        private void OnHubConnected(object sender)
        {
            m_HubConnected.Invoke( sender, new EventArgs());
        }

        #endregion

        #region Hub Disconnected Event

        /// <summary>
        /// The event fired when the last hub is removed from the "connected" list.
        /// </summary>
        public event EventHandler<EventArgs> HubDisconnected
        {
            add
            {
                m_HubDisconnected.Add(value);
            }
            remove
            {
                m_HubDisconnected.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<EventArgs> m_HubDisconnected = new Pyxis.Utilities.EventHelper<EventArgs>();

        /// <summary>
        /// Raises the HubDisconnected event.
        /// </summary>
        /// <param name="sender"></param>
        private void OnHubDisconnected(object sender)
        {
            m_HubDisconnected.Invoke( sender, new EventArgs());
        }

        #endregion

        #region Event Handlers

        private void AddedElementHandler(object sender, DynamicList<NodeInfo>.ElementEventArgs e)
        {
            System.Diagnostics.Debug.Assert(e != null);

            Stack stack = m_stack;
            if (stack != null && 1 == e.ElementCount)
            {
                OnHubConnected(this);
            }
        }

        private void RemovedElementHandler(object sender, DynamicList<NodeInfo>.ElementEventArgs e)
        {
            System.Diagnostics.Debug.Assert(e != null);

            Stack stack = m_stack;
            if (stack != null && 0 == e.ElementCount)
            {
                OnHubDisconnected(this);
            }
        }

        /// <summary>
        /// Connects the event handlers for the lists.
        /// </summary>
        private void ConnectEventHandlers()
        {
            m_connectedHubs.AddedElement += AddedElementHandler;
            m_connectedHubs.RemovedElement += RemovedElementHandler;
        }

        #endregion

        #endregion

        #region Constructors

        /// <summary>
        /// Default constructor.
        /// </summary>
        private KnownHubList()
        {
            m_connectedHubs = new DynamicList<NodeInfo>();
            m_knownHubs = new DynamicList<NodeInfo>();

            ConnectEventHandlers();

            m_tracer.DebugWriteLine("Default construction: {0}", ToString());
        }

        /// <summary>
        /// Constructor that copies the elements from each list.
        /// </summary>
        /// <param name="connected">The connected hub list to copy.</param>
        /// <param name="known">The known hub list to copy.</param>
        public KnownHubList(IEnumerable<NodeInfo> connected, IEnumerable<NodeInfo> known)
        {
            m_connectedHubs = new DynamicList<NodeInfo>(connected);
            m_knownHubs = new DynamicList<NodeInfo>(known);

            ConnectEventHandlers();

            // Check that there are no duplicates.  Done after the copy to guarantee.
            foreach (NodeInfo element in m_connectedHubs)
            {
                if (m_knownHubs.Contains(element)) 
                {
                    throw new ArgumentException("The connected and known lists are mutually exclusive.");
                }
            }
            foreach (NodeInfo element in m_knownHubs)
            {
                if (m_connectedHubs.Contains(element))
                {
                    throw new ArgumentException("The connected and known lists are mutually exclusive.");
                }
            }

            m_tracer.DebugWriteLine("Construction from lists: {0}", ToString());
        }

        /// <summary>
        /// Constructs the known hub list for a stack.
        /// </summary>
        /// <param name="stack">The stack for which this is the known hub list.</param>
        public KnownHubList(Stack stack) : this()
        {
            m_stack = stack;

            m_tracer.DebugWriteLine("Construction from stack: {0}", ToString());
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public KnownHubList(Message message) : this()
        {
            FromMessage(message);

            m_tracer.DebugWriteLine("Construction from message: {0}", ToString()); 
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public KnownHubList(MessageReader reader) : this()
        {
            FromMessageReader(reader);

            m_tracer.DebugWriteLine("Construction from message reader: {0}", ToString());
        }

        #endregion

        #region Equality

        /// <summary>
        /// Check for equality of contents.
        /// </summary>
        /// <param name="rhs">The object to compare to.</param>
        /// <returns>True if the objects are equal.</returns>
        public override bool Equals(object rhs)
        {
            if (null == rhs)
            {
                return false;
            }

            KnownHubList khl = rhs as KnownHubList;
            if (null != khl)
            {
                return Equals(khl);
            }

            return false;
        }

        /// <summary>
        /// Check for equality of contents.
        /// </summary>
        /// <param name="khl">The object to compare to.</param>
        /// <returns>True if the objects are equal.</returns>
        public bool Equals(KnownHubList khl)
        {
            if (null == khl)
            {
                return false;
            }

            lock (m_lockThis)
            {
                if (m_connectedHubs.Count != khl.m_connectedHubs.Count ||
                    m_knownHubs.Count != khl.m_knownHubs.Count)
                {
                    return false;
                }

                for (int count = 0; count < m_connectedHubs.Count; ++count)
                {
                    if (m_connectedHubs[count].LeafCount != khl.m_connectedHubs[count].LeafCount ||
                        m_connectedHubs[count].HubCount != khl.m_connectedHubs[count].HubCount)
                    {
                        return false;
                    }
                }
                for (int count = 0; count < m_knownHubs.Count; ++count)
                {
                    if (m_knownHubs[count].Mode != khl.m_knownHubs[count].Mode ||
                        m_knownHubs[count].NodeGUID != khl.m_knownHubs[count].NodeGUID)
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        /// <summary>
        /// Need to override GetHashCode() when you override Equals.  
        /// </summary>
        /// <returns>The hash code corresponding to this known hub list.</returns>
        public override int GetHashCode()
        {
            throw new NotImplementedException("Not yet implemented.");
        }

        #endregion

        #region Operations

        /// <summary>
        /// Checks to see if a Node is in this list -- checks by the Guid of the node.
        /// </summary>
        /// <param name="value">The node you wish to find.</param>
        /// <returns>True if the node was in this list.</returns>
        public bool Contains(NodeInfo value)
        {
            lock (m_lockThis)
            {
                return (m_knownHubs.Contains(value) || m_connectedHubs.Contains(value));
            }
        }

        /// <summary>
        /// Thread safe method to add a Node to the list.  If the node is a leaf, then it 
        /// will not be added to the list.  Only hubs are added to the list.  IF the node 
        /// is in one of the lists already, we will adjust which list it is in to reflect
        /// its connected state.  We return false to indicate no changes 
        /// to the list.  If the Node is a hub and not in the list we determine if it is connected
        /// using the connections list passsed in, and then add it to the correct list based on 
        /// this test.
        /// </summary>
        /// <param name="value">The node to add.</param>
        /// <returns>True if either of the lists where changed.</returns>
        public bool Add(NodeInfo value)
        {
            if (null == value)
            {
                throw new ArgumentNullException("value");
            }

            // No need to add leaves.
            if (value.Mode != NodeInfo.OperatingMode.Hub)
            {
                // Ensure that it is removed from all lists, in case it used to be a hub.
                m_tracer.DebugWriteLine("Making sure node {0} is not in our lists.", value.FriendlyName);
                System.Diagnostics.Debug.Assert(!(m_connectedHubs.Contains(value) && m_knownHubs.Contains(value)));
                lock (m_lockThis)
                {
                    return m_connectedHubs.Remove(value) || m_knownHubs.Remove(value);
                }
            }

            lock (m_lockThis)
            {
                bool removed = false;

                Stack stack = m_stack;
                if (null != stack)
                {
                    // Don't add self.
                    if (value.Equals(stack.NodeInfo))
                    {
                        return false;
                    }

                    // If we find a persistent connection, add to connected hubs; otherwise, add to known hubs.
                    // We add any matching connection we find, regardless of whether other connections
                    // have null remote node info.
                    if (null != stack.FindConnection(value, true))
                    {
                        // Remove from known, add to connected.
                        removed = m_knownHubs.Remove(value);
                        m_tracer.DebugWriteLine("Adding connected hub {0}", value.FriendlyName);
                        return m_connectedHubs.Add(value, false) || removed;
                    }
                }

                // Remove from connected, add to known.
                removed = m_connectedHubs.Remove(value);
                m_tracer.DebugWriteLine("Adding known hub {0}", value.FriendlyName);
                return m_knownHubs.Add(value, false) || removed;
            }
        }

        /// <summary>
        /// Add each node info in "addList" to this known hub list.
        /// </summary>
        /// <param name="addList">The known hub list containing the hubs to add.</param>
        /// <returns>True if this hub list changed.</returns>
        public bool Add(KnownHubList addList)
        {
            if (null == addList)
            {
                throw new ArgumentNullException("addList");
            }

            bool hubsChanged = false;

            // Add its known hubs
            addList.m_knownHubs.ForEach(delegate(NodeInfo rni)
            {
                if (Add(rni))
                {
                    hubsChanged = true;
                }
            });

            // Add its connected hubs
            addList.m_connectedHubs.ForEach(delegate(NodeInfo rni)
            {
                if (Add(rni))
                {
                    hubsChanged = true;
                }
            });

            return hubsChanged;
        }

        /// <summary>
        /// Remove a node info from the inner known hub list.
        /// If the node is connected, it is not removed.
        /// </summary>
        /// <param name="info">The node info to remove.</param>
        /// <returns>True if it was removed from the known hubs list.</returns>
        public bool Remove(NodeInfo info)
        {
            return m_knownHubs.Remove(info);
        }

        #endregion

        #region Convert to/from message

        /// <summary>
        /// Build a PyxNet message that contains the Known Hub List.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(StackConnection.KnownHubListMessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the Known Hub List to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            lock (m_lockThis)
            {
                message.Append(m_connectedHubs.Count);
                message.Append(m_knownHubs.Count);

                // append the directly connected hubs.
                m_connectedHubs.ForEach(delegate(NodeInfo connectedHub)
                {
                    connectedHub.ToMessage(message);
                });

                // append the known hubs.
                m_knownHubs.ForEach(delegate(NodeInfo knownHub)
                {
                    knownHub.ToMessage(message);
                });
            }
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(StackConnection.KnownHubListMessageID))
            {
                throw new System.ArgumentException(
                    "Message is not a Known Hub List message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            if (!reader.AtEnd)
            {
                throw new IndexOutOfRangeException("Extra data in a KnownHubList Message.");
            }
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a Known Hub List.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            int directCount = reader.ExtractInt();
            int knownCount = reader.ExtractInt();

            lock (m_lockThis)
            {
                m_connectedHubs.Clear();
                // read in the directly connected hubs.
                for (int count = 0; count < directCount; ++count)
                {
                    m_connectedHubs.Add(new NodeInfo(reader));
                }

                m_knownHubs.Clear();
                // read in the known hubs.
                for (int count = 0; count < knownCount; ++count)
                {
                    m_knownHubs.Add(new NodeInfo(reader));
                }
            }
        }

        #endregion
    }
}