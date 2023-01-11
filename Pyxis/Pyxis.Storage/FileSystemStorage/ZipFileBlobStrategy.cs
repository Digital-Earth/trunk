using System;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Threading.Tasks;

namespace Pyxis.Storage.FileSystemStorage
{
    internal class ZipFileBlobStrategy : BasicFileBlobStrategy
    {
        //empty ctor for dependency injection
        public ZipFileBlobStrategy()
        {
        }

        public ZipFileBlobStrategy(IBlobProvider blobProvider)
            : this()
        {
            BlobProvider = blobProvider;
        }

        /// <summary>
        /// Download a file blob to the specified target file. The folder containing the target file
        /// is created if it does not exist yet. The file is downloaded as a temporary zip file then
        /// unzipped to the target location.
        /// </summary>
        /// <param name="fileBlob">The fileBlob</param>
        /// <param name="targetFile">The name of the targetFile with path</param>
        /// <param name="progressTracker">An optional progress tracker</param>
        /// <returns>true if the blob was downloaded, otherwise false</returns>
        public override bool DownloadFileBlob(FileBlob fileBlob, string targetFile, ProgressTracker<bool> progressTracker = null)
        {
            if (!FileSystemUtilities.TryCreateDirectory(Path.GetDirectoryName(targetFile), false))
            {
                return false;
            }

            // temporary zip folder and file is created in base class DownloadFileBlob
            var unzippingFolder = Path.Combine(FileSystemStorage.TmpPath, Path.GetRandomFileName());
            var zipfile = Path.Combine(unzippingFolder, Path.GetRandomFileName() + ".zip");

            try
            {
                long zippedSize = 0;

                // progress is reported in compressed bytes
                if (!base.DownloadFileBlob(fileBlob, zipfile, progressTracker))
                {
                    return false;
                }

                FileSystemUtilities.TryGetFileSize(zipfile, out zippedSize);

                ZipFile.ExtractToDirectory(zipfile, unzippingFolder);
                var unzippedFile = Directory.EnumerateFiles(unzippingFolder).Where(x => x != zipfile).First();

                long unzippedSize = 0;
                File.Move(unzippedFile, targetFile);
                FileSystemUtilities.TryGetFileSize(targetFile, out unzippedSize);
                if (progressTracker != null)
                {
                    // make an adjustment because we reported progress in compressed bytes but
                    // the progress tracker was expecting uncompressed bytes
                    if (unzippedSize > zippedSize)
                    {
                        progressTracker.Report(unzippedSize - zippedSize);
                    }
                }

                return true;
            }
            catch (Exception e)
            {
                var s = e.Message;
                return false;
            }
            finally
            {
                // cleanup
                Task.Factory.StartNew(() =>
                {
                    Directory.Delete(unzippingFolder, true);
                });
            }
        }

        public override FileBlob UploadFileBlob(string fileName, ProgressTracker<string> progressTracker = null)
        {
            var name = new FileInfo(fileName).Name;
            var zippingFolder = Path.Combine(FileSystemStorage.TmpPath, Path.GetRandomFileName());
            var zippedFileLocation = Path.Combine(FileSystemStorage.TmpPath, Path.GetRandomFileName());
            var zippedFileName = Path.Combine(zippedFileLocation, name + ".zip");
            long unzippedSize = 0;
            long zippedSize = 0;

            try
            {
                Directory.CreateDirectory(zippingFolder);
                Directory.CreateDirectory(zippedFileLocation);

                File.Copy(fileName, Path.Combine(zippingFolder, name));

                ZipFile.CreateFromDirectory(zippingFolder, zippedFileName, CompressionLevel.Optimal, false);

                //If zipfile is larger than the original file fall back to basic

                if (FileSystemUtilities.TryGetFileSize(fileName, out unzippedSize) &&
                        FileSystemUtilities.TryGetFileSize(zippedFileName, out zippedSize))
                {
                    if (unzippedSize <= zippedSize)
                    {
                        return base.UploadFileBlob(fileName, progressTracker);
                    }
                }

                var blob = base.UploadFileBlob(zippedFileName, progressTracker);
                blob.Version = FileStorageVersion.ZippedFileBlobV1;
                if (progressTracker != null)
                {
                    // make an adjustment because we reported progress in compressed bytes but
                    // the progress tracker was expecting uncompressed bytes
                    progressTracker.Report(unzippedSize - zippedSize);
                }
                return blob;
            }
            finally
            {
                // cleanup
                Task.Factory.StartNew(() =>
                    {
                        Directory.Delete(zippingFolder, true);
                        Directory.Delete(zippedFileLocation, true);
                    });
            }
        }
    }
}