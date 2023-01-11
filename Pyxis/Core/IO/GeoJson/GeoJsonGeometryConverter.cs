using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Pyxis.Core.IO.GeoJson.Specialized;

namespace Pyxis.Core.IO.GeoJson
{    
    internal class GeoJsonGeometryConverter : JsonConverter
    {
        public override bool CanConvert(Type objectType)
        {
            return objectType.IsSubclassOf(typeof(Geometry));
        }

        /// <summary>
        /// Pyxis.Core.IO.IGeometry is to be converted automatically into the right GeoJsonGeometry object.
        /// This class is placed on the IGeometry interface to achieve this.
        /// Which causes all derived classes to not be converted by the default JSON converter.
        /// In order to over come this issue all derived type need to have a constructor that accepts a JSON object.
        /// </summary>
        /// <exception cref="NotImplementedException">If Json geometry type is not supported</exception>
        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var jsonObject = (JObject)serializer.Deserialize(reader);

            switch (jsonObject["type"].ToString())
            {                
                case "Point":
                    return new PointGeometry(jsonObject);
                case "LineString":
                    return new LineStringGeometry(jsonObject);
                case "MultiLineString":
                    return new MultiLineStringGeometry(jsonObject);
                case "Polygon":
                    return new PolygonGeometry(jsonObject);
                case "MultiPolygon":
                    return new MultiPolygonGeometry(jsonObject);

                case "Circle":
                    return new CircleGeometry(jsonObject);
                case "BBox":
                    return new BBoxGeometry(jsonObject);
                case "PYXCell":
                    return new CellGeometry(jsonObject);
                case "PYXTileCollection":
                    return new TileCollectionGeometry(jsonObject);

                case "FeatureRef":
                    return new FeatureRefGeometry(jsonObject);
                case "Condition":
                    return new ConditionGeometry(jsonObject);
                case "Boolean":
                    return new BooleanGeometry(jsonObject);

                default:
                    throw new NotImplementedException();
            }        
        }

        /// <summary>
        /// I want IGeometry to be converted automatically into the right GeoJsonGeometry object
        /// This class is placed on the IGeometry interface.
        /// Which cause all derived class not be converted by the default json converter.
        /// In order to over come this issue all derived type need to to implement ToJObject function.
        /// </summary>
        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            serializer.Serialize(writer, (value as Geometry).ToJObject() );
        }
    }
}
