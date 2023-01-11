using NUnit.Framework;

namespace PyxNet.Test
{
    [TestFixture]
    public class TcpNetworkTests
    {
        [Test]
        public void TestTCPIPFunctionality()
        {
            System.Net.IPEndPoint listenOn = new System.Net.IPEndPoint(
                System.Net.IPAddress.Parse("127.0.0.1"), 42044);
            NetworkAddress testAddress = new NetworkAddress(listenOn);
            using (TcpNetwork testNetwork = new TcpNetwork())
            {
                NetworkTests networkTests = new NetworkTests();
                networkTests.TestAnyNetwork(testNetwork, testAddress);
            }
        }

        [Test]
        public void TestTCPIPMultiThreadedSuitability()
        {
            System.Net.IPEndPoint listenOn = new System.Net.IPEndPoint(
                System.Net.IPAddress.Parse("127.0.0.1"), 42044);
            NetworkAddress testAddress = new NetworkAddress(listenOn);
            using (TcpNetwork testNetwork = new TcpNetwork())
            {
                NetworkMultiThreadedHammerTests networkTests = new NetworkMultiThreadedHammerTests();
                networkTests.HammerAnyNetwork(testNetwork, testAddress);
            }
        }

        [Test]
        public void TestTCPIPMultiThreadedSuitabilitySimplified()
        {
            System.Net.IPEndPoint listenOn = new System.Net.IPEndPoint(
                System.Net.IPAddress.Parse("127.0.0.1"), 42044);
            NetworkAddress testAddress = new NetworkAddress(listenOn);
            using (TcpNetwork testNetwork = new TcpNetwork())
            {
                NetworkSimplifiedMultiThreadedHammerTests networkTests = new NetworkSimplifiedMultiThreadedHammerTests();
                networkTests.HammerAnyNetwork(testNetwork, testAddress);
            }
        }
    }
}