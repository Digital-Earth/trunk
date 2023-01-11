using System;
using System.Collections.Generic;
using ApplicationUtility;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;

namespace Pyxis.Core.IO.GeoJson.Specialized
{
    /// <summary>
    /// A Pyxis.Core.IO.GeoJson.Geometry representing all cells matching a specific condition
    /// based on a Pyxis.Contract.Publishing.Resource.
    /// This is implements a specialized, non-standard GeoJSON geometry.
    /// </summary>
    public class ConditionGeometry : Geometry
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
        public ReferenceOrExpression Reference { get; set; }

        /// <summary>
        /// Gets or sets the property to apply the condition on.
        /// </summary>
        [JsonProperty("property", NullValueHandling = NullValueHandling.Ignore)]
        public string Property { get; set; }

        /// <summary>
        /// Gets or sets the feature identifier.
        /// </summary>
        [JsonProperty("range", NullValueHandling = NullValueHandling.Ignore)]
        public Range<object> Range { get; set; }

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.FeatureRefGeometry.
        /// </summary>
        public ConditionGeometry()
        {
            Type = GeoJsonObjectType.Condition;
        }

        internal ConditionGeometry(JObject json)
            : this()
        {
            ValidateType(json);
            if (json["resource"] != null)
            {
                Resource = json["resource"].ToObject<ResourceReference>();    
            }
            if (json["reference"] != null)
            {
                Reference = new ReferenceOrExpression()
                {
                    Reference = json["reference"].ToObject<string>()
                };
            }
            if (json["expression"] != null)
            {
                Reference = new ReferenceOrExpression()
                {
                    Expression = json["expression"].ToObject<string>()
                };
                if (json["symbols"] != null)
                {
                    Reference.Symbols = json["symbols"].ToObject<Dictionary<string, ReferenceOrExpression>>();
                }
            }
            if (json["property"] != null)
            {
                Property = json["property"].ToObject<string>();
            }
            if (json["range"] != null)
            {
                Range = json["range"].ToObject<Range<object>>();
            }
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.Specialized.ConditionGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.Specialized.ConditionGeometry.</returns>
        /// <exception cref="InvalidOperationException">Condition geometries can't be converted directly to PYXGeomtry. use WhereQuery for that</exception>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            throw new InvalidOperationException("Condition Geometry can only be converted using a where query");            
        }

        internal override JObject ToJObject()
        {
            var json = new JObject();
            json.Add("type", Type.ToString());
            if (Resource != null)
            {
                json.Add("resource", JToken.FromObject(Resource));
            }
            if (Reference != null)
            {
                if (Reference.Reference.HasContent())
                {
                    json.Add("reference", JToken.FromObject(Reference.Reference));    
                }
                else if (Reference.Expression.HasContent())
                {
                    json.Add("expression", JToken.FromObject(Reference.Expression));
                    if (Reference.Symbols != null)
                    {
                        json.Add("symbols", JObject.FromObject(Reference.Symbols));
                    }
                }
            }
            if (Property != null)
            {
                json.Add("property", JToken.FromObject(Property));
            }
            if (Range != null)
            {
                json.Add("range", JToken.FromObject(Range));
            }
            return json;
        }
    }
}
