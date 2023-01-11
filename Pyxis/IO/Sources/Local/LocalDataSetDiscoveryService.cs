using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.DataDiscovery;

namespace Pyxis.IO.Sources.Local
{
    /// <summary>
    /// Helper class for manipulations with local GeoSources
    /// </summary>
    public class LocalDataSetDiscoveryService : IDataSetDiscoveryService
    {
        /// <summary>
        /// Gets the title. This is the (human readable) text that identifies this service.
        /// </summary>
        /// <value>The title.</value>
        public string Title { get; private set; }

        /// <summary>
        /// The category of the discovery service
        /// </summary>
        public string Category { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        public LocalDataSetDiscoveryService()
        {
            Title = "Local Computer";
            Category = "Local";
        }

        /// <summary>
        /// Does the uri represent a catalog of one or more data sets from this service?
        /// </summary>
        /// <param name="uri">Address of the server</param>
        /// <returns>true if the url represents a catalog of one or more data sets, otherwise false</returns>
        public bool IsUriSupported(string uri)
        {
            return IsUriSupported(uri, IPipeBuilder.eCheckOptions.knPartial);
        }

        /// <summary>
        /// Does the uri represent a catalog of one or more data sets from this service?
        /// </summary>
        /// <param name="uri">Address of the catalog.</param>
        /// <param name="options">knRestrictive or knPermissive. If knRestrictive attempt to minimize the
        /// number of false positives.</param>
        /// <returns>true if the url represents a catalog of one or more data sets, otherwise false</returns>
        public bool IsUriSupported(string uri, IPipeBuilder.eCheckOptions options)
        {
            return PipelineConstructionUtility.IsDataSourceSupported(uri, options) || Directory.Exists(uri);
        }



        /// <summary>
        /// Create an empty file system catalog for the given url
        /// </summary>
        /// <param name="url">The url representing this catalog</param>
        /// <returns></returns>
        private DataSetCatalog CreateCatalog(string url)
        {
            return new DataSetCatalog()
            {
                DataType = DataSetType.Local.ToString(),
                Uri = url,
                Metadata = new SimpleMetadata()
                {
                    // Set the default Metadata values
                    Name = "Directory",
                    Description = "",
                    Tags = new List<string> { "File System", "Directory" }
                },

                SubCatalogs = new List<DataSetCatalog>(),
                DataSets = new List<DataSet>()
            };
        }

        private DataSetCatalog BuildCatalogFromDirectory(string directory)
        {
            var result = CreateCatalog(directory);

            var entries = Directory.EnumerateFileSystemEntries(directory);
            foreach (var entry in entries)
            {
                if (Directory.Exists(entry))
                {
                    //create empty catalog for sub directory
                    result.SubCatalogs.Add(CreateCatalog(entry));
                }
                else if (IsUriSupported(entry))
                {
                    //create catalog for each entry
                    var catalog = BuildCatalog(entry);

                    if (catalog != null)
                    {
                        if (catalog.DataSets != null)
                        {
                            result.DataSets.AddRange(catalog.DataSets);
                        }

                        if (catalog.SubCatalogs != null)
                        {
                            result.SubCatalogs.AddRange(catalog.SubCatalogs);
                        }
                    }
                }
            }

            return result;
        }

        /// <summary>
        /// Get the catalogs accessible from this uri (non-recursive).
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The catalogs</returns>
        public List<DataSetCatalog> GetCatalogs(string uri, IPermit permit)
        {
            var catalogs = new List<DataSetCatalog>();

            if (System.IO.File.Exists(uri) || Directory.Exists(uri))
            {
                var catalog = BuildCatalog(uri, permit);
                if (catalog != null && catalog.SubCatalogs != null)
                {
                    catalogs = catalog.SubCatalogs;
                }
            }

            if (Directory.Exists(uri) || catalogs.Count == 0)
            {
                // check the contents of the directory for more catalogs
                var entries = Directory.EnumerateFileSystemEntries(uri);
                foreach (var entry in entries)
                {
                    // entries are catalogs if they contain multiple data sets or subcatalogs
                    var added = false;
                    var catalog = BuildCatalog(entry);
                    if (catalog != null && 
                        ((catalog.DataSets != null && catalog.DataSets.Count > 1) ||
                        (catalog.SubCatalogs != null && catalog.SubCatalogs.Count > 0)))
                    {
                        catalog.DataSets = null;
                        catalog.SubCatalogs = null;
                        catalogs.Add(catalog);
                        added = true;
                    }

                    if (Directory.Exists(entry) && !added)
                    {
                        // if directory was not added above, add it here
                        catalogs.Add(CreateCatalog(entry));
                    }
                }
            }

            return catalogs;
        }


        /// <summary>
        /// Get the data sets accessible from this uri.
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The data sets</returns>
        public List<DataSet> GetDataSets(string uri, IPermit permit)
        {
            var dataSets = new List<DataSet>();

            if (System.IO.File.Exists(uri) || Directory.Exists(uri))
            {
                var catalog = BuildCatalog(uri, permit);
                if (catalog != null && catalog.DataSets != null)
                {
                    dataSets = catalog.DataSets;
                }
            }

            if (Directory.Exists(uri) && dataSets.Count == 0)
            {
                // check the contents of the directory for more data sets
                var entries = Directory.EnumerateFileSystemEntries(uri);
                foreach (var entry in entries)
                {
                    // entries are data sets if they contain a single data set
                    var catalog = BuildCatalog(entry);
                    if (catalog != null &&
                        ((catalog.DataSets != null && catalog.DataSets.Count == 1) &&
                        (catalog.SubCatalogs == null || catalog.SubCatalogs.Count == 0)))
                    {
                        dataSets.AddRange(catalog.DataSets);
                    }
                }
            }

            return dataSets;
        }

        /// <summary>
        /// Builds the catalog of data sets available from this server.
        /// </summary>
        /// <param name="uri">Address of the file or directory.</param>
        /// <param name="permit">Provides credentials to access the server, if required</param>
        /// <returns>The data set catalog or null if the catalog could not be loaded.</returns>
        public DataSetCatalog BuildCatalog(string uri, IPermit permit = null)
        {
            var catalog = DataDiscoveryExtensions.FromPyxis(PipelineConstructionUtility.BuildCatalog(uri));

            if (catalog == null && Directory.Exists(uri))
            {
                catalog = BuildCatalogFromDirectory(uri);
            }

            PopulateDomainsIfPossible(catalog);
            return catalog;
        }

        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            return new LocalDataSetImportService(dataSet, permit);
        }

        private void PopulateDomainsIfPossible(DataSetCatalog catalog)
        {
            if (catalog == null || catalog.DataSets == null || catalog.DataSets.Count == 0)
            {
                return;
            }

            var possibleGroups = catalog.DataSets.GroupBy(x => x.Uri + ":" + x.Layer).Where(x=>x.Count()>1);

            foreach (var group in possibleGroups)
            {
                var datasets = group.ToList();

                //checked if we can create domains based on fields
                if (datasets.Any(x => x.Fields == null) || datasets.Any(x => x.Fields.Count != 1))
                {
                    continue;
                }

                //populate field as a domain
                foreach (var dataset in datasets)
                {
                    if (dataset.Domains == null)
                    {
                        dataset.Domains = new List<Domain>();
                    }

                    dataset.Domains.Add(new Domain()
                    {
                        Name = "Field",
                        Type = PipelineSpecification.FieldType.String,
                        Value = dataset.Fields[0],
                        Metadata = new SimpleMetadata()
                        {
                            Name = "Field",
                            Description = ""
                        }
                    });
                }
            }
        }

        
        /// <summary>
        /// Checks whether a file system item should be excluded from GeoSource search results.
        /// </summary>
        /// <param name="path">Path to the file system item</param>
        /// <returns>true if it should be ignored, otherwise false</returns>
        public bool IsExcluded(string path)
        {
           // Verify whether the path is excluded from the search
            var input = path.ToLower();
            // Note: this verification may cause some (very few) false negatives
            return s_exclusionPatterns.Any(pattern => Regex.IsMatch(input, pattern));
        }

        /// <summary>
        /// Regular expression patterns for file system item that should be excluded from any GeoSource search
        /// </summary>
        private static readonly List<string> s_exclusionPatterns = new List<string>
        {
            // Program files and Program Data
            "^[a-z]:" + Regex.Escape("\\") + "program[ files|data].*",
            // C:\Windows
            "^c:" + Regex.Escape("\\") + "windows.*",
            // Recycle Bin
            "^[a-z]:" + Regex.Escape("\\$") + "recycle.bin.*",
            // Application Data
            "^.*appdata" + Regex.Escape("\\") + "[local|roaming|locallow]"
        };
    }
}
