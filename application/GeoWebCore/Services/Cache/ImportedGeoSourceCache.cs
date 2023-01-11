using System;
using System.Text;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Utilities;

namespace GeoWebCore.Services.Cache
{
    /// <summary>
    /// A Cache for all GeoJson FeatureCollection objects that have been imported into GeoSource (usually selections)
    /// </summary>
    internal static class ImportedGeoSourceCache
    {
        private static readonly LimitedSizeDictionary<string, GeoSource> s_hashedGeoSources = new LimitedSizeDictionary<string,GeoSource>(10000);
        private static readonly object s_hashedGeoSourcesLock = new object();

        /// <summary>
        /// Try to resolve a GeoSource from the given FeatureCollection.
        /// </summary>
        /// <param name="featureCollection">A GeoJson FeatureCollection.</param>
        /// <param name="geoSource">Output GeoSource if found.</param>
        /// <returns>Returns true if GeoSource found in Cache, else false.</returns>
        public static bool TryGet(FeatureCollection featureCollection, out GeoSource geoSource)
        {
            var hash = GetHash(featureCollection);

            lock (s_hashedGeoSourcesLock)
            {
                return s_hashedGeoSources.TryGetValue(hash, out geoSource);
            }
        }

        /// <summary>
        /// Register a GeoSource that match a GeoJson FeatureCollection
        /// </summary>
        /// <param name="featureCollection">A GeoJson FeatureCollection.</param>
        /// <param name="geoSource">GeoSource created for the given FeatueCollection</param>
        public static void Add(FeatureCollection featureCollection, GeoSource geoSource)
        {
            var hash = GetHash(featureCollection);

            lock (s_hashedGeoSourcesLock)
            {
                s_hashedGeoSources[hash] = geoSource;
            }
        }

        /// <summary>
        /// Generate a hash string for a given GeoJson FeatureCollection
        /// </summary>
        /// <param name="featureCollection">A GeoJson FeatureCollection.</param>
        /// <returns>String hash.</returns>
        private static string GetHash(FeatureCollection featureCollection)
        {
            var bytes = Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(featureCollection));
            var sha256ManagedChecksum = new System.Security.Cryptography.SHA256Managed();
            var checksum = sha256ManagedChecksum.ComputeHash(bytes);

            var key = "sha256=" + Convert.ToBase64String(checksum) + ",features=" + featureCollection.Features.Count;

            return key;
        }
    }
}
