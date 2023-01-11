namespace Pyxis.Storage.FileSystemStorage
{
    /// <summary>
    /// Knows how to create a Blob from a File and how to read a File from a blob
    /// </summary>
    internal interface IFileBlobStrategy
    {
        bool DownloadFileBlob(FileBlob fileBlob, string targetFile, ProgressTracker<bool> tracker = null);

        FileBlob UploadFileBlob(string fileName, ProgressTracker<string> progressTracker = null);
    }

    //Knows which IFileBlobClient to use to store a file and which one to use to retireve a file
    public class FileBlobClient
    {
        private IBlobProvider BlobProvider;

        public FileBlobClient(IBlobProvider BlobProvider)
        {
            this.BlobProvider = BlobProvider;
        }

        public bool DownloadFileBlob(FileBlob fileBlob, string targetFile, ProgressTracker<bool> progressTracker = null)
        {
            IFileBlobStrategy client = null;
            switch (fileBlob.Version)
            {
                case FileStorageVersion.BasicFileBlobV1:
                    client = new BasicFileBlobStrategy(BlobProvider);
                    break;

                case FileStorageVersion.ZippedFileBlobV1:
                    client = new ZipFileBlobStrategy(BlobProvider);
                    break;

                default:
                    return false;
            }
            return client.DownloadFileBlob(fileBlob, targetFile, progressTracker);
        }

        public FileBlob UploadFileBlob(string fileName, ProgressTracker<string> progressTracker = null)
        {
            //Currently we only use ZippedFile client in  the future this might be more intelligent
            var client = new ZipFileBlobStrategy(BlobProvider);
            //var client = new BasicFileBlobStrategy(BlobProvider);
            return client.UploadFileBlob(fileName, progressTracker);
        }
    }
}