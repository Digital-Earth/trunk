using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace Pyxis.Storage.FileSystemStorage
{
    internal class BasicDirectoryBlobStrategy : IDirectoryBlobStrategy
    {
        /// <summary>
        /// Magic number - the maximum block size for file reads.
        /// </summary>
        protected static int s_maxBlockSize = 1024 * 1024;

        public IBlobProvider BlobProvider { get; set; }

        private FileBlobClient m_fileBlobClient;

        public BasicDirectoryBlobStrategy(IBlobProvider blobProvider)
        {
            BlobProvider = blobProvider;
            m_fileBlobClient = new FileBlobClient(blobProvider);
        }

        public virtual bool DownloadDirectoryBlob(DirectoryBlob directoryBlob, string path, ProgressTracker<bool> tracker = null)
        {
            if (directoryBlob == null)
            {
                return false;
            }


            if (!FileSystemUtilities.TryCreateDirectory(path, true))
            {
                return false;
            }

            var result = true;

            foreach (var subDir in directoryBlob.Directories)
            {
                var subDirName = Path.Combine(path, subDir.Name);
                result &= DownloadDirectoryBlob(subDir, subDirName, tracker);
            }

            var largeFiles = directoryBlob.Files.Where(x => x.Size >= s_maxBlockSize).ToList();
            var smallFiles = directoryBlob.Files.Where(x => x.Size < s_maxBlockSize).OrderByDescending(x => x.Size).ToList();
            var halfOfLargeFiles = largeFiles.Count / 2;

            // to make better use of threading, group small files together and split large files
            // into two groups
            var tasks = new Task<bool>[]
            {
                Task.Factory.StartNew(() => {
                    var smallResult = DownloadSmallFiles(tracker, path, smallFiles.ToList());
                    return smallResult;}),

                Task.Factory.StartNew(() => {
                    var chunk = largeFiles.Take(halfOfLargeFiles).ToList();
                    var largeResult = DownloadLargeFiles(tracker, path, chunk);
                    return largeResult;}),

                Task.Factory.StartNew(() => {
                    var chunk = largeFiles.Skip(halfOfLargeFiles).ToList();
                    var largeResult = DownloadLargeFiles(tracker, path, chunk);
                    return largeResult;})
            };

            Task.WaitAll(tasks);
            result &= tasks.All(x => x.Result);
            return result;
        }

        public virtual DirectoryBlob UploadDirectoryBlob(string directoryPath, ProgressTracker<string> progressTracker = null)
        {
            var dirInfo = new DirectoryInfo(directoryPath);
            var directoryBlob = new DirectoryBlob(dirInfo.Name, DirectoryStorageVersion.BasicDirectoryBlobV1);

            foreach (string directory in Directory.EnumerateDirectories(directoryPath))
            {
                var dirBlob = UploadDirectoryBlob(directory, progressTracker);
                directoryBlob.DirectorySize += dirBlob.DirectorySize;
                directoryBlob.Directories.Add(dirBlob);
            }

            foreach (var file in dirInfo.EnumerateFiles())
            {
                var fileBlob = m_fileBlobClient.UploadFileBlob(file.FullName, progressTracker);
                directoryBlob.DirectorySize += fileBlob.FileSize;
                var fileKey = BlobProvider.AddBlob(fileBlob);
                var fileSize = file.Length;
                directoryBlob.Files.Add(new DirectoryBlob.ManifestEntry() { Key = fileKey, Size = fileSize, Name = file.Name });
            }

            return directoryBlob;
        }

        private bool DownloadLargeFiles(ProgressTracker<bool> tracker, string path, List<DirectoryBlob.ManifestEntry> largeFiles)
        {
            bool result = true;

            foreach (var item in largeFiles)
            {
                var fileLocation = Path.Combine(path, item.Name);

                FileBlob fileBlob = BlobProvider.GetBlob<FileBlob>(item.Key);
                if (fileBlob == null)
                {
                    return false;
                }

                result &= m_fileBlobClient.DownloadFileBlob(fileBlob, fileLocation, tracker);
            }
            return result;
        }

        /// <summary>
        /// Download a list of small files in batches.
        /// </summary>
        /// <param name="tracker"> progress tracker</param>
        /// <param name="path"> location to put the downloaded files</param>
        /// <param name="smallFiles"> list of files to download - must be sorted by descending size</param>
        /// <returns> true if the downloads were successful</returns>
        private bool DownloadSmallFiles(ProgressTracker<bool> tracker, string path, List<DirectoryBlob.ManifestEntry> smallFiles)
        {
            bool result = true;
            try
            {
                while (smallFiles.Count > 0)
                {
                    // collect the small files into batches less than the maximum block size
                    // insert the largest files first, then top up with the smallest files
                    long batchSize = 0;
                    var batchEntries = new List<DirectoryBlob.ManifestEntry>();

                    while (smallFiles.Count > 0 && batchSize + smallFiles[0].Size < s_maxBlockSize)
                    {
                        batchEntries.Add(smallFiles[0]);
                        batchSize += smallFiles[0].Size;
                        smallFiles.RemoveAt(0);
                    }

                    while (smallFiles.Count > 0 && batchSize + smallFiles.Last().Size < s_maxBlockSize)
                    {
                        batchEntries.Add(smallFiles.Last());
                        batchSize += smallFiles.Last().Size;
                        smallFiles.RemoveAt(smallFiles.Count - 1);
                    }

                    var blobs = BlobProvider.GetBlobs(batchEntries.Select(x => x.Key));

                    foreach (var file in batchEntries)
                    {
                        FileBlob fileBlob = JsonConvert.DeserializeObject<FileBlob>(blobs[file.Key]);

                        var fileLocation = Path.Combine(path, file.Name);

                        result &= m_fileBlobClient.DownloadFileBlob(fileBlob, fileLocation, tracker);
                    }
                }
            }
            catch (Exception e)
            {
                Trace.TraceError("DownloadSmallFiles: " + e.Message);
                Trace.TraceError("Exception Type: " + e.GetType().FullName);
                throw new Exception("Failed to download small file.", e);
            }

            return result;
        }
    }
}