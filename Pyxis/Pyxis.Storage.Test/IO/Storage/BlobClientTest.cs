using NUnit.Framework;
using Pyxis.Storage;
using Pyxis.Storage.BlobProviders;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Pyxis.Core.Test.IO.Blob
{
    [TestFixture]
    public class BlobClientTest
    {
        private List<IBlobProvider> m_blobProviders;

        [SetUp]
        public void SetUp()
        {
            var serverAddress = "http://storage-pyxis.azurewebsites.net";
            var connectionString = "DefaultEndpointsProtocol=https;AccountName=pyxisstorage;AccountKey=SaDuqL2iehi3T9IQmwi5dv+cXsKb/KeOTPD1U6Y5Aj2kiAqx1VQWxwjRfUbfuPfDTW8h4BdGub6Hiibi2mEuOw==";
            var tmpDir = Path.Combine(Path.GetTempPath(), "BlobTest");

            var memoryBlobProvider = new MemoryBlobProvider();
            var azureBlobProvider = new AzureBlobProvider(connectionString);
            var localBlobProvider = new LocalBlobProvider(tmpDir);
            var pyxisBlobProvider = new PyxisBlobProvider(serverAddress);
            var cachedBlobProvider = new CachedBlobProvider(serverAddress, tmpDir);
            var multiBlobProvider = new MultiProviderBlobProvider();
            multiBlobProvider.BlobProviders.Add(memoryBlobProvider);
            multiBlobProvider.BlobProviders.Add(localBlobProvider);

            //var tempFile = Path.Combine(tmpDir, "SingleFileBlob");
            //var singleFileBlobProvider = new SingleFileLocalBlobProvider(tempFile);
            //singleFileBlobProvider.Initialize();

            m_blobProviders = new List<IBlobProvider>()
            {
               //singleFileBlobProvider,
               //memoryBlobProvider,
               //localBlobProvider,
               //azureBlobProvider,
               // pyxisBlobProvider,    // leave disabled as this consumes a small amount of disk space for each test run
               //cachedBlobProvider,
               //multiBlobProvider
            };
        }

        [TearDown]
        public void TearDown()
        {
            foreach (var client in m_blobProviders.Where(x => x is IDisposable))
            {
                ((IDisposable)client).Dispose();
            }
        }

        [Test]
        public void MissingKeysTest()
        {
            foreach (var client in m_blobProviders)
            {
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
        }
        [Test]
        public void ManualChecks()
        {
           
        }

        [Test]
        public void UploadAndDownloadTest()
        {
            var data = new TestClass();
            data.Child = new TestClass();
            data.Text = "this is a test " + Guid.NewGuid();
            data.Number = 12;
            data.Child.Text = "Hello";
            foreach (var client in m_blobProviders)
            {
                var key = client.AddBlob(data);
                var downloaded = client.GetBlob<TestClass>(key);

                Assert.AreEqual(data.Text, downloaded.Text);
                Assert.AreEqual(data.Number, downloaded.Number);
                Assert.AreEqual(data.Child.Text, downloaded.Child.Text);
            }
        }

        [Test]
        public void FailedDownloadTest()
        {
            foreach (var client in m_blobProviders)
            {
                var key = Guid.NewGuid().ToString();
                var result = client.GetBlob(key, null);
                Assert.IsFalse(result);
                var read = client.GetBlob<string>(key);
                Assert.IsNull(read);
            }
        }

        [Test]
        public void UploadTest()
        {
            foreach (var client in m_blobProviders)
            {
                var guid = Guid.NewGuid();
                var data = guid.ToByteArray();
                var key = guid.ToString();
                using (var stream = new MemoryStream(data))
                {
                    var result1 = client.AddBlob(key, stream);
                    Assert.IsTrue(result1);
                    var result2 = client.AddBlob(key, stream);
                    Assert.IsFalse(result2);
                }
            }
        }

        private class TestClass
        {
            public string Text;
            public int Number;
            public TestClass Child;
        }
    }
}