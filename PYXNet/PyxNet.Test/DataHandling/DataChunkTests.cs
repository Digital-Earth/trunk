using System;
using System.IO;
using NUnit.Framework;
using PyxNet.DataHandling;

namespace PyxNet.Test.DataHandling
{
    [TestFixture]
    public class DataChunkTests
    {
        /// <summary>
        /// The size of chunk we test with.
        /// </summary>
        const int ChunkSize = 4096;

        /// <summary>
        /// The smallest file we will test with.
        /// </summary>
        const int MinFileSize = 50000;

        const int MaxFileSize = 10000000;

        /// <summary>
        /// Find a file in the current directory that is big enough to test on.
        /// </summary>
        /// <returns>A suitable file.</returns>
        public FileInfo GetASuitableFile()
        {
            DirectoryInfo di = new DirectoryInfo(Environment.CurrentDirectory);
            int index;
            for (index = 0; (index < di.GetFiles().Length) &&
                            (di.GetFiles()[index].Length < MinFileSize || di.GetFiles()[index].Length > MaxFileSize); ++index)
            {
                // look for a file that is big enough.
            }
            FileInfo returnFile = di.GetFiles()[index];
            Assert.IsTrue(returnFile.Length >= MinFileSize,
                "Could not find a file big enough to test with.");
            Assert.IsTrue(returnFile.Length <= MaxFileSize, "File is to big");
            return returnFile;
        }

        /// <summary>
        /// Test creating a file chunk and move to and from a message.
        /// </summary>
        [Test]
        public void TestDataChunkConstruction()
        {
            FileInfo copyFile = GetASuitableFile();

            DataChunk testChunk = new DataChunk(copyFile, 0, ChunkSize);
            Assert.IsTrue(testChunk.Data != null, "File Chunk Data was null.");
            Assert.IsTrue(testChunk.Data.Data.Length == ChunkSize,
                "File Chunk Data was the wrong size.");

            Message aMessage = testChunk.ToMessage();
            DataChunk reconstructedChunk = new DataChunk(aMessage);

            Assert.IsTrue(reconstructedChunk.Data.ValidCheckSum,
                "The check sum did not agree after reconstruction.");
            Assert.IsTrue(reconstructedChunk.Data.Data.Length == ChunkSize,
                "The improper data length after reconstruction.");
            Assert.IsTrue(reconstructedChunk.DataSetID.Equals(testChunk.DataSetID),
                "The Data Set ID did not transfer properly.");

            // test the extra message section
            testChunk.ExtraInfo = aMessage;
            Message bMessage = testChunk.ToMessage();
            reconstructedChunk = new DataChunk(bMessage);

            Assert.IsTrue(reconstructedChunk.ExtraInfo.Equals(aMessage), "The extra info message did not transfer properly.");
        }

        /// <summary>
        /// Test copying a file  using a file chunk.
        /// </summary>
        [Test]
        public void TestDataChunkCopy()
        {
            FileInfo sourceFile = GetASuitableFile();
            MemoryStream destFile = new MemoryStream((int)sourceFile.Length);
            int copyPosition = 0;
            while (copyPosition < sourceFile.Length)
            {
                int currentChunkSize = ChunkSize;
                if (copyPosition + currentChunkSize > sourceFile.Length)
                {
                    currentChunkSize = (int)sourceFile.Length - copyPosition;
                }

                DataChunk copyChunk = new DataChunk(sourceFile, copyPosition, currentChunkSize);

                DataChunk newChunk = new DataChunk(copyChunk.ToMessage());

                newChunk.WriteFileChunk(destFile);

                copyPosition += currentChunkSize;
            }

            // now make sure that we have the same bytes in the memory stream as are in the file.
            FileStream sourceStream = sourceFile.OpenRead();
            for (int index = 0; index < sourceStream.Length; ++index)
            {
                Assert.IsTrue(sourceStream.ReadByte() == destFile.GetBuffer()[index],
                    "The improper data after file copy.");
            }
        }
    }
}