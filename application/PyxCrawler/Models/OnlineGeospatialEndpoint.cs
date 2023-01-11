using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace PyxCrawler.Models
{    
    public class OnlineGeospatialEndpoint
    {
        public int Id { get; set; }

        public Uri Uri { get; set; }

        public string Name { get; set; }

        public List<OnlineGeospatialService> Services { get; set; }

        public List<string> Tags { get; set; }

        /// <summary>
        /// Provider for the Gallery associated with the endpoint in the PYXIS Channel.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public Provider Provider { get; set; }

        public OnlineGeospatialEndpoint(Uri uri) : this()
        {
            Uri = uri;
            Name = uri.ToString();
        }

        public OnlineGeospatialEndpoint()
        {
            Services = new List<OnlineGeospatialService>();
        }

        public OnlineGeospatialService GetService(string protocol, string version)
        {
            var service = Services.FirstOrDefault(x => x.Protocol.Equals(protocol, StringComparison.InvariantCultureIgnoreCase) && x.Version.Equals(version, StringComparison.InvariantCultureIgnoreCase));

            if (service != null)
            {
                return service;
            }

            service = new OnlineGeospatialService(protocol.ToUpper(), version.ToUpper(), OnlineGeospatialServiceStatus.Unknown);
            Services.Add(service);
            return service;
        }

        public void SetTag(string tag)
        {
            if (Tags == null)
            {
                Tags = new List<string>(){tag};
                return;
            }
            if (!Tags.Contains(tag))
            {
                Tags.Add(tag);
            }
        }

        public void RemoveTag(string tag)
        {
            if (Tags != null)
            {
                Tags.Remove(tag);
                if (Tags.Count == 0)
                {
                    Tags = null;
                }
            }
        }
    }

    public class Information
    {
        public string Name;
        public string Value;
    }

    public enum InformationType
    {
        Info,        
        Warning,
        Error
    }
}