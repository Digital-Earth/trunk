using System;
using System.IO;
using System.Linq;
using NUnit.Framework;
using PyxNet.Publishing.Files;

namespace PyxNet.Test.Publishing.Files
{
    [TestFixture]
    internal class PublishedFileRepositoryTests
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

        [Test]
        public void TestPublishedFileRepository()
        {
            PublishedFileInfo info1 = new PublishedFileInfo(m_helloFile1);
            PublishedFileInfo info2 = new PublishedFileInfo(m_helloFile2);
            PublishedFileInfo info3 = new PublishedFileInfo(m_goodByeFile);

            var repository = new PublishedFileRepository();
            int keywordChangedRaisedCount = 0;
            var lockObj = new object();
            repository.KeywordsChanged += delegate(object sender, EventArgs e)
            {
                lock (lockObj)
                {
                    keywordChangedRaisedCount++;
                }
            };

            repository.Publish(m_helloFile1);

            Assert.That(repository.PublishedFileInfos.Count() == 1);

            var dataInfo = repository.Publish(m_helloFile2);

            Assert.That(repository.PublishedFileInfos.Count() == 1);

            repository.Publish(m_helloFile1);

            Assert.That(repository.PublishedFileInfos.Count() == 1);
            System.Threading.Thread.Sleep(6000);
            Assert.That(repository.Keywords.Contains(m_helloFile1.Name));
            Assert.That(repository.Keywords.Contains(m_helloFile2.Name));

            var newDataInfo = repository.Publish(m_goodByeFile);

            Assert.That(repository.PublishedFileInfos.Count() == 2);

            repository.Unpublish(m_helloFile1);
            Assert.That(repository.PublishedFileInfos.Count() == 2);

            repository.Unpublish(m_helloFile2);
            Assert.That(repository.PublishedFileInfos.Count() == 2);

            repository.Unpublish(m_helloFile1);
            Assert.That(repository.PublishedFileInfos.Count() == 1);

            repository.Clear();
            Assert.That(repository.PublishedFileInfos.Count() == 0);
            Assert.That(repository.Keywords.Count() == 0);

            info1 = new PublishedFileInfo(m_helloFile1);

            repository.Publish(m_helloFile1);
            repository.Publish(m_helloFile1);
            repository.Publish(m_helloFile1);
            repository.Publish(m_helloFile1);

            repository.Delete(m_helloFile1);
            Assert.That(repository.PublishedFileInfos.Count() == 0);

            repository.Publish(m_goodByeFile);
            repository.Publish(m_goodByeFile);
            File.WriteAllText(m_goodByeFile.FullName, "New Content");
            repository.Unpublish(m_goodByeFile);
            Assert.IsNotNull(repository.PublishedFileInfos.FirstOrDefault(x => x.SourceFiles.Contains(m_goodByeFile)));
        }
    }
}