using Pyxis.Contract.Publishing;

namespace PyxisCLI.Server.Cache
{
    /// <summary>
    /// GeometryCache to stored styles created by users as hash to shorten urls
    /// </summary>
    internal class StyleCache : PersistentCache<Style>
    {
        public StyleCache()
            : base(FileSystemCacheStorage.CreateCacheWithName("styles"))
        {
        }
    }

    internal static class StyleCacheSingleton
    {
        private static readonly StyleCache s_cache;

        static StyleCacheSingleton()
        {
            s_cache = new StyleCache();
        }

        public static Style Get(string key)
        {
            return s_cache.Get(key);
        }

        public static string Add(Style geometry)
        {
            return s_cache.Add(geometry);
        }
    }
}
