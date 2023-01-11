using System;
using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    [TestFixture]
    public class CertificateServerTests
    {
        [Test]
        [Ignore("Trying to get autobuild to work.")]
        public void IntegrationTest()
        {
            Pyxis.Utilities.TraceTool t = new Pyxis.Utilities.TraceTool(false);

            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, 50))
            {
                // create a certificate server on node 4 (a hub)
                DemoCertificateServer certServer = new DemoCertificateServer(testHelper.Stacks[4]);
                testHelper.WaitForHashUpdates();
                // Wait a little longer, because we're reading our certificates out of the repository.
                System.Threading.Thread.Sleep(500);
                testHelper.WaitForHashUpdates();

                testHelper.Stacks[0].Tracer.Enabled = true;
                testHelper.Stacks[1].Tracer.Enabled = true;
                testHelper.Stacks[4].Tracer.Enabled = true;

                // Find the certificate server from node 0 (a node)
                ServiceFinder finder = new ServiceFinder(testHelper.Stacks[0]);

                ServiceInstance foundService = finder.FindService(
                    CertificateServer.CertificateAuthorityServiceId,
                    TimeSpan.FromSeconds(15));

                Assert.IsNotNull(foundService, "Unable to find the certificate service.");
            }
        }
    }
}