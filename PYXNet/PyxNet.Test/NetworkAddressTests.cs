using System.Collections.Generic;
using NUnit.Framework;

namespace PyxNet.Test
{
    /// <summary>
    /// Test Class to test network address.
    /// </summary>
    [TestFixture]
    public class NetworkAddressTests
    {
        /// <summary>
        /// Entry point for the test code.
        /// </summary>
        [Test]
        public void Test()
        {
            // Construct internal IP addresses.
            List<System.Net.IPEndPoint> internalAddresses = new List<System.Net.IPEndPoint>();
            internalAddresses.Add(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 34044));
            internalAddresses.Add(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 34045));
            internalAddresses.Add(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.15"), 34046));

            // Construct external IP addresses.
            List<System.Net.IPEndPoint> externalAddresses = new List<System.Net.IPEndPoint>();
            externalAddresses.Add(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("128.99.0.32"), 34050));
            externalAddresses.Add(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("128.99.0.33"), 34051));

            // Construct with ip addresses.
            NetworkAddress addressFromIP = new NetworkAddress(internalAddresses);
            externalAddresses.ForEach(delegate(System.Net.IPEndPoint element)
            {
                addressFromIP.ExternalIPEndPoints.Add(element);
            });

            // Get the internal ip addresses, and check them.
            Assert.IsTrue(internalAddresses.Count == addressFromIP.InternalIPEndPoints.Count, 
                "Network address has the wrong number of internal IP addresses after construction.");
            for (int index = internalAddresses.Count - 1; index >= 0; --index)
            {
                System.Net.IPEndPoint ipAddressToCheck = addressFromIP.InternalIPEndPoints[index];
                Assert.IsTrue(ipAddressToCheck.Equals(internalAddresses[index]), 
                    "Network address has the wrong internal IP address after construction.");
            }

            // Get external ip addresses, and check them.
            Assert.IsTrue(externalAddresses.Count == addressFromIP.ExternalIPEndPoints.Count,
                "Network address has the wrong number of external IP addresses after construction.");
            for (int index = externalAddresses.Count - 1; index >= 0; --index)
            {
                System.Net.IPEndPoint ipAddressToCheck = addressFromIP.ExternalIPEndPoints[index];
                Assert.IsTrue(ipAddressToCheck.Equals(externalAddresses[index]),
                    "Network address has the wrong external IP address after construction.");
            }

            // Generate a message.
            Message message = new Message("test");
            addressFromIP.ToMessage(message);

            // Construct a new address from the message.
            MessageReader reader = new MessageReader(message);
            NetworkAddress addressFromMessage = new NetworkAddress(reader);

            // Assert that it's the same.
            Assert.IsTrue(addressFromMessage.Equals(addressFromIP), "They're not equal.");

            // Get the internal ip addresses, and check them.
            Assert.IsTrue(internalAddresses.Count == addressFromMessage.InternalIPEndPoints.Count,
                "Network address has the wrong number of internal IP addresses after reconstruction.");
            for (int index = internalAddresses.Count - 1; index >= 0; --index)
            {
                System.Net.IPEndPoint ipAddressToCheck = addressFromMessage.InternalIPEndPoints[index];
                Assert.IsTrue(ipAddressToCheck.Equals(internalAddresses[index]),
                    "Network address has the wrong internal IP address after reconstruction.");
            }

            // Get the external ip addresses, and check them.
            Assert.IsTrue(externalAddresses.Count == addressFromMessage.ExternalIPEndPoints.Count,
                "Network address has the wrong number of external IP addresses after reconstruction.");
            for (int index = externalAddresses.Count - 1; index >= 0; --index)
            {
                System.Net.IPEndPoint ipAddressToCheck = addressFromMessage.ExternalIPEndPoints[index];
                Assert.IsTrue(ipAddressToCheck.Equals(externalAddresses[index]),
                    "Network address has the wrong external IP address after reconstruction.");
            }
        }
    }
}