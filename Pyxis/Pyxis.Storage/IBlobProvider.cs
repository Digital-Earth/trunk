using Newtonsoft.Json;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Pyxis.Storage
{
    /// <summary>
    /// Provides methods for adding and removing blobs of data from storage.
    /// Blobs are passed as streams or objects and each is referenced by a unique key.
    /// </summary>
    public interface IBlobProvider
    {
        /// <summary>
        /// Get a blob with the given key from storage.
        /// </summary>
        /// <param name="key"> blob key</param>
        /// <param name="data"> stream to write the blob into</param>
        /// <returns> false if the blob is not found</returns>
        bool GetBlob(string key, Stream stream);

        /// <summary>
        /// Add a blob with the given key to storage.
        /// </summary>
        /// <param name="key"> blob key</param>
        /// <param name="payload"> contents of the blob</param>
        /// <returns> false if the key already exists</returns>
        bool AddBlob(string key, Stream payload);

        /// <summary>
        /// Removes the blob with the given key from storage.
        /// </summary>
        /// <param name="key"> blob key</param>
        /// <returns> false if the key does not exist </returns>
        bool RemoveBlob(string key);

        /// <summary>
        /// Checks if a key exists
        /// </summary>
        /// <param name="key"> blob key</param>
        /// <returns> true if a blob exists for the given key</returns>
        bool BlobExists(string key);

        /// <summary>
        /// Checks for the existence of multiple keys
        /// </summary>
        /// <param name="keys"> a list of keys to check for existence</param>
        /// <returns> a list of the missing keys</returns>
        IEnumerable<string> MissingBlobs(IEnumerable<string> keys);

        /// <summary>
        /// Retrieve an object stored as a blob
        /// </summary>
        /// <typeparam name="T"> object type</typeparam>
        /// <param name="key"> blob key</param>
        /// <returns> the stored object or null if the object is not found</returns>
        T GetBlob<T>(string key);

        /// <summary>
        /// Add an object as a blob. Return the key associated with the object
        /// even if the object has already been added to the blob.
        /// </summary>
        /// <param name="obj"> object to be stored</param>
        /// <returns> key associated with the object</returns>
        string AddBlob(object obj);

        /// <summary>
        /// Retrieve multiple blobs at once
        /// </summary>
        /// <param name="keys"> list of blob keys</param>
        /// <returns>  a dictionary containing the blob keys and associated blobs as strings</returns>
        IDictionary<string, string> GetBlobs(IEnumerable<string> keys);
    }

    /// <summary>
    /// Provides implementations for some of the IBlobProvider methods.
    /// </summary>
    public abstract class AbstractBlobProvider : IBlobProvider
    {
        public abstract bool GetBlob(string key, Stream data);

        public abstract bool AddBlob(string key, Stream payload);

        public abstract bool RemoveBlob(string key);

        public abstract bool BlobExists(string key);

        public virtual IEnumerable<string> MissingBlobs(IEnumerable<string> keys)
        {
            return keys.Where(x => !BlobExists(x)).ToList();
        }

        public T GetBlob<T>(string key)
        {
            using (var stream = new MemoryStream())
            {
                GetBlob(key, stream);
                // rely on JsonConvert.DeserializeObject<T> to create a null obj from an empty stream if blob not found

                stream.Position = 0;
                var reader = new StreamReader(stream);
                var obj = JsonConvert.DeserializeObject<T>(reader.ReadToEnd());
                return obj;
            }
        }

        public string AddBlob(object obj)
        {
            using (var stream = new MemoryStream())
            {
                var streamWriter = new StreamWriter(stream);
                streamWriter.Write(JsonConvert.SerializeObject(obj));
                streamWriter.Flush();
                stream.Position = 0;
                var key = BlobKeyFactory.GenerateKey(obj);
                AddBlob(key, stream);
                return key;
            }
        }

        public virtual IDictionary<string, string> GetBlobs(IEnumerable<string> keys)
        {
            var blobs = new Dictionary<string, string>();

            foreach (var key in keys)
            {
                using (var stream = new MemoryStream())
                {
                    if (GetBlob(key, stream))
                    {
                        stream.Position = 0;
                        var reader = new StreamReader(stream);
                        blobs.Add(key, reader.ReadToEnd());
                    }
                }
            }
            return blobs;
        }
    }
}