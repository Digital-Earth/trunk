using System;
using Newtonsoft.Json;

namespace Pyxis.Utilities
{
    /// <summary>
    /// AutomationLog defeine an interface between CLI output and automation log parsers to extract machine readable state changes.
    /// 
    /// In order to make them easy decectable the construct of automation log is:
    /// 
    /// *** [COMMAND] *** [DATA]
    /// 
    /// For example:
    /// 
    /// *** UPDATE *** progress=20 --> would allow the log parser to know that progress is now 20 percent.
    /// 
    /// *** PUSH *** results={id:12,dataSize:532673,status:"working"} --> wouuld allow the log parser to collect all results into an array.
    /// </summary>
    public static class AutomationLog
    {
        public static void EmitEvent(string type, string message)
        {
            Console.WriteLine(@"*** {0} *** {1}", type, message);
        }

        public static void EmitKeyValueEvent(string type, string key, object value)
        {
            EmitEvent(type, String.Format("{0}={1}", key, JsonConvert.SerializeObject(value)));
        }

        public static void UpdateInfo(string key, object value)
        {
            EmitKeyValueEvent("UPDATE", key, value);
        }

        /// <summary>
        /// Push an item into a named collection. the object will be seralized as json
        /// </summary>
        /// <param name="collection">collection name</param>
        /// <param name="value">json value</param>
        public static void PushInfo(string collection, object value)
        {
            EmitKeyValueEvent("PUSH", collection, value);
        }

        public static void Error(string message)
        {
            EmitEvent("ERROR", message);
        }

        public static void Info(string message)
        {
            EmitEvent("INFO", message);
        }

        public static void Endpoint(string endpointName, string url)
        {
            EmitKeyValueEvent("ENDPOINT", endpointName, url);
        }
    }
}
