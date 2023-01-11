using System;
using System.Linq;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates a network message.
    /// </summary>
    [Serializable]
    public class Message : IComparable, ITransmissible
    {
        /// <summary>
        /// This is the stream that contains all the bytes of the message.
        /// This could also have been implemented as a List<byte>,
        /// but MemoryStream provides access to the underlying byte array,
        /// removing the need for extra copying to populate our own byte array
        /// to return with the Array property.
        /// </summary>
        private System.IO.MemoryStream m_memoryStream;

        #region Constructors
        /// <summary>
        /// Construct a new message.
        /// </summary>
        public Message()
        {
            m_memoryStream = new System.IO.MemoryStream();
        }

        /// <summary>
        /// Construct a new message.
        /// </summary>
        public Message(int bufferSize)
        {
            m_memoryStream = new System.IO.MemoryStream(bufferSize);
        }

        /// <summary>
        /// Construct a new message containing a copy of the specified bytes.
        /// </summary>
        /// <param name="bytes">The byte array to be copied into the message.</param>
        public Message(byte[] bytes)
        {
            m_memoryStream = new System.IO.MemoryStream(bytes.Length);
            m_memoryStream.Write(bytes, 0, bytes.Length);
        }

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="copyMe">The message to be copied to the new message.</param>
        public Message(Message copyMe)
        {
            m_memoryStream = new System.IO.MemoryStream(copyMe.Length);
            m_memoryStream.Write(copyMe.m_memoryStream.GetBuffer(), 0, copyMe.Length);
        }

        /// <summary>
        /// Construct a new message with the header values plugged in.
        /// </summary>
        /// <param name="id">The message identifier.</param>
        public Message(string id)
            : this(4) // Typically, a 4-byte message ID will be passed.
        {
            AppendRaw(id);
        }

        /// <summary>
        /// Construct from a message reader.  The message reader
        /// should be properly set to point at the start of a complete message.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public Message(MessageReader reader) : this()
        {
            int length = reader.ExtractInt();
            Append(reader.ExtractBytes(length));
        }
        #endregion

        #region Comparison and Equality
        /// <summary>
        /// IComparable.CompareTo implementation.
        /// </summary>
        /// <param name="comp">The object that you want to compare with.</param>
        /// <returns> less than zero, zero, or greater than zero</returns>
        public int CompareTo(object comp)
        {
            Message message = comp as Message;
            if (message != null)
            {
                return CompareTo(message);
            }

            throw new System.ArgumentException("object is not a Message");
        }

        /// <summary>
        /// IComparable.CompareTo implementation for a Message type.
        /// </summary>
        /// <param name="message">The message that you want to compare with.</param>
        /// <returns>compares the internal memory stream's contents</returns>
        public int CompareTo(Message message)
        {
            if (message == null)
            {
                // null values will go to the end of the list.
                return -1;
            }

            long thisLength = m_memoryStream.Length;
            long messageLength = message.m_memoryStream.Length;

            for (long index = 0; index < thisLength && index < messageLength; ++index)
            {
                byte thisByte = Bytes.Array[index];
                byte messageByte = message.Bytes.Array[index];

                if (thisByte != messageByte)
                {
                    return thisByte - messageByte;
                }
            }

            // because we only return an int, and the length is a long we must 
            // do the calculations using long, and then decide on our return value.
            long difference = thisLength - messageLength;
            if (difference < 0)
            {
                return -1;
            }
            if (difference > 0)
            {
                return 1;
            }
            return 0;
        }

        /// <summary>
        /// Check for equality of contents.
        /// </summary>
        /// <param name="message"></param>
        /// <returns>True if the messages contain the same bytes.</returns>
        public override bool Equals(object message)
        {
            return Equals(message as Message);
        }

        /// <summary>
        /// Check for equality of contents.
        /// </summary>
        /// <param name="message"></param>
        /// <returns>True if the messages contain the same bytes.</returns>
        public bool Equals(Message message)
        {
            if (null == message)
            {
                return false;
            }

            long length = m_memoryStream.Length;

            if (length != message.m_memoryStream.Length)
            {
                return false;
            }

            for (long index = length - 1; 0 <= index; --index)
            {
                if (Bytes.Array[index] != message.Bytes.Array[index])
                {
                    return false;
                }
            }

            return true;
        }

        /// <summary>
        /// Need to override GetHashCode() when you override Equals.  
        /// </summary>
        /// <returns>the hash code of the contained memory stream</returns>
        public override int GetHashCode()
        {
            return m_memoryStream.GetHashCode();
        }

        /// <summary>
        /// Grab the identifier out of a message.  This is defined as the first
        /// four bytes of the message.
        /// </summary>
        public string Identifier
        {
            get
            {
                if (m_memoryStream.Length < 4)
                {
                    return null;
                }
                return System.Text.Encoding.UTF8.GetString(m_memoryStream.GetBuffer(), 0, 4);
            }
        }

        /// <summary>
        /// See if the message starts with this string.  Uses UTF8 encoding.
        /// </summary>
        /// <param name="start">The string you hope to find at the start of the message.</param>
        /// <returns></returns>
        public bool StartsWith(string start)
        {
            byte[] startBytes = System.Text.Encoding.UTF8.GetBytes(start);
            int startByteCount = startBytes.Length;

            if (m_memoryStream.Length < startByteCount)
            {
                return false;
            }
            return startBytes.SequenceEqual(m_memoryStream.GetBuffer().Take(startByteCount));           
        }
        #endregion
        
        /// <summary>
        /// The underlying array segment that contains the message.
        /// </summary>
        public ArraySegment<byte> Bytes
        {
            get
            {
                byte[] buffer = m_memoryStream.GetBuffer();

                if (m_memoryStream.Length < buffer.Length)
                {
                    return new ArraySegment<byte>(buffer, 0, (int)m_memoryStream.Length);
                }

                return new ArraySegment<byte>(buffer);
            }
        }

        /// <summary>
        /// The number of bytes that are contained in this message.
        /// </summary>
        public int Length
        {
            get
            {
                return (int)m_memoryStream.Length;
            }
        }

        /// <summary>
        /// A string form of the message for [de]serialization.
        /// </summary>
        public string SerializationString
        {
            get
            {
                return Pyxis.Utilities.UUEncoder.Encode(Bytes);
            }
            set
            {
                byte[] bytes = Pyxis.Utilities.UUEncoder.Decode(value);
                m_memoryStream = new System.IO.MemoryStream(bytes.Length);
                m_memoryStream.Write(bytes, 0, bytes.Length);
            }
        }

        /// <summary>
        /// Dump the message in a human readable form as the raw bytes of the message.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            ArraySegment<byte> byteValues = Bytes;
            System.Text.StringBuilder returnValue = new System.Text.StringBuilder("Message Size : ");
            returnValue.Append(byteValues.Count);
            if (byteValues.Count > 0)
            {
                returnValue.AppendLine();
                returnValue.AppendLine(" Values : ");
                int lineFeedCounter = 0;
                System.Text.StringBuilder charRepresentation = new System.Text.StringBuilder();
                for (int index = 0; index < byteValues.Count; index++)
                {
                    returnValue.Append(byteValues.Array[index].ToString("d3"));
                    if (byteValues.Array[index] > (byte)'0' && byteValues.Array[index] < (byte)'z')
                    {
                        charRepresentation.Append((char)byteValues.Array[index]);
                    }
                    else
                    {
                        charRepresentation.Append(".");
                    }

                    lineFeedCounter++;
                    returnValue.Append(" ");
                    if (lineFeedCounter == 10)
                    {
                        returnValue.AppendLine(charRepresentation.ToString());
                        charRepresentation = new System.Text.StringBuilder();
                        lineFeedCounter = 0;
                    }
                }
            }
            return returnValue.ToString();
        }

        #region Appending to the message.
        /// <summary>
        /// Append an array segment to the message via copy.
        /// </summary>
        /// <param name="value">An array segment to copy the bytes from.</param>
        public void Append(ArraySegment<byte> value)
        {
            Append(value.Array, value.Offset, value.Count);
        }

        /// <summary>
        /// Append an array to the message via copy.
        /// </summary>
        /// <param name="value">An array segment to copy the bytes from.</param>
        public void Append(byte[] value)
        {
            Append(value, 0, value.Length);
        }

        /// <summary>
        /// Append a "formatted" array to the message by first appending the 
        /// length (as an int), then appending the actual array via copy.
        /// </summary>
        /// <param name="value">An array segment to copy the bytes from.</param>
        public void AppendCountedBytes(byte[] value)
        {
            Append(value.Length);
            Append(value, 0, value.Length);
        }

        /// <summary>
        /// Append a string to the message via copy using UTF8 encoding.
        /// This differs from the regular string append in that it does not 
        /// place the length of the string first.  It should be used only in
        /// situations where extraction can know ahead of time how much data
        /// is coming, or in test cases where extraction of the message is not
        /// needed.
        /// </summary>
        /// <param name="value">An string to append the bytes from.</param>
        public void AppendRaw(string value)
        {
            Append(System.Text.Encoding.UTF8.GetBytes(value));
        }

        /// <summary>
        /// Append an extractable string to the message via copy using UTF8 encoding,
        /// placing the length of the string as an integer first followed by the 
        /// UTF8 encoded data.
        /// </summary>
        /// <param name="value">An string to append the bytes from.</param>
        public void Append(string value)
        {
            if (null == value)
            {
                Append(0);
            }
            else
            {
                byte[] contentBytes = System.Text.Encoding.UTF8.GetBytes(value);
                Append(contentBytes.Length);
                Append(contentBytes);
            }
        }

        /// <summary>
        /// Append an integer to the message via copy.  This will add 4 bytes to the message.
        /// </summary>
        /// <param name="value">An integer to append the bytes from.</param>
        public void Append(int value)
        {
            Append(BitConverter.GetBytes(value));
        }
        
        /// <summary>
        /// Append a 64 bit integer to the message via copy.  This will add 8 bytes to the message.
        /// </summary>
        /// <param name="value">A 64 bit integer to append the bytes from.</param>
        public void Append(Int64 value)
        {
            Append(BitConverter.GetBytes(value));
        }

        /// <summary>
        /// Append an integer to the message via copy.  This will add 2 bytes to the message.
        /// </summary>
        /// <param name="value">A 16-bit integer to append the bytes from.</param>
        public void Append(UInt16 value)
        {
            Append(BitConverter.GetBytes(value));
        }

        /// <summary>
        /// Append an integer to the message via copy.  This will add 4 bytes to the message.
        /// </summary>
        /// <param name="value">A 32-bit integer to append the bytes from.</param>
        public void Append(UInt32 value)
        {
            Append(BitConverter.GetBytes(value));
        }

        /// <summary>
        /// Append a char as a byte to the message via copy.  This will add 1 byte to the message.
        /// </summary>
        /// <param name="value">A char to append.</param>
        public void Append(char value)
        {
            m_memoryStream.WriteByte((byte)value);
        }

        /// <summary>
        /// Append a bool as a byte to the message via copy.  This will add 1 byte to the message.
        /// A byte value of 1 will indicate true, and a 0 will indicate false.
        /// </summary>
        /// <param name="value">A bool to append.</param>
        public void Append(bool value)
        {
            m_memoryStream.WriteByte((byte)(value ? 1 : 0));
        }

        /// <summary>
        /// Append a byte to the message via copy.  This will add 1 byte to the message.
        /// </summary>
        /// <param name="value">A byte to append.</param>
        public void Append(byte value)
        {
            m_memoryStream.WriteByte(value);
        }

        /// <summary>
        /// Append an array segment to the message via copy.
        /// </summary>
        /// <param name="array">The array to copy bytes from.</param>
        /// <param name="offset">The offset at which to start.</param>
        /// <param name="count">The number of bytes to copy.</param>
        public void Append(byte[] array, int offset, int count)
        {
            m_memoryStream.Write(array, offset, count);
        }

        /// <summary>
        /// Append a Guid to the message.  16 bytes will be added to the message.
        /// </summary>
        /// <param name="value"></param>
        public void Append(Guid value)
        {
            byte[] guidBytes = value.ToByteArray();
            if (guidBytes.Length != 16)
            {
                throw new ArgumentException("Appending Guid with incorrect length.");
            }
            Append(guidBytes);
        }

        /// <summary>
        /// Appends the given object, using an XML representation.
        /// </summary>
        /// <param name="o">The object.</param>
        public void AppendXmlObject(object o)
        {
            Append(Pyxis.Utilities.XmlTool.ToXml(o));
        }

        /// <summary>
        /// Appends the given object, using an XML representation.
        /// </summary>
        /// <param name="o">The object.</param>
        /// <param name="extendedTypemap">The extended typemap.</param>
        public void AppendXmlObject(object o, Type[] extendedTypemap)
        {
            System.IO.StringWriter outputStream = new System.IO.StringWriter(
                new System.Text.StringBuilder());
            string result = "";

            System.Xml.Serialization.XmlSerializer s =
                new System.Xml.Serialization.XmlSerializer(o.GetType(), extendedTypemap);

            s.Serialize(outputStream, o);

            // To cut down on the size of the xml, we'll strip extraneous stuff.
            result = outputStream.ToString().Replace("xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"", "");
            result = result.Replace("xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"", "");
            result = result.Replace("<?xml version=\"1.0\" encoding=\"utf-16\"?>", "").Trim();

            Append(result);
        }

        #endregion

        #region Extracting from the message.
        /// <summary>
        /// Extract a 4 byte integer value from the message.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <returns>The integer value that is stored at offset in the message.</returns>
        public int ExtractInt(int offset)
        {
            if ((offset + sizeof(int) > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            return BitConverter.ToInt32(m_memoryStream.GetBuffer(), offset);
        }
        
        /// <summary>
        /// Extract an 8 byte integer value from the message.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <returns>The 64 bit integer value that is stored at offset in the message.</returns>
        public Int64 ExtractInt64(int offset)
        {
            if ((offset + sizeof(Int64) > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            return BitConverter.ToInt64(m_memoryStream.GetBuffer(), offset);
        }

        /// <summary>
        /// Extract a UInt16 value from the message.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <returns>The unsigned 16-bit integer that is stored at offset in the message.</returns>
        public UInt16 ExtractUInt16(int offset)
        {
            if ((offset + sizeof(UInt16) > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            return BitConverter.ToUInt16(m_memoryStream.GetBuffer(), offset);
        }

        /// <summary>
        /// Extract a UInt32 value from the message.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <returns>The unsigned 32-bit integer that is stored at offset in the message.</returns>
        public UInt32 ExtractUInt32(int offset)
        {
            if ((offset + sizeof(UInt32) > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            return BitConverter.ToUInt32(m_memoryStream.GetBuffer(), offset);
        }

        /// <summary>
        /// Extract a byte value from the message.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <returns>The byte that is stored at offset in the message.</returns>
        public byte ExtractByte(int offset)
        {
            if ((offset + sizeof(byte) > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            return m_memoryStream.GetBuffer()[offset];
        }

        /// <summary>
        /// Extract a byte array from the message.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <param name="count">The count of bytes to retrieve from the message.</param>
        /// <returns>The byte array is stored at offset in the message.</returns>
        public byte[] ExtractBytes(int offset, int count)
        {
            if ((offset + (sizeof(byte) * count) > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            byte[] bytes = new byte[count];
            Array.Copy(m_memoryStream.GetBuffer(), offset, bytes, 0, count);
            return bytes;
        }

        /// <summary>
        /// Extract a char value from the message.  This extracts a one byte value 
        /// from the message and casts it as a char. In effect this is treated like a 7 bit 
        /// ASCII value.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <returns>The char that is stored at offset in the message.</returns>
        public char ExtractChar(int offset)
        {
            if ((offset + 1 > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            return (char)m_memoryStream.GetBuffer()[offset];
        }

        /// <summary>
        /// Extract a bool value from the message.
        /// A byte value (coming from the message) of 1 will indicate true, 
        /// and a 0 will indicate false.
        /// </summary>
        /// <param name="value">The offset of the value to retrieve from the message.</param>
        /// <returns>The bool that is stored at offset in the message.</returns>
        public bool ExtractBool(int offset)
        {
            if ((offset + 1 > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            return m_memoryStream.GetBuffer()[offset] == 1;
        }

        /// <summary>
        /// Extract a UTF8 encoded string from the message that has been placed in the message
        /// with the Append(string value) method.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <returns>The string that is stored at the offset in the message.</returns>
        public string ExtractUTF8(int offset, int length)
        {
            // look to see if reading the string contents will be outside of the message.
            if ((offset + length) > m_memoryStream.Length)
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            // Grab the whole string...
            return System.Text.Encoding.UTF8.GetString(m_memoryStream.GetBuffer(), offset, length);
        }

        /// <summary>
        /// Extract a Guid value from the message.
        /// </summary>
        /// <param name="offset">The offset of the value to retrieve from the message.</param>
        /// <returns>The Guid that is stored at offset in the message.</returns>
        public Guid ExtractGuid(int offset)
        {
            const int GuidBytes = 16;
            if ((offset + GuidBytes > m_memoryStream.Length) ||
                (offset < 0))
            {
                throw new ArgumentException(
                    "Extracting outside of the message contents.");
            }

            byte[] guidBytes = new byte[GuidBytes];

            for (int index = 0; index < GuidBytes; ++index)
            {
                guidBytes[index] = m_memoryStream.GetBuffer()[index + offset];
            }

            return new Guid(guidBytes);
        }

        #endregion

        #region To/From Message
        /// <summary>
        /// Append the Message to an existing message.  
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(Length);
            message.Append(m_memoryStream.GetBuffer(), 0, Length);
        }

        public Message ToMessage()
        {
            return this;
        }

        #endregion
    }
}