using System;
using System.Collections.Generic;
using NUnit.Framework;

namespace PyxNet.Test
{
    /// <summary>
    /// Class for testing the IPAddressAndPort class.
    /// </summary>
    [TestFixture]
    public class IpAddressAndPortTests
    {
        /// <summary>
        /// Entry point for the test code.
        /// </summary>
        [Test]
        public void TestIPEndPointDeserialization()
        {
            // Create some dummy IP end points.
            List<System.Net.IPEndPoint> endPoints = new List<System.Net.IPEndPoint>();
            endPoints.Add(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 44044));
            endPoints.Add(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 44045));
            endPoints.Add(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.15"), 44046));

            // Serialize them to string.
            String serialized = IPAddressAndPort.SerializeIPEndPoints(endPoints);

            // Deserialize them from string.
            List<System.Net.IPEndPoint> deserialized = IPAddressAndPort.DeserializeIPEndPoints(serialized);

            // Verify.
            Assert.AreEqual(endPoints, deserialized, "Original and deserialized end points are not the same.");
        }

        /// <summary>
        /// Tests deserialization from string.  Also conveniently shows us the format of such a string.
        /// </summary>
        [Test]
        public void DeserializeFromString()
        {
            string constantString =
                @"<ArrayOfIPAddressAndPort>
  <IPAddressAndPort>
    <IPAddress>127.0.0.15</IPAddress>
    <Port>44046</Port>
  </IPAddressAndPort>
</ArrayOfIPAddressAndPort>";

            // Deserialize them from string.
            List<System.Net.IPEndPoint> deserialized = IPAddressAndPort.DeserializeIPEndPoints(constantString);

            // Verify.
            Assert.AreEqual( 1, deserialized.Count, "Error reading back a single setting.");
            Assert.AreEqual(44046, deserialized[0].Port, "Read an incorrect port.");
            Assert.AreEqual("127.0.0.15", deserialized[0].Address.ToString(), "Read an incorrect address.");
        }
    }
}