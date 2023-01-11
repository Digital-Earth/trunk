using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;

namespace GeoWebCore.Services.Cache
{
    /// <summary>
    /// This class is used to create cache of GeoSources information, and athurization info
    /// </summary>
    internal class InMemoryGeoSourceCache
    {
        private readonly List<Func<Guid, GeoSource>> m_resolvers = new List<Func<Guid,GeoSource>>();

        private const int CacheSize = 1000;

        //default time to keep GeoSouce alive.
        public readonly TimeSpan CacheTimeToLive = TimeSpan.FromHours(1);

        //default timeo to keep GeoSource error
        public readonly TimeSpan CacheErrorTimeToLive = TimeSpan.FromSeconds(10);

        // this cache is for storing geoSource private visibility
        private readonly object m_cacheLock = new object();
        private readonly LimitedSizeDictionary<Guid, TemporaryObject<CacheEntry>> m_cache = new LimitedSizeDictionary<Guid, TemporaryObject<CacheEntry>>(CacheSize);

        private class CacheEntry
        {
            private Task<GeoSource> Request { get; set; }

            public GeoSource GetGeoSource()
            {
                return Request.Result;
            }

            public CacheEntry(InMemoryGeoSourceCache cache, Guid id)
            {
                Request = Task<GeoSource>.Factory.StartNew(() => ResolveGeoSource(cache,id));
            }

            private GeoSource ResolveGeoSource(InMemoryGeoSourceCache cache, Guid id)
            {
                foreach (var resolver in cache.m_resolvers)
                {
                    try
                    {
                        var geoSource = resolver(id);
                        if (geoSource != null)
                        {
                            return geoSource;
                        }
                    }
                    catch (Exception)
                    {
                        // ignored
                    }
                }
                return null;
            }

            public bool IsFailed
            {
                get { return Request.IsCompleted && GetGeoSource() == null; }
            }
        }

        public InMemoryGeoSourceCache()
        {
            
        }

        public InMemoryGeoSourceCache(TimeSpan timeToLive, TimeSpan errorTimeToLive)
        {
            CacheTimeToLive = timeToLive;
            CacheErrorTimeToLive = errorTimeToLive;
        }

        /// <summary>
        /// Add GeoSource Resolver. this function would be called if GeoSource doesn't exist inside the cache
        /// </summary>
        /// <param name="resolver">Function that return a GeoSource from an Id</param>
        public void AssignResolver(Func<Guid,GeoSource> resolver)
        {
            m_resolvers.Add(resolver);
        }

        /// <summary>
        /// Remove entry from the cache.
        /// </summary>
        /// <param name="id">Id of the GeoSource.</param>
        /// <returns>bool.</returns>
        public bool InvalidateGeoSource(Guid id)
        {
            lock (m_cache)
            {
                return m_cache.Remove(id);
            }
        }

        public bool InvalidateGeoSource(Guid id, TimeSpan minAge)
        {
            lock (m_cache)
            {
                if (m_cache.ContainsKey(id) && m_cache[id].Age > minAge)
                {
                    return m_cache.Remove(id);
                }
            }
            return false;
        }

        /// <summary>
        /// Try to fetch a GeoSource from a cache. if GeoSource doesn't exists, a thread-safe resolving process will be invoked.
        /// </summary>
        /// <param name="id">Id of the GeoSource.</param>
        /// <returns>GeoSource object or null if couldn't be resolved</returns>
        public GeoSource GetGeoSource(Guid id)
        {
            CacheEntry entry = null;

            //try to fetch entry from cache
            lock (m_cache)
            {
                TemporaryObject<CacheEntry> tempEntry;
                if (m_cache.TryGetValue(id, out tempEntry) && 
                    tempEntry.TryGetValue(out entry))
                {
                    if (entry.IsFailed && tempEntry.Age > CacheErrorTimeToLive)
                    {
                        entry = null;
                    }
                }

                if (entry == null)
                {
                    entry = new CacheEntry(this,id);                    
                    m_cache[id] = new TemporaryObject<CacheEntry>(entry,CacheTimeToLive);
                }
            }

            //resolve geosource. this will cause Task.Wait if GeoSource has not been resolved yet. awesome!
            return entry.GetGeoSource();
        }

        /// <summary>
        /// Clear Cache.
        /// </summary>
        internal void Clear()
        {
            lock (m_cacheLock)
            {
                m_cache.Clear();
            }
        }
    }
}
