using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.DataDiscovery
{
    public interface IDiscoveryService
    {
        string ServiceIdentifier { get; }
        
        bool IsUriSupported(string uri);
        
        Task<DiscoveryResult> DiscoverAsync(IDiscoveryContext context);

        /// <summary>
        /// Create a DataImportService that can generate IProcess from DataSet and also Enrich the result GeoSource with more metadata
        /// </summary>
        /// <param name="dataSet">The data set to be opened.</param>
        /// <param name="permit">Provides credentials to access the resource, if required</param>
        /// <returns>An IDataSetImportService instance to import the dataset</returns>
        IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null);
    }

    public interface IDiscoveryContext
    {
        DiscoveryRequest Request { get; }

        Task<IDiscoveryNetworkResult> SendRequestAsync(IDiscoveryNetworkRequest request);
    }

    public class DiscoveryRequest
    {
        public string Uri { get; set; }

        public IPermit Permit { get; set; }

        public string ServiceIdentifier { get; set; }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            return Equals(obj as DiscoveryRequest);
        }

        protected bool Equals(DiscoveryRequest other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }

            if (ReferenceEquals(other, null))
            {
                return false;
            }

            return string.Equals(Uri, other.Uri) && Equals(Permit, other.Permit) && string.Equals(ServiceIdentifier, other.ServiceIdentifier);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = (Uri != null ? Uri.GetHashCode() : 0);
                hashCode = (hashCode*397) ^ (Permit != null ? Permit.GetHashCode() : 0);
                hashCode = (hashCode*397) ^ (ServiceIdentifier != null ? ServiceIdentifier.GetHashCode() : 0);
                return hashCode;
            }
        }

        public static bool operator == (DiscoveryRequest a,DiscoveryRequest b)
        {
            if (ReferenceEquals(a, b))
            {
                return true;
            }
            return !ReferenceEquals(a, null) && a.Equals(b);
        }

        public static bool operator !=(DiscoveryRequest a, DiscoveryRequest b)
        {
            return !(a == b);
        }
    }

    public class DiscoveryResult
    {
        public string Uri { get; set; }

        public IPermit Permit { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public DataSet DataSet { get; set; }

        /// <summary>
        /// This can be used by the discovery service to enumerate more datasets for the same lead.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<DataSet> AdditionalDataSets { get; set; }

        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Ignore)]
        public DateTime LastUpdated { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public SimpleMetadata Metadata { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<DiscoveryRequest> Leads { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<DiscoveryRequest> AdditionalRoots { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string ServiceIdentifier { get; set; }

        public DiscoveryResult()
        {
            
        }

        public DiscoveryResult(DiscoveryRequest request)
        {
            Uri = request.Uri;
            Permit = request.Permit;
        }

        public void AddLead(DiscoveryRequest request)
        {
            if (Leads == null)
            {
                Leads = new List<DiscoveryRequest>();
            }
            Leads.Add(request);
        }

        public void AddDataSet(DataSet dataset)
        {
            if (AdditionalDataSets == null)
            {
                AdditionalDataSets = new List<DataSet>();
            }
            AdditionalDataSets.Add(dataset);
        }
    }
}
