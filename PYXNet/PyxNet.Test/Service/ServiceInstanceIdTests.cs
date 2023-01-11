using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    [TestFixture]
    public class ServiceInstanceIdTests
    {
        /// <summary>
        /// Test creating a ServiceInstanceId and move to and from a message.
        /// </summary>
        [Test]
        public void TestServiceInstanceIdConstruction()
        {
            ServiceInstanceId test = new ServiceInstanceId();

            Message aMessage = new Message("XXXX");
            test.ToMessage(aMessage);
            MessageReader aReader = new MessageReader(aMessage);
            ServiceInstanceId reconstructed = new ServiceInstanceId(aReader);

            Assert.IsTrue(test == reconstructed,
                "The ServiceInstanceId was not the same after reconstruction.");
        }

        /// <summary>
        /// Test the copy constructor for ServiceInstanceId.
        /// </summary>
        [Test]
        public void TestServiceInstanceIdCopyConstruction()
        {
            ServiceInstanceId test = new ServiceInstanceId();
            ServiceInstanceId reconstructed = new ServiceInstanceId(test);

            Assert.IsTrue(test == reconstructed,
                "The ServiceInstanceId was not the same after copy.");
        }
    }
}