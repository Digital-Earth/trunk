/******************************************************************************
DataChunkRequest.cs

begin      : 06/03/2007 12:24:59 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.DataHandling
{
    /// <summary>
    /// Ecapsulates the message to request part of a data set. "A Data Chunk"
    /// </summary>
    public class DataChunkRequest : ITransmissible
    {
        public const string MessageID = "DaCR";

        #region Properties
        /// <summary>
        /// Storage for the identity of the data set that this chunk belongs to.
        /// </summary>
        private DataGuid m_DataSetID = new DataGuid();

        /// <summary>
        /// The identity of the data set that this chunk belongs to.
        /// </summary>
        public DataGuid DataSetID
        {
            get { return m_DataSetID; }
            set { m_DataSetID = value; }
        }

        /// <summary>
        /// Storage for the position in the data that this chunk starts at.
        /// </summary>
        private int m_offset;

        /// <summary>
        /// The position in the data that this chunk starts at.
        /// </summary>
        public int Offset
        {
            get { return m_offset; }
            set { m_offset = value; }
        }

        /// <summary>
        /// Storage for the number of bytes of data that are stored in this chunk.
        /// </summary>
        private int m_chunkSize;

        /// <summary>
        /// The number of bytes of data that are stored in this chunk.
        /// </summary>
        public int ChunkSize
        {
            get { return m_chunkSize; }
            set { m_chunkSize = value; }
        }

        /// <summary>
        /// Set to true to request that the sender encrypts the data.
        /// </summary>
        /// <remarks>Unused!</remarks>
        private bool m_encrypt;

        /// <summary>
        /// Set to true to request that the sender encrypts the data.
        /// </summary>
        /// <remarks>Unused!</remarks>
        public bool Encrypt
        {
          get { return m_encrypt; }
          set { m_encrypt = value; }
        }

        /// <summary>
        /// Set to true to request that the sender signs the data.
        /// </summary>
        /// <remarks>Unused!</remarks>
        private bool m_sign;

        /// <summary>
        /// Set to true to request that the sender signs the data.
        /// </summary>
        /// <remarks>Unused!</remarks>
        public bool Sign
        {
          get { return m_sign; }
          set { m_sign = value; }
        }

        /// <summary>
        /// Storage for the data extra info
        /// </summary>
        private Message m_extraInfo = new Message();

        /// <summary>
        /// Extended information about what the data is to download.
        /// One use case would be the tile index of data to download when 
        /// the downloaded data is a tile from a coverage.
        /// </summary>
        public Message ExtraInfo
        {
            get { return m_extraInfo; }
            set { m_extraInfo = value; }
        }

        private Service.Certificate m_certificate;

        /// <summary>
        /// Gets or sets the (optional) certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public Service.Certificate Certificate
        {
            get { return m_certificate; }
            set { m_certificate = value; }
        }

        #endregion

        /// <summary>
        /// Construct a DataChunkRequest.
        /// </summary>
        public DataChunkRequest()
        {
        }

        /// <summary>
        /// Construct a DataChunkRequest initializing all components.
        /// </summary>
        /// <param name="id">The ID of the data set you are requesting data from.</param>
        /// <param name="offset">The starting position of the portion of the data you are requesting.</param>
        /// <param name="chunkSize">The amount of data you are requesting in bytes.</param>
        /// <param name="useEncryption">Set to true to ask for encrypted transmission.</param>
        /// <param name="useSigning">Set to true to ask for signed transmission.</param>
        public DataChunkRequest(DataGuid id, int offset, int chunkSize, 
            bool useEncryption, bool useSigning, Service.Certificate certificate)
        {
            DataSetID = id;
            m_offset = offset;
            m_chunkSize = chunkSize;
            m_encrypt = useEncryption;
            m_sign = useSigning;
            m_certificate = certificate;
        }

        /// <summary>
        /// Construct a DataChunkRequest from a message.  The message must be a 
        /// PyxNet DataChunkRequest message.
        /// </summary>
        /// <param name="message"></param>
        public DataChunkRequest(Message message)
        {
            FromMessage(message);
        }

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the DataChunkRequest.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the DataChunkRequest to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            DataSetID.ToMessage(message);
            message.Append(m_offset);
            message.Append(m_chunkSize);
            message.Append(m_encrypt);
            message.Append(m_sign);
            ExtraInfo.ToMessage(message);
            message.Append(m_certificate != null);
            if (m_certificate != null)
            {
                m_certificate.ToMessage(message);
            }
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(MessageID))
            {
                throw new System.ArgumentException(
                    "Message is not a DataChunkRequest message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a DataChunkRequest message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a DataChunkRequest.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            DataSetID.FromMessageReader(reader);
            m_offset = reader.ExtractInt();
            m_chunkSize = reader.ExtractInt();
            m_encrypt = reader.ExtractBool();
            m_sign = reader.ExtractBool();
            ExtraInfo = new Message(reader);
            if (reader.AtEnd)
            {
                m_certificate = null;
            }
            else
            {
                bool hasCertificate = reader.ExtractBool();
                if (hasCertificate)
                {
                    m_certificate = new Service.Certificate(reader);
                }
                else
                {
                    m_certificate = null;
                }
            }
        }
        #endregion
    }
}