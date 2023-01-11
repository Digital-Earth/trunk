using Microsoft.Practices.Unity;
using NUnit.Framework;
using Pyxis.Core.IO.Storage;
using System;
using System.IO;

namespace Pyxis.Core.Test.IO.Storage
{
    internal class FileClientTest
    {
        private string m_fileName1;
        private string m_fileName2;
        private string m_fileName3;
        private string m_file1Key;
        private string m_testContent;
        private string m_dirName;
        private string m_dirKey;
        private string m_bigDir;
        private string m_target;
        private FileClient m_fileClient;

        [TestFixtureSetUp]
        public void SetUp()
        {
            var container = new UnityContainer();
            container.RegisterInstance<IBlobProvider>( new  Pyxis.Core.IO.Storage.BlobClients.MemoryBlobProvider());
            m_fileClient = container.Resolve<FileClient>();
            
            m_dirName = Path.Combine(Path.GetTempPath(),"BlobStorageTest");
            m_bigDir = Path.Combine(Path.GetTempPath(), "BlobStorageTest-bigDir");
            m_target = m_dirName + "-downloaded";
            Directory.CreateDirectory(m_target);
            Directory.CreateDirectory(m_bigDir);
            Directory.CreateDirectory(m_dirName);


            m_fileName1 = Path.Combine(m_dirName, "File1.txt");
            m_fileName2 = Path.Combine(m_dirName, "File2.txt");
            m_fileName3 = Path.Combine(m_bigDir, "BigFile.txt");

            var streamWriter = new StreamWriter(m_fileName1);
            m_testContent = "HELLO this is a test for File uploader client" + Environment.NewLine + Guid.NewGuid();
            streamWriter.Write(m_testContent);
            streamWriter.Flush();
            streamWriter.Close();

            streamWriter = new StreamWriter(m_fileName2);
            streamWriter.Write("HELLO this is a test for File uploader client");
            streamWriter.Flush();
            streamWriter.Close();

            streamWriter = new StreamWriter(m_fileName3);
            var content = new char[1024 * 512 * 11]; //5.5 mb;
            streamWriter.Write(content);
            streamWriter.Flush();
            streamWriter.Close();

        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            File.Delete(m_fileName1);
            File.Delete(m_fileName2);
            File.Delete(m_dirName);
        }

        [Test]
        public void TestUploadFile()
        {
            m_file1Key = m_fileClient.UploadFile(m_fileName1);
            Assert.IsNotNull(m_file1Key);
        }

        [Test]
        public void TestUploadAndDownloadFile()
        {
            m_file1Key = m_fileClient.UploadFile(m_fileName1);
            Assert.IsNotNull(m_file1Key);
            using (var stream = new MemoryStream())
            {
                var result = m_fileClient.DownloadFile(m_file1Key, stream);
                Assert.IsTrue(result);

                stream.Position = 0;
                var reader = new StreamReader(stream);
                var readContent = reader.ReadToEnd();
                Assert.AreEqual(readContent, m_testContent);
            }
        }

        [Test]
        public void TestUploadAndDownloadBigFile()
        {
            m_file1Key = m_fileClient.UploadFile(m_fileName3);
            Assert.IsNotNull(m_file1Key);
            var result = m_fileClient.DownloadFile(m_file1Key, m_target);
            Assert.IsTrue(result);
            var info2 = new FileInfo(Path.Combine(m_target, "BigFile.txt"));
            var info1 = new FileInfo(m_fileName3);
            Assert.AreEqual(info1.Length, info2.Length);
        }

        [Test]
        public void TestDownloadFile()
        {
            m_file1Key = m_fileClient.UploadFile(m_fileName1);
            Assert.IsNotNull(m_file1Key);
            var result = m_fileClient.DownloadFile(m_file1Key, m_target);
            Assert.IsTrue(result);

            var downloadedPath = Path.Combine(m_target, "File1.txt");
            Assert.IsTrue(File.Exists(downloadedPath));

            var info2 = new FileInfo(downloadedPath);
            var info1 = new FileInfo(m_fileName1);
            Assert.AreEqual(info1.Length, info2.Length);
        }

        [Test]
        public void TestUploadDirectory()
        {
            m_dirKey = m_fileClient.UploadDirectory(m_dirName);
            Assert.IsNotNull(m_dirKey);
        }

        [Test]
        public void TestDownloadDirectory()
        {
            m_dirKey = m_fileClient.UploadDirectory(m_dirName);
            Assert.IsNotNull(m_dirKey);
            var result = m_fileClient.DownloadDirectory(m_dirKey, m_target);
            Assert.IsTrue(result);
        }
    }
}