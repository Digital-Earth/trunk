using System;
using System.Drawing;
using MongoDB.Bson.IO;
using MongoDB.Bson.Serialization;
using MongoDB.Bson.Serialization.Serializers;
using Pyxis.Contract.Converters;

namespace LicenseServer.App_Utilities.Serializers
{
    /// <summary>
    /// Custom serializer for System.Drawing.Color.
    /// Required because Mongo's BsonSerializer does not support structs by default.
    /// </summary>
    public class CustomColorSerializer : SerializerBase<Color>
    {
        public override Color Deserialize(BsonDeserializationContext context, BsonDeserializationArgs args)
        {
            return CssColorConverter.FromCss(context.Reader.ReadString());
        }

        public override void Serialize(BsonSerializationContext context, BsonSerializationArgs args, Color value)
        {
            context.Writer.WriteString(CssColorConverter.ToCss(value));
        }
    }
}