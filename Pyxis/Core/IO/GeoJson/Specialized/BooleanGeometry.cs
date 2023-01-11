using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Newtonsoft.Json.Linq;
using Pyxis.Core.Analysis;

namespace Pyxis.Core.IO.GeoJson.Specialized
{
    /// <summary>
    /// A Pyxis.Core.IO.GeoJson.Geometry representing a geometry created from boolean operation.
    /// This is implements a specialized, non-standard GeoJSON geometry.
    /// </summary>
    public class BooleanGeometry : Geometry
    {
        /// <summary>
        /// Operations to apply for given IGeometries.
        /// </summary>
        public enum Operation
        {
            /// <summary>
            /// The area that is shared by two geometries (also conjunction)
            /// </summary>
            Intersection,

            /// <summary>
            /// The remaining area when one geometry is removed from another
            /// </summary>
            Subtraction,

            /// <summary>
            /// The area covered by both geometries (also union)
            /// </summary>
            Disjunction
        }

        /// <summary>
        /// a boolean clause which is made of an boolean Operation and a Geometry.
        /// </summary>
        public class Clause
        {
            /// <summary>
            /// The operation
            /// </summary>
            [JsonProperty("operation")]
            [JsonConverter(typeof(StringEnumConverter))]
            public Operation Operation { get; set; }

            /// <summary>
            /// The geometry
            /// </summary>
            [JsonProperty("geometry")]
            public IGeometry Geometry { get; set; }
        }

        /// <summary>
        /// Gets or sets the reference to a Pyxis.Contract.Publishing.Resource
        /// </summary>
        [JsonProperty("operations")]
        public List<Clause> Operations { get; set; }
        
        /// <summary>
        /// Initializes a new instance of Pyxis.Core.IO.GeoJson.Specialized.FeatureRefGeometry.
        /// </summary>
        public BooleanGeometry()
        {
            Type = GeoJsonObjectType.Boolean;
            Operations = new List<Clause>();
        }

        /// <summary>
        /// Copy constructor.
        /// </summary>
        /// <param name="other">The boolean geometry to copy.</param>
        public BooleanGeometry(BooleanGeometry other) : this()
        {
            Operations = new List<Clause>(other.Operations);
        }

        internal BooleanGeometry(JObject json)
            : this()
        {
            ValidateType(json);
            Operations = json["operations"].ToObject<List<Clause>>();            
        }

        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.Specialized.BooleanGeometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">resolution is been ingnored by this implementation</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.Specialized.BooleanGeometry.</returns>
        /// <exception cref="InvalidOperationException">Failed to calculate Boolean geometry.</exception>
        public override PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1)
        {
            var result = Calculate(engine, resolution);
            return result != null ? result.ToPyxGeometry(engine, resolution) : new PYXEmptyGeometry().clone();
        }

        internal override JObject ToJObject()
        {
            var json = new JObject
            {
                {"type", Type.ToString()},
                {"operations", JToken.FromObject(Operations)}                
            };
            return json;
        }

        private int CalculateResultResolution(Engine engine)
        {
            if (Operations.Count == 0)
            {
                throw new Exception("can't calculate result resolution without operations");
            }

            var geometry = Operations.First().Geometry;

            //find good resolution to run analysis on
            var circle = geometry.ToPyxGeometry(engine).getBoundingCircle();
            var resolution = Math.Min(
                PYXMath.knMaxAbsResolution,
                PYXBoundingCircle.estimateResolutionFromRadius(circle.getRadius()) + PYXTile.knDefaultTileDepth);

            return resolution;
        }

        /// <summary>
        /// Calculate the result of a Pyxis.Core.IO.GeoJson.Specialized.BooleanGeometry as IGeometry
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">Pyxis resolution to calculate the result geometry on. or -1 for default resolution</param>
        /// <returns>result IGeometry or null if result was empty geometry.</returns>        
        public IGeometry Calculate(Engine engine, int resolution = -1)
        {
            if (resolution == -1)
            {
                resolution = CalculateResultResolution(engine);
            }
            if (Operations.Count == 0)
            {
                return null;
            }
            if (Operations.Count == 1)
            {
                return Operations[0].Geometry;
            }
            if (!(Operations[0].Geometry is ConditionGeometry) && Operations[0].Operation == Operation.Disjunction)
            {
                return WhereQuery.Create(engine, Operations.Skip(1)).On(Operations[0].Geometry, resolution);
            }
            throw new InvalidOperationException("Unable to compute boolean geometry");
        }
    }
}
