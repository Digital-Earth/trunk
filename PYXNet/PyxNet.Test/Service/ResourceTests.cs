using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    //[TestFixture]
    public class ResourceTests
    {
        /// <summary>
        /// Test creating a Resource and move to and from a message.
        /// </summary>
        [Test]
        public void TestResourceConstruction()
        {
            ResourceId test = new ResourceId();

            Message aMessage = new Message("XXXX");
            test.ToMessage(aMessage);
            MessageReader aReader = new MessageReader(aMessage);
            ResourceId reconstructed = new ResourceId(aReader);

            Assert.IsTrue(test == reconstructed,
                "The Resource was not the same after reconstruction.");
        }

        /// <summary>
        /// Test the copy constructor for Resource.
        /// </summary>
        [Test]
        public void TestResourceCopyConstruction()
        {
            ResourceId test = new ResourceId();
            ResourceId reconstructed = new ResourceId(test);

            Assert.IsTrue(test == reconstructed,
                "The Resource was not the same after copy.");
        }
    }
}