using Microsoft.Practices.Unity;
using NUnit.Framework;
using Pyxis.Core.IO.Storage;
using Pyxis.Core.IO.Storage.BlobClients;
using System;
using System.IO;

namespace Pyxis.Core.Test.IO.Blob
{
    [TestFixture]
    public class BlobClientTest
    {
        private static string s_serverAddress = "http://localhost:17071";
        private UnityContainer m_container;

        [SetUp]
        public void SetUp()
        {
            m_container = new UnityContainer();
            //m_container.RegisterInstance<IBlobProvider>(new BlobClient(s_serverAddress));
            m_container.RegisterInstance<IBlobProvider>(new MemoryBlobProvider());
        }

        [Test]
        public void MissingKeysTest()
        {
            var client = m_container.Resolve<IBlobProvider>();
            string data = "this is a test : " + Guid.NewGuid();
            string key = "";

            var firstKey = BlobKeyFactory.GenerateKey(data);

            Assert.IsFalse(client.BlobExists(firstKey));
            key = client.AddBlob(data);
            Assert.IsTrue(client.BlobExists(key));

            string downloaded;
            using (var readStream = new MemoryStream())
            {
                downloaded = client.GetBlob<string>(key);
                Assert.AreEqual(data, downloaded);
            }
        }

        [Test]
        public void UploadTest()
        {
            var client = m_container.Resolve<IBlobProvider>();
            var data = "this is a test";
            var key = client.AddBlob(data);
            var downloaded = client.GetBlob<string>(key);
            Assert.AreEqual(data, downloaded);
        }

        private class TestClass
        {
            public string text;
            public int a;
            public TestClass child;
        }
    }
}