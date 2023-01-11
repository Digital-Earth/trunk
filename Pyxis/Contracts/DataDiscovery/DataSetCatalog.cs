using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace Pyxis.Contract.DataDiscovery
{
    /// <summary>
    /// Data wrapper around a localized (within an URI) collection of data sets that represent GeoSources
    /// </summary>
    //[Serializable] - json serialization is broken when "Serializable" is present without DataMembers attributes
    public class DataSetCatalog
    {
        /// <summary>
        /// Type of the resource
        /// </summary>
        public string Type { get { return ResourceType.Catalog.ToString(); } }

        /// <summary>
        /// Type of the data
        /// </summary>
        public string DataType { get; set; }

        /// <summary>
        /// URI to use to connect to the resource
        /// </summary>
        public string Uri { get; set; }

        /// <summary>
        /// Collection of datasets inside the resource
        /// </summary>
        public List<DataSet> DataSets { get; set; }

        /// <summary>
        /// Collection of catalogs inside the resource
        /// </summary>
        public List<DataSetCatalog> SubCatalogs { get; set; }

        /// <summary>
        /// Resource Metadata
        /// </summary>
        public SimpleMetadata Metadata { get; set; }

        /// <summary>
        /// If Catalog is paged, NextPage can be used to request next catalog page
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public NextPageResult NextPage { get; set; }

        /// <summary>
        /// Constructor ensures there are no null fields.
        /// </summary>
        public DataSetCatalog()
        {
            DataType = DataSetType.Local.ToString();
            Uri = "";
            NextPage = null;
            DataSets = new List<DataSet>();
            SubCatalogs = new List<DataSetCatalog>();
            Metadata = new SimpleMetadata();
        }
    }
}
