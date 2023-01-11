using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Web;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace PyxCrawler.Models
{
    public class OnlineGeospatialBase
    {
        public int Id { get; set; }

        public string DatasetId { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> Tags { get; set; }
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> SystemTags { get; set; }
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> Fields { get; set; }
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<Domain> Domains { get; set; }

        /// <summary>
        /// Reference to the Resource in the PYXIS Channel.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public ServiceResourceReference Reference { get; set; }

        public Uri Server { get; set; }
        public List<AnnotatedOnlineGeospatialService> Services { get; set; }
    }

    public class AnnotatedOnlineGeospatialService : OnlineGeospatialService
    {
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public Uri Uri { get; set; }
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Definition { get; set; }
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string ProcRef { get; set; }
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Error { get; set; }
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Trace { get; set; }
    }

    public class OnlineGeospatialDataSet : OnlineGeospatialBase
    {
        private object _lock = new object();
        
        public Wgs84BoundingBox Wgs84BoundingBox { get; set; }
        
        public OnlineGeospatialDataSet()
        {
            Services = new List<AnnotatedOnlineGeospatialService>();
        }

        public OnlineGeospatialService GetService(string protocol, string version)
        {
            lock(_lock )
            {
                var service = Services.FirstOrDefault(x => x.Protocol == protocol && x.Version == version);
                if (service == null)
                {
                    service = new AnnotatedOnlineGeospatialService() { Protocol = protocol, Version = version, Status = OnlineGeospatialServiceStatus.Unknown };
                    Services.Add(service);
                }
                return service;
            }
        }

        public bool Match(params string[] searchTerms)
        {
            return searchTerms.All(x => (Name ?? "").IndexOf(x, StringComparison.InvariantCultureIgnoreCase) >= 0 
                || (Description ?? "").IndexOf(x, StringComparison.InvariantCultureIgnoreCase) >= 0 
                || Services.Any(y=>y.Protocol == x.ToUpper()) );
        }
    }

    public class Wgs84BoundingBox
    {
        public double North { get; set; }
        public double South { get; set; }
        public double East { get; set; }
        public double West { get; set; }
    }

    public class ServiceResourceReference
    {
        public ResourceReference Resource { get; set; }
        public OnlineGeospatialService Service { get; set; }
    }
}