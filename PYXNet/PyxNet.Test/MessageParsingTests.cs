using System;
using System.Text;
using NUnit.Framework;

namespace PyxNet.Test
{
    [TestFixture]
    public class MessageParsingTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [Test]
        public void TestMessageParser()
        {
            const int anInt = 33037;
            const char aChar = 'b';
            Guid aGuid = new Guid("22D8CA35-0CC0-4945-A6B1-1073AD9E47EA");

            Message testMessage = new Message("test");
            testMessage.Append(anInt);
            testMessage.Append(aChar);
            testMessage.Append(aGuid);
            int secondIntOffset = testMessage.Length;
            testMessage.Append(secondIntOffset);

            MessageReader parser = new MessageReader(testMessage);

            // they should all get back what we put in.
            Assert.IsTrue(parser.ExtractInt() == anInt);
            Assert.IsTrue(parser.ExtractChar() == aChar);
            Assert.IsTrue(parser.ExtractGuid() == aGuid);
            Assert.IsTrue(parser.ExtractInt() == secondIntOffset);
            
            // the reader should be at the end.
            Assert.IsTrue(parser.AtEnd, "The message reader was not at the end.");
        }

        [Serializable]
        public class SerializedClass
        {
            int m_Value = 42;

            public int Value
            {
                get { return m_Value; }
                set { m_Value = value; }
            }
        }

        [Test]
        public void XmlSerialization()
        {
            Type[] emptyTypemap = { };
            SerializedClass test = new SerializedClass();
            test.Value = 123;

            Message serializedMessage = new Message("XMLT");
            serializedMessage.AppendXmlObject(test, emptyTypemap);

            MessageReader reader = new MessageReader(serializedMessage);
            SerializedClass extractedObject = (SerializedClass)
                reader.ExtractXmlObject(typeof(SerializedClass), emptyTypemap);
            Assert.AreEqual(123, extractedObject.Value);
        }

        [Test]
        public void XmlFastSerialization()
        {
            SerializedClass test = new SerializedClass();
            test.Value = 123;

            Message serializedMessage = new Message("XMLT");
            serializedMessage.AppendXmlObject(test);

            MessageReader reader = new MessageReader(serializedMessage);
            SerializedClass extractedObject = 
                reader.ExtractXmlObject < SerializedClass>();
            Assert.AreEqual(123, extractedObject.Value);
        }

        [Test]
        [ExpectedException(typeof(IndexOutOfRangeException))]
        public void TestBufferProtection()
        {
            const int anInt = 12345;
            string longString = "This is a short test";
            byte[] plainBytes = Encoding.Unicode.GetBytes(longString.ToCharArray());

            Message testMessage = new Message("test");
            testMessage.Append(anInt);
            testMessage.Append(plainBytes);
            testMessage.Append(plainBytes);

            MessageReader parser = new MessageReader(testMessage);

            // they should all get back what we put in.
            Assert.AreEqual(parser.ExtractInt(), anInt);
            byte[] output = parser.ExtractBytes(plainBytes.Length);
            Assert.AreEqual(output, plainBytes);
            parser.ExtractByte();
            // this should throw!
            byte[] anotherOutput = parser.ExtractBytes(plainBytes.Length);
        }

        [Test]
        public void CountedBytes()
        {
            string longString = "This is a short test";
            byte[] plainBytes = Encoding.Unicode.GetBytes(longString.ToCharArray());

            Message testMessage = new Message("test");
            testMessage.AppendCountedBytes(plainBytes);

            MessageReader parser = new MessageReader(testMessage);

            // they should all get back what we put in.
            byte[] output = parser.ExtractCountedBytes();
            Assert.AreEqual(output, plainBytes);
        }
    }
}