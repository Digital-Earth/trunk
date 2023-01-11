using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;

namespace Pyxis.Storage.BlobProviders
{
    public class SingleFileLocalBlobProvider : AbstractBlobProvider
    {
        private string m_file;
        private IndexBlob m_index;
        private long m_indexPosition;
        private int m_version;

        private ReaderWriterLockSlim m_readerWriterLock = new ReaderWriterLockSlim();
        private bool m_initialzed;

        public SingleFileLocalBlobProvider(string filePath)
        {
            m_file = filePath;
        }

        public void Initialize()
        {
            var directory = new FileInfo(m_file).Directory.FullName;
            if (!Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }
            var fileStream = new FileStream(m_file, FileMode.OpenOrCreate);

            //Load blob file
            if (fileStream.Length != 0)
            {
                var reader = new BinaryReader(fileStream);
                m_version = reader.ReadInt32();
                switch (m_version)
                {
                    case 1:
                        m_indexPosition = reader.ReadInt64();
                        fileStream.Seek(m_indexPosition, SeekOrigin.Begin);
                        m_index = IndexBlob.FromStream(fileStream);
                        break;

                    default:
                        throw new Exception("Unsupported single blob file version : " + m_version);
                }
            }
            else
            // Create a new blob file
            {
                var writer = new BinaryWriter(fileStream);
                m_version = 1;
                m_indexPosition = 12;
                m_index = new IndexBlob();
                writer.Write(m_version);
                writer.Write(m_indexPosition);
                SaveIndex(fileStream);
            }
            fileStream.Close();
            m_initialzed = true;
        }

        public override bool GetBlob(string key, Stream data)
        {
            ThrowIfNotInitialized();
            if (BlobExists(key))
            {
                var indexEntry = m_index.Get(key);
                var buffer = new byte[indexEntry.Size];
                try
                {
                    m_readerWriterLock.EnterReadLock();
                    var streamReader = new FileStream(m_file, FileMode.Open, FileAccess.Read);
                    streamReader.Seek(indexEntry.Location, SeekOrigin.Begin);
                    streamReader.Read(buffer, 0, indexEntry.Size);
                    streamReader.Close();
                }
                finally
                {
                    m_readerWriterLock.ExitReadLock();
                }
                data.Write(buffer, 0, indexEntry.Size);
                return true;
            }
            return false;
        }

        private void ThrowIfNotInitialized()
        {
            if (!m_initialzed)
            {
                throw new Exception("Single File Blob Provider Not Initialized!");
            }
        }

        public override IEnumerable<string> MissingBlobs(IEnumerable<string> keys)
        {
            ThrowIfNotInitialized();
            return keys.Where(x => !BlobExists(x)).ToList();
        }

        public override bool BlobExists(string key)
        {
            ThrowIfNotInitialized();
            return m_index.ContainsKey(key);
        }

        public override bool RemoveBlob(string key)
        {
            ThrowIfNotInitialized();
            try
            {
                m_readerWriterLock.EnterWriteLock();
                if (m_index.TryRemove(key))
                {
                    var fileStream = new FileStream(m_file, FileMode.Open, FileAccess.Write);
                    fileStream.Seek(m_indexPosition, SeekOrigin.Begin);
                    SaveIndex(fileStream);
                    fileStream.Close();
                    return true;
                }
                return false;
            }
            finally
            {
                m_readerWriterLock.ExitWriteLock();
            }
        }

        public override bool AddBlob(string key, Stream blob)
        {
            ThrowIfNotInitialized();
            if (BlobExists(key))
            {
                return false;
            }
            try
            {
                m_readerWriterLock.EnterWriteLock();
                var fileStream = new FileStream(m_file, FileMode.Open, FileAccess.Write);

                using (var oldIndexStream = new MemoryStream())
                using (var newIndexStream = new MemoryStream())
                {
                    //add the blob to the new index and find out the size
                    m_index.ToStream(oldIndexStream);
                    oldIndexStream.Position = 0;
                    m_index.Add(key, m_indexPosition, (int)blob.Length);
                    m_index.ToStream(newIndexStream);
                    newIndexStream.Position = 0;
                    var newIndexSize = newIndexStream.Length;

                    //back up the old index at (indexPosition +blobsize+ newIndexSize)
                    var backUpPosition = m_indexPosition + blob.Length + newIndexSize;
                    fileStream.Seek(backUpPosition, SeekOrigin.Begin);
                    oldIndexStream.CopyTo(fileStream);

                    //change the indexposition in file to point to the back up
                    var writer = new BinaryWriter(fileStream);
                    writer.Seek(4, SeekOrigin.Begin);
                    writer.Write(backUpPosition);

                    //Write the blob at the indexPosition (overwrite the old index)
                    fileStream.Seek(m_indexPosition, SeekOrigin.Begin);
                    blob.CopyTo(fileStream);

                    //write the new index
                    m_indexPosition = fileStream.Position;
                    newIndexStream.CopyTo(fileStream);

                    //change the indexposition in file to point to the new Index
                    fileStream.Seek(4, SeekOrigin.Begin);
                    writer.Seek(4, SeekOrigin.Begin);
                    writer.Write(m_indexPosition);

                    fileStream.Flush();
                    fileStream.Close();
                    return true;
                }
            }
            finally
            {
                m_readerWriterLock.ExitWriteLock();
            }
        }

        public void SaveIndex(Stream stream)
        {
            try
            {
                m_readerWriterLock.EnterWriteLock();
                stream.Seek(m_indexPosition, SeekOrigin.Begin);
                m_index.ToStream(stream);
                stream.Seek(4, SeekOrigin.Begin);
                var writer = new BinaryWriter(stream);
                writer.Write(m_indexPosition);
                writer.Flush();
            }
            finally
            {
                m_readerWriterLock.ExitWriteLock();
            }
        }
    }

    internal class IndexBlob
    {
        public struct IndexEntry
        {
            public long Location;
            public int Size;
        }

        public Dictionary<string, IndexEntry> Index;

        public IndexBlob()
        {
            Index = new Dictionary<string, IndexEntry>();
        }

        public static IndexBlob FromStream(Stream stream)
        {
            var reader = new BinaryReader(stream);
            var index = Newtonsoft.Json.JsonConvert.DeserializeObject<IndexBlob>(reader.ReadString());
            return index;
        }

        public void ToStream(Stream stream)
        {
            var writer = new BinaryWriter(stream);
            var indexJson = Newtonsoft.Json.JsonConvert.SerializeObject(this);
            writer.Write(indexJson);
            writer.Flush();
        }

        internal bool TryRemove(string key)
        {
            return Index.Remove(key);
        }

        internal bool ContainsKey(string key)
        {
            return Index.ContainsKey(key);
        }

        internal IndexEntry Get(string key)
        {
            return Index[key];
        }

        internal void Add(string key, long location, int size)
        {
            Index.Add(key, new IndexEntry() { Location = location, Size = size });
        }
    }
}