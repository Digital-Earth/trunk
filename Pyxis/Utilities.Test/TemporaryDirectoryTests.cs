using System;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for TemporaryDirectory
    /// </summary>
    [TestFixture]
    public class TemporaryDirectoryTests
    {
        private string GenerateBaseDir()
        {
            return string.Format("{0}{1}", System.IO.Path.GetTempPath(),
                System.IO.Path.GetRandomFileName());
        }

        [Test]
        public void EmptyDirectory()
        {
            string dirName;
            using (TemporaryDirectory temporaryDirectory = new TemporaryDirectory(GenerateBaseDir()))
            {
                dirName = temporaryDirectory.Name;
                Assert.IsTrue(System.IO.Directory.Exists(dirName));
            }
            Assert.IsFalse(System.IO.Directory.Exists(dirName));
        }

        public void FilledDirectory()
        {
            string dirName;
            using (TemporaryDirectory temporaryDirectory = 
                new TemporaryDirectory(TestData.CreateRandomDirectory()))
            {
                dirName = temporaryDirectory.Name;
                Assert.IsTrue(System.IO.Directory.Exists(dirName));
                Assert.IsTrue(System.IO.Directory.GetFiles(dirName).Length > 0);
            }
            TimedTest.Verify(
                delegate() {return !System.IO.Directory.Exists(dirName);}, 
                TimeSpan.FromSeconds( 10)); 
        }
    }
}