using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;

namespace PyxNet.Service
{
    /// <summary>
    /// A specific permission on a specific GeoSource.
    /// </summary>
    public class GeoSourcePermissionFact : ICertifiableFact
    {
        /// <summary>
        /// Allows limitations to be put on the use of the GeoSource.
        /// </summary>
        [Serializable]
        public class PermissionLimitations
        {
            /// <summary>
            /// Gets or sets the limitation on area.
            /// </summary>
            public string Area { get; set; }
            /// <summary>
            /// Gets or sets the limitation on resolution.
            /// </summary>
            public uint? Resolution { get; set; }
            /// <summary>
            /// Gets or sets the limitation on time.
            /// </summary>
            public TimeSpan? Time { get; set; }
            /// <summary>
            /// Gets or sets the watermark to be used.
            /// </summary>
            public string Watermark { get; set; }

            /// <summary>
            /// Initializes a new instance of PermissionLimitations.
            /// </summary>
            public PermissionLimitations()
            {
            }

            /// <summary>
            /// Initializes a new instance of PermissionLimitations from an existing instance.
            /// </summary>
            /// <param name="limitations">Existing instance of PermissionLimitations to use.</param>
            public PermissionLimitations(PermissionLimitations limitations)
            {
                Area = limitations.Area;
                Resolution = limitations.Resolution;
                Time = limitations.Time;
                Watermark = limitations.Watermark;
            }

            /// <summary>
            /// Initialize the members from a message reader.  The message reader
            /// should be properly set to point at the start of the identity record.
            /// </summary>
            /// <param name="reader">The message reader to read from.</param>
            public PermissionLimitations(MessageReader reader)
            {
                FromMessageReader(reader);
            }

            /// <summary>
            /// Initialize the members from a message reader.  The message reader
            /// should be properly set to point at the start of the PermissionLimitations
            /// record.
            /// </summary>
            /// <param name="reader">The message reader to read from.</param>
            public void FromMessageReader(MessageReader reader)
            {
                var hasAreaValue = reader.ExtractBool();
                if (hasAreaValue)
                {
                    Area = reader.ExtractUTF8();
                }
                var hasResolutionValue = reader.ExtractBool();
                if (hasResolutionValue)
                {
                    Resolution = reader.ExtractUInt32();
                }
                var hasTimeValue = reader.ExtractBool();
                if (hasTimeValue)
                {
                    Time = XmlConvert.ToTimeSpan(reader.ExtractUTF8());
                }
                var hasWatermarkValue = reader.ExtractBool();
                if (hasWatermarkValue)
                {
                    Watermark = reader.ExtractUTF8();
                }
            }

            /// <summary>
            /// Append "this" to an existing message.  
            /// (Does not include the message header.)
            /// </summary>
            /// <param name="message">The message to append to. (will be modified)</param>
            public void ToMessage(Message message)
            {
                // A null string is deserialized as an empty string. Writing null flag to preserve null.
                message.Append(Area != null);
                if (Area != null)
                {
                    message.Append(Area);
                }
                message.Append(Resolution.HasValue);
                if (Resolution.HasValue)
                {
                    message.Append(Resolution.Value);
                }
                message.Append(Time.HasValue);
                if (Time.HasValue)
                {
                    message.Append(XmlConvert.ToString(Time.Value));
                }
                message.Append(Watermark != null);
                if (Watermark != null)
                {
                    message.Append(Watermark);
                }
            }
        }

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
        private ResourceId m_userResourceId;

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

        /// <summary>
        /// Gets or sets the user resource id.  This is the user for which
        /// permission has been granted to.
        /// </summary>
        /// <value>The user resource id.</value>
        public ResourceId UserResourceId
        {
            get { return m_userResourceId; }
            set { m_userResourceId = value; }
        }

        public string ProcRef { get; set; }
        public string SerializedGeometry { get; set; }

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

        public PermissionLimitations Limitations { get; set; }

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

        public GeoSourcePermissionFact(ResourceId resourceId, ResourceId userId, NodeId authorizedNode, string procRef)
            : this(resourceId, userId, authorizedNode, procRef, null, new PermissionLimitations())
        {
        }

        public GeoSourcePermissionFact(ResourceId resourceId, ResourceId userId, NodeId authorizedNode, string procRef, string serializedGeometry, PermissionLimitations limitations)
        {
            m_resourceId = resourceId;
            m_userResourceId = userId;
            m_authorizedNode = authorizedNode;
            ProcRef = procRef;
            SerializedGeometry = serializedGeometry;
            Limitations = limitations;
        }

        /// <summary>
        /// Only used for serialization.
        /// </summary>
        public GeoSourcePermissionFact()
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
            m_userResourceId = new ResourceId(reader);
            m_permissions = (PermissionFlags) reader.ExtractInt();
            m_authorizedNode = new NodeId(reader);
            ProcRef = reader.ExtractUTF8();
            SerializedGeometry = reader.ExtractUTF8();
            var hasLimitations = reader.ExtractBool();
            if (hasLimitations)
            {
                Limitations = new PermissionLimitations(reader);
            }
        }

        #endregion

        #region ITransmissible Members

        public const string MessageId = "GSPF";
        public Message ToMessage()
        {
            Message result = new Message(MessageId);
            ToMessage(result);
            return result;
        }

        public void ToMessage(Message message)
        {
            ResourceId.ToMessage(message);
            UserResourceId.ToMessage(message);
            message.Append((int)this.Permissions);
            m_authorizedNode.ToMessage(message);
            message.Append(ProcRef);
            message.Append(SerializedGeometry);
            var hasLimitations = Limitations != null;
            message.Append(hasLimitations);
            if (hasLimitations)
            {
                Limitations.ToMessage(message);
            }
        }

        #endregion

        #region ICertifiableFact Members

        public IEnumerable<string> Keywords
        {
            get
            {
                yield return string.Format("GEOP:{0}:{1}:{2}",
                    m_resourceId.Guid.ToString(),
                    m_userResourceId.Guid.ToString(),
                    m_authorizedNode.Identity.ToString());
                yield return string.Format("GEOP:{0}:{1}", m_resourceId.Guid.ToString(), m_userResourceId.Guid.ToString());
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