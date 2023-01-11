using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using System.IO;

namespace Pyxis.Storage.BlobProviders
{
    public class AzureBlobProvider : AbstractBlobProvider
    {
        private CloudBlobContainer m_container;

        public AzureBlobProvider(string connectionString)
        {
            // Retrieve storage account from connection string.
            var storageAccount = CloudStorageAccount.Parse(connectionString);

            // Create the blob client.
            var blobClient = storageAccount.CreateCloudBlobClient();

            // Retrieve reference to a previously created container.
            m_container = blobClient.GetContainerReference("blobserver");
        }

        public override bool GetBlob(string key, Stream data)
        {
            if (!BlobExists(key))
            {
                return false;
            }
            CloudBlockBlob blockBlob = m_container.GetBlockBlobReference(key);
            blockBlob.OpenRead().CopyTo(data);
            return true;
        }

        public override bool AddBlob(string key, Stream data)
        {
            if (BlobExists(key))
            {
                return false;
            }
            CloudBlockBlob blockBlob = m_container.GetBlockBlobReference(key);
            blockBlob.UploadFromStream(data);
            return true;
        }

        public override bool RemoveBlob(string key)
        {
            if (!BlobExists(key))
            {
                return false;
            }

            CloudBlockBlob blockBlob = m_container.GetBlockBlobReference(key);
            blockBlob.Delete();
            return true;
        }

        public override bool BlobExists(string key)
        {
            return m_container.GetBlockBlobReference(key).Exists();
        }
    }
}