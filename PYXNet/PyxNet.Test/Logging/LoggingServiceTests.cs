using System;
using NUnit.Framework;
using PyxNet.Logging;
using PyxNet.Service;

namespace PyxNet.Test.Logging
{
    [TestFixture]
    public class LoggingServiceTests
    {
        [Test]
        [Ignore("Trying to get autobuild to work.")]
        public void LoggingServiceIsFindable()
        {
            Pyxis.Utilities.TraceTool t = new Pyxis.Utilities.TraceTool(false);

            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, 50))
            {
                // create a certificate server on node 4 (a hub)
                LoggingService loggingService = new LoggingService( testHelper.Stacks[4]);

                testHelper.WaitForHashUpdates();
                System.Threading.Thread.Sleep(500);
                testHelper.WaitForHashUpdates();

                ServiceFinder finder = new ServiceFinder(testHelper.Stacks[0]);

                ServiceInstance foundService = finder.FindService(
                    LoggingService.LoggingServiceId,
                    TimeSpan.FromSeconds(15));

                Assert.IsNotNull(foundService, "Unable to find the logging service.");
            }
        }
    }
}