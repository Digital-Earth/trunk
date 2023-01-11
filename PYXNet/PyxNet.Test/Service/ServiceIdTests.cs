using System;
using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    [TestFixture]
    public class ServiceIdTests
    {
        /// <summary>
        /// Test creating a ServiceId and move to and from a message.
        /// </summary>
        [Test]
        public void TestServiceIdConstruction()
        {
            ServiceId test = new ServiceId();

            {// Scope to let us re-use the same variable names.
                Message aMessage = new Message("XXXX");
                test.ToMessage(aMessage);
                MessageReader aReader = new MessageReader(aMessage);
                ServiceId reconstructed = new ServiceId(aReader);

                Assert.IsTrue(test == reconstructed,
                    "The ServiceId was not the same after reconstruction.");
            }

            ServiceId secondId = new ServiceId(test.Guid, Guid.NewGuid());
            {// Scope to let us re-use the same variable names.
                Message aMessage = new Message("XXXX");
                secondId.ToMessage(aMessage);
                MessageReader aReader = new MessageReader(aMessage);
                ServiceId reconstructed = new ServiceId(aReader);

                Assert.IsTrue(secondId == reconstructed,
                    "The ServiceId with subservice id was not the same after reconstruction.");
            }
        }

        /// <summary>
        /// Test the copy constructor for ServiceId.
        /// </summary>
        [Test]
        public void TestServiceIdCopyConstruction()
        {
            {// Scope to let us re-use identifiers.
                ServiceId test = new ServiceId();
                ServiceId reconstructed = new ServiceId(test);

                Assert.IsTrue(test == reconstructed,
                    "The ServiceId was not the same after copy.");
            }

            {// Scope to let us re-use identifiers.
                ServiceId test = new ServiceId( Guid.NewGuid(), Guid.NewGuid());
                ServiceId reconstructed = new ServiceId(test);

                Assert.IsTrue(test == reconstructed,
                    "The ServiceId with subservice id was not the same after copy.");
            }
        }
    }
}