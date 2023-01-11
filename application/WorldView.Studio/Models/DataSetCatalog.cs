using System.Collections.Generic;
using System.Linq;
using Pyxis.Contract.Publishing;
using Pyxis.IO.DataSet;

namespace Pyxis.IO.DataSet
{
    /// <summary>
    /// Data wrapper around a localized (within an URI) collection of data sets that represent GeoSources
    /// </summary>
    class DataSetCatalogModel
    {
        /// <summary>
        /// Type of the resource
        /// </summary>
        public string Type { get; set; }

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
        public List<DataSetModel> DataSets { get; set; }

        /// <summary>
        /// Collection of datasets inside the resource
        /// </summary>
        public List<DataSetCatalogModel> SubCatalogs { get; set; }

        /// <summary>
        /// Resource Metadata
        /// </summary>
        public SimpleMetadata Metadata { get; set; }

        /// <summary>
        /// Named constructor
        /// </summary>
        public static DataSetCatalogModel Create(IDataSetCatalog catalog)
        {
            if (catalog == null)
            {
                return null;
            }
            // Copy the properties
            var result = new DataSetCatalogModel
            {
                Type = "Catalog",
                DataType = catalog.DataType ?? "",
                Uri = catalog.Uri ?? "",
                DataSets = new List<DataSetModel>(),
                SubCatalogs = new List<DataSetCatalogModel>(),
                Metadata = catalog.Metadata ?? new SimpleMetadata()
            };
            // Copy the data sets
            if (catalog.DataSetEntries != null)
            {
                // Copy the data sets one by one
                foreach (var dataSet in catalog.DataSetEntries)
                {
                    result.DataSets.Add(DataSetModel.Create(dataSet));
                }
            }
            // Copy the sub-catalogs
            if (catalog.SubCatalogs != null)
            {
                // Copy the data sets one by one
                foreach (var subCatalog in catalog.SubCatalogs)
                {
                    result.SubCatalogs.Add(DataSetCatalogModel.Create(subCatalog));
                }
            }

            // Custom type specific decorations
            switch (result.DataType)
                {
            case "Local":
                // For local catalogs set a simple description
                result.Metadata.Description = string.Format(
                    "Found {0} data set{1} and {2} sub-catalog{3}.",
                    result.DataSets.Count(), result.DataSets.Count() != 1 ? "s" : "",
                    result.SubCatalogs.Count(), result.SubCatalogs.Count() != 1 ? "s" : ""
                        );
                break;
            }

            return result;
        }
    }
}
