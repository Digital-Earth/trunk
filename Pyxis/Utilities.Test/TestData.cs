/******************************************************************************
TestData.cs

begin      : April 9, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Pyxis.Utilities.Test
{
    public static class TestData
    {
        private static Random r = new Random(3010);

        /// <summary>
        /// Generates an array of bytes.  The array will contain byteCount items.
        /// </summary>
        /// <param name="byteCount">The byte count.</param>
        /// <returns></returns>
        public static byte[] GenerateBytes(int byteCount)
        {
            byte[] buffer = new byte[byteCount];
            r.NextBytes(buffer);
            return buffer;
        }

        /// <summary>
        /// Generates the int.
        /// </summary>
        /// <param name="maxValue">The max value.</param>
        /// <returns></returns>
        public static int GenerateInt(int maxValue)
        {
            return r.Next(maxValue);
        }

        public static string CreateRandomFile(string directoryName)
        {
            string fileName = directoryName +
                System.IO.Path.DirectorySeparatorChar +
                System.IO.Path.GetRandomFileName();

            using (FileStream fs = new FileStream(fileName, FileMode.CreateNew))
            {
                using (System.IO.BinaryWriter writer = new BinaryWriter(fs))
                {
                    writer.Write(TestData.GenerateBytes(TestData.GenerateInt(32000)));
                }
            }

            return fileName;
        }

        private static void PopulateDirectory(string directoryName)
        {
            for (int fileCount = TestData.GenerateInt(5);
                fileCount > 0;
                --fileCount)
            {
                CreateRandomFile(directoryName);
            }
        }

        /// <summary>
        /// Creates a random directory, off of the temp directory, containing a 
        /// random collection of files.  It returns the directory name.  Use
        /// <see cref="TestData.CreateRandomTemporaryDirectory"/> if you want
        /// the directory to self-delete (when Dispose is called.)
        /// </summary>
        /// <returns>The directory name.</returns>
        public static string CreateRandomDirectory()
        {
            // Generate a new directory in the temp area.  
            return CreateRandomDirectory(System.IO.Path.GetTempPath());
        }

        public static Pyxis.Utilities.TemporaryDirectory CreateRandomTemporaryDirectory()
        {
            return new Pyxis.Utilities.TemporaryDirectory(CreateRandomDirectory());
        }

        public static string CreateRandomDirectory(string baseDirectory)
        {
            // Note that GetRandomFileName also works for directory names.
            string directoryPath = baseDirectory +
                System.IO.Path.DirectorySeparatorChar +
                System.IO.Path.GetRandomFileName();
            System.IO.Directory.CreateDirectory(directoryPath);

            // Populate the directory.
            PopulateDirectory(directoryPath);

            // Create a single subdirectory.
            string subdirectoryPath = directoryPath +
                System.IO.Path.DirectorySeparatorChar +
                System.IO.Path.GetRandomFileName();
            System.IO.Directory.CreateDirectory(subdirectoryPath);

            // Populate the directory.
            PopulateDirectory(subdirectoryPath);

            return directoryPath;
        }

    }
    
    namespace InternalTest
    {
        using NUnit.Framework;

        /// <summary>
        /// Unit tests for TestData
        /// </summary>
        [TestFixture]
        public class TestDataTests
        {
            [Test]
            public void DataGetsGenerated()
            {
                byte[] rawData = TestData.GenerateBytes(1000);
                Assert.AreEqual(rawData.Length, 1000);
                Assert.Less(TestData.GenerateInt(5), 6);
            }
        }
    }

}
