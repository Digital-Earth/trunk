namespace Pyxis.Storage.FileSystemStorage
{
    /// <summary>
    /// Know how to create a Blob from a Directory and How to read a Directory from a blob
    /// </summary>
    internal interface IDirectoryBlobStrategy
    {
        bool DownloadDirectoryBlob(DirectoryBlob directoryBlob, string target, ProgressTracker<bool> tracker = null);

        DirectoryBlob UploadDirectoryBlob(string directoryPath, ProgressTracker<string> progressTracker = null);
    }

    /// <summary>
    /// Knows which FileBlobClient to use to store a directory and which one to use to retrieve a directory
    /// </summary>
    public class DirectoryBlobClient
    {
        private IBlobProvider BlobProvider;

        public DirectoryBlobClient(IBlobProvider BlobProvider)
        {
            this.BlobProvider = BlobProvider;
        }

        public bool DownloadDirectoryBlob(DirectoryBlob directoryBlob, string target, ProgressTracker<bool> tracker = null)
        {
            IDirectoryBlobStrategy client = null;
            switch (directoryBlob.Version)
            {
                case DirectoryStorageVersion.BasicDirectoryBlobV1:
                    client = new BasicDirectoryBlobStrategy(BlobProvider);
                    break;
            }
            if (client == null)
            {
                return false;
            }
            return client.DownloadDirectoryBlob(directoryBlob, target, tracker);
        }

        public DirectoryBlob UploadDirectoryBlob(string directoryPath, ProgressTracker<string> progressTracker = null)
        {
            var client = new BasicDirectoryBlobStrategy(BlobProvider);
            return client.UploadDirectoryBlob(directoryPath, progressTracker);
        }
    }
}