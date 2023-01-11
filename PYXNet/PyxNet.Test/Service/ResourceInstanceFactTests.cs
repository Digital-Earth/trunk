using NUnit.Framework;
using Pyxis.Utilities;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    [TestFixture]
    public class ResourceInstanceFactTests
    {
        public static ManifestEntry CreateFakeManifestEntry()
        {
            return new ManifestEntry("Fake file name", "c:file.txt", "ssss", 1042);
        }

        /// <summary>
        /// Creates a fake resource instance fact.  (For testing purposes.)
        /// </summary>
        /// <returns></returns>
        public static ResourceInstanceFact CreateFakeResourceInstanceFact()
        {
            ManifestEntry manifestEntry = CreateFakeManifestEntry();
            ResourceInstanceFact result = new ResourceInstanceFact(manifestEntry);
            return result;
        }

        [Test]
        public void ConstructAndSerialize()
        {
            ResourceInstanceFact newFact = CreateFakeResourceInstanceFact();

            Message aMessage = new Message("XXXX");
            newFact.ToMessage(aMessage);
            MessageReader aReader = new MessageReader(aMessage);
            ResourceInstanceFact reconstructed = new ResourceInstanceFact(aReader);

            Assert.AreEqual( newFact, reconstructed,
                "The Resource was not the same after reconstruction.");
        }
    }
}