using Microsoft.Practices.Unity;
using NUnit.Framework;
using Pyxis.Storage;
using Pyxis.Storage.FileSystemStorage;
using System;
using System.IO;
using System.Security.Cryptography;

namespace Pyxis.Core.Test.IO.Storage
{
    internal class FileClientTest
    {
        private string m_fileName1;
        private string m_fileName2;
        private string m_bigFileName;
        private string m_testContent;
        private string m_dirName;
        private string m_bigDirName;
        private string m_targetDirName;
        private IFileSystemStorage m_storage;

        [TestFixtureSetUp]
        public void SetUp()
        {
            var container = new UnityContainer();

            //var blobProvider = new Pyxis.Storage.BlobProviders.PyxisBlobProvider("http://storage-pyxis.azurewebsites.net");
            var blobProvider = new Pyxis.Storage.BlobProviders.MemoryBlobProvider();

            container.RegisterInstance<IBlobProvider>(blobProvider);
            container.RegisterType<IFileSystemStorage, Pyxis.Storage.FileSystemStorage.FileSystemStorage>();


            m_storage = container.Resolve<IFileSystemStorage>();

            m_dirName = Path.Combine(Path.GetTempPath(), "BlobStorageTest");
            m_bigDirName = Path.Combine(Path.GetTempPath(), "BlobStorageTest-bigDir");
            m_targetDirName = m_dirName + "-downloaded";

            Directory.CreateDirectory(m_targetDirName);
            Directory.CreateDirectory(m_bigDirName);
            Directory.CreateDirectory(m_dirName);

            m_fileName1 = Path.Combine(m_dirName, "File1.txt");
            m_fileName2 = Path.Combine(m_dirName, "File2.txt");
            m_bigFileName = Path.Combine(m_bigDirName, "BigFile.txt");

            using (var streamWriter = new StreamWriter(m_fileName1))
            {
                m_testContent = "HELLO this is a test for File uploader client" + Environment.NewLine + Guid.NewGuid();
                streamWriter.Write(m_testContent);
            }

            using (var streamWriter = new StreamWriter(m_fileName2))
            {
                streamWriter.Write("HELLO this is a test for File uploader client");
            }

            using (var streamWriter = new StreamWriter(m_bigFileName))
            {
                var content = new char[1024 * 512 * 11]; //5.5 mb;
                for (int i = 0; i < content.Length; i++)
                {
                    content[i] = (char)(i % 256);
                }
                streamWriter.Write(content);
            }
        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            Directory.Delete(m_dirName, true);
            Directory.Delete(m_bigDirName, true);
            Directory.Delete(m_targetDirName, true);
        }


        [Test]
        public void TestUploadFile()
        {
            var file1Key = m_storage.UploadFile(m_fileName1);
            Assert.IsNotNullOrEmpty(file1Key);
        }

        [Test]
        public void TestUploadFileFail()
        {
            bool failed = false;
            try
            {
                var file1Key = m_storage.UploadFile("");
            }
            catch (Exception)
            {
                failed = true;
            }
            Assert.IsTrue(failed);
        }

        [Test]
        public void TestDownloadFile()
        {
            var file1Key = m_storage.UploadFile(m_fileName1);
            Assert.IsNotNullOrEmpty(file1Key);

            var targetfile = Path.Combine(m_targetDirName, new FileInfo(m_fileName1).Name);
            var result = m_storage.DownloadFile(file1Key, targetfile);
            Assert.IsTrue(result);

            Assert.IsTrue(File.Exists(targetfile));

            var info2 = new FileInfo(targetfile);
            var info1 = new FileInfo(m_fileName1);
            Assert.AreEqual(info1.Length, info2.Length);
        }

        [Test]
        public void TestUploadAndDownloadFile()
        {
            var file1Key = m_storage.UploadFile(m_fileName1);
            Assert.IsNotNullOrEmpty(file1Key);

            var path = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            Directory.CreateDirectory(path);
            var targetfile = Path.Combine(path, new FileInfo(m_fileName1).Name);

            var result = m_storage.DownloadFile(file1Key, targetfile);
            Assert.IsTrue(result);

            var reader = new StreamReader(targetfile);
            var readContent = reader.ReadToEnd();
            Assert.AreEqual(readContent, m_testContent);
        }

        [Test]
        public void TestUploadAndDownloadFileWithProgress()
        {
            int progress = 0;

            var uploadTracker = m_storage.UploadFileAsync(m_fileName1);
            Assert.IsNotNull(uploadTracker);
            uploadTracker.ProgressMade += delegate(ProgressTracker<string> tracker)
            {
                progress++;
            };

            uploadTracker.Task.ContinueWith(
                (t) =>
                {
                    Assert.IsNotNullOrEmpty(t.Result);
                });

            uploadTracker.Wait();
            Assert.AreEqual(100, uploadTracker.Percent);

            var key = uploadTracker.Task.Result;
            Assert.IsNotNull(key);

            var path = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            Directory.CreateDirectory(path);
            var targetfile = Path.Combine(path, new FileInfo(m_fileName1).Name);

            var downloadTracker = m_storage.DownloadFileAsync(key, targetfile);
            Assert.IsNotNull(downloadTracker);
            downloadTracker.ProgressMade += delegate(ProgressTracker<bool> tracker)
            {
                progress++;
            };

            downloadTracker.Task.ContinueWith(
                (t) =>
                {
                    Assert.IsTrue(t.Result);
                });

            downloadTracker.Wait();
            Assert.AreEqual(100, downloadTracker.Percent);

            var info2 = new FileInfo(targetfile);
            var info1 = new FileInfo(m_fileName1);
            Assert.AreEqual(info1.Length, info2.Length);

            var reader = new StreamReader(targetfile);
            var readContent = reader.ReadToEnd();
            Assert.AreEqual(readContent, m_testContent);
        }

        [Test]
        public void TestUploadAndDownloadBigFile()
        {
            var file1Key = m_storage.UploadFile(m_bigFileName);
            Assert.IsNotNullOrEmpty(file1Key);

            var targetfile = Path.Combine(m_targetDirName, new FileInfo(m_bigFileName).Name);
            var result = m_storage.DownloadFile(file1Key, targetfile);
            Assert.IsTrue(result);

            var info2 = new FileInfo(targetfile);
            var info1 = new FileInfo(m_bigFileName);

            Assert.AreEqual(info1.Length, info2.Length);
            Assert.AreEqual(calcCheckSum(targetfile), calcCheckSum(m_bigFileName));
        }
        private byte[] calcCheckSum(string fileName)
        {
            using (var md5 = MD5.Create())
            {
                using (var stream = File.OpenRead(fileName))
                {
                    return md5.ComputeHash(stream);
                }
            }
        }

        [Test]
        public void TestUploadAndDownloadBigFileWithProgress()
        {
            int progress = 0;

            var uploadTracker = m_storage.UploadFileAsync(m_bigFileName);
            Assert.IsNotNull(uploadTracker);
            uploadTracker.ProgressMade += delegate(ProgressTracker<string> tracker)
            {
                Console.WriteLine("uploaded " + tracker.Percent);
                progress++;
            };

            uploadTracker.Task.ContinueWith(
                (t) =>
                {
                    Assert.IsNotNullOrEmpty(t.Result);
                });

            uploadTracker.Wait();
            Assert.AreEqual(100, uploadTracker.Percent);

            var key = uploadTracker.Task.Result;
            Assert.IsNotNull(key);

            var targetfile = Path.Combine(m_targetDirName, new FileInfo(m_bigFileName).Name);
            var downloadTracker = m_storage.DownloadFileAsync(key, targetfile);
            Assert.IsNotNull(downloadTracker);
            downloadTracker.ProgressMade += delegate(ProgressTracker<bool> tracker)
            {
                Console.WriteLine("downloaded " + tracker.Percent);
                progress++;
            };

            downloadTracker.Task.ContinueWith(
                (t) =>
                {
                    Assert.IsTrue(t.Result);
                });

            downloadTracker.Wait();

            Assert.AreEqual(100, downloadTracker.Percent);

            var info2 = new FileInfo(targetfile);
            var info1 = new FileInfo(m_bigFileName);
            Assert.AreEqual(info1.Length, info2.Length);
        }

        [Test]
        public void TestUploadDirectory()
        {
            var dirKey = m_storage.UploadDirectory(m_dirName);
            Assert.IsNotNullOrEmpty(dirKey);
        }

        [Test]
        public void TestDownloadDirectory()
        {
            //m_dirName = @"C:\Users\Nader\AppData\Roaming\Pyxis\WorldViewStudio 0.10.0.905";
            var dirKey = m_storage.UploadDirectory(m_dirName);
            Assert.IsNotNullOrEmpty(dirKey);
            var downloadedDir = Path.Combine(m_targetDirName, "downloaded");
            var result = m_storage.DownloadDirectory(dirKey, downloadedDir);
            Assert.IsTrue(result);

            var baseSize = FileSystemUtilities.GetDirectorySize(m_dirName);
            var downloadedSize = FileSystemUtilities.GetDirectorySize(downloadedDir);
            Assert.AreEqual(baseSize, downloadedSize);
        }

        [Test]
        public void TestUploadDownloadDirectoryWithProgress()
        {
            int progress = 0;

            var uploadTracker = m_storage.UploadDirectoryAsync(m_bigDirName);
            Assert.IsNotNull(uploadTracker);
            uploadTracker.ProgressMade += delegate(ProgressTracker<string> tracker)
            {
                progress++;
            };

            uploadTracker.Task.ContinueWith(
                (t) =>
                {
                    Assert.IsNotNullOrEmpty(t.Result);
                });

            uploadTracker.Wait();

            Assert.AreEqual(100, uploadTracker.Percent);

            var key = uploadTracker.Task.Result;
            Assert.IsNotNullOrEmpty(key);
            var downloadedDir = Path.Combine(m_targetDirName, "downloaded");
            var downloadTracker = m_storage.DownloadDirectoryAsync(key, downloadedDir);
            Assert.IsNotNull(downloadTracker);
            downloadTracker.ProgressMade += delegate(ProgressTracker<bool> tracker)
                {
                    progress++;
                };

            downloadTracker.Task.ContinueWith(
                (t) =>
                {
                    Assert.IsTrue(t.Result);
                });

            downloadTracker.Wait();
            Assert.AreEqual(100, downloadTracker.Percent);

        }

        [Test]
        public void TestDownloadDirectoryWithProgressFail()
        {
            var nonExistingKey = Guid.NewGuid().ToString();

            var progressTracker = m_storage.DownloadDirectoryAsync(nonExistingKey, m_targetDirName);
            Assert.IsNotNull(progressTracker);

            bool completeIsCalled = false;
            var c2 = progressTracker.Task.ContinueWith(
                (T) =>
                {
                    completeIsCalled = true;
                    Assert.IsFalse(T.Result);
                });
            progressTracker.Wait();
            c2.Wait();
            Assert.IsTrue(completeIsCalled);
        }
    }
}