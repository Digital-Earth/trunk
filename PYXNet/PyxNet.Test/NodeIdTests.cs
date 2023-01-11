using System;
using NUnit.Framework;
using PyxNet.DLM;

namespace PyxNet.Test
{
    [TestFixture]
    public class NodeIdTests
    {
        private static Random s_randomizer = new Random();
        public static NodeId GenerateRandomNodeId()
        {
            NodeId result = new NodeId();
            result.PublicKey = new RSATool().PublicKey;
            return result;
        }

        [Test]
        public void BasicFunctionality()
        {
            NodeId one = GenerateRandomNodeId();
            NodeId two = GenerateRandomNodeId();
            Assert.AreNotEqual(one, two, "Two random nodes must not be equal");
            //Assert.AreNotEqual(one.GetHashCode(), two.GetHashCode());
            one.Identity = two.Identity;
            one.PublicKey = two.PublicKey;
            Assert.AreEqual(one, two);
            //Assert.AreEqual(one.GetHashCode(), two.GetHashCode());
        }

        [Test]
        public void MessageSerialization()
        {
            NodeId one = GenerateRandomNodeId();
            Message m = new Message("TEST");
            one.ToMessage(m);

            NodeId two = GenerateRandomNodeId();
            Assert.AreNotEqual(one, two);
            //Assert.AreNotEqual(one.GetHashCode(), two.GetHashCode());

            MessageReader r = new MessageReader(m);
            two.FromMessageReader(r);
            Assert.AreEqual(one, two);
            //Assert.AreEqual(one.GetHashCode(), two.GetHashCode());
        }
    }
}