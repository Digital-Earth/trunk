using System;
using NUnit.Framework;
using PyxNet.GeoStreamServer;
using PyxNet.Service;

namespace PyxNet.Test.GeoStreamServer
{
    [TestFixture]
    public class GeoStreamServerServiceTests
    {
        [Test]
        [Ignore( "Trying to get autobuild to work.")]
        public void GeoStreamServerServiceIsFindable()
        {
            Pyxis.Utilities.TraceTool t = new Pyxis.Utilities.TraceTool(false);

            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, 50))
            {
                // create a certificate server on node 4 (a hub)
                GeoStreamServerService geoStreamServerService = new GeoStreamServerService(testHelper.Stacks[4]);

                testHelper.WaitForHashUpdates();
                System.Threading.Thread.Sleep(500);
                testHelper.WaitForHashUpdates();

                ServiceFinder finder = new ServiceFinder(testHelper.Stacks[0]);

                ServiceInstance foundService = finder.FindService(
                    GeoStreamServerService.GeoStreamServerServiceId,
                    TimeSpan.FromSeconds(15));

                Assert.IsNotNull(foundService, "Unable to find the GeoStreamServer service.");
            }
        }
    }
}