using System;
using System.Collections.Generic;
using System.IO;
using NUnit.Framework;
using Pyxis.Contract.Publishing;

namespace StudioLauncher.Test
{
    [TestFixture]
    public class LocalVersionServerTester
    {
        private string m_baseDirectory;

        [TestFixtureSetUp]
        public void Setup()
        {
            m_baseDirectory = Path.Combine(Path.GetTempPath(), "localVersionServerTest");
            if (Directory.Exists(m_baseDirectory))
            {
                Directory.Delete(m_baseDirectory, true);
            }
            Directory.CreateDirectory(m_baseDirectory);
        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            Directory.Delete(m_baseDirectory, true);
        }
    }

    [TestFixture]
    public class StudioInstallerTester
    {
        [Test]
        public void TestProductVersionManager()
        {
            var outDatedVersionServer = new TestVersionServer();
            outDatedVersionServer.SetLatestVersion(
            new Product() { ProductVersion = new Version("0.0.1.00") }
           );

            var newerVersionServer = new TestVersionServer();
            newerVersionServer.SetLatestVersion(
            new Product() { ProductVersion = new Version("0.0.2.00") }
           );

            var upToDateVersionManager = new ProductVersionManager(ProductType.TestProductType, "test.exe", outDatedVersionServer, outDatedVersionServer);
            Assert.IsFalse(upToDateVersionManager.UpdateAvailable());

            var versionManager = new ProductVersionManager(ProductType.TestProductType, "test.exe", newerVersionServer, outDatedVersionServer);
            Assert.IsTrue(versionManager.UpdateAvailable());
        }

        [Category("Integration")]   // allows tests marked "Integration" to be included and excluded from test run
        [Test]
        public void TestNewStudio()
        {
            var product = new Product()
            {
                ProductType = ProductType.WorldViewStudio,
                ProductVersion = new Version("1.0.0.954"),
                TransferType = TransferType.BlobClientV1,
                Key = "{\"Size\":27316,\"Hash\":\"NETAEgwjrd5vvNdSYqQDS3FxFZrMbtFGAuU4m59uLzA=\"}",
                Url = "http://storage-pyxis.azurewebsites.net/"
            };

            var versionServer = new TestVersionServer();
            versionServer.SetLatestVersion(product);
            var versionManager = new ProductVersionManager(ProductType.WorldViewStudio, "WorldView.Studio.exe", versionServer, new LocalVersionServer());
            versionManager.DownloadUpdate(true, new string[0]);
            Assert.IsTrue(versionManager.TryRunTheLocalVersion(new String[0]));
        }
    }

    public class TestVersionServer : IVersionServer
    {
        private Product m_product;

        public Product GetLatestVersion(ProductType productType)
        {
            return m_product;
        }

        public void SetLatestVersion(Product product)
        {
            m_product = product;
        }

        public List<Product> GetAllVersions(ProductType productType)
        {
            var products = new List<Product> {GetLatestVersion(productType)};
            return products;
        }

    }
}