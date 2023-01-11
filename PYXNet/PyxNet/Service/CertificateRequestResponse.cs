using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Service
{
    /// <summary>
    /// Encapsulates a PyxNet certificate request response.
    /// </summary>
    /// <remarks>TODO: Correct documentation of this file (refers to license still.)</remarks>
    public class CertificateRequestResponse : ITransmissible
    {
        /// <summary>
        /// The Message ID.
        /// </summary>
        public const string MessageID = "CReR";

        #region Fields and properties

        /// <summary>
        /// The identity of this request.  Used to identify the request in
        /// asynchronous responses.
        /// </summary>
        private readonly Guid m_id;

        /// <summary>
        /// Gets the identity of this request.  Used to identify the request in
        /// asynchronous responses.
        /// </summary>
        public Guid Id
        {
            get { return m_id; }
        }

        /// <summary>
        /// True if permitted to publish.
        /// </summary>
        private readonly bool m_permissionGranted;

        /// <summary>
        /// Gets true if permitted to publish.
        /// </summary>
        public bool PermissionGranted
        {
            get
            {
                return m_permissionGranted;
            }
        }

        /// <summary>
        /// The url of the site to visit for more license information.
        /// </summary>
        private readonly String m_url = "";

        /// <summary>
        /// Gets the url of the site to visit for more license information.
        /// </summary>
        public String Url
        {
            get
            {
                return m_url;
            }
        }

        private List<Service.Certificate> m_certificateList = 
            new List<Certificate>();

        /// <summary>
        /// Gets the certificate list.
        /// </summary>
        /// <value>The certificate list.</value>
        public List<Service.Certificate> CertificateList
        {
            get { return m_certificateList; }
        }

        private Dictionary<Guid, byte[]> m_remappings =
            new Dictionary<Guid, byte[]>();

        /// <summary>
        /// Gets the remappings.
        /// TODO: Currently, CertificateRequestResponse.Remappings is not used.
        /// </summary>
        public Dictionary<Guid, byte[]> Remappings
        {
            get { return m_remappings; }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Component constructor.
        /// </summary>
        /// <param name="metaData">The message containing the publishing metadata.</param>
        /// <param name="permissionGranted">A bool indicating whether permission was granted.</param>
        /// <param name="url">The url of a web page containing more information.</param>
        public CertificateRequestResponse(Guid requestId, 
            bool permissionGranted, String url,
            params Certificate[] certificates)
        {
            m_id = requestId;
            m_permissionGranted = permissionGranted;
            m_url = url;
            
            foreach (Certificate c in certificates)
            {
                m_certificateList.Add(c);
            }
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        /// <param name="reader">
        /// The message reader to construct from.
        /// This doesn't include the message header.
        /// </param>
        public CertificateRequestResponse(MessageReader reader)
        {
            m_id = reader.ExtractGuid();
            m_permissionGranted = reader.ExtractBool();
            m_url = reader.ExtractUTF8();
            
            int certificateCount = reader.ExtractInt();
            for (int c = 0; c < certificateCount; ++c)
            {
                Service.Certificate certificate =
                    new PyxNet.Service.Certificate(reader);
                m_certificateList.Add(certificate);
            }

            int remapCount = reader.ExtractInt();
            for (int r = 0; r < remapCount; ++r)
            {
                Guid from = reader.ExtractGuid();
                byte[] to = reader.ExtractCountedBytes();
                m_remappings.Add(from, to);
            }
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

            message.Append(m_id);
            message.Append(m_permissionGranted);
            message.Append(m_url);

            message.Append(this.CertificateList.Count);
            foreach (Service.Certificate c in this.CertificateList)
            {
                c.ToMessage(message);
            }

            message.Append(Remappings.Count);
            foreach (Guid from in Remappings.Keys)
            {
                message.Append(from);
                message.AppendCountedBytes(Remappings[from]);
            }
        }

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public static CertificateRequestResponse FromMessage(Message message)
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
            CertificateRequestResponse result = new CertificateRequestResponse(reader);
            reader.AssertAtEnd( "Extra data in message.");
            return result;
        }

        #endregion
    }
}

