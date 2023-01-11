using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace PyxCrawler.Models
{
    public class OnlineGeospatialService
    {
        public string Protocol { get; set; }
        public string Version { get; set; }

        private OnlineGeospatialServiceStatus m_status;
        [JsonConverter(typeof(StringEnumConverter))]
        public OnlineGeospatialServiceStatus Status
        {
            get
            {
                return m_status;
            }
            set
            {
                m_status = value;
                StatusTime = DateTime.UtcNow;
            }
        }
        public DateTime StatusTime { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public Dictionary<string,string> Attributes { get; set; }

        public OnlineGeospatialService()
        {
        }

        public OnlineGeospatialService(string protocol, string version, OnlineGeospatialServiceStatus status)
        {
            Protocol = protocol;
            Version = version;
            Status = status;
        }

        public OnlineGeospatialService(OnlineGeospatialService service)
        {
            Protocol = service.Protocol;
            Version = service.Version;
            Status = service.Status;
            StatusTime = service.StatusTime;
        }

        public void SetAttribute(string name, string value)
        {
            if (Attributes == null)
            {
                Attributes = new Dictionary<string, string>();
            }
            Attributes[name] = value;
        }

        public void SetAttribute<T>(string name, T value)
        {
            SetAttribute(name, value.ToString());
        }

        public string GetAttribute(string name)
        {
            if (Attributes != null && Attributes.ContainsKey(name))
            {
                return Attributes[name];
            }
            return null;
        }

        public T GetAttribute<T>(string name, T defaultValue)
        {
            var result = GetAttribute(name);
            if (result == null)
            {
                return defaultValue;
            }
            return (T)Convert.ChangeType(result, typeof(T));
        }

        public void DeleteAttribute(string name)
        {
            if (Attributes != null && Attributes.ContainsKey(name))
            {
                Attributes.Remove(name);

                if (Attributes.Count == 0)
                {
                    Attributes = null;
                }
            }
        }
    }

    public enum OnlineGeospatialServiceStatus
    {
        Unknown,
        Crawled,
        Accessible,
        NeedsVerifying,
        Working,
        WorkingButNotUsable,
        Offline,
        Broken,
        Forbidden
    }
}