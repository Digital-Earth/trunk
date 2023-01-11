using System;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates the information that we send and recieve in a 
    /// Node Info message in PYXNet.  
    /// </summary>
    [Serializable]
    public class NodeInfo : ITransmissible
    {
        /// <summary>
        /// The types of nodes that make up a PyxNet Network.
        /// </summary>
        public enum OperatingMode { Unknown, Hub, Leaf };

        #region Properties

        #region Mode

        /// <summary>
        /// Storage for which mode we are currectly operating in.
        /// </summary>
        private OperatingMode m_mode = OperatingMode.Unknown;

        /// <summary>
        /// Which mode we are currectly operating in.
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public OperatingMode Mode
        {
            get { return m_mode; }
            set {
                if (m_mode != value)
                {
                    m_mode = value;
                    OnModeChanged(this, m_mode);
                }
            }
        }

        #region ModeChanged Event

        /// <summary> EventArgs for a ModeChanged event. </summary>    
        public class ModeChangedEventArgs : EventArgs
        {
            private OperatingMode m_OperatingMode;

            /// <summary>The OperatingMode.</summary>
            public OperatingMode OperatingMode
            {
                get { return m_OperatingMode; }
                set { m_OperatingMode = value; }
            }

            internal ModeChangedEventArgs(OperatingMode theOperatingMode)
            {
                m_OperatingMode = theOperatingMode;
            }
        }

        /// <summary> Event handler for ModeChanged. </summary>
        public event EventHandler<ModeChangedEventArgs> ModeChanged
        {
            add
            {
                m_ModeChanged.Add(value);
            }
            remove
            {
                m_ModeChanged.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<ModeChangedEventArgs> m_ModeChanged = new Pyxis.Utilities.EventHelper<ModeChangedEventArgs>();

        /// <summary>
        /// Raises the ModeChanged event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="theOperatingMode"></param>
        public void OnModeChanged(object sender, OperatingMode theOperatingMode)
        {
            m_ModeChanged.Invoke( sender, new ModeChangedEventArgs(theOperatingMode));
        }

        #endregion ModeChanged Event

        /// <summary>
        /// Helper property (determines if this nodeinfo identifies a hub.)
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public bool IsHub
        {
            get { return Mode == OperatingMode.Hub; }
            set
            {
                if (value)
                { 
                    Mode = OperatingMode.Hub; 
                }
                else
                { 
                    Mode = OperatingMode.Leaf; 
                }
            }
        }

        /// <summary>
        /// Helper property (determines if this nodeinfo identifies a leaf.)
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public bool IsLeaf
        {
            get { return Mode == OperatingMode.Leaf; }
            set
            {
                if (value)
                {
                    Mode = OperatingMode.Leaf;
                }
                else
                {
                    Mode = OperatingMode.Hub;
                }
            }
        }

        #endregion /* Mode */

        /// <summary>
        /// Storage for the number of hub nodes we are connected to.
        /// </summary>
        private int m_hubCount = 0;

        /// <summary>
        /// The number of hub nodes we are connected to.
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public int HubCount
        {
            get { return m_hubCount; }
            set { m_hubCount = value; }
        }

        /// <summary>
        /// Storage for the number of leaf nodes we are connected to.
        /// </summary>
        private int m_leafCount = 0;

        /// <summary>
        /// The number of leaf nodes we are connected to.
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public int LeafCount
        {
            get { return m_leafCount; }
            set { m_leafCount = value; }
        }

        /// <summary>
        /// Storage for the unique ID which identifies this node.
        /// </summary>
        private NodeId m_nodeId = new NodeId();

        /// <summary>
        /// The "node-id" uniquely defines this node.
        /// </summary>
        public NodeId NodeId
        {
            get { return m_nodeId; }
        }

        /// <summary>
        /// The unique ID which identifies this node.
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public Guid NodeGUID
        {
            get { return m_nodeId.Identity; }
            set { m_nodeId.Identity = value; }
        }

        /// <summary>
        /// The public key for the associated user.
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public DLM.PublicKey PublicKey
        {
            get { return m_nodeId.PublicKey; }
            set { m_nodeId.PublicKey = value; }
        }

        /// <summary>
        /// Storage for the network address of this node.
        /// </summary>
        private NetworkAddress m_address = new NetworkAddress();

        /// <summary>
        /// The network address of this node.
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public NetworkAddress Address
        {
            get { return m_address; }
            set { m_address = value; }
        }

        /// <summary>
        /// The "friendly name" for this node.
        /// </summary>
        private string m_friendlyName = "";

        /// <summary>
        /// The "friendly name" for this node.
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public string FriendlyName
        {
            get { return m_friendlyName; }
            set { m_friendlyName = value; }
        }

        #endregion

        #region Constructors

        // TODO: Consider implementing a constructor that takes a NodeId,
        // to prevent a default NodeId (with a random Guid) from being generated.
        /// <summary>
        /// Default constructor.
        /// </summary>
        public NodeInfo()
        {
        }

        /// <summary>
        /// Convenience constructor.
        /// </summary>
        /// <param name="serializeAsMessage"></param>
        public NodeInfo(String serializeAsMessage)
        {
            SerializeAsMessage = serializeAsMessage;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        public NodeInfo(Message message)
        {
            this.FromMessage(message);
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public NodeInfo(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }

        #endregion

        #region Convert to/from message format

        public string SerializeAsMessage
        {
            get
            {
                ArraySegment<byte> message = this.ToMessage().Bytes;
                string messageAsString = Pyxis.Utilities.UUEncoder.Encode(message);
                return string.Format("{0}:{1}", messageAsString.Length, messageAsString);
            }
            set
            {
                string simpleSubstring = value.Substring(value.IndexOf(":") + 1);
                byte[] messageContent = Pyxis.Utilities.UUEncoder.Decode(simpleSubstring);
                this.FromMessage(new Message(messageContent));
            }
        }

        /// <summary>
        /// Constant used to indicate an operating mode of hub.
        /// </summary>
        const char ModeHubId = 'H'; 

        /// <summary>
        /// Constant used to indicate an operating mode of leaf.
        /// </summary>
        const char ModeLeafId = 'L';

        /// <summary>
        /// Constant used to indicate an operating mode of unknown.
        /// </summary>
        const char ModeUnknownId = 'U';

        /// <summary>
        /// Build a PyxNet message that contains the Node Info.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(StackConnection.LocalNodeInfoMessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the Node Info to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        public void ToMessage(Message message)
        {
            if (message == null)
            {
                throw new System.ArgumentNullException("message");
            }

            switch (Mode)
            {
                case OperatingMode.Hub:
                    message.Append(ModeHubId);
                    break;
                case OperatingMode.Leaf:
                    message.Append(ModeLeafId);
                    break;
                case OperatingMode.Unknown:
                default:
                    message.Append(ModeUnknownId);
                    break;
            }

            message.Append(HubCount);
            message.Append(LeafCount);
            message.Append(NodeGUID);
            Address.ToMessage(message);
            message.Append(FriendlyName);
            message.AppendCountedBytes((PublicKey == null) ? new byte[0] : PublicKey.Key);
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        private void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(StackConnection.LocalNodeInfoMessageID))
            {
                throw new System.ArgumentException(
                    "Message is not a Node Info message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            if (!reader.AtEnd)
            {
                throw new ApplicationException("Extra data in a NodeInfo Message.");
            }
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the mode for the Node Info.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            if (reader == null)
            {
                throw new System.ArgumentException(
                    "Null message reader.");
            }

            // get the mode
            char mode = reader.ExtractChar();
            switch (mode)
            {
                case ModeHubId:
                    Mode = OperatingMode.Hub;
                    break;
                case ModeLeafId:
                    Mode = OperatingMode.Leaf;
                    break;
                case ModeUnknownId:
                    Mode = OperatingMode.Unknown;
                    break;
                default:
                    throw new System.ArgumentException(
                        "Node Info message had a bad mode code.");
            }

            HubCount = reader.ExtractInt();
            LeafCount = reader.ExtractInt();
            NodeGUID = reader.ExtractGuid();
            Address = new NetworkAddress(reader);
            FriendlyName = reader.ExtractUTF8();
            PublicKey = new PyxNet.DLM.PublicKey(reader.ExtractCountedBytes());
        }

        #endregion

        #region Equality

        /// <summary>
        /// Check for equality of contents.
        /// </summary>
        /// <param name="lniObject"></param>
        /// <returns>True if the objects are equal.</returns>
        public override bool Equals(object lniObject)
        {
            return Equals(lniObject as NodeInfo);
        }

        /// <summary>
        /// Check for equality of contents.
        /// </summary>
        /// <param name="lni"></param>
        /// <returns>True if the objects are equal.</returns>
        public bool Equals(NodeInfo lni)
        {
            return (null != lni && this.NodeGUID == lni.NodeGUID);
        }

        /// <summary>
        /// Need to override GetHashCode() when you override Equals.  
        /// </summary>
        /// <returns>the hash code of the contained GUID</returns>
        public override int GetHashCode()
        {
            return m_nodeId.GetHashCode();
        }

        #endregion

        /// <summary>
        /// Finds the specified node-info corresponding to the given node-id 
        /// on the specified stack.  Throws <see cref="TimeoutException"/> if 
        /// the node cannot be found within the given timeout.
        /// </summary>
        /// <param name="stack">The stack.</param>
        /// <param name="nodeId">The node id.</param>
        /// <param name="timeout">The timeout.</param>
        /// <returns></returns>
        public static NodeInfo Find(Stack stack, NodeId nodeId, TimeSpan timeout)
        {
            NodeInfo result = null;
            if (stack.AllKnownNodes.TryGetValue( nodeId, out result))
            {
                return result;
            }

            foreach (NodeInfo hub in stack.KnownHubList.KnownHubs)
            {
                if (hub.NodeId == nodeId)
                {
                    return hub;
                }
            }
            
            // TODO: Consider what the "100" timeout should be...
            Querier q = new Querier(stack,
                CreateQuery(stack.NodeInfo, nodeId), 100);
            
            PyxNet.Querier.ResultHandler resultHandler = 
                delegate(object sender, PyxNet.Querier.ResultEventArgs args)
                {
                    try
                    {
                        if (args.QueryResult.ExtraInfo.StartsWith(StackConnection.LocalNodeInfoMessageID))
                        {
                            result = new NodeInfo(args.QueryResult.ExtraInfo);
                            stack.AllKnownNodes[nodeId] = result;
                        }
                    }
                    catch (Exception ex)
                    {
                        stack.Tracer.ForcedWriteLine(
                             "Ignoring exception in Querier.OnResult: {0}", ex.ToString());
                    }
                };
            q.Result += resultHandler;
            q.Start();
            for (DateTime endTime = DateTime.Now + timeout;
                (result == null) && (DateTime.Now < endTime);
                System.Threading.Thread.Sleep(50)) ;
            q.Stop();
            if (result == null)
            {
                throw new TimeoutException(String.Format(
                    "Unable to find NodeInfo for NodeId {0}.", nodeId.Identity.ToString()));
            }
            q.Result -= resultHandler;
            return result;
        }

        /// <summary>
        /// Creates a query that will be sent from the nodeInfo's stack to find the nodeId.
        /// </summary>
        /// <param name="nodeInfo">The node info of the node that is issuing the query.</param>
        /// <param name="nodeId">The node id of the node we are searching for.</param>
        /// <returns></returns>
        public static Query CreateQuery(NodeInfo myNodeInfo, NodeId nodeId)
        {
            return new XPathQuery(myNodeInfo, String.Format("//NodeInfo[@id=\"{0}\"]",
                            nodeId.Identity.ToString())).ToQuery();
        }

        /// <summary>
        /// Return a string describing the node info.
        /// </summary>
        /// <returns>A human-readable string identifying the node info.</returns>
        public override string ToString()
        {
            string friendly = FriendlyName;
            if (friendly == "")
            {
                friendly = NodeId.ToString();
            }
            return friendly;
        }

        /// <summary>
        /// Return a string describing the node info in detail.
        /// </summary>
        /// <returns>A detailed human-readable string identifying the node info.</returns>
        public string ToStringVerbose()
        {
            System.Text.StringBuilder output = new System.Text.StringBuilder();
            output.AppendFormat(
                "NodeInfo: friendly name {0}, ID {1}, with address info {2}, mode {3}\n  Connected hubs {4}, leaves {5}.",
                FriendlyName, NodeId.ToString(), Address, Mode, HubCount, LeafCount);
            return output.ToString();
        }
    }
}