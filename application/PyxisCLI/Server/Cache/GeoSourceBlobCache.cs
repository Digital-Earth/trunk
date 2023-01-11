using System;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace PyxisCLI.Server.Cache
{
    /// <summary>
    /// BlobCahce can be used to stored blobs for a geoSource.
    /// This can be used to store data that can speed up inital responses for a GeoSource, like Style and low res rhombuses
    /// </summary>
    internal class BlobCache
    {
        private readonly ICacheStorage m_storage;

        public BlobCache(ICacheStorage storage)
        {
            m_storage = storage;
        }

        private string GetFileKey(Guid geoSourceId, string filename, string category = null)
        {
            return category.HasContent()
                ? String.Format("{0}/{2}/{1}", geoSourceId, filename, category)
                : String.Format("{0}/{1}", geoSourceId, filename);
        }

        public byte[] GetBytes(Guid geoSourceId, string key, int size, string format)
        {
            var fileKey = GetFileKey(geoSourceId, key + "." + size, format);
            if (m_storage.Has(fileKey))
            {
                return m_storage.ReadBytes(fileKey);
            }
            return null;
        }

        public void WriteBytes(Guid geoSourceId, string key, int size, string format, byte[] buffer)
        {
            m_storage.WriteBytes(GetFileKey(geoSourceId, key + "." + size, format), buffer);
        }

        public string Get(Guid geoSourceId, string filename, string category = null)
        {
            var fileKey = GetFileKey(geoSourceId, filename, category);
            if (m_storage.Has(fileKey))
            {
                return m_storage.Read(fileKey);
            }
            return null;
        }

        public void Write(Guid geoSourceId, string filename, string category, string data)
        {
            m_storage.Write(GetFileKey(geoSourceId, filename, category), data);
        }
    }

    internal static class GeoSourceBlobCacheSingleton
    {
        private static readonly BlobCache s_cache;
        private const int ResolutionLimit = 3;

        static GeoSourceBlobCacheSingleton()
        {
            s_cache = new BlobCache(FileSystemCacheStorage.CreateCacheWithName("blobs-cache"));
        }

        private static string GetMetadataFileName<T>(Guid version, string extenstion = ".json")
        {
            var name = typeof(T).Name + extenstion;

            if (version != default(Guid))
            {
                name = String.Format("{0}.{1}", version, name);
            }

            return name;
        }

        public static T GetOrGenerateMetadata<T>(GeoSource geoSource, Func<GeoSource, T> generateMetadata)
        {
            try
            {
                var cachedValue = GetMetadata<T>(geoSource);

                if (cachedValue != null)
                {
                    return cachedValue;
                }
            }
            catch (Exception ex)
            {
                throw new Exception("Failed to recover metadata item " + typeof(T).Name + " for geoSource " + geoSource.Id, ex);
            }
            
            var value = generateMetadata.Invoke(geoSource);

            try
            {
                WriteMetadata(geoSource, value);
            }
            catch (Exception ex)
            {
                throw new Exception("Failed to save metadata item " + typeof(T).Name + " for geoSource " + geoSource.Id, ex);
            }

            return value;
        }

        public static T GetMetadata<T>(GeoSource geoSoruce)
        {
            return GetMetadata<T>(geoSoruce.Id, geoSoruce.Version);
        }

        public static T GetMetadata<T>(Guid geoSourceId, Guid geoSourceVersion = default(Guid))
        {
            var data = s_cache.Get(geoSourceId, GetMetadataFileName<T>(geoSourceVersion));
            return data.HasContent() ? JsonConvert.DeserializeObject<T>(data) : default(T);
        }

        public static void WriteMetadata<T>(GeoSource geoSoruce, T value)
        {
            WriteMetadata(geoSoruce.Id,value,geoSoruce.Version);
        }

        public static void WriteMetadata<T>(Guid geoSourceId, T value, Guid geoSourceVersion = default(Guid))
        {
            s_cache.Write(geoSourceId,GetMetadataFileName<T>(geoSourceVersion),null,JsonConvert.SerializeObject(value));
        }

        public static byte[] GetBlob(Guid geoSourceId, string key, int size, string format)
        {
            if (key.Length <= ResolutionLimit)
            {
                return s_cache.GetBytes(geoSourceId, key, size, format);
            }
            return null;
        }

        public static void WriteBlob(Guid geoSourceId, string key, int size, string format, byte[] value)
        {
            if (key.Length <= ResolutionLimit)
            {
                s_cache.WriteBytes(geoSourceId, key, size, format, value);
            }
        }

        public static T GetBlob<T>(Guid geoSourceId, string category, string filename)
        {
            var json = s_cache.Get(geoSourceId, filename, category);

            if (!json.HasContent())
            {
                return default(T);
            }

            return JsonConvert.DeserializeObject<T>(json);
        }

        public static void WriteBlob<T>(Guid geoSourceId, string category, string filename, T value)
        {
            s_cache.Write(geoSourceId, filename, category, JsonConvert.SerializeObject(value));
        }
    }
}
