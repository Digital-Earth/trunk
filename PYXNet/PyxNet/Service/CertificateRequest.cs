using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// Encapsulates a PyxNet certificate request.  A specific node is 
    /// requesting a <see cref="Certificate"/> that authorizes a specific
    /// service (and potentially a subservice) to be run by that node.
    /// If another node has authority to issue such a certificate, then
    /// it will send an answer in one (or more) CertificateRequestResponse
    /// message(s).
    /// </summary>
    public class CertificateRequest : ITransmissible
    {
        /// <summary>
        /// The Message ID.
        /// </summary>
        public const string MessageID = "CReq";

        #region Fields and properties

        /// <summary>
        /// The identity of this request.  Used to identify the request in
        /// asynchronous responses.
        /// </summary>
        private readonly Guid m_id;

        /// <summary>
        /// The identity of this request.  Used to identify the request in
        /// asynchronous responses.
        /// </summary>
        public Guid Id
        {
            get { return m_id; }
        }

        public enum RequestTypeFlags
        {
            QueryOnly = 1,
            GenerateCertificate = 2,
            FindAuthority = 4
        }

        private RequestTypeFlags m_requestType = RequestTypeFlags.GenerateCertificate;

        /// <summary>
        /// Gets or sets the type of the request.
        /// </summary>
        /// <remarks>
        /// TODO: This setting is not currently used.  Add support for different request types.
        /// </remarks>
        /// <value>The type of the request.</value>
        public RequestTypeFlags RequestType
        {
            get { return m_requestType; }
            set { m_requestType = value; }
        }

        /// <summary>
        /// <see cref="Facts"/>
        /// </summary>
        private readonly FactList m_facts = new FactList();

        /// <summary>
        /// Gets or sets the certifiable facts.  These are the facts for which 
        /// we are requesting a certificate.
        /// </summary>
        /// <value>The certifiable facts.</value>
        public FactList FactList
        {
            get
            {
                return m_facts;
            }
        }

        /// <summary>
        /// The info about the requesting node.
        /// </summary>
        private readonly NodeInfo m_requester;

        /// <summary>
        /// The info about the requesting node.
        /// </summary>
        public NodeInfo Requester
        {
            get
            {
                return m_requester;
            }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="CertificateRequest"/> class.
        /// </summary>
        /// <param name="id">The id for the request.</param>
        /// <param name="requester">The node that wants to be certified.</param>
        /// <param name="facts">The facts to be certified.</param>
        public CertificateRequest(Guid id, NodeInfo requester, params ICertifiableFact[] facts)
        {
            m_requester = requester;
            foreach (ICertifiableFact fact in facts)
            {
                m_facts.Add(fact);
            }
            m_id = id;
        }
        
        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        /// <param name="reader">
        /// The message reader to construct from.
        /// This doesn't include the message header.
        /// </param>
        public CertificateRequest(MessageReader reader)
        {
            m_requester = new NodeInfo(reader);
            m_id = reader.ExtractGuid();
            m_facts.FromMessage(reader);
        }

        #endregion

        #region Convert to/from message

        /// <summary>
        /// Build a message that contains the object.
        /// </summary>
        /// <returns>The resulting message.</returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the object to an existing message.
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to (will be modified).</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }

            m_requester.ToMessage(message);

            message.Append(m_id);

            m_facts.ToMessage( message);
        }

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public static CertificateRequest FromMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }
            if (message.Identifier != MessageID)
            {
                throw new System.ArgumentException(
                    String.Format("Incorrect message type: {0} instead of {1}.", message.Identifier, MessageID));
            }

            MessageReader reader = new MessageReader(message);
            CertificateRequest result = new CertificateRequest(reader);
            reader.AssertAtEnd( "Extra data in message.");
            return result;
        }

        #endregion
    }
}