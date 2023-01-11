using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Web;

namespace Pyxis.Storage.BlobProviders
{
    public class LocalBlobProvider : AbstractBlobProvider
    {
        private string m_storageFolder;

        public LocalBlobProvider(string storageFolder)
        {
            m_storageFolder = storageFolder;
            if (!Directory.Exists(m_storageFolder))
            {
                Directory.CreateDirectory(m_storageFolder);
            }
        }

        public override bool GetBlob(string key, Stream data)
        {
            var path = GetStoragePath(key);
            if (File.Exists(path))
            {
                var reader = new FileStream(path, FileMode.Open);
                reader.CopyTo(data);
                reader.Close();
                return true;
            }
            return false;
        }

        private string GetStoragePath(string key)
        {
            return Path.Combine(m_storageFolder, HttpUtility.UrlEncode(key));
        }

        public override IEnumerable<string> MissingBlobs(IEnumerable<string> keys)
        {
            return keys.Where(x => !BlobExists(x)).ToList();
        }

        public override bool BlobExists(string key)
        {
            var path = GetStoragePath(key);
            return File.Exists(path);
        }

        public override bool RemoveBlob(string key)
        {
            return DeleteDirectory(GetStoragePath(key));
        }

        public override bool AddBlob(string key, Stream blob)
        {
            if (BlobExists(key))
            {
                return false;
            }
            var path = GetStoragePath(key);
            var writer = new FileStream(path + "-tmp", FileMode.Create);
            blob.CopyTo(writer);
            writer.Flush();
            writer.Close();
            File.Move(path + "-tmp", path);
            return true;
        }

        public static bool DeleteDirectory(string target_dir)
        {
            string[] files = Directory.GetFiles(target_dir);
            string[] dirs = Directory.GetDirectories(target_dir);

            foreach (string file in files)
            {
                File.SetAttributes(file, FileAttributes.Normal);
                File.Delete(file);
            }

            foreach (string dir in dirs)
            {
                DeleteDirectory(dir);
            }

            Directory.Delete(target_dir, false);
            return true;
        }
    }
}