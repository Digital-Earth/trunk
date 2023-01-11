using System.Collections.Generic;
using System.Net;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;

namespace Pyxis.IO.Sources.Remote
{
    /// <summary>
    /// Base implementation of a web data service helper class
    /// </summary>
    public abstract class WebDataSetServiceBase : IWebDataSetDiscoveryService
    {
        /// <summary>
        /// Gets the title.  This is the (human readable) text that identifies this service.
        /// </summary>
        /// <value>The title.</value>
        public string Title { get; protected set; }

        /// <summary>
        /// Gets the service identifier.  This is the (machine readable) text that identifies this service.
        /// </summary>
        /// <value>The service identifier.</value>
        public string ServiceIdentifier { get; protected set; }

        /// <summary>
        /// The category of the discovery service
        /// </summary>
        public abstract string Category { get; }

        /// <summary>
        /// Checks if a URI presumably represents a resource of this kind.
        /// </summary>
        /// <param name="uri">URI of the resource</param>
        /// <returns>True if the url represents a resource of this kind, otherwise false.</returns>
        public abstract bool IsUriSupported(string uri);

        /// <summary>
        /// Get the catalogs accessible from this uri (non-recursive).
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The catalogs</returns>
        public abstract List<DataSetCatalog> GetCatalogs(string uri, IPermit permit);

        /// <summary>
        /// Get the data sets accessible from this uri.
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The data sets</returns>
        public abstract List<DataSet> GetDataSets(string uri, IPermit permit);

        /// <summary>
        /// Contacts an external server and returns an object that encapsulates general
        /// information about it and its GeoSource data sets.
        /// </summary>
        /// <param name="url">Address of the server</param>
        /// <param name="permit">Provides credentials to access the server, if required</param>
        /// <returns>Catalog object</returns>
        public abstract DataSetCatalog BuildCatalog(string url, IPermit permit);

        /// <summary>
        /// Create a pipeline for importing a GeoSource from a data set
        /// </summary>
        /// <param name="dataSet">The data set to be imported.</param>
        /// <param name="permit">Provides credentials to access the data set, if required</param>
        /// <returns>IProcess_SPtr object</returns>
        public abstract IProcess_SPtr BuildPipeline(DataSet dataSet, IPermit permit);

        /// <summary>
        /// Create a DataImportService that can generate IProcess from DataSet and also Enrich the result GeoSource with more metadata
        /// </summary>
        /// <param name="dataSet">The data set to be opened.</param>
        /// <param name="permit">Provides credentials to access the resource, if required</param>
        /// <returns>An IDataSetImportService instance to import the dataset</returns>
        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            return new DefaultWebDataSetServiceImport(this, dataSet, permit);
        }

        /// <summary>
        /// Retrieves network credential information.
        /// </summary>
        /// <param name="permit">Permission to access the resource.</param>
        /// <returns>NetworkCredential object</returns>
        protected NetworkCredential RetrieveCredentials(IPermit permit)
        {
            var networkPermit = permit as INetworkPermit;
            return networkPermit != null ? networkPermit.Credentials : null;
        }
    }

    internal class DefaultWebDataSetServiceImport : IDataSetImportService
    {
        public WebDataSetServiceBase DiscoveryService { get; set; }

        public DataSet DataSet { get; set; }
        public IPermit Permit { get; set; }

        public DefaultWebDataSetServiceImport(WebDataSetServiceBase webDataSetServiceBase, DataSet dataSet, IPermit permit)
        {
            DiscoveryService = webDataSetServiceBase;
            DataSet = dataSet;
            Permit = permit;
        }


        public IProcess_SPtr BuildPipeline(Engine engine, IImportSettingProvider settingsProvider)
        {
            return DiscoveryService.BuildPipeline(DataSet, Permit);
        }

        public void EnrichGeoSource(Engine engine, GeoSource geoSource)
        {
            return;
        }
    }
}