using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;

namespace PyxNet.Service
{
    #region Helper Classes, Interfaces

    /// <summary>
    /// Any object that has an Id.
    /// </summary>
    public interface IIdentifiable
    {
        /// <summary>
        /// Gets the object's id.
        /// </summary>
        /// <value>The id.</value>
        Guid Id { get; }
    }

    /// <summary>
    /// Any fact can be "certified".
    /// </summary>
    public interface ICertifiableFact : ITransmissibleWithReader, IIdentifiable
    {
        /// <summary>
        /// Gets or sets the certificate.  Note that setting is only
        /// done as part of the construction process.
        /// </summary>
        /// <value>The certificate.</value>
        Certificate Certificate { get; set; }

        /// <summary>
        /// Gets the keywords.
        /// </summary>
        /// <value>The keywords.</value>
        IEnumerable<string> Keywords { get; }

        /// <summary>
        /// Get a unique keyword suitable for searching over PyxNet.
        /// </summary>
        String UniqueKeyword { get; }
    }

    /// <summary>
    /// Provides a simple mapping between message id's and types to serialize.
    /// </summary>
    public class KnownMessageTypes
    {
        private Dictionary<string, Type> m_knownTypes = new Dictionary<string, Type>();

        /// <summary>
        /// Reads the object from the given message.  The object must be of one of
        /// the known types.
        /// </summary>
        /// <param name="m">The m.</param>
        /// <returns></returns>
        public object ReadObject(Message m)
        {
            Type typeToExtract;
            if (!m_knownTypes.TryGetValue(m.Identifier, out typeToExtract))
            {
                throw new ArgumentException(string.Format(
                    "Unexpected message type '{0}'.", m.Identifier));
            }
            // First, create a new instance of the class.
            // TODO: We could also look for a Message ctor, or MessageReader.
            // TODO: We could cache this result.

            System.Reflection.ConstructorInfo defaultConstructor =
                typeToExtract.GetConstructor(new Type[0]);
            if (defaultConstructor == null)
            {
                throw new InvalidOperationException(string.Format(
                    "Internal failure: {0} does not have a default constructor.",
                    typeToExtract.Name));
            }
            object result = defaultConstructor.Invoke(new Object[0]);

            // Extract the reader interface.
            ITransmissibleWithReader transmissibleResult =
                result as ITransmissibleWithReader;
            if (transmissibleResult == null)
            {
                throw new InvalidOperationException(string.Format(
                    "Internal failure: {0} is not ITransmissibleWithReader.",
                    typeToExtract.Name));
            }

            transmissibleResult.FromMessage(m);
            return result;
        }

        /// <summary>
        /// Registers the type.
        /// </summary>
        /// <param name="typeIdentifier">The type identifier.</param>
        /// <param name="actualType">Type of the actual.</param>
        public void RegisterType(string typeIdentifier, Type actualType)
        {
            if (typeIdentifier.Length != 4)
            {
                throw new ArgumentException(string.Format(
                    "Expecting a four character string (saw {0} in '{1}').",
                    typeIdentifier.Length, typeIdentifier));
            }

            System.Reflection.ConstructorInfo defaultConstructor =
                actualType.GetConstructor(new Type[0]);
            if (defaultConstructor == null)
            {
                throw new InvalidOperationException(string.Format(
                    "Internal failure: {0} does not have a default constructor.",
                    actualType.Name));
            }

            // TODO: Add test for ITransmissibleWithReader.

            m_knownTypes[typeIdentifier] = actualType;
        }
    }

    public class FactList
    { 
        private readonly Certificate m_certificate;

        /// <summary>
        /// Initializes a new instance of the <see cref="FactList"/> class.
        /// </summary>
        /// <param name="certificate">The certificate.</param>
        public FactList(Certificate certificate)
        {
            m_certificate = certificate;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="FactList"/> class.
        /// </summary>
        public FactList(): this(null)
        {
        }

        /// <summary>
        /// A ListItem is a combination fact/message (the two are interchangeable.)
        /// </summary>
        private class ListItem
        {
            #region Properties

            /// <summary>
            /// The certificate.  This is not serialized, so it must be set
            /// in the certifiable fact.
            /// </summary>
            private readonly Certificate m_certificate;

            /// <summary>
            /// Gets the certificate.
            /// </summary>
            /// <value>The certificate.</value>
            public Certificate Certificate
            {
                get { return m_certificate; }
            }

            private Message m_message;

            /// <summary>
            /// Gets or sets the message.
            /// </summary>
            /// <value>The message.</value>
            public Message Message
            {
                get
                {
                    if ((m_message == null) && (m_certifiableFact != null))
                    {
                        m_message = m_certifiableFact.ToMessage();
                    }
                    return m_message;
                }
                set
                {
                    m_message = value;
                    m_certifiableFact = null;
                }
            }

            private ICertifiableFact m_certifiableFact;

            /// <summary>
            /// Gets or sets the certifiable fact.
            /// </summary>
            /// <value>The certifiable fact.</value>
            public ICertifiableFact CertifiableFact
            {
                get
                {
                    if ((m_certifiableFact == null) && (m_message != null))
                    {
                        try
                        {
                            m_certifiableFact = Certificate.KnownTypes.ReadObject(m_message) as ICertifiableFact;
                        }
                        catch (Exception ex )
                        {
                            //--
                            //-- todo:  trying to read unregistered fact.  
                            //-- need to put error on the outbound fact, not the inbound.
                            //--
                            System.Diagnostics.Trace.TraceWarning("CertifiableFact warning: {0}", ex.Message);
                            return null;
                        }
                    }
                    if (m_certifiableFact.Certificate == null)
                    {
                        m_certifiableFact.Certificate = this.Certificate;
                    }
                    return m_certifiableFact;
                }
                set
                {
                    m_certifiableFact = value;
                    m_message = null;
                }
            }

            #endregion // Properties

            #region Constructors

            /// <summary>
            /// Initializes a new instance of the <see cref="ListItem"/> class.
            /// </summary>
            /// <param name="certificate">The certificate.</param>
            /// <param name="reader">The reader.</param>
            public ListItem(Certificate certificate, MessageReader reader)
            {
                m_certificate = certificate;
                Message = new Message(reader);
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="ListItem"/> class.
            /// </summary>
            /// <param name="certificate">The certificate.</param>
            /// <param name="fact">The fact.</param>
            public ListItem(Certificate certificate, ICertifiableFact fact)
            {
                m_certificate = certificate;
                CertifiableFact = fact;
            }

            /// <summary>
            /// Initializes a new instance of the <see cref="ListItem"/> class.
            /// </summary>
            /// <param name="certificate">The certificate.</param>
            /// <param name="message">The message.</param>
            public ListItem(Certificate certificate, Message message)
            {
                m_certificate = certificate;
                Message = message;
            }

            #endregion // Constructors
        }

        /// <summary>
        /// The fact that this certificate authorizes.
        /// </summary>
        private readonly List<ListItem> m_facts = new List<ListItem>();

        /// <summary>
        /// The facts that this certificate authorizes.
        /// </summary>
        public IEnumerable<ICertifiableFact> Facts
        {
            get
            {
                foreach (ListItem fact in m_facts)
                {
                    ICertifiableFact result = fact.CertifiableFact;
                    if (result != null)
                    {
                        yield return result;
                    }
                }
            }
        }

        /// <summary>
        /// Adds the specified fact.
        /// </summary>
        /// <param name="fact">The fact.</param>
        public void Add(ICertifiableFact fact)
        {
            m_facts.Add(new ListItem(m_certificate, fact));
            if (m_certificate != null)
            {
                m_certificate.Invalidate();
            }            
        }

        /// <summary>
        /// Adds the specified facts.
        /// </summary>
        /// <param name="facts">The facts.</param>
        public void Add(IEnumerable<ICertifiableFact> facts)
        {
            foreach (ICertifiableFact fact in facts)
            {
                Add(fact);
            }
        }

        internal void FromMessage(MessageReader reader)
        {
            int factCount = reader.ExtractInt();
            for (int loop = 0; loop < factCount; ++loop)
            {
                this.m_facts.Add(new ListItem( m_certificate, reader));
            }
        }

        internal void ToMessage(Message message)
        {
            message.Append(m_facts.Count);
            foreach (ListItem item in m_facts)
            {
                message.AppendCountedBytes(item.Message.Bytes.Array);
            }
        }
    }

    #endregion // Helper Classes, Interfaces

    /// <summary>
    /// A Certificate is a signed, time-limited document that authorizes a 
    /// set of specific facts.  Note that "fact" is a fairly generic term.
    /// </summary>
    public sealed class Certificate
    {
        /// <summary>
        /// Gets the empty certificate.
        /// </summary>
        /// <value>The empty certificate.</value>
        public static Certificate EmptyCertificate
        {
            get
            {
                return null;
            }
        }

        #region Properties

        /// <summary>
        /// Gets or sets the serialization string.  (Intended for serialization use only.)
        /// </summary>
        /// <value>The serialization string.</value>    
        public string SerializationString
        {
            get
            {
                return this.ToMessage().SerializationString;
            }
            set
            {
                Message m = new Message();
                m.SerializationString = value;
                this.FromMessageReader(new MessageReader(m));
            }
        }

        private static KnownMessageTypes s_knownTypes;

        /// <summary>
        /// Gets the known types.  Used for registering supported types for input.
        /// </summary>
        /// <value>The known types.</value>
        public static KnownMessageTypes KnownTypes
        {
            get
            {
                if (Certificate.s_knownTypes == null)
                {
                    Certificate.s_knownTypes = new KnownMessageTypes();

                    Certificate.s_knownTypes.RegisterType(ServiceInstanceFact.MessageId, typeof(ServiceInstanceFact));
                    Certificate.s_knownTypes.RegisterType(ResourceInstanceFact.MessageId, typeof(ResourceInstanceFact));
                    Certificate.s_knownTypes.RegisterType(ResourceDefinitionFact.MessageId, typeof(ResourceDefinitionFact));
                    Certificate.s_knownTypes.RegisterType(ResourcePermissionFact.MessageId, typeof(ResourcePermissionFact));
                    Certificate.s_knownTypes.RegisterType(GeoSourcePermissionFact.MessageId, typeof(GeoSourcePermissionFact));
                    Certificate.s_knownTypes.RegisterType(PyxNet.Service.PublishedPipelineFact.MessageId, typeof(PyxNet.Service.PublishedPipelineFact));
                }
                return Certificate.s_knownTypes;
            }
        }

        private FactList m_factList = new FactList();

        /// <summary>
        /// The facts that this certificate authorizes.
        /// </summary>
        public IEnumerable<ICertifiableFact> Facts
        {
            get
            {
                if (this.Valid)
                {
                    foreach (ICertifiableFact fact in this.m_factList.Facts)
                    {
                        fact.Certificate = this;
                        yield return fact;
                    }
                }
            }
        }

        public T FindFirstFact<T>() where T : class, ICertifiableFact
        {
            return Facts.FirstOrDefault(f => f is T) as T;
        }

        /// <summary>
        /// Gets the fact list that this certificate authorizes.
        /// </summary>
        /// <value>The fact list.</value>
        public IList<ICertifiableFact> FactList
        {
            get
            {
                List<ICertifiableFact> result = new List<ICertifiableFact>();
                result.AddRange(this.Facts);
                return result;
            }
        }

        /// <summary>
        /// Adds the specified fact.
        /// </summary>
        /// <param name="fact">The fact.</param>
        public void Add(ICertifiableFact fact)
        {
            m_factList.Add(fact);
            m_valid = null; 
        }

        /// <summary>
        /// The authority that has authorized these facts.
        /// </summary>
        private ServiceInstance m_authority;

        /// <summary>
        /// The authority that has authorized these facts.
        /// </summary>
        public ServiceInstance Authority
        {
            get { return m_authority; }
        }

        /// <summary>
        /// The time at which this certificate expires.
        /// </summary>
        private DateTime m_expireTime;

        /// <summary>
        /// The time at which this certificate expires.
        /// </summary>
        public DateTime ExpireTime
        {
            get { return m_expireTime; }
        }

        private Nullable<DateTime> m_issuedTime;

        /// <summary>
        /// Gets or sets the time when this certificate was issued/certified.
        /// </summary>
        /// <remarks>
        /// TODO: We don't use the issued time in the signature, so it could be
        /// modified.  (We really need to use XML in message formats!)
        /// </remarks>
        /// <value>The issued time.</value>
        public DateTime IssuedTime
        {
            get 
            {
                // Use the issued time if its known.  Don't allow it to be later
                //  than the expire time though!
                if (m_issuedTime.HasValue && (m_issuedTime.Value >= m_expireTime))
                {
                    return m_issuedTime.Value;
                }
                else
                {
                    return m_expireTime;
                }
            }
            set 
            { 
                m_issuedTime = value;
            }
        }

        /// <summary>
        /// A signature on this certificate.  (Authority has signed it.)
        /// </summary>
        private byte[] m_signature;

        /// <summary>
        /// A signature on this certificate.  (Authority has signed it.)
        /// </summary>
        public byte[] Signature
        {
            get { return m_signature; }
        }

        #endregion /* Properties */

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="Certificate"/> class.
        /// (This constructor is used for serialization.)
        /// </summary>
        public Certificate()
        {
        }

        /// <summary>
        /// Constructor.  
        /// </summary>
        /// <param name="authority">The authority.</param>
        /// <param name="expiration">The expiration.</param>
        /// <param name="facts">The facts.</param>
        public Certificate(
            ServiceInstance authority,
            DateTime expiration,
            params ICertifiableFact[] facts)
        {
            if ((authority == null) || (authority.Server == null))
            {
                throw new ArgumentException(
                    "The given authority is not valid.", "authority");
            }
            m_authority = authority;
            m_expireTime = expiration;
            foreach (ICertifiableFact fact in facts)
            {
                this.Add(fact);
            }
            m_signature = null;
        }

        /// <summary>
        /// Signs the certificate.
        /// </summary>
        /// <param name="authorityKey">The authority key.</param>
        public void SignCertificate(DLM.PrivateKey authorityKey)
        {
            // TODO: Test that m_authority matches authorityKey.

            m_signature = SignedMessageHelper.GenerateSignature(
                RawMessage.Bytes.Array, authorityKey);
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        public Certificate(MessageReader reader)
        {
            FromMessageReader(reader);
        }

        /// <summary>
        /// Construct from a database row.
        /// </summary>
        public Certificate(System.Data.DataRow row)
            :
            this(new MessageReader(new Message(
            Convert.FromBase64String(
            (String)row[ContentColumnName]))))
        {
            this.Id = (Guid) row[IndexColumnName];
        }
        #endregion /* Constructors */

        /// <summary>
        /// Helper for diagnostics.
        /// </summary>
        private static Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(false);

        /// <summary>
        /// Tracks all certificates that have been verified.
        /// </summary>
        private static Dictionary<Certificate, Certificate> s_knownCertificates =
            new Dictionary<Certificate, Certificate>();

        /// <summary>
        /// Holder for valid setting.  Set to null if validity has never been 
        /// tested.  Note that m_valid only means that it was valid the last
        /// time it was read, and that the certificate may have expired in the
        /// mean-time.  Call Valid to get the real value.
        /// </summary>
        private bool? m_valid = null;

        /// <summary>
        /// Tests to see if the expiry date is okay, and the signature checks out.
        /// </summary>
        public bool Valid
        {
            get
            {
                // If we're over the time limit, don't bother to test anything else.
                if (DateTime.Now > m_expireTime)
                {
                    m_valid = false;
                }

                // Ensure that the signature is valid.
                if (!m_valid.HasValue)
                {
                    // Check to see if we've seen a similar certificate previously.
                    lock (s_knownCertificates)
                    {
                        if (!s_knownCertificates.ContainsKey(this))
                        {
                            Trace.DebugWriteLine("About to validate ({0}).", this.ToString());
                            m_valid = ((m_signature != null) &&
                                SignedMessageHelper.VerifySignature(RawMessage.Bytes.Array,
                                m_signature, m_authority.Server.PublicKey));
                            s_knownCertificates.Add(this, this);
                            Pyxis.Utilities.GlobalPerformanceCounters.Counters.CertificatesValidated.Increment();
                        }
                        else
                        {
                            Trace.DebugWriteLine("About to re-validate ({0}).", this.ToString());
                            m_valid = s_knownCertificates[this].Valid;
                        }
                    }
                }

                return m_valid.Value;
            }
        }

        /// <summary>
        /// Returns a <see cref="T:System.String"/> that represents the current <see cref="T:System.Object"/>.
        /// </summary>
        /// <returns>
        /// A <see cref="T:System.String"/> that represents the current <see cref="T:System.Object"/>.
        /// </returns>
        public override string ToString()
        {
            return string.Format("Certificate( Authority=\"{0}\", Issued=\"{3}\", Expires=\"{1}\", Facts=\"{2}\")",
                m_authority.ToString(), m_expireTime, m_factList, m_issuedTime);
        }

        public const string MessageID = "CERT";
        public const string IndexColumnName = "Id";
        public const string ContentColumnName = "MessageData";

        /// <summary>
        /// Writes this to the given row.
        /// </summary>
        /// <param name="row">The row.</param>
        public void ToRow(System.Data.DataRow row)
        {
            // TODO: Consider adding "keys" to the database, as additional 
            //  relation tables.

            row[IndexColumnName] = this.Id;
            Message rawContents = this.ToMessage();
            row[ContentColumnName] = Convert.ToBase64String(rawContents.Bytes.Array);
        }

        /// <summary>
        /// Creates a DataTable to store CertificateBases.
        /// </summary>
        /// <param name="tableName"></param>
        /// <returns></returns>
        static public System.Data.DataTable CreateDataTable(string tableName)
        {
            System.Data.DataTable t = new System.Data.DataTable(tableName);
            t.Columns.Add(IndexColumnName, typeof(Guid));
            t.Columns.Add(ContentColumnName, typeof(string));
            return t;
        }

        #region To/From Message

        /// <summary>
        /// Helper function to extract the raw message for signing.
        /// </summary>
        private Message RawMessage
        {
            get
            {
                Message result = new Message();
                ToRawMessage(result);
                return result;
            }
        }

        /// <summary>
        /// Helper function to actually do the work of RawMessage.
        /// </summary>
        /// <param name="message"></param>
        private void ToRawMessage(Message message)
        {
            m_authority.ToMessage(message);
            m_factList.ToMessage(message);
            message.Append(m_expireTime.Ticks);
        }

        /// <summary>
        /// Append the CertificateBase to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        public void ToMessage(Message message)
        {
            if (message == null)
            {
                throw new System.ArgumentNullException("message");
            }
            ToRawMessage(message);
            message.AppendCountedBytes(m_signature);
            message.Append(this.Id);
            if (m_issuedTime.HasValue)
            {
                message.Append(m_issuedTime.Value.Ticks);
            }
        }

        /// <summary>
        /// Extracts this object from the the message reader.
        /// </summary>
        /// <param name="reader">The reader.</param>
        private void FromMessageReader(MessageReader reader)
        {
            m_authority = new ServiceInstance(reader);
            m_factList.FromMessage(reader);
            m_expireTime = new DateTime(reader.ExtractInt64());
            m_signature = reader.ExtractCountedBytes();
            if (!reader.AtEnd)
            {
                Id = reader.ExtractGuid();
            }
            if (!reader.AtEnd)
            {
                try
                {
                    m_issuedTime = new DateTime(reader.ExtractInt64());
                }
                catch (Exception ex)
                {
                    StackSingleton.Stack.Tracer.WriteLine("Certificate.FromMessageReader ignoring error while reading issued time. {0}", ex.ToString());
                }
            }
        }

        #endregion /* To/From Message */

        public Message ToMessage()
        {
            Message result = new Message(MessageID);
            this.ToMessage(result);
            return result;
        }

        public override bool Equals(object obj)
        {
            Certificate objectAsCertificate = obj as Certificate;
            return ((objectAsCertificate != null) &&
                this.Authority.Equals( objectAsCertificate.Authority) &&
                (this.ExpireTime == objectAsCertificate.ExpireTime) &&
                (this.IssuedTime == objectAsCertificate.IssuedTime) &&
                this.RawMessage.Equals( objectAsCertificate.RawMessage));
        }

        public override int GetHashCode()
        {
            return this.m_signature.GetHashCode();
        }

        /// <summary>
        /// This is really a hack...
        /// </summary>
        public ServiceInstance ServiceInstance
        {
            get
            {
                foreach (ICertifiableFact fact in Facts)
                {
                    ServiceInstanceFact actualFact = fact as ServiceInstanceFact;
                    if (actualFact != null)
                    {
                        return actualFact.ServiceInstance;
                    }
                }
                Trace.DebugWriteLine("This certificate does not contain a ServiceInstance fact.");
                return null;
            }
        }

        private Guid m_id = Guid.Empty;

        public Guid Id
        {
            get
            {
                if (m_id == Guid.Empty)
                {
                    m_id = Guid.NewGuid();
                }
                return m_id;
            }
            set
            {
                m_id = value;
            }
        }

        internal void Invalidate()
        {
            m_valid = null;
        }
    }

    /// <summary>
    /// TODO: Move this function into ServiceInstanceFact.
    /// TODO: Replicate for resource instance fact.
    /// </summary>
    public static class ServiceInstanceCertificateHelper
    {
        /// <summary>
        /// Public "constructor".
        /// </summary>
        /// <param name="authorityKey">The authority key.</param>
        /// <param name="authority">The authority.</param>
        /// <param name="expiration">The expiration.</param>
        /// <param name="fact">The fact.</param>
        /// <returns></returns>
        static public Certificate Create(
            DLM.PrivateKey authorityKey,
            ServiceInstance authority,
            DateTime expiration,
            ServiceInstance fact)
        {
            Certificate result = new Certificate(
                authority, expiration, new ServiceInstanceFact( fact));
            result.SignCertificate(authorityKey);
            return result;
        }

    }

    public static class CertifiableFactCertificateHelper
    {
        static public Certificate Create(DLM.PrivateKey authorityKey, ServiceInstance authority, DateTime expiration, ICertifiableFact fact)
        {
            Certificate result = new Certificate(authority, expiration, fact);
            result.SignCertificate(authorityKey);
            return result;
        }
    }
}
