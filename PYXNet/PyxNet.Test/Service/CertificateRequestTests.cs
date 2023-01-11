using System;
using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    /// <summary>
    /// Testing for the class.
    /// </summary>
    [TestFixture]
    public class CertificateRequestTests
    {
        /// <summary>
        /// Create an object to test with.
        /// </summary>
        /// <returns>The new object.</returns>
        private CertificateRequest CreateCertificateRequest()
        {
            NodeInfo requester = new NodeInfo();
            requester.Mode = NodeInfo.OperatingMode.Hub;
            requester.HubCount = 10;
            requester.LeafCount = 972;
            requester.NodeGUID = new Guid("C2BDECF2-8FEB-48f9-BEFA-FE9E76B53B92");
            requester.PublicKey = new PyxNet.DLM.PrivateKey().PublicKey;
            requester.Address = new NetworkAddress(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 42044));
            requester.FriendlyName = "Your Friendly Neighbourhood Hub";

            ServiceId serviceId = new ServiceId(Guid.NewGuid(), Guid.NewGuid());
            ServiceInstance serviceInstance = ServiceInstance.Create(serviceId, requester.NodeId);
            ServiceInstanceFact fact = new ServiceInstanceFact(serviceInstance);

            return new CertificateRequest(new Guid(), requester, fact);
        }

        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestCertificateRequest()
        {
            // Create the original object.
            CertificateRequest original = CreateCertificateRequest();

            // Convert it to a message.
            Message message = original.ToMessage();

            // Construct a new one from the message.
            CertificateRequest reconstructed = CertificateRequest.FromMessage(message);

            // Ensure that the new one contains the same information as the old one.
            NUnit.Framework.Assert.AreEqual(original.Requester, reconstructed.Requester);
            NUnit.Framework.Assert.AreEqual(original.Id, reconstructed.Id);
            AssertFactListsAreEqual(original.FactList, reconstructed.FactList);
        }

        /// <summary>
        /// Asserts the fact lists are equal.
        /// </summary>
        /// <param name="first">The first.</param>
        /// <param name="second">The second.</param>
        private void AssertFactListsAreEqual(FactList first, FactList second)
        {
            foreach (ICertifiableFact fact in first.Facts)
            {
                bool found = false;
                foreach (ICertifiableFact secondFact in second.Facts)
                {
                    if (fact.Equals(secondFact))
                    {
                        found = true;
                    }
                }
                NUnit.Framework.Assert.IsTrue(found);
            }

            foreach (ICertifiableFact secondFact in second.Facts)
            {
                bool found = false;
                foreach (ICertifiableFact firstFact in first.Facts)
                {
                    if (firstFact.Equals(secondFact))
                    {
                        found = true;
                    }
                }
                NUnit.Framework.Assert.IsTrue(found);
            }
        }
    }
}