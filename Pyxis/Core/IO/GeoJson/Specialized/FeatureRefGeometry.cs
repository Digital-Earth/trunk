using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Pyxis.Contract.Publishing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO.GeoJson.Specialized
{
    /// <summary>
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a feature referencing a Pyxis.Contract.Publishing.Resource.
    /// This is implements a specialized, non-standard GeoJSON geometry.
    /// </summary>
    public class FeatureRefGeometry : Geometry
    {
        /// <summary>
        /// Gets or sets the reference to a Pyxis.Contract.Publishing.Resource
        /// </summary>
        [Obsolete]
        [JsonProperty("resource", NullValueHandling = NullValueHandling.Ignore)]
        public ResourceReference Resource { get; set; }

        /// <summary>
        /// Gets or sets the reference to a Pyxis.Contract.Workspace.IImport
        /// </summary>
        [JsonProperty("reference", NullValueHandling = NullValueHandling.Ignore)]
        public string Reference { get; set; }

        /// <summary>
        /// Gets or sets the feature identifier.
        /// </summary>
        [JsonProperty("id")]
        public string FeatureId { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.FeatureRefGeometry.
        /// </summary>
        public FeatureRefGeometry()
        {
            Type = GeoJsonObjectType.FeatureRef;
        }

        internal FeatureRefGeometry(JObject json)
            : this()
        {
            ValidateType(json);
            if (json["resource"] != null)
            {
                Resource = json["resource"].ToObject<ResourceReference>();
            }
            if (json["reference"] != null)
            {
                Reference = json["reference"].ToObject<string>();
            }
            if (json["id"] != null)
            {
                FeatureId = json["id"].ToObject<string>();
            }
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.Specialized.FeatureRefGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.Specialized.FeatureRefGeometry.</returns>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            if (Resource != null)
            {
                return
                    pyxlib.QueryInterface_IFeatureCollection(engine.GetProcess(Resource).getOutput())
                        .getFeature(FeatureId)
                        .getGeometry();
            }
            if (Reference != null)
            {
                var geoSource = engine.ResolveReference(Reference);

                return
                    pyxlib.QueryInterface_IFeatureCollection(engine.GetProcess(geoSource).getOutput())
                        .getFeature(FeatureId)
                        .getGeometry();
            }

            throw new Exception("neither Reference nor Resource fields were provided");
        }

        internal override JObject ToJObject()
        {
            var json = new JObject();
            json.Add("type", (JToken)Type.ToString());
            if (Resource != null)
            {
                json.Add("resource", JToken.FromObject(Resource));
            }
            if (Reference != null)
            {
                json.Add("reference", JToken.FromObject(Reference));
            }
            json.Add("id", (JToken)FeatureId);
            return json;
        }
    }
}
