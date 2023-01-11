using NUnit.Framework;

namespace PyxNet.Test
{
    [TestFixture]
    public class StatusMessageTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [Test]
        public void Serialization()
        {
            StatusMessage statusMessage = new StatusMessage { Text = "This is a little test." };

            // Convert it to a message.
            Message encodedMessage = statusMessage.ToMessage();

            // Construct a new one from the message.
            StatusMessage reconstructed = new StatusMessage(encodedMessage);

            Assert.AreEqual( statusMessage.Text, reconstructed.Text);
        }
    }
}