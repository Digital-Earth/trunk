using System;
using System.Collections.Generic;
using Pyxis.Contract.DataDiscovery;
using UrlDiscoveryStatus = GeoWebCore.Services.Storage.UrlDiscoveryStatus;

namespace GeoWebCore.Models
{
    /// <summary>
    /// Content of the Gallery on the GeoWebCore. Catalogs and Datasets (root level only), Imports and Linked Servers
    /// </summary>
    public class GalleryContent
    {
        /// <summary>
        /// Id of the Gallery
        /// </summary>
        public Guid Id { get; set; }

        /// <summary>
        /// Id of the Gallery
        /// </summary>
        public string Uri { get; set; }

        /// <summary>
        /// List of Root level Catalogs from local files
        /// </summary>
        public List<DataSetCatalog> Catalogs { get; set; }

        /// <summary>
        /// List of Linked Servers
        /// </summary>
        public List<DataSetCatalog> Servers { get; set; }

        /// <summary>
        /// List of Root level datasets
        /// </summary>
        public List<DataSet> DataSets { get; set; }

        /// <summary>
        /// List of All Imports on gallery
        /// </summary>
        public List<ImportDataSetRequestProgress> Imports { get; set; }

        /// <summary>
        /// List of Linked Urls
        /// </summary>
        public List<UrlDiscoveryStatus> Urls { get; set; }
    }
}