using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Newtonsoft.Json;
using Pyxis.Utilities;

namespace PyxisCLI.Server.Cache
{
    /// <summary>
    /// Persistent Cache for objects. 
    /// This cache can keep small set of objects in memory (configured on contruction).
    /// Persistent objects are stored as files under AppServices.getCacheDir(cacheName) (configured on construction).
    /// 
    /// Objects are stored to disk using JsonConvert, and then SHA256 checkedsummed avoid duplicated entries.
    /// </summary>
    /// <typeparam name="T">Object Type</typeparam>
    internal class PersistentCache<T> where T : class
    {
        public const int DefaultCacheSize = 1000;
        public const int DefaultMaxKeySize = 256;

        /// <summary>
        /// Maximum number of entries to store in memory
        /// </summary>
        public int MaxCacheSize
        {
            get
            {
                return m_entries.SizeLimit;
            }
        }

        /// <summary>
        /// Number of entries in memory.
        /// </summary>
        public int InMemoryCount
        {
            get
            {
                lock (m_lock)
                {
                    return m_entries.Count;
                }
            }
        }

        private readonly object m_lock = new object();
        private readonly int m_maxKeySize;
        private readonly LimitedSizeDictionary<string, T> m_entries;
        private readonly ICacheStorage m_storage;

        public PersistentCache(ICacheStorage storage, int maxCacheSize = DefaultCacheSize, int keySize = DefaultMaxKeySize)
        {
            m_entries = new LimitedSizeDictionary<string, T>(maxCacheSize);
            m_storage = storage;
            m_maxKeySize = keySize;
        }

        /// <summary>
        /// try to recover an entry from cache for the given key. if entry not found or not recoved by the storage, return null
        /// </summary>
        /// <param name="key">cache entry key</param>
        /// <returns>return value or null if entry not been able to load</returns>
        public T Get(string key)
        {
            if (string.IsNullOrEmpty(key))
            {
                return null;
            }

            //check if key looks like a json string
            if (key[0] == '{' || key[0] == '[' || key[0] == '"' || key[0] == '\'')
            {
                return JsonConvert.DeserializeObject<T>(key);
            }

            lock (m_lock)
            {
                T item;
                if (m_entries.TryGetValue(key, out item))
                {
                    return item;
                }
            }

            ValidateKey(key);

            if (m_storage.Has(key))
            {
                var item = JsonConvert.DeserializeObject<T>(m_storage.Read(key));

                if (item != null)
                {
                    lock (m_lock)
                    {
                        m_entries[key] = item;
                    }
                    return item;
                }
            }

            throw new Exception("failed to recover " + typeof(T).Name + " from key '" + key + "'");
        }

        private void ValidateKey(string key)
        {
            if (!key.All(Char.IsLetterOrDigit))
            {
                throw new Exception("key can only be letters or digits");
            }
        }

        protected virtual KeyValuePair<string, string> CreateEntry(T item)
        {
            var serializedItem = JsonConvert.SerializeObject(item);

            if (serializedItem.Length < m_maxKeySize)
            {
                return new KeyValuePair<string, string>(serializedItem, serializedItem); ;
            }

            var bytes = Encoding.UTF8.GetBytes(serializedItem);
            var sha256ManagedChecksum = new System.Security.Cryptography.SHA256Managed();
            var checksum = sha256ManagedChecksum.ComputeHash(bytes);
            var key = serializedItem.Length + Convert.ToBase64String(checksum).Replace("=", "").Replace("/", "").Replace("+", "").Replace("-", "");

            return new KeyValuePair<string, string>(key, serializedItem);
        }

        /// <summary>
        /// Store a given item in the cache and return the unqiue generated key for that item.
        /// </summary>
        /// <param name="item">item to store in cache</param>
        /// <returns>string key to be used to retrive the given item</returns>
        public string Add(T item)
        {
            var entry = CreateEntry(item);

            if (entry.Key == entry.Value)
            {
                //key == value - no need to add into cachce
                return entry.Value;
            }

            //store it into disk if needed.

            if (!m_storage.Has(entry.Key))
            {
                m_storage.Write(entry.Key,entry.Value);
            }

            //store geometry in memory
            lock (m_lock)
            {
                m_entries[entry.Key] = item;
            }


            return entry.Key;
        }

    }
}
