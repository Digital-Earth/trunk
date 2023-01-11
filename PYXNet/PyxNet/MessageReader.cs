using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// This is a helper class used to sequentially read elements out of 
    /// a message.
    /// </summary>
    public class MessageReader
    {
        /// <summary>
        /// the message that we are parsing
        /// </summary>
        private Message m_message;

        /// <summary>
        /// The position of the next element to parse.
        /// This is initialized to be just after the Message ID block.
        /// </summary>
        private int m_position = 4;

        public readonly Pyxis.Utilities.NumberedTraceTool<MessageReader> Tracer =
            new Pyxis.Utilities.NumberedTraceTool<MessageReader>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="message">The message that you wish to parse.</param>
        public MessageReader(Message message)
        {
            m_message = message;
        }

        /// <summary>
        /// True if the message reader is positioned at the end of the message it is reading from.
        /// </summary>
        public bool AtEnd
        {
            get
            {
                return (m_position == m_message.Length);
            }
        }

        /// <summary>
        /// Asserts that the current reader is at the end.
        /// </summary>
        /// <param name="context">The context.</param>
        public void AssertAtEnd( string context)
        {
            if (!AtEnd)
            {
                throw new ArgumentException(context);
            }
        }

        /// <summary>
        /// Asserts that the current reader is at the end.
        /// </summary>
        public void AssertAtEnd()
        {
            AssertAtEnd( "Message contains extra (unexpected) data.");
        }

        /// <summary>
        /// Utility function to check for enough message left to parse.
        /// </summary>
        /// <param name="sizeOfNextElement"></param>
        private bool EnoughMessage(int sizeOfNextElement)
        {
            return ((m_position + sizeOfNextElement) <= m_message.Length);
        }

        /// <summary>
        /// Get a 4 byte integer out of the message.
        /// </summary>
        /// <returns>The integer value that is next in the message.</returns>
        public int ExtractInt()
        {
            const int parseSize = sizeof(int);
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            int returnVal = m_message.ExtractInt(m_position);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Get an 8 byte integer out of the message.
        /// </summary>
        /// <returns>The 64 bit integer value that is next in the message.</returns>
        public Int64 ExtractInt64()
        {
            const int parseSize = sizeof(Int64);
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            Int64 returnVal = m_message.ExtractInt64(m_position);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Get a char value out of the message.
        /// </summary>
        /// <returns>The char that is next in the message.</returns>
        public char ExtractChar()
        {
            const int parseSize = 1;
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            char returnVal = m_message.ExtractChar(m_position);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Get a bool value out of the message.
        /// </summary>
        /// <returns>The bool that is next in the message.</returns>
        public bool ExtractBool()
        {
            const int parseSize = 1;
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            bool returnVal = m_message.ExtractBool(m_position);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Get an unsigned 16-bit integer out of the message.
        /// </summary>
        /// <returns>The unsigned 16-bit integer that is next in the message.</returns>
        public UInt16 ExtractUInt16()
        {
            const int parseSize = sizeof(UInt16);
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            UInt16 returnVal = m_message.ExtractUInt16(m_position);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Get an unsigned 32-bit integer out of the message.
        /// </summary>
        /// <returns>The unsigned 32-bit integer that is next in the message.</returns>
        public UInt32 ExtractUInt32()
        {
            const int parseSize = sizeof(UInt32);
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            UInt32 returnVal = m_message.ExtractUInt32(m_position);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Get a byte out of the message.
        /// </summary>
        /// <returns>The byte that is next in the message.</returns>
        public byte ExtractByte()
        {
            const int parseSize = sizeof(byte);
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            byte returnVal = m_message.ExtractByte(m_position);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Get a byte array out of the message.
        /// </summary>
        /// <param name="count">The number of bytes to read.</param>
        /// <returns>The byte array that is next in the message.</returns>
        public byte[] ExtractBytes(int count)
        {
            int parseSize = (sizeof(byte) * count);
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            byte[] returnVal = m_message.ExtractBytes(m_position, count);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Get a byte array out of the message, where the byte array is 
        /// preceded by a count (int) of the number of bytes to read.
        /// </summary>
        /// <returns>The byte array that is next in the message.</returns>
        public byte[] ExtractCountedBytes()
        {
            int count = ExtractInt();
            return ExtractBytes( count);
        }

        /// <summary>
        /// Get a Guid value out of the message.
        /// </summary>
        /// <returns>The Guid that is next in the message.</returns>
        public Guid ExtractGuid()
        {
            const int parseSize = 16;
            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }
            Guid returnVal = m_message.ExtractGuid(m_position);
            m_position += parseSize;
            return returnVal;
        }

        /// <summary>
        /// Extract a UTF8 string from the message where the length of the string 
        /// data is stored at the current message position.  This is compatible with
        /// the Message.Append(string) function.
        /// </summary>
        /// <remarks>This is not analagous to the ExtractUTF8 base method function
        /// that is defined in message.  This function needs the length encoded as an 
        /// integer value stored at the current position, whereas the method defined in
        /// the message class is passed in a length of the string data to parse.
        /// Message.ExtractUTF8(int, int) is used by this method to do the extraction.
        /// </remarks>
        /// <returns>The UTF8 decoded string.</returns>
        public string ExtractUTF8()
        {
            Tracer.DebugWriteLine("ExtractUTF8: Starting.");

            int parseSize = ExtractInt();
            Tracer.DebugWriteLine("ExtractUTF8: Extracted int.");

            if (parseSize == 0)
            {
                return "";
            }

            if (!EnoughMessage(parseSize))
            {
                throw new IndexOutOfRangeException("Parsing outside of the message contents.");
            }

            string returnVal = m_message.ExtractUTF8(m_position, parseSize);
            Tracer.DebugWriteLine("ExtractUTF8: Called ExtractUTF8 on message; returned string.");

            m_position += parseSize;

            Tracer.DebugWriteLine("ExtractUTF8: Finished.");
            return returnVal;
        }

        /// <summary>
        /// Extracts an XML object of the given type from the message.
        /// </summary>
        /// <returns>The object that was deserialized.</returns>
        public ReturnType ExtractXmlObject<ReturnType>() where ReturnType : class
        {
            string xmlBuffer = ExtractUTF8();
            xmlBuffer = xmlBuffer.Trim();
            xmlBuffer = xmlBuffer.Trim('\0');

            return Pyxis.Utilities.XmlTool.FromXml<ReturnType>(xmlBuffer);
        }

        /// <summary>
        /// Extracts an XML object from the message.
        /// </summary>
        /// <param name="objectType">The type of the object to extract.</param>
        /// <param name="extendedTypemap">The extended type map passed to the XmlSerializer.</param>
        /// <returns>The object that was deserialized.</returns>
        public object ExtractXmlObject(Type objectType, Type[] extendedTypemap)
        {
            Tracer.DebugWriteLine("ExtractXmlObject: Extracting object type: {0}", objectType);
#if DEBUG
            for (int typeIndex = 0; typeIndex < extendedTypemap.Length; ++typeIndex)
            {
                Tracer.DebugWriteLine("ExtractXmlObject: Extracting extended type map {0}: {1}",
                    typeIndex, extendedTypemap[typeIndex]);
            }
#endif

            // Grab the XML string
            Tracer.DebugWriteLine("ExtractXmlObject: Calling ExtractUTF8...");
            string xmlBuffer = ExtractUTF8();
            xmlBuffer = xmlBuffer.Trim();
            xmlBuffer = xmlBuffer.Trim('\0');

            Tracer.DebugWriteLine("ExtractXmlObject: Creating string reader for xml buffer string...");
            System.IO.StringReader input = new System.IO.StringReader(xmlBuffer);

            Tracer.DebugWriteLine("ExtractXmlObject: Creating Xml serializer...");
            System.Xml.Serialization.XmlSerializer s;
            try
            {
                // The XmlSerializer constructor can fail if the Windows temp directory doesn't exist,
                // or there are insufficient permissions.  Details here:
                // http://msdn.microsoft.com/en-us/library/aa302290.aspx
                s = new System.Xml.Serialization.XmlSerializer(objectType, extendedTypemap);
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine(e.Message);
                throw;
            }

            Tracer.DebugWriteLine("ExtractXmlObject: Deserializing...");
            try
            {
                DateTime startTime = DateTime.Now;
                object result = s.Deserialize(input);
                TimeSpan deserializingTime = startTime - DateTime.Now;
                Pyxis.Utilities.GlobalPerformanceCounters.Counters.MillisecondsDeserializingXML.RawValue = 
                    (long) deserializingTime.TotalMilliseconds;

                Tracer.DebugWriteLine("ExtractXmlObject: Finished deserializing.");
                return result;
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine(e.Message);
                throw;
            }
        }
    }
}
