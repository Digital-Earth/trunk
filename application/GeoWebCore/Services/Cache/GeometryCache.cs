using System.Collections.Generic;
using Pyxis.Core.IO;

namespace GeoWebCore.Services.Cache
{
    /// <summary>
    /// GeometryCache to stored geometries created by users as hash to shorten urls
    /// </summary>
    internal class GeometryCache : PersistentCache<IGeometry>
    {
        public GeometryCache() : base(FileSystemCacheStorage.CreateCacheWithName("geometries"), keySize: 0)
        {
        }

        protected override KeyValuePair<string,string> CreateEntry(IGeometry item)
        {
 	        var entry = base.CreateEntry(item);

            if (entry.Key == entry.Value)
            {
                return entry;
            }

            if (item is Pyxis.Core.IO.GeoJson.Geometry)
            {
                var geoJsonGeometry = item as Pyxis.Core.IO.GeoJson.Geometry;
                var newKey = geoJsonGeometry.Type + entry.Key;
                return new KeyValuePair<string,string>(newKey,entry.Value);
            }

            return entry;
        }
    }

    internal static class GeometryCacheSingleton
    {
        private static readonly GeometryCache s_cache;

        static GeometryCacheSingleton()
        {
            s_cache = new GeometryCache();
        }

        public static IGeometry Get(string key)
        {
            return s_cache.Get(key);
        }

        public static string Add(IGeometry geometry)
        {
            return s_cache.Add(geometry);
        }
    }
}
