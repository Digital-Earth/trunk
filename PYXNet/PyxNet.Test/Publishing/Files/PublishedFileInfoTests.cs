using System.IO;
using System.Linq;
using NUnit.Framework;
using Pyxis.Utilities;
using PyxNet.Publishing.Files;

namespace PyxNet.Test.Publishing.Files
{
    [TestFixture]
    public class PublishedFileInfoTests
    {
        private FileInformation m_goodByeFile;
        private FileInformation m_helloFile1;
        private FileInformation m_helloFile2;

        [SetUp]
        public void SetUp()
        {
            m_helloFile1 = new FileInformation(Path.GetTempFileName());
            m_helloFile2 = new FileInformation(Path.GetTempFileName());
            m_goodByeFile = new FileInformation(Path.GetTempFileName());

            File.WriteAllText(m_helloFile1.FullName, "Hello");
            File.WriteAllText(m_helloFile2.FullName, "Hello");
            File.WriteAllText(m_goodByeFile.FullName, "GoodBye");
        }

        [TearDown]
        public void TearDown()
        {
            File.Delete(m_helloFile1.FullName);
            File.Delete(m_helloFile2.FullName);
            File.Delete(m_goodByeFile.FullName);
        }

        //TESTS
        [Test]
        public void TestPublishedFileInfo()
        {
            var m_tempFile1 = new FileInformation(System.IO.Path.GetTempFileName());
            var m_tempFile2 = new FileInformation(System.IO.Path.GetTempFileName());
            File.WriteAllText(m_tempFile1.FullName, "Hello");
            File.WriteAllText(m_tempFile2.FullName, "Hello");

            PublishedFileInfo info = new PublishedFileInfo(m_tempFile1);

            //Test Increase
            info.IncreaseSource(m_tempFile2);
            Assert.That(info.SourceFiles.Contains(m_tempFile1));
            Assert.That(info.SourceFiles.Contains(m_tempFile2));
            Assert.AreEqual(
                ChecksumSingleton.Checksummer.CalculateFileCheckSumNoCache(m_tempFile1.FullName),
                info.SHA256Checksum);
            Assert.That(info.Keywords.Contains(m_tempFile1.Name));
            Assert.That(info.Keywords.Contains(m_tempFile2.Name));

            //Test Decrease
            info.DecreaseSource(m_tempFile2);
            Assert.That(info.SourceFiles.Contains(m_tempFile1));
            Assert.IsFalse(info.SourceFiles.Contains(m_tempFile2));

            Assert.That(info.Keywords.Contains(m_tempFile1.Name));
            Assert.IsFalse(info.Keywords.Contains(m_tempFile2.Name));

            //Test Delete
            //Test Decrease
            info.IncreaseSource(m_tempFile1);
            info.IncreaseSource(m_tempFile1);

            info.DeleteSource(m_tempFile1);

            Assert.IsFalse(info.SourceFiles.Contains(m_tempFile1));
            Assert.IsFalse(info.Keywords.Contains(m_tempFile1.Name));

        }
    }
}