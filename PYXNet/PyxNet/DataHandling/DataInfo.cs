/******************************************************************************
DataInfo.cs

begin      : 01/03/2007 12:32:08 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.DataHandling
{
    /// <summary>
    /// Ecapsulates the information that is returned when a DataInfoRequest message is sent.
    /// Contains all information about the data contained in a file.
    /// </summary>
    public class DataInfo : ITransmissible
    {
        public const string MessageID = "DaIn";

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
        /// Storage for the length of the data set.
        /// </summary>
        private long m_dataLength;

        /// <summary>
        /// The total size of the data set in bytes.
        /// </summary>
        public long DataLength
        {
            get { return m_dataLength; }
            set { m_dataLength = value; }
        }

        /// <summary>
        /// Storage for the Data Chunk Size.
        /// </summary>
        private int m_dataChunkSize;

        /// <summary>
        /// The size of data chunks that will be used to calculate hashes and indicate 
        /// data availablility.  This is not neccessarily the same as the size that will be used
        /// to transmit the data around the netork, but it is the smallest amount of data that
        /// a node can have before it can start sharing data.
        /// </summary>
        public int DataChunkSize
        {
            get { return m_dataChunkSize; }
            set { m_dataChunkSize = value; }
        }

        /// <summary>
        /// Set to true to require encrypted transmission.
        /// </summary>
        private bool m_useEncryption;

        // TODO: Get rid of this.  Unused.
        /// <summary>
        /// Set to true to require encrypted transmission.
        /// </summary>
        public bool UseEncryption
        {
            get { return m_useEncryption; }
            set { m_useEncryption = value; }
        }

        /// <summary>
        /// Set to true to require signed transmission.
        /// </summary>
        private bool m_useSigning;

        /// <summary>
        /// Set to true to require signed transmission.
        /// TODO: Get rid of this.  Unused.
        /// </summary>
        public bool UseSigning
        {
            get { return m_useSigning; }
            set { m_useSigning = value; }
        }

        // TODO:  what about licensing?  Does a use licensing belong at this level?
        // Possibly that address of the license server for this data.

        /// <summary>
        /// Set to true is you have all of this data available to transmit.
        /// </summary>
        private bool m_allAvailable;

        /// <summary>
        /// Set to true is you have all of this data available to transmit.
        /// </summary>
        public bool AllAvailable
        {
            get { return m_allAvailable; }
            set { m_allAvailable = value; }
        }

        private System.Collections.BitArray m_availableChunks;

        /// <summary>
        /// A picture of what portions of this data set are available.
        /// 
        /// If the AllAvailable property is true, then this data is indeterminant.
        /// Most likely it will be null.
        /// 
        /// If the AllAvailable property is false, then this array of bits is set so that
        /// if a bit is on (== 1) then there is data available for the coresponding 
        /// chunk.  For instance, if the DataLength is 100,000 and the ChunkSize is 26,000
        /// then AvailableChunks will be an array of 4 bits.  If bit number 0 == 1 then 
        /// data from 0 to 25,999 is available, otherwise ti is unavailable.  If bit number
        /// 1 == 1 then data from position 26,000 to 51,999 is available, and so on.
        /// </summary>
        public System.Collections.BitArray AvailableChunks
        {
            get { return m_availableChunks; }
            set { m_availableChunks = value; }
        }

        /// <summary>
        /// Set to true if this data is verified with hash codes.
        /// </summary>
        private bool m_usesHashCodes;

        /// <summary>
        /// Set to true if this data is verified with hash codes.
        /// </summary>
        public bool UsesHashCodes
        {
            get { return m_usesHashCodes; }
            set { m_usesHashCodes = value; }
        }

        /// <summary>
        /// Storage for the hash codes.
        /// </summary>
        private List<byte[]> m_chunkHashCodes;

        /// <summary>
        /// If the UseHashCodes property is set to true, then this contains
        /// an array of hash codes, one for each ChunkSize portion of the data.  The element
        /// count for this array (if it is in use) will be the same as the number of elements
        /// in the AvailableChunks array.
        /// </summary>
        public List<byte[]> ChunkHashCodes
        {
            get { return m_chunkHashCodes; }
            set { m_chunkHashCodes = value; }
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

        #endregion

        #region Construction

        /// <summary>
        /// Default Constructor
        /// </summary>
        public DataInfo()
        {
        }

        /// <summary>
        /// Construct a DataInfo from a message.  The message must be a 
        /// PyxNet DataInfo message.
        /// </summary>
        /// <param name="message"></param>
        public DataInfo(Message message)
        {
            FromMessage(message);
        }

        // Copy Constructor
        public DataInfo(DataInfo copyMe)
        {
            DataSetID = copyMe.DataSetID;
            DataLength = copyMe.DataLength;
            DataChunkSize = copyMe.DataChunkSize;
            UseEncryption = copyMe.UseEncryption;
            UseSigning = copyMe.UseSigning;
            UsesHashCodes = copyMe.UsesHashCodes;
            if (UsesHashCodes)
            {
                copyMe.ChunkHashCodes.ForEach(delegate(byte[] hashCode)
                {
                    ChunkHashCodes.Add(hashCode);
                });
            }
            AllAvailable = copyMe.AllAvailable;
            if (!AllAvailable)
            {
                AvailableChunks = new System.Collections.BitArray(copyMe.AvailableChunks);
            }
            ExtraInfo = new Message(copyMe.ExtraInfo);
        }

        #endregion

        #region Convert to/from message format

        /// <summary>
        /// Build a PyxNet message that contains the DataInfo.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the DataInfo to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            DataSetID.ToMessage(message);
            message.Append(DataLength);
            message.Append(DataChunkSize);
            message.Append(UseEncryption);
            message.Append(UseSigning);
            message.Append(UsesHashCodes);
            if (UsesHashCodes)
            {
                message.Append(ChunkHashCodes.Count);
                ChunkHashCodes.ForEach(delegate (byte[] hashCode) {
                    message.Append(hashCode.Length);
                    message.Append(hashCode);
                });
            }
            message.Append(AllAvailable);
            message.Append(null != AvailableChunks);
            if (null != AvailableChunks)
            {
                // write out the available chunks using a DataPackage to 
                // help in compressing to the message.
                int sizeInBytes = (AvailableChunks.Count + 7) >> 3;
                byte[] byteTable = new byte[sizeInBytes];
                AvailableChunks.CopyTo(byteTable, 0);
                DataPackage messagifier = new DataPackage(byteTable, true, false);
                messagifier.ToMessage(message);
            }
            ExtraInfo.ToMessage(message);
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
                    "Message is not a DataInfo message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a DataInfo message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a DataInfo.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            DataSetID.FromMessageReader(reader);
            DataLength = reader.ExtractInt64();
            DataChunkSize = reader.ExtractInt();
            UseEncryption = reader.ExtractBool();
            UseSigning = reader.ExtractBool();
            UsesHashCodes = reader.ExtractBool();
            ChunkHashCodes = new List<byte[]>();
            if (UsesHashCodes)
            {
                int numberToRead = reader.ExtractInt();
                for (int count = 0; count < numberToRead; count++)
                {
                    int size = reader.ExtractInt();
                    ChunkHashCodes.Add(reader.ExtractBytes(size));
                }
            }
            AllAvailable = reader.ExtractBool();
            bool readAvailability = reader.ExtractBool();
            if (readAvailability)
            {
                // read in the bit array of available chunks
                DataPackage demessagifier = new DataPackage(reader);
                m_availableChunks = new System.Collections.BitArray(demessagifier.Data);
            }
            ExtraInfo = new Message(reader);
        }

        #endregion
    }

    /// <summary>
    /// Ecapsulates the base class for the DataInfoRequest and DataNoInfo message.
    /// </summary>
    public class DataInfoIDBase
    {
        #region Properties
        /// <summary>
        /// Storage for the identity of the data set.
        /// </summary>
        private DataGuid m_DataSetID;

        /// <summary>
        /// The identity of the data set that this request pertains to.
        /// </summary>
        public DataGuid DataSetID
        {
            get { return m_DataSetID; }
            set { m_DataSetID = value; }
        }
        #endregion

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

        #region Construction
        /// <summary>
        /// Default constructor
        /// </summary>
        public DataInfoIDBase()
        {
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public DataInfoIDBase(DataGuid dataRequestID, Message extraInfo)
        {
            m_DataSetID = new DataGuid(dataRequestID);
            if (null != extraInfo)
            {
                m_extraInfo = extraInfo;
            }
        }
        #endregion

        #region Convert to/from message format

        /// <summary>
        /// Append the DataInfoBase to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            DataSetID.ToMessage(message);
            ExtraInfo.ToMessage(message);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a DataInfoRequest.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            DataSetID = new DataGuid(reader);
            ExtraInfo = new Message(reader);
        }
        #endregion
    }

    /// <summary>
    /// Ecapsulates the DataInfoRequest message.
    /// </summary>
    public class DataInfoRequest : DataInfoIDBase, ITransmissible
    {
        public const string MessageID = "DaRe";

        #region Construction
        /// <summary>
        /// Constructor
        /// </summary>
        public DataInfoRequest(DataGuid dataRequestID) :
            base(dataRequestID, null)
        {
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public DataInfoRequest(DataGuid dataRequestID, Message extraInfo) :
            base(dataRequestID, extraInfo)
        {
        }

        /// <summary>
        /// Construct a DataInfoRequest from a message.  The message must be a 
        /// PyxNet DataInfoRequest message.
        /// </summary>
        /// <param name="message"></param>
        public DataInfoRequest(Message message)
        {
            FromMessage(message);
        }
        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the DataInfoRequest.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
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
                    "Message is not a DataInfoRequest message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a DataInfoRequest message.");
        }
        #endregion
    }

    /// <summary>
    /// Ecapsulates the DataNoInfo message.
    /// </summary>
    public class DataNoInfo : DataInfoIDBase, ITransmissible
    {
        public const string MessageID = "DaNI";

        #region Construction
        /// <summary>
        /// Constructor
        /// </summary>
        public DataNoInfo(DataGuid dataRequestID) :
            base(dataRequestID, null)
        {
        }

        /// <summary>
        /// Construct a DataNoInfo from a message.  The message must be a 
        /// PyxNet DataNoInfo message.
        /// </summary>
        /// <param name="message"></param>
        public DataNoInfo(Message message)
        {
            FromMessage(message);
        }
        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the DataNoInfo.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
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
                    "Message is not a DataNoInfo message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a DataNoInfo message.");
        }
        #endregion
    }
}