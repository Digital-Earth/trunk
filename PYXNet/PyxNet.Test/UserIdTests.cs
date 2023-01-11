using System;
using NUnit.Framework;
using PyxNet.DLM;

namespace PyxNet.Test
{
    [TestFixture]
    public class UserIdTests
    {
        private static Random s_randomizer = new Random();
        private UserId GenerateRandomUserId()
        {
            return new UserId(new RSATool().PublicKey);
        }

        [Test]
        public void BasicFunctionality()
        {
            UserId one = GenerateRandomUserId();
            UserId two = GenerateRandomUserId();
            Assert.AreNotEqual(one, two, "Two random user IDs must not be equal");
            one.PublicKey = two.PublicKey;
            Assert.AreEqual(one, two);
        }

        [Test]
        public void MessageSerialization()
        {
            UserId one = GenerateRandomUserId();
            Message m = new Message("TEST");
            one.ToMessage(m);

            UserId two = GenerateRandomUserId();
            Assert.AreNotEqual(one, two);

            UserId three = new UserId(new MessageReader(m));
            Assert.AreEqual(one, three);
        }
    }
}