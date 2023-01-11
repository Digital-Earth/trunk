/******************************************************************************
FileChunk.cs

begin      : 13/02/2007 1:53:58 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace PyxNet.DataHandling
{
    /// <summary>
    /// A PyxNet message class that can read and write portions of a file,
    /// and translate them to and from PyxNet messages for transmission over a
    /// PyxNet network.
    /// </summary>
    public class DataChunk : ITransmissible
    {
        public const string MessageID = "DaCh";

        #region Data and Properties
        /// <summary>
        /// The information about the file that was read into this DataChunk.
        /// Not all DataChunks have an associated file.
        /// </summary>
        private FileInfo m_associatedFile;

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

        /// <summary>
        /// Storage for the data.
        /// </summary>
        private DataPackage m_data;

        /// <summary>
        /// The data for this chunk.
        /// This also contains the check sum of the data in MD5 format.
        /// </summary>
        public DataPackage Data
        {
            get { return m_data; }
        }
        #endregion

        #region Construction
        /// <summary>
        /// Construct a data chunk from a byte array.
        /// </summary>
        /// <param name="data">The chunk of data that you wish to encapsulate.</param>
        /// <param name="offset">The position in the complete data set that this chunk comes from.</param>
        public DataChunk(byte[] data, int offset)
        {
            m_offset = offset;
            m_chunkSize = data.Length;
            m_data = new DataPackage(data);
        }

        /// <summary>
        ///  Construct a data chunk from a portion of a file.
        /// </summary>
        /// <param name="fromFile">The file you wish to read.</param>
        /// <param name="offset">The position in the file that you wish to start reading.</param>
        /// <param name="chunkSize">The number of bytes that you want to read from the file.</param>
        public DataChunk(FileInfo fromFile, int offset, int chunkSize)
        {
            m_associatedFile = fromFile;
            m_offset = offset;
            m_chunkSize = chunkSize;
            ReadFileChunk();
        }

        /// <summary>
        /// Construct a DataChunk from a message.  The message must be a 
        /// PyxNet DataChunk message.
        /// </summary>
        /// <param name="message"></param>
        public DataChunk(Message message)
        {
            FromMessage(message);
        }
        #endregion

        #region I/O
        /// <summary>
        /// Used to initialize the chunk of data from a file.
        /// </summary>
        private void ReadFileChunk()
        {
            if (m_associatedFile == null)
            {
                return;
            }

            try
            {
                FileStream fileStream = m_associatedFile.OpenRead();
                fileStream.Seek(m_offset, SeekOrigin.Begin);
                byte[] readData = new byte[m_chunkSize];
                fileStream.Read(readData, 0, m_chunkSize);
                m_data = new DataPackage(readData);
            }
            catch
            {
                m_data = null;
            }
        }

        /// <summary>
        /// Write this chunk of data to a stream.  The stream must support seeking
        /// for proper positioning of the chunk.
        /// </summary>
        /// <param name="toStream">The stream to write.</param>
        /// <returns>True if the operation completed successfully.</returns>
        public bool WriteFileChunk(Stream toStream)
        {
            if (null == toStream)
            {
                throw new ArgumentNullException();
            }

            if (m_data == null || m_data.Data == null || m_data.Data.Length != m_chunkSize)
            {
                return false;
            }

            if (!toStream.CanSeek)
            {
                return false;
            }

            toStream.Seek(m_offset, SeekOrigin.Begin);

            toStream.Write(m_data.Data, 0, m_chunkSize);

            return true;
        }

        /// <summary>
        /// Write this chunk of data to a file.
        /// </summary>
        /// <param name="toFile">The file that will be written.</param>
        /// <returns>True if the operation completed successfully.</returns>
        public bool WriteFileChunk(FileInfo toFile)
        {
            return WriteFileChunk(toFile.Open(FileMode.OpenOrCreate, FileAccess.Write, FileShare.ReadWrite));
        }
        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the FileChunk.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the FileChunk to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            DataSetID.ToMessage(message);
            message.Append(m_offset);
            message.Append(m_chunkSize);
            ExtraInfo.ToMessage(message);
            m_data.ToMessage(message);
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
                    "Message is not a DataChunk message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd("Extra data in a DataChunk message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a DataChunk.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            DataSetID.FromMessageReader(reader);
            m_offset = reader.ExtractInt();
            m_chunkSize = reader.ExtractInt();
            ExtraInfo = new Message(reader);
            m_data = new DataPackage(reader);
        }
        #endregion
    }
}
