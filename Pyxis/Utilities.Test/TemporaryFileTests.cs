using System.IO;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for TemporaryFile
    /// </summary>
    [TestFixture]
    public class TemporaryFileTests
    {
        [Test]
        public void FileDoesntExist()
        {
            string fileName;
            using (TemporaryFile TemporaryFile = new TemporaryFile())
            {
                fileName = TemporaryFile.Name;
                Assert.IsFalse(File.Exists( fileName));
            }
            Assert.IsFalse(File.Exists(fileName));
        }

        //[NUnit.Framework.Ignore("The file sometimes exists after the 'using' scope.")] 
        [Test]
        public void FileExists()
        {
            string fileName;
            using (TemporaryFile temporaryFile = new TemporaryFile())
            {                    
                fileName = temporaryFile.Name;
                using (FileStream fileStream = File.OpenWrite(fileName))
                {
                    fileStream.WriteByte(42);
                }

                Assert.IsTrue(File.Exists(fileName));
            }
            Assert.IsFalse(Directory.Exists(fileName));
        }
    }
}