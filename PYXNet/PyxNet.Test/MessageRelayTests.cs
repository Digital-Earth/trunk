using System;

namespace PyxNet.Test
{
    [NUnit.Framework.TestFixture]
    public class MessageRelayTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void Test()
        {
            Message relayMessage = new Message("FUNK");
            relayMessage.Append("Get on up");

            Guid toGuid = Guid.NewGuid();

            // Create an object.
            MessageRelay original = new MessageRelay(relayMessage, toGuid);

            // Convert it to a message.
            Message message = original.ToMessage();

            // Construct a new one from the message.
            MessageRelay reconstructed = new MessageRelay(message);

            // Ensure that the contents match.
            NUnit.Framework.Assert.IsTrue(original.Guid.Equals(reconstructed.Guid));
            NUnit.Framework.Assert.IsTrue(original.RelayedMessage.Equals(reconstructed.RelayedMessage));
            NUnit.Framework.Assert.IsTrue(reconstructed.ToNodeGuid.Equals(toGuid));
        }
    }
}