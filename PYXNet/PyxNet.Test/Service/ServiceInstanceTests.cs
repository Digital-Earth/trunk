using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    [TestFixture]
    public class ServiceInstanceTests
    {
        [Test]
        public void CreateAndSerialize()
        {
            ServiceInstance s = CreateRandomServiceInstance();
            Message test = new Message( "TEST");
            s.ToMessage(test);
            Assert.AreNotEqual(0, test.Bytes.Count);

            // Exract from message.
            ServiceInstance extractedService = new ServiceInstance(
                new MessageReader(test));

            // Test equality.
            Assert.AreEqual(s.Server, extractedService.Server);
            Assert.AreEqual(s.ServiceId, extractedService.ServiceId);
            Assert.AreEqual(s.ServiceInstanceId, extractedService.ServiceInstanceId);
        }

        /// <summary>
        /// Creates a random service instance, using the given RSA credentials.  
        /// (For testing purposes only.)
        /// </summary>
        /// <param name="key">The key.</param>
        /// <returns></returns>
        internal static ServiceInstance CreateRandomServiceInstance(PyxNet.DLM.RSATool key)
        {
            NodeId node = new NodeId();
            node.PublicKey = key.PublicKey;

            ServiceInstance s = ServiceInstance.Create(
                new ServiceId(), node);
            return s;
        }

        /// <summary>
        /// Creates a random service instance.  (For testing purposes only.)
        /// </summary>
        /// <returns></returns>
        internal static ServiceInstance CreateRandomServiceInstance()
        {
            return CreateRandomServiceInstance(new PyxNet.DLM.RSATool());
        }
    }
}