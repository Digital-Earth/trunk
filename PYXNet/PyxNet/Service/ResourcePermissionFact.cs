using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// Permissions that can be granted, as flags.
    /// </summary>
    public enum PermissionFlags
    {
        AccessResource = 1
    }

    /// <summary>
    /// A specific permission on a specific resource.
    /// </summary>
    public class ResourcePermissionFact : ICertifiableFact
    {
        private Certificate m_certificate;

        /// <summary>
        /// Gets or sets the certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public Certificate Certificate
        {
            get { return m_certificate; }
            set { m_certificate = value; }
        }

        private ResourceId m_resourceId;

        /// <summary>
        /// Gets or sets the resource id.  This is the resource for which
        /// permission has been granted.
        /// </summary>
        /// <value>The resource id.</value>
        public ResourceId ResourceId
        {
            get { return m_resourceId; }
            set { m_resourceId = value; }
        }

        private PermissionFlags m_permissions = PermissionFlags.AccessResource;

        /// <summary>
        /// Gets or sets the permissions.
        /// </summary>
        /// <value>The permissions.</value>
        public PermissionFlags Permissions
        {
            get { return m_permissions; }
            set { m_permissions = value; }
        }

        private NodeId m_authorizedNode;

        /// <summary>
        /// Gets or sets the authorized node.  This is the node that has been 
        /// authorized to have the given permission on the specified resource.
        /// </summary>
        /// <value>The authorized node.</value>
        public NodeId AuthorizedNode
        {
            get { return m_authorizedNode; }
            set { m_authorizedNode = value; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ResourcePermissionFact"/> class.
        /// </summary>
        /// <param name="resourceId">The resource id.</param>
        /// <param name="authorizedNode">The authorized node.</param>
        public ResourcePermissionFact(ResourceId resourceId, NodeId authorizedNode)
        {
            m_resourceId = resourceId;
            m_authorizedNode = authorizedNode;
        }

        /// <summary>
        /// Only used for serialization.
        /// </summary>
        public ResourcePermissionFact()
        {
        }

        #region ICertifiableFact Members

        public Guid Id
        {
            get { return ResourceId; }
        }

        #endregion

        #region ITransmissibleWithReader Members

        public void FromMessage(Message message)
        {
            if (message.Identifier != MessageId)
            {
                throw new ArgumentException(string.Format(
                    "Invalid message identifier.  Saw {0}, but was expecting {1}.",
                    message.Identifier, MessageId));
            }
            MessageReader reader = new MessageReader(message);
            FromMessage(reader);
        }

        public void FromMessage(MessageReader reader)
        {
            m_resourceId = new ResourceId(reader);
            m_permissions = (PermissionFlags)reader.ExtractInt();
            m_authorizedNode = new NodeId(reader);
        }

        #endregion

        #region ITransmissible Members

        public const string MessageId = "RAPF";
        public Message ToMessage()
        {
            Message result = new Message(MessageId);
            ToMessage(result);
            return result;
        }

        public void ToMessage(Message message)
        {
            ResourceId.ToMessage(message);
            message.Append((int)this.Permissions);
            m_authorizedNode.ToMessage(message);
        }

        #endregion

        #region ICertifiableFact Members

        public IEnumerable<string> Keywords
        {
            get
            {
                yield return string.Format("RESP:{0}:{1}",
                    m_resourceId.Guid.ToString(),
                    m_authorizedNode.Identity.ToString());
                yield return string.Format("RESP:{0}", m_resourceId.Guid.ToString());
            }
        }

        public String UniqueKeyword
        {
            get
            {
                foreach (string keyword in Keywords)
                {
                    return keyword;
                }
                return null;
            }
        }

        #endregion
    }
}
