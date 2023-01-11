using System;
using NUnit.Framework;

namespace PyxNet.Test
{
    /// <summary>
    /// Test class for StackConnectionCreator class.
    /// </summary>
    [TestFixture]
    public class StackConnectorTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [Test]
        public void TestStackConnector()
        {
            NodeInfo toNode = new NodeInfo();
            toNode.Mode = NodeInfo.OperatingMode.Hub;
            toNode.HubCount = 10;
            toNode.LeafCount = 972;
            toNode.NodeGUID = new Guid("C2BDECF2-8FEB-48f9-BEFA-FE9E76B53B92");
            toNode.PublicKey = new DLM.PrivateKey().PublicKey;
            toNode.Address = new NetworkAddress(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 42044));
            toNode.FriendlyName = "Your Friendly Neighbourhood Hub";

            bool isPersistent = true;

            // Create the object.
            StackConnector original = new StackConnector(toNode, isPersistent);

            // Convert it to a message.
            Message message = original.ToMessage();

            // Construct a new one from the message.
            StackConnector reconstructed = StackConnector.FromMessage(message);

            // Ensure that the new one contains the same information as the old one.
            Assert.IsTrue(original.ToNode.Equals(toNode));
            Assert.IsTrue(reconstructed.ToNode.Equals(toNode));
            Assert.IsTrue(original.IsPersistent == isPersistent);
            Assert.IsTrue(reconstructed.IsPersistent == isPersistent);
        }
    }
}