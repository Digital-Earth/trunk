using System;
using System.IO;

namespace PyxisCLI.Server.Cache
{
    /// <summary>
    /// Store cache files value on local file system
    /// </summary>
    internal class FileSystemCacheStorage : ICacheStorage
    {
        private readonly string m_rootPath;

        public FileSystemCacheStorage(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                throw new ArgumentNullException("path");
            }
            m_rootPath = path;
        }

        public static FileSystemCacheStorage CreateCacheWithName(string cacheName)
        {
            if (string.IsNullOrEmpty(cacheName))
            {
                throw new ArgumentNullException("cacheName");
            }
            var path = AppServices.getCacheDir(cacheName);
            return new FileSystemCacheStorage(path);
        }

        public bool Has(string key)
        {
            if (string.IsNullOrEmpty(key))
            {
                throw new ArgumentNullException("key");
            }

            var fileName = GetKeyPath(key);
            return File.Exists(fileName);
        }

        public string Read(string key)
        {
            if (string.IsNullOrEmpty(key))
            {
                throw new ArgumentNullException("key");
            }

            var fileName = GetKeyPath(key);
            return File.ReadAllText(fileName);
        }

        public byte[] ReadBytes(string key)
        {
            if (string.IsNullOrEmpty(key))
            {
                throw new ArgumentNullException("key");
            }

            var fileName = GetKeyPath(key);
            return File.ReadAllBytes(fileName);
        }

        public void Write(string key, string value)
        {
            if (string.IsNullOrEmpty(key))
            {
                throw new ArgumentNullException("key");
            }

            if (string.IsNullOrEmpty(value))
            {
                throw new ArgumentNullException("value");
            }

            if (Has(key))
            {
                return;
            }

            var fileName = GetKeyPath(key);

            //ensure directory exists
            EnsureDirectoryExisits(fileName);

            var tempFile = fileName + "." + DateTime.Now.Ticks;

            try
            {

                File.WriteAllText(tempFile, value);
                if (!File.Exists(fileName))
                {
                    File.Move(tempFile, fileName);
                }
            }
            catch (Exception ex)
            {
                if (!File.Exists(fileName))
                {
                    throw new Exception("failed to write " + fileName + " to disk", ex);
                }
            }
            finally
            {
                if (File.Exists(tempFile))
                {
                    File.Delete(tempFile);
                }
            }
        }

        public void WriteBytes(string key, byte[] value)
        {
            if (string.IsNullOrEmpty(key))
            {
                throw new ArgumentNullException("key");
            }

            if (value == null)
            {
                throw new ArgumentNullException("value");
            }

            if (Has(key))
            {
                return;
            }

            var fileName = GetKeyPath(key);

            //ensure directory exists
            EnsureDirectoryExisits(fileName);

            var tempFile = fileName + "." + DateTime.Now.Ticks;

            try
            {

                File.WriteAllBytes(tempFile, value);
                if (!File.Exists(fileName))
                {
                    File.Move(tempFile, fileName);
                }
            }
            catch (Exception ex)
            {
                if (!File.Exists(fileName))
                {
                    throw new Exception("failed to write " + fileName + " to disk", ex);
                }
            }
            finally
            {
                if (File.Exists(tempFile))
                {
                    File.Delete(tempFile);
                }
            }
        }

        private void EnsureDirectoryExisits(string fileName)
        {
            var directory = Path.GetDirectoryName(fileName);
            if (m_rootPath != directory && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }
        }

        private string GetKeyPath(string key)
        {
            try
            {
                return Path.Combine(m_rootPath, key);
            }
            catch (Exception e)
            {
                throw new Exception("failed to create file path for key: " + key, e);
            }
        }
    }
}
