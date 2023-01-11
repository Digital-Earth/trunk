using Microsoft.Practices.Unity;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Pyxis.Storage.FileSystemStorage
{
    internal class BasicFileBlobStrategy : IFileBlobStrategy
    {
        /// <summary>
        /// Magic number - the maximum block size for file reads.
        /// </summary>
        protected static int s_maxBlockSize = 1024 * 1024;

        [Dependency]
        public IBlobProvider BlobProvider { get; set; }

        public BasicFileBlobStrategy()
        {
        }

        public BasicFileBlobStrategy(IBlobProvider blobProvider)
            : this()
        {
            BlobProvider = blobProvider;
        }

        /// <summary>
        /// Download a file blob to the specified target file. The folder containing the target file
        /// is created if it does not exist yet.
        /// </summary>
        /// <param name="fileBlob">The fileBlob</param>
        /// <param name="targetFile">The name of the targetFile with path</param>
        /// <param name="progressTracker">An optional progress tracker</param>
        /// <returns>true if the blob was downloaded, otherwise false</returns>
        public virtual bool DownloadFileBlob(FileBlob fileBlob, string targetFile, ProgressTracker<bool> tracker = null)
        {
            var tmpFile = targetFile + ".tmp." + DateTime.Now.Ticks;
            
            while (File.Exists(tmpFile))
            {
                tmpFile = targetFile + ".tmp." + DateTime.Now.Ticks;
            }

            try
            {
                var parentDir = Path.GetDirectoryName(targetFile);
                bool result = FileSystemUtilities.TryCreateDirectory(parentDir, false);
                // rely on FileStream constructor to throw exception if directory can't be created
                using (var stream = new FileStream(tmpFile, FileMode.Create))
                {
                    if (fileBlob == null)
                    {
                        return false;
                    }

                    long prevPosition = stream.Position;

                    // write the first part of the file
                    var buffer = Convert.FromBase64String(fileBlob.Data);
                    stream.Write(buffer, 0, buffer.Length);

                    if (tracker != null)
                    {
                        tracker.Report(stream.Position - prevPosition);
                        prevPosition = stream.Position;
                    }

                    // if there are more parts read them too
                    foreach (var item in fileBlob.Parts)
                    {
                        result &= BlobProvider.GetBlob(item, stream);
                        if (tracker != null)
                        {
                            tracker.Report(stream.Position - prevPosition);
                            prevPosition = stream.Position;
                        }
                    }
                }
                File.Move(tmpFile, targetFile);
                return result;
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine("Unable to download file" + e.Message + "\n" + e.StackTrace);
                return false;
            }
            finally
            {
                //make sure we clean up
                if (File.Exists(tmpFile))
                {
                    try
                    {
                        File.Delete(tmpFile);
                    }
                    catch (Exception)
                    {
                        System.Diagnostics.Trace.WriteLine("Unable to remove tmp file :" + tmpFile);
                    }
                }
            }
        }

        public virtual FileBlob UploadFileBlob(string fileName, ProgressTracker<string> progressTracker = null)
        {
            var fileBlob = new FileBlob(FileStorageVersion.BasicFileBlobV1);
            long fileSize;
            FileSystemUtilities.TryGetFileSize(fileName, out fileSize);
            fileBlob.FileSize = fileSize;
            // note: rely on FileStream constructor to throw exception if file doesn't exist

            var parts = GetParts(fileName).GetEnumerator();
            parts.MoveNext();
            var firstPart = parts.Current;
            fileBlob.Data = Convert.ToBase64String(firstPart.Buffer, 0, firstPart.Size);

            long progressAdjustment = fileBlob.FileSize;
            // if the file is large break it into blocks and...
            var partsIndex = new Dictionary<string, KeyValuePair<long, int>>();

            while (parts.MoveNext())
            {
                var part = parts.Current;
                // generate the checksum of the block
                var checkSum = BlobKeyFactory.GenerateKey(part.Buffer, 0, part.Size);

                // store where we can find this block in the file
                if (!partsIndex.ContainsKey(checkSum))
                {
                    partsIndex.Add(checkSum, new KeyValuePair<long, int>(part.Index, part.Size));
                }

                // add the key to the parts list
                fileBlob.Parts.Add(checkSum);

                if (progressTracker != null)
                {
                    // report progress against 1/2 the block bytes in this pass, other 1/2 will
                    // be reported when the block is uploaded, note small rounding error
                    progressTracker.Report(part.Size / 2);
                    progressAdjustment -= part.Size / 2;
                }
            }
            // send the list of checksums and get the missing ones
            var missingParts = BlobProvider.MissingBlobs(partsIndex.Keys.ToList());

            // upload the missing blocks
            using (var fileReader = new FileStream(fileName, FileMode.Open, FileAccess.Read))
            {
                foreach (var item in missingParts)
                {
                    var missingPart = partsIndex[item];
                    var buffer = new byte[missingPart.Value];
                    fileReader.Seek(missingPart.Key, SeekOrigin.Begin);
                    var numberOfBytes = fileReader.Read(buffer, 0, buffer.Length);
                    using (var stream = new MemoryStream(buffer, 0, numberOfBytes))
                    {
                        BlobProvider.AddBlob(item, stream);
                    }

                    if (progressTracker != null)
                    {
                        // report progress against the remaining 1/2 bytes in the block
                        // note small rounding error
                        progressTracker.Report(numberOfBytes / 2);
                        progressAdjustment -= numberOfBytes / 2;
                    }
                }

                if (progressTracker != null)
                {
                    // correct for any rounding error or skipped blocks
                    progressTracker.Report(progressAdjustment);
                }
            }
            return fileBlob;
        }

        private class FilePart
        {
            public byte[] Buffer;
            public int Size;
            public long Index;
        }

        private IEnumerable<FilePart> GetParts(string fileName)
        {
            using (var fileReader = new FileStream(fileName, FileMode.Open))
            {
                // read first block
                var buffer = new byte[s_maxBlockSize];
                int numberOfBytes = fileReader.Read(buffer, 0, s_maxBlockSize);
                yield return new FilePart { Buffer = buffer, Size = numberOfBytes, Index = 0 };
                // if the file is large break it into blocks and...
                var index = fileReader.Position;
                while ((numberOfBytes = fileReader.Read(buffer, 0, s_maxBlockSize)) > 0)
                {
                    yield return new FilePart { Buffer = buffer, Size = numberOfBytes, Index = index };
                    index = fileReader.Position;
                }
            }
        }
    }
}