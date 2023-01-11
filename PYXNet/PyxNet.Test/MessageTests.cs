using System;
using NUnit.Framework;
using Pyxis.Utilities.Test;

namespace PyxNet.Test
{
    [TestFixture]
    public class MessageTests
    {
        /// <summary>
        /// Tests construction of a message using every possible means.
        /// Tests each form of append.
        /// </summary>
        [Test]
        public void TestMessage()
        {
            // Empty message.
            {
                Message message = new Message();
                Assert.IsTrue(0 == message.Bytes.Count, "Message is wrong size.");
            }

            // Message with buffer size.
            {
                // Zero size.
                {
                    Message message = new Message(0);
                    Assert.IsTrue(0 == message.Bytes.Count, "Message is wrong size.");
                    Assert.IsTrue(0 == message.Bytes.Array.Length, "Message buffer is wrong size.");
                }

                // Normal size.
                {
                    int size = 1025;
                    Message message = new Message(size);
                    Assert.IsTrue(0 == message.Bytes.Count, "Message is wrong size.");
                    Assert.IsTrue(size == message.Bytes.Array.Length, "Message buffer is wrong size.");
                }
            }

            // Message from byte array.
            {
                byte[] byteArray = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
                Message message = new Message(byteArray);
                Assert.IsTrue(byteArray.Length == message.Bytes.Count, "Message is wrong size.");
                Assert.IsTrue(
                    CompareByteArrays(new ArraySegment<byte>(byteArray), message.Bytes),
                    "Message has wrong contents.");

                // Test appending a byte.
                message.Append((byte)11);
                byte[] newByteArray = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 11 };
                Assert.IsTrue(
                    message.Equals(new Message(newByteArray)),
                    "Message.Equals indicates that the messages are unequal.");

                // Test appending an integer.
                int testInt = 1000;
                byte[] testIntBytes = BitConverter.GetBytes(testInt);
                message.Append(testInt);
                Assert.IsTrue(
                    CompareByteArrays(
                        new ArraySegment<byte>(newByteArray),
                        new ArraySegment<byte>(message.Bytes.Array, 0, newByteArray.Length)),
                    "Message has wrong contents.");
                Assert.IsTrue(
                    CompareByteArrays(
                        new ArraySegment<byte>(testIntBytes),
                        new ArraySegment<byte>(message.Bytes.Array, newByteArray.Length, testIntBytes.Length)),
                    "Message has wrong contents.");

                // CompareTo.
                Assert.IsTrue(
                    (new Message(byteArray)).CompareTo(new Message(newByteArray)) < 0,
                    "Message.CompareTo returns the wrong value.");
                Assert.IsTrue(
                    (new Message(newByteArray)).CompareTo(new Message(byteArray)) > 0,
                    "Message.CompareTo returns the wrong value.");
                Assert.IsTrue(
                    0 == (new Message(byteArray)).CompareTo(new Message(byteArray)),
                    "Message.CompareTo returns the wrong value.");
                Assert.IsTrue(
                    0 == (new Message(newByteArray)).CompareTo(new Message(newByteArray)),
                    "Message.CompareTo returns the wrong value.");
                Assert.IsTrue(
                    (new Message(newByteArray)).CompareTo(null) < 0,
                    "Message.CompareTo returns the wrong value.");
            }

            // Create string containing non-ASCII and null characters.
            string text = "Null character \0 and strange character '\u4002'";

            // Encode it using an encoding incompatible with UTF-8.
            byte[] utf7Encoding = System.Text.Encoding.UTF7.GetBytes(text);
            ArraySegment<byte> utf7Segment = new ArraySegment<byte>(utf7Encoding);

            // Append byte array.
            {
                // Append to empty message.
                Message message = new Message();
                message.Append(utf7Encoding);
                Assert.IsTrue(
                    CompareByteArrays(utf7Segment, message.Bytes),
                    "Message has wrong contents.");

                // Append to existing message.
                message.Append(utf7Encoding);
                Assert.IsTrue(
                    CompareByteArrays(
                        utf7Segment,
                        new ArraySegment<byte>(message.Bytes.Array, 0, utf7Encoding.Length)),
                    "Message has wrong contents.");
                Assert.IsTrue(
                    CompareByteArrays(
                        utf7Segment,
                        new ArraySegment<byte>(message.Bytes.Array, utf7Encoding.Length, utf7Encoding.Length)),
                    "Message has wrong contents.");
            }

            // Append array segment.
            {
                // Append to empty message.
                Message message = new Message();
                message.Append(utf7Segment);
                Assert.IsTrue(
                    CompareByteArrays(utf7Segment, message.Bytes),
                    "Message has wrong contents.");

                // Append to existing message.
                message.Append(utf7Segment);
                Assert.IsTrue(
                    CompareByteArrays(
                        utf7Segment,
                        new ArraySegment<byte>(message.Bytes.Array, 0, utf7Segment.Count)),
                    "Message has wrong contents.");
                Assert.IsTrue(
                    CompareByteArrays(
                        utf7Segment,
                        new ArraySegment<byte>(message.Bytes.Array, utf7Segment.Count, utf7Segment.Count)),
                    "Message has wrong contents.");
            }

            // Append segment of array.
            {
                // Append to empty message.
                Message message = new Message();
                message.Append(utf7Encoding, 0, utf7Encoding.Length);
                Assert.IsTrue(
                    CompareByteArrays(utf7Segment, message.Bytes),
                    "Message has wrong contents.");

                // Append to existing message.
                message.Append(utf7Encoding, 0, utf7Encoding.Length);
                Assert.IsTrue(
                    CompareByteArrays(
                        utf7Segment,
                        new ArraySegment<byte>(message.Bytes.Array, 0, utf7Segment.Count)),
                    "Message has wrong contents.");
                Assert.IsTrue(
                    CompareByteArrays(
                        utf7Segment,
                        new ArraySegment<byte>(message.Bytes.Array, utf7Segment.Count, utf7Segment.Count)),
                    "Message has wrong contents.");
            }

            // Append string.
            {
                // Append to empty message.
                Message message = new Message();
                message.AppendRaw(text);
                byte[] utf8Bytes = System.Text.Encoding.UTF8.GetBytes(text);
                ArraySegment<byte> utf8Segment = new ArraySegment<byte>(utf8Bytes);
                Assert.IsTrue(
                    CompareByteArrays(
                        utf8Segment,
                        new ArraySegment<byte>(message.Bytes.Array, 0, utf8Segment.Count)),
                    "Message has wrong contents.");

                // Equals.
                Assert.IsTrue(
                    message.Equals(new Message(utf8Bytes)),
                    "Message.Equals indicates that the messages are unequal.");
                Assert.IsTrue(
                    new Message(utf8Bytes).Equals(message),
                    "Message.Equals indicates that the messages are unequal.");
                Assert.IsTrue(
                    message.Equals(new Message(utf8Bytes) as object),
                    "Message.Equals indicates that the messages are unequal.");
                Assert.IsTrue(
                    (new Message(utf8Bytes) as object).Equals(message),
                    "Message.Equals indicates that the messages are unequal.");

                // CompareTo.
                Assert.IsTrue(
                    0 == message.CompareTo(new Message(utf8Bytes)),
                    "Message.CompareTo indicates that the messages are unequal.");
                Assert.IsTrue(
                    0 == (new Message(utf8Bytes)).CompareTo(message),
                    "Message.CompareTo indicates that the messages are unequal.");

                // Append to existing message.
                message.AppendRaw(text);
                Assert.IsTrue(
                    CompareByteArrays(
                        utf8Segment,
                        new ArraySegment<byte>(message.Bytes.Array, 0, utf8Segment.Count)),
                    "Message has wrong contents.");
                Assert.IsTrue(
                    CompareByteArrays(
                        utf8Segment,
                        new ArraySegment<byte>(message.Bytes.Array, utf8Segment.Count, utf8Segment.Count)),
                    "Message has wrong contents.");
            }
        }

        public static bool CompareByteArrays(byte[] array1, byte[] array2)
        {
            int count = array1.Length;

            if (count != array2.Length)
            {
                return false;
            }

            for (int index = count - 1; index >= 0; --index)
            {
                if (array1[ index] != array2[ index])
                {
                    return false;
                }
            }

            return true;
        }

        public static bool CompareByteArrays(ArraySegment<byte> array1, ArraySegment<byte> array2)
        {
            int count = array1.Count;

            if (count != array2.Count)
            {
                return false;
            }

            for (int index = count - 1; index >= 0; --index)
            {
                if (array1.Array[array1.Offset + index] != array2.Array[array2.Offset + index])
                {
                    return false;
                }
            }

            return true;
        }

        /// <summary>
        /// Testing for method StartsWith in the Message class.
        /// </summary>
        [Test]
        public void TestMessageStartsWith()
        {
            const string shortString = "Bill";
            string longString = shortString + " Reid";
            string differentLongString = "not Bill Reid";

            Message shortMessage = new Message(shortString);
            Message longMessage = new Message(longString);
            Message differentLongMessage = new Message(differentLongString);

            // they should all start with what they were constructed with
            Assert.IsTrue(shortMessage.StartsWith(shortString));
            Assert.IsTrue(longMessage.StartsWith(longString));
            Assert.IsTrue(differentLongMessage.StartsWith(differentLongString));

            // the long string should start with the short string
            Assert.IsTrue(longMessage.StartsWith(shortString));

            // and we should not be able to mix and match
            Assert.IsFalse(shortMessage.StartsWith(longString));
            Assert.IsFalse(differentLongMessage.StartsWith(shortString));
            Assert.IsFalse(differentLongMessage.StartsWith(longString));
        }

        /// <summary>
        /// Testing for extraction methods in the Message class.
        /// </summary>
        [Test]
        public void TestMessageExtraction()
        {
            const int anInt = 34567;
            const char aChar = 'b';
            Guid aGuid = new Guid("22D8CA35-0CC0-4945-A6B1-1073AD9E47EA");

            Message aMessage = new Message("TEST");
            int intOffset = aMessage.Length;
            aMessage.Append(anInt);
            int charOffset = aMessage.Length;
            aMessage.Append(aChar);
            int guidOffset = aMessage.Length;
            aMessage.Append(aGuid);
            int secondIntOffset = aMessage.Length;
            aMessage.Append(secondIntOffset);

            // they should all get back what we put in.
            Assert.IsTrue(aMessage.ExtractInt(intOffset) == anInt);
            Assert.IsTrue(aMessage.ExtractChar(charOffset) == aChar);
            Assert.IsTrue(aMessage.ExtractGuid(guidOffset) == aGuid);
            Assert.IsTrue(aMessage.ExtractInt(secondIntOffset) == secondIntOffset);

            // try it in a different order
            Assert.IsTrue(aMessage.ExtractGuid(guidOffset) == aGuid);
            Assert.IsTrue(aMessage.ExtractChar(charOffset) == aChar);
            Assert.IsTrue(aMessage.ExtractInt(secondIntOffset) == secondIntOffset);
            Assert.IsTrue(aMessage.ExtractInt(intOffset) == anInt);
        }

        [Test]
        public void Identifier()
        {
            Message simpleMessage = new Message("TEST");
            Assert.AreEqual(simpleMessage.Identifier, "TEST");
        }

        [Test]
        public void ToFromMessage()
        {
            Message hostMessage = new Message("TEST");

            // create sub messages
            Message subMessage1 = new RandomMessage();
            Message subMessage2 = new RandomMessage();

            // add sub messages to the host
            subMessage1.ToMessage(hostMessage);
            subMessage2.ToMessage(hostMessage);

            MessageReader reader = new MessageReader(hostMessage);

            // read back sub messages from the host message
            Message copySubMessage1 = new Message(reader);
            Message copySubMessage2 = new Message(reader);

            Assert.IsTrue(AreEqual(subMessage1, copySubMessage1));
            Assert.IsTrue(AreEqual(subMessage2, copySubMessage2));
            Assert.IsFalse(AreEqual(subMessage1, copySubMessage2));
        }

        internal class RandomMessage : Message
        {
            internal RandomMessage()
                : this(TestData.GenerateInt( 100))
            {
            }

            internal RandomMessage(int messageSize)
                : base(TestData.GenerateBytes(messageSize))
            {
            }
        }

        internal static bool AreEqual(Message one, Message two)
        {
            return one.CompareTo(two) == 0;
        }
    }
}