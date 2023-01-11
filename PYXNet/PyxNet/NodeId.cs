using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Text;
using PyxNet.DLM;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates a "node identity", which is the information that identifies
    /// a node.  Currently, this includes a guid and a public key.  
    /// 
    /// A node is a combination of a unique user, versioned application and machine
    /// on the network.  The public key belongs to the user, and the node GUID
    /// corresponds to the versioned application on a specific machine.
    /// 
    /// In the future, we may choose to transmit only the "header" (id) of this
    /// record, and have clients access the rest of the data as a query.
    /// </summary>
    [Serializable]
    [DataContract]
    public class NodeId
    {
        /// <summary>
        /// A node's identity never changes, and is unique across the network. 
        /// </summary>
        [DataMember(Name = "Id")]
        public Guid Identity
        {
            get;
            set;
        }

        /// <summary>
        /// The user associated with this node.
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public UserId UserId
        {
            get;
            set;
        }

        /// <summary>
        /// Each node user publishes their public key to all who want it.
        /// </summary>
        [DataMember(Name = "PublicKey")]
        public PublicKey PublicKey
        {
            get
            {
                return UserId == null ? null : UserId.PublicKey;
            }
            set
            {
                if (value == null)
                {
                    UserId = null;
                }
                else if (UserId == null)
                {
                    UserId = new UserId(value);
                }
                else
                {
                    UserId.PublicKey = value;
                }
            }
        }

        /// <summary>
        /// Default constructor that generates a random node id.
        /// Note that this should never be used in 
        /// "production" code, except for serialization.
        /// In "production" code the Stack will generate a real private key and Guid,
        /// and will store them in it's own NodeId constructed via the other constructor,
        /// then those will be sent around the network.
        /// </summary>
        public NodeId()
        {
            Identity = Guid.NewGuid();
            UserId = null;
        }

        /// <summary>
        /// Constructor that uses the specified identity.
        /// </summary>
        public NodeId(Guid identity)
        {
            Identity = identity;
            UserId = null;
        }

        public static NodeId FindNodeIdOnNetwork(Stack stack, Guid identity)
        {
            return FindNodeIdOnNetwork(stack, identity, TimeSpan.FromSeconds(10));
        }
        public static NodeId FindNodeIdOnNetwork(Stack stack, Guid identity,TimeSpan timeout)
        {
            //find the node over the stack network, and return the initialized NodeId with the PublicKey
            return NodeInfo.Find(stack, new NodeId(identity) , timeout).NodeId;
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of the identity record.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public NodeId(MessageReader reader)
        {
            this.FromMessageReader(reader);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of the identity record.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            Identity = reader.ExtractGuid();
            UserId = new UserId(reader);
        }

        /// <summary>
        /// Append "this" to an existing message.  
        /// (Does not include the message header.)
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        public void ToMessage(Message message)
        {
            message.Append(Identity);
            message.AppendCountedBytes(PublicKey.Key);
        }

        #region Equality

        public override bool Equals(object obj)
        {
            return Equals(obj as NodeId);
        }

        public bool Equals(NodeId other)
        {
            if (other != null)
            {
                return (other.Identity == Identity) &&
                    (PublicKey == other.PublicKey ||
                    (PublicKey != null && (PublicKey.Equals(other.PublicKey))));
            }
            return false;
        }

        private static int CombineHashCodes(int h1, int h2)
        {
            return ((h1 << 5) + h1) ^ h2;
        }

        public override int GetHashCode()
        {
            return CombineHashCodes(Identity.GetHashCode(), 
                (PublicKey == null) ? 0 : PublicKey.GetHashCode());
        }

        #endregion /* Equality */

        public override string ToString()
        {
            return String.Format("NodeId({0})", Identity.ToString());
        }
    }
}

