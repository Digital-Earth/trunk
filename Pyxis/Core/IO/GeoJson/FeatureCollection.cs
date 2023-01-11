using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// Represents a collection of features.
    /// </summary>
    public class FeatureCollection : GeoJsonObject
    {
        /// <summary>
        /// Gets or sets the collection of Pyxis.Core.IO.GeoJson.Feature.
        /// </summary>
        [JsonProperty("features")]
        public List<Feature> Features { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.FeatureCollection.
        /// </summary>
        public FeatureCollection()
        {
            Type = GeoJsonObjectType.FeatureCollection;
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.FeatureCollection from a PYXIS feature iterator.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use to create the Pyxis.Core.IO.GeoJson.FeatureCollection.</param>
        /// <param name="iterator">The PYXIS feature iterator.</param>
        /// <param name="geometry">The PYXIS geometry to check if a feature intersects with before adding to the collection, if specified.</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns>The created Pyxis.Core.IO.GeoJson.FeatureCollection.</returns>
        public static FeatureCollection FromPYXFeatureIterator(Engine engine,FeatureIterator_SPtr iterator, PYXGeometry geometry = null, FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            var fc = new FeatureCollection();
            fc.Features = new List<Feature>();

            for (; !iterator.end(); iterator.next())
            {
                var feature = iterator.getFeature();

                //check if feature really intersects the given geometry (if given)
                if (geometry == null || feature.getGeometry().intersects(geometry))
                {
                    fc.Features.Add(Feature.FromIFeature(feature, extractionFlags));
                }
            }

            return fc;
        }

        /// <summary>
        /// Create a Pyxis.Core.IO.GeoJson.FeatureCollection from a PYXIS feature iterator specifying how many features to skip and take.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use to create the Pyxis.Core.IO.GeoJson.FeatureCollection.</param>
        /// <param name="iterator">The PYXIS feature iterator.</param>
        /// <param name="skip">The number of features to skip over before adding to the collection.</param>
        /// <param name="take">The number of features to add to the collection.</param>
        /// <param name="geometry">The PYXIS geometry to check if a feature intersects with before adding to the collection, if specified.</param>
        /// <param name="extractionFlags">Define which parts of the feature to extract.</param>
        /// <returns>The created Pyxis.Core.IO.GeoJson.FeatureCollection.</returns>
        public static FeatureCollection FromPYXFeatureIterator(Engine engine, FeatureIterator_SPtr iterator, int skip, int take, PYXGeometry geometry = null, FeatureExtractionFlags extractionFlags = FeatureExtractionFlags.All)
        {
            var fc = new FeatureCollection();
            fc.Features = new List<Feature>();


            for (; !iterator.end(); iterator.next())
            {
                var feature = iterator.getFeature();

                //check if feature realy interests the given geometry (if given)
                if (geometry == null || feature.getGeometry().intersects(geometry))
                {
                    //check if we skip enough features
                    if (skip > 0)
                    {
                        skip--;
                        continue;
                    }

                    //check if we took enough features
                    if (take > 0)
                    {
                        take--;
                        fc.Features.Add(Feature.FromIFeature(feature, extractionFlags));
                        continue;
                    }

                    //iteration completed
                    break;
                }
            }

            return fc;
        }
    }
}
