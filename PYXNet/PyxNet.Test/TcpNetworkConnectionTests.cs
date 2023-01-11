using System;
using NUnit.Framework;

namespace PyxNet.Test
{
    [TestFixture]
    public class TcpNetworkConnectionTests
    {
        [Test]
        public void TestTcpNetworkConnection()
        {
            System.Net.IPEndPoint listenOn = new System.Net.IPEndPoint(
                System.Net.IPAddress.Parse("127.0.0.1"), 44052);
            NetworkAddress testAddress = new NetworkAddress(listenOn);
            using (TcpNetwork testNetwork = new TcpNetwork())
            {
                // Regular-sized message.
                var networkTests = new NetworkTests();
                var outgoingMessageSent = new Message(1024);
                outgoingMessageSent.Append("Hello there server.");
                var incomingMessageSent = new Message(1024);
                incomingMessageSent.Append("Hello there client.");
                networkTests.TestAnyNetwork(testNetwork, testAddress, outgoingMessageSent, incomingMessageSent);

                // Buffer-sized minus the length size message.
                networkTests = new NetworkTests();

                // The message plus the length will be a buffer size.
                int sizeOfTheLength = 4;
                int bufferLengthToTest = TcpNetworkConnection.BufferSize - sizeOfTheLength;

                char[] outgoingMessageBytes = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };
                int outgoingMessageByteCount = outgoingMessageBytes.Length;
                outgoingMessageSent = new Message(bufferLengthToTest);
                for (int index = 0; index < bufferLengthToTest; ++index)
                {
                    outgoingMessageSent.Append(outgoingMessageBytes[index % outgoingMessageByteCount]);
                }
                char[] incomingMessageBytes = { '9', '8', '7', '6', '5', '4', '3', '2', '1' };
                int incomingMessageByteCount = incomingMessageBytes.Length;
                incomingMessageSent = new Message(bufferLengthToTest);
                for (int index = 0; index < bufferLengthToTest; ++index)
                {
                    incomingMessageSent.Append(incomingMessageBytes[index % incomingMessageByteCount]);
                }
                networkTests.TestAnyNetwork(testNetwork, testAddress, outgoingMessageSent, incomingMessageSent);

                // A buffer sized message (add in the 4 bytes we didn't add in before).
                networkTests = new NetworkTests();
                for (int index = 0; index < sizeOfTheLength; ++index)
                {
                    incomingMessageSent.Append(incomingMessageBytes[index % incomingMessageByteCount]);
                    outgoingMessageSent.Append(outgoingMessageBytes[index % outgoingMessageByteCount]);
                }
                networkTests.TestAnyNetwork(testNetwork, testAddress, outgoingMessageSent, incomingMessageSent);

                // Larger-than-buffer message.
                networkTests = new NetworkTests();
                String outgoingMessageString = new String(outgoingMessageBytes);
                String incomingMessageString = new String(incomingMessageBytes);
                for (int i = 0; i < TcpNetworkConnection.BufferSize; ++i)
                {
                    outgoingMessageSent.Append(outgoingMessageString);
                    incomingMessageSent.Append(incomingMessageString);
                }
                networkTests.TestAnyNetwork(testNetwork, testAddress, outgoingMessageSent, incomingMessageSent);
            }
        }
    }
}