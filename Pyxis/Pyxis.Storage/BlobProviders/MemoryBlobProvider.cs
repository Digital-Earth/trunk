using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Pyxis.Storage.BlobProviders
{
    public class MemoryBlobProvider : AbstractBlobProvider
    {
        private ConcurrentDictionary<string, byte[]> m_blobs;

        public MemoryBlobProvider()
        {
            m_blobs = new ConcurrentDictionary<string, byte[]>();
        }

        public override bool GetBlob(string key, Stream data)
        {
            if (!m_blobs.ContainsKey(key))
            {
                return false;
            }
            data.Write(m_blobs[key], 0, m_blobs[key].Length);
            return true;
        }

        public override bool AddBlob(string key, Stream dataStream)
        {
            if (m_blobs.ContainsKey(key))
            {
                return false;
            }
            var buffer = ReadFully(dataStream);
            return m_blobs.TryAdd(key, buffer);
        }

        public override bool RemoveBlob(string key)
        {
            byte[] data;
            return m_blobs.TryRemove(key, out data);
        }

        public override bool BlobExists(string key)
        {
            return m_blobs.ContainsKey(key);
        }

        public static byte[] ReadFully(Stream input)
        {
            using (MemoryStream ms = new MemoryStream())
            {
                input.CopyTo(ms);
                return ms.ToArray();
            }
        }

        public override IEnumerable<string> MissingBlobs(IEnumerable<string> keys)
        {
            return keys.Where(x => !BlobExists(x)).ToList();
        }
    }
}