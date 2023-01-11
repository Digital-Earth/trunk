using System;

namespace PyxNet.Test
{
    [NUnit.Framework.TestFixture]
    public class StackConnectionRequestTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void Test()
        {
            NodeInfo fromNode = new NodeInfo();
            fromNode.Mode = NodeInfo.OperatingMode.Hub;
            fromNode.HubCount = 10;
            fromNode.LeafCount = 972;
            fromNode.NodeGUID = new Guid("C2BDECF2-8FEB-48f9-BEFA-FE9E76B53B92");
            fromNode.PublicKey = new PyxNet.DLM.PrivateKey().PublicKey;
            fromNode.Address = new NetworkAddress(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 42044));
            fromNode.FriendlyName = "Your Friendly Neighbourhood Hub";

            KnownHubList khl = KnownHubListTests.CreateKnownHubList();

            Guid toGuid = Guid.NewGuid();

            // Create an object.
            StackConnectionRequest original = new StackConnectionRequest(
                false, fromNode, khl, toGuid);

            // Convert it to a message.
            Message message = original.ToMessage();

            // Construct a new one from the message.
            StackConnectionRequest reconstructed = new StackConnectionRequest(message);

            // Ensure that the contents match.
            NUnit.Framework.Assert.IsTrue(original.IsPersistent == reconstructed.IsPersistent);
            NUnit.Framework.Assert.IsTrue(original.FromNodeInfo.Equals(reconstructed.FromNodeInfo));
            NUnit.Framework.Assert.IsTrue(original.FromKnownHubList.Equals(reconstructed.FromKnownHubList));
            NUnit.Framework.Assert.IsTrue(reconstructed.IsToNodeGUID(toGuid));
        }
    }
}