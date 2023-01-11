using System.IO;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Provides some helper functions for JSON streaming.
    /// </summary>
    /// TODO: rename file Serialization.cs
    public static class JsonTool<T>
    {
        private static System.Runtime.Serialization.Json.DataContractJsonSerializer s_serializer;

        static JsonTool()
        {
            s_serializer = new System.Runtime.Serialization.Json.DataContractJsonSerializer(typeof(T));
        }

        public static string ToJson(T instance)
        {
            using (var tempStream = new MemoryStream())
            {
                s_serializer.WriteObject(tempStream, instance);
                return Encoding.Default.GetString(tempStream.ToArray());
            }
        }

        public static T FromJson(string json)
        {
            using (var tempStream = new MemoryStream(Encoding.Unicode.GetBytes(json)))
            {
                return (T)s_serializer.ReadObject(tempStream);
            }
        }
    }
}
