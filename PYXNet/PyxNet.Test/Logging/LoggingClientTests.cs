using System;
using NUnit.Framework;
using Pyxis.Utilities.Test;
using PyxNet.Logging;

namespace PyxNet.Test.Logging
{
    /// <summary>
    /// Unit tests for LoggingClient
    /// </summary>
    [TestFixture]
    public class LoggingClientTests
    {
        [Test]
        [Ignore("Trying to get autobuild to work.")]
        public void LoggingServiceUnavailable_ClientSendsMessage_FailsSilently()
        {
            using (StackTestHelper stacks = new StackTestHelper(
                StackTestHelper.Topology.One,
                StackTests.ShortInterval))
            {
                LoggingClient client = new LoggingClient( stacks.Stacks[0]);
                client.Send( "Dummy Topic", "A message that nobody will hear.");
            }
        }

        [Test]
        [Ignore("Trying to get autobuild to work.")]
        public void LoggingServiceOnline_ClientSendsMessage_MessageReceived()
        {
            Pyxis.Utilities.TraceTool t = new Pyxis.Utilities.TraceTool(false);

            using (StackTestHelper testHelper = new StackTestHelper(
                StackTestHelper.Topology.Two,
                StackTests.ShortInterval))
            {
                // create a certificate server on node 4 (a hub)
                LoggingService loggingService = new LoggingService(testHelper.Stacks[4]);

                bool messageReceived = false;
                testHelper.WaitForHashUpdates();
                loggingService.LoggedEventReceived +=
                    delegate(object sender, LoggingService.LoggedEventReceivedEventArgs args)
                    {
                        messageReceived = true;
                    };

                LoggingClient client = new LoggingClient(testHelper.Stacks[0]);
                client.Send("Test Topic", "A message that the server will receive.");
                TimedTest.Verify(delegate() { return messageReceived; }, TimeSpan.FromSeconds(5));

                Assert.IsTrue(messageReceived);
            }
        }
    }
}