using NUnit.Framework;
using PyxNet.Logging;

namespace PyxNet.Test.Logging
{
    /// <summary>
    /// Testing for the class.
    /// </summary>
    [TestFixture]
    public class LoggedEventMessageTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void Messaging()
        {
            // Create the original object.
            LoggedEventMessage original = new LoggedEventMessage( "My Category", "A brief description");

            // Convert it to a message.
            Message message = original.ToMessage();

            // Construct a new one from the message.
            LoggedEventMessage reconstructed = LoggedEventMessage.FromMessage(message);

            // Ensure that the new one contains the same information as the old one.
            NUnit.Framework.Assert.AreEqual(original.Category, reconstructed.Category);
            NUnit.Framework.Assert.AreEqual(original.Description, reconstructed.Description);
        }
    }
}