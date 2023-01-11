using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Core;
using Pyxis.IO.Import;
using Pyxis.Utilities;

namespace GeoWebCore.Services.Cache
{
    internal static class InMemoryServersCatalogsCache
    {
        const int CacheSize = 100;

        //default time to keep GeoSouce alive.
        public static readonly TimeSpan CacheTimeToLive = TimeSpan.FromHours(1);

        private static readonly object s_cacheLock = new object();

        private static readonly LimitedSizeDictionary<string, TemporaryObject<CacheEntry>> s_cache =
            new LimitedSizeDictionary<string, TemporaryObject<CacheEntry>>(CacheSize);

        private class CacheEntry
        {
            private readonly Engine m_engine;

            private readonly string m_uri;
            private readonly Task<List<DataSetCatalog>> m_catalogsRequest;
            private readonly Task<List<DataSet>> m_datasetsRequest;

            public CacheEntry(Engine engine, string uri)
            {
                m_engine = engine;
                m_uri = uri;
                m_catalogsRequest = Task<List<DataSetCatalog>>.Factory.StartNew(BuildCatalogs);
                m_datasetsRequest = Task<List<DataSet>>.Factory.StartNew(BuildDataSets);
            }

            private List<DataSetCatalog> BuildCatalogs()
            {
                return m_engine.GetCatalogs(m_uri);
            }

            private List<DataSet> BuildDataSets()
            {
                return m_engine.GetDataSets(m_uri);
            }

            public List<DataSetCatalog> GetCatalogs()
            {
                return m_catalogsRequest.Result;
            }

            public List<DataSet> GetDatasets()
            {
                return m_datasetsRequest.Result;
            }
        }

        private static CacheEntry GetCacheEntry(string uri)
        {
            CacheEntry entry;
            lock (s_cacheLock)
            {
                TemporaryObject<CacheEntry> tempEntry;
                if (s_cache.TryGetValue(uri, out tempEntry) && tempEntry.TryGetValue(out entry))
                {
                    return entry;
                }
                entry = new CacheEntry(GeoSourceInitializer.Engine, uri);
                s_cache[uri] = new TemporaryObject<CacheEntry>(entry, CacheTimeToLive);
            }
            return entry;
        }

        public static List<DataSetCatalog> GetCatalogs(string uri)
        {
            var entry = GetCacheEntry(uri);

            return entry.GetCatalogs();
        }

        public static List<DataSet> GetDatasets(string uri)
        {
            var entry = GetCacheEntry(uri);

            return entry.GetDatasets();
        }
    }
}
