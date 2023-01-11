using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// This fact verifies that the given resource has a particular definition.
    /// </summary>
    public class ResourceDefinitionFact : ICertifiableFact
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

        private string m_resourceDefinition;

        /// <summary>
        /// Gets or sets the resource definition.
        /// </summary>
        /// <value>The resource definition.</value>
        public string ResourceDefinition
        {
            get { return m_resourceDefinition; }
            set { m_resourceDefinition = value; }
        }

        /// <summary>
        /// Physical storage for the resourceId.  Note that this
        /// is always initialized to a value, even if the serialization
        /// logic over-writes it immediately afterward.
        /// </summary>
        private ResourceId m_resourceId = new ResourceId();

        public ResourceId ResourceId
        {
            get { return m_resourceId; }
            set { m_resourceId = value; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ResourceDefinitionFact"/> class.
        /// </summary>
        /// <param name="manifestEntry">The manifest entry.</param>
        public ResourceDefinitionFact(string resourceDefinition)
        {
            m_resourceDefinition = resourceDefinition;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ResourceDefinitionFact"/> class.
        /// Intended for serialization use (creating an object to read data into.)
        /// </summary>
        public ResourceDefinitionFact()
            :
            this(null)
        {
        }

        #region IIdentifiable Members

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
            m_resourceDefinition = reader.ExtractUTF8();
            m_resourceId = new ResourceId(reader);
        }

        #endregion

        #region ITransmissible Members

        public const string MessageId = "RDFT";
        public Message ToMessage()
        {
            Message result = new Message(MessageId);
            ToMessage(result);
            return result;
        }

        public void ToMessage(Message message)
        {
            message.Append(m_resourceDefinition);
            ResourceId.ToMessage(message);
        }

        #endregion

        #region ICertifiableFact Members

        public IEnumerable<string> Keywords
        {
            get
            {
                // Note that we don't currently return the resourceId itself.  Should we?
                if (this.ResourceDefinition != null)
                {
                    yield return string.Format("RESD:{0}", this.ResourceDefinition);
                }
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
