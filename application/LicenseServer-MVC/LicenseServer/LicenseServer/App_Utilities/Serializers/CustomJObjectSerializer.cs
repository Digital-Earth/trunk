using System;
using System.Xml.Schema;
using MongoDB.Bson.IO;
using MongoDB.Bson.Serialization;
using MongoDB.Bson.Serialization.Serializers;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace LicenseServer.App_Utilities.Serializers
{
    /// <summary>
    /// Used to serialize JSON.NET JObjects to string until the mongo driver supports dynamics in driver release 2.0
    /// </summary>
    public class CustomJObjectSerializer : SerializerBase<JObject>
    {
        public override JObject Deserialize(BsonDeserializationContext context, BsonDeserializationArgs args)
        {
            var json = context.Reader.ReadString();
            return JObject.Parse(json);
        }

        public override void Serialize(BsonSerializationContext context, BsonSerializationArgs args, JObject value)
        {
            var json = Newtonsoft.Json.JsonConvert.SerializeObject(value);
            context.Writer.WriteString(json);
        }
    }
}