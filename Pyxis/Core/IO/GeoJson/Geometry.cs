using System;
using Newtonsoft.Json.Linq;
using Pyxis.Core.IO.GeoJson.Specialized;
using Pyxis.Core.Measurements;

namespace Pyxis.Core.IO.GeoJson
{
    /// <summary>
    /// Represents a geometry that can be serialized to JSON.
    /// </summary>
    public abstract class Geometry : GeoJsonObject, IGeometry
    {
        /// <summary>
        /// Convert the Pyxis.Core.IO.GeoJson.Geometry to a PYXIS geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <param name="resolution">Resolution to use when converting into PYXGeometry, -1 for default</param>
        /// <returns>The PYXIS geometry created by converting the Pyxis.Core.IO.GeoJson.Geometry.</returns>
        public abstract PYXGeometry_SPtr ToPyxGeometry(Engine engine, int resolution = -1);

        /// <summary>
        /// Calculate the area the geometry covers on earth.
        /// </summary>
        /// <param name="engine">A running Pyxis.Core.Engine to use for calculating the PYXIS geometry.</param>
        /// <returns>Area of the geometry.</returns>
        public SphericalArea GetArea(Engine engine)
        {
            var pyxGeometry = ToPyxGeometry(engine);

            var resolution = Math.Min(40, PYXBoundingCircle.estimateResolutionFromRadius(pyxGeometry.getBoundingCircle().getRadius()) + 11);

            var tileCollection = PYXTileCollection.create();
            pyxGeometry.copyTo(tileCollection.get(), resolution);

            return tileCollection.getAreaOnReferenceShpere() * SphericalArea.SquareMeter;
        }

        /// <summary>
        /// Validate the JSON object is the expected Pyxis.Core.IO.GeoJson.GeoJsonObjectType.
        /// </summary>
        /// <param name="json">JSON object to validate.</param>
        /// <exception cref="System.Exception">The type of the JSON object was not what was expected.</exception>
        protected void ValidateType(JObject json)
        {
            if (json["type"].ToObject<string>() != this.Type.ToString())
            {
                throw new Exception("object type mismatch. Expected " + this.Type + " but got " + json["type"]);
            }
        }

        internal abstract JObject ToJObject();

        /// <summary>
        /// Create an instance of Pyxis.Core.IO.GeoJson.Geometry from a PYXIS geometry.
        /// </summary>
        /// <param name="geometry">The Pyxis.Core.Engine to perform the conversion with.</param>
        /// <returns>The Pyxis.Core.IO.GeoJson.Geometry created by converting the PYXIS geometry.</returns>
        /// <exception cref="System.Exception">A Pyxis.Core.IO.GeoJson.Geometry is not able to be created from <paramref name="geometry"/>.</exception>
        public static Geometry FromPYXGeometry(PYXGeometry_SPtr geometry)
        {
            var cell = pyxlib.DynamicPointerCast_PYXCell(geometry);
            if (cell.isNotNull())
            {
                return new CellGeometry(cell.getIndex());
            }
            var vectorGeometry2 = pyxlib.DynamicPointerCast_PYXVectorGeometry2(geometry);
            if (vectorGeometry2.isNotNull())
            {
                var region = vectorGeometry2.getRegion();

                var point = pyxlib.DynamicPointerCast_PYXVectorPointRegion(region);

                if (point.isNotNull())
                {
                    return new PointGeometry(point);
                }

                var circle = pyxlib.DynamicPointerCast_PYXCircleRegion(region);

                if (circle.isNotNull())
                {
                    return new CircleGeometry(circle);
                }

                var curve = pyxlib.DynamicPointerCast_PYXCurveRegion(region);

                if (curve.isNotNull())
                {
                    return new LineStringGeometry(curve);
                }

                var multiCurve = pyxlib.DynamicPointerCast_PYXMultiCurveRegion(region);

                if (multiCurve.isNotNull())
                {
                    return new MultiLineStringGeometry(multiCurve);
                }

                var multiPolygon = pyxlib.DynamicPointerCast_PYXMultiPolygonRegion(region);

                if (multiPolygon.isNotNull())
                {
                    return MultiPolygonGeometry.FromPYXGeometry(multiPolygon);
                }
            }

            var boundingBoxGeometry = pyxlib.DynamicPointerCast_PYXXYBoundsGeometry(geometry);
            if (boundingBoxGeometry.isNotNull())
            {
                return new BBoxGeometry(boundingBoxGeometry);
            }

            var tileCollection = pyxlib.DynamicPointerCast_PYXTileCollection(geometry);
            if (tileCollection.isNotNull())
            {
                return new TileCollectionGeometry(tileCollection);
            }
            
            throw new Exception("Unsupported geometry type");
        }
    }
}
