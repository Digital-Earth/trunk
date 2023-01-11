using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// This fact verifies that the given manifest entry (resource) is an 
    /// instance of the named resource.
    /// </summary>
    [Serializable]
    public class ResourceInstanceFact : ICertifiableFact
    {
        private Certificate m_certificate;

        /// <summary>
        /// Gets or sets the certificate.
        /// </summary>
        /// <value>The certificate.</value>
        [System.Xml.Serialization.XmlIgnore]
        public Certificate Certificate
        {
            get { return m_certificate; }
            set { m_certificate = value; }
        }

        private Pyxis.Utilities.ManifestEntry m_manifestEntry;

        /// <summary>
        /// Gets or sets the manifest entry.
        /// </summary>
        /// <value>The manifest entry.</value>
        public Pyxis.Utilities.ManifestEntry ManifestEntry
        {
            get { return m_manifestEntry; }
            set { m_manifestEntry = value; }
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
        /// Initializes a new instance of the <see cref="ResourceInstanceFact"/> class.
        /// </summary>
        /// <param name="manifestEntry">The manifest entry.</param>
        public ResourceInstanceFact(Pyxis.Utilities.ManifestEntry manifestEntry)
        {
            m_manifestEntry = manifestEntry;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ResourceInstanceFact"/> class.
        /// Intended for serialization use (creating an object to read data into.)
        /// </summary>
        public ResourceInstanceFact()
            :
            this((Pyxis.Utilities.ManifestEntry)null)
        {
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public ResourceInstanceFact(MessageReader reader)
        {
            FromMessage(reader);
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
            m_manifestEntry = reader.ExtractXmlObject < Pyxis.Utilities.ManifestEntry>();
            m_resourceId = new ResourceId(reader);
        }

        #endregion

        #region ITransmissible Members

        public const string MessageId = "RIFT";
        public Message ToMessage()
        {
            Message result = new Message(MessageId);
            ToMessage(result);
            return result;
        }

        public void ToMessage(Message message)
        {
            message.AppendXmlObject(m_manifestEntry, new Type[0]);
            ResourceId.ToMessage(message);
        }

        #endregion

        #region ICertifiableFact Members

        public IEnumerable<string> Keywords
        {
            get
            {
                // Note that we don't currently return the resourceId itself.  Should we?
                if (this.ManifestEntry != null)
                {
                    yield return string.Format("RESS:{0}", this.ManifestEntry.FileStamp);
                    yield return string.Format("RESN:{0}", this.ManifestEntry.FileName);
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

        #region Helper Functions

        /// <summary>
        /// Finds a certified fact that matches the given uncertified fact.
        /// </summary>
        /// <param name="stack">
        /// The stack (which owns the searched repository).
        /// </param>
        /// <param name="uncertifiedFact">The uncertified fact.</param>
        /// <returns>A certified fact, or null if none is found.</returns>
        public static ResourceInstanceFact FindCertifiedFact(
            PyxNet.Stack stack, 
            ResourceInstanceFact uncertifiedFact)
        {
            // Search for a certificate.
            foreach (PyxNet.Service.ResourceInstanceFact repositoryFact in
                stack.CertificateRepository.GetMatchingFacts(
                    uncertifiedFact.UniqueKeyword, typeof(PyxNet.Service.ResourceInstanceFact)))
            {
                if (repositoryFact.ManifestEntry.FileStamp.Equals(
                    uncertifiedFact.ManifestEntry.FileStamp) &&
                    repositoryFact.ManifestEntry.FileName.Equals(
                    uncertifiedFact.ManifestEntry.FileName))
                {
                    return repositoryFact;
                }
            }

            return null;
        }

        #endregion Helper Functions

        #region Equality

        /// <summary>
        /// Tests for the equivalence of contents.
        /// </summary>
        /// <param name="rhs">The value to test against this object.</param>
        /// <returns>True if the contents are equivalent.</returns>
        public override bool Equals(object rhs)
        {
            return Equals(rhs as ResourceInstanceFact);
        }

        /// <summary>
        /// Tests for the equivalence of contents.
        /// </summary>
        /// <param name="rhs">The value to test against this object.</param>
        /// <returns>True if the contents are equivalent.</returns>
        public bool Equals(ResourceInstanceFact rhs)
        {
            if (null == rhs)
            {
                return false;
            }

            return (this.Certificate == rhs.Certificate) && 
                (this.Id == rhs.Id) &&
                (this.ManifestEntry == rhs.ManifestEntry) &&
                (this.ResourceId == rhs.ResourceId);
        }

        /// <summary>
        /// Returns a hash code for the object.
        /// </summary>
        /// <returns>A hash code for the object.</returns>
        public override int GetHashCode()
        {
            // TODO: Implement as necessary.
            return base.GetHashCode();
        }

        #endregion    

        #region ToString

        /// <summary>
        /// Generate a string representation of the resource instance fact.
        /// </summary>
        /// <returns></returns>
        public override String ToString()
        {
        	// TODO: Indicate that this is a resource instance fact.
            return this.Id.ToString();
        }

        #endregion
    }

    namespace Test
    {
    }
}
