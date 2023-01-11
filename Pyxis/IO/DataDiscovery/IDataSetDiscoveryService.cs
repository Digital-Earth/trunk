using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.Import;
using Pyxis.Utilities;

namespace Pyxis.IO.DataDiscovery
{
    /// <summary>
    /// Interface for a class, capable of communicating with services that provide data set catalogs.
    /// </summary>
    public interface IDataSetDiscoveryService
    {
        /// <summary>
        /// Gets the title.  This is the (human readable) text that identifies this service.
        /// </summary>
        /// <value>The title.</value>
        string Title { get; }

        /// <summary>
        /// The category of the discovery service (corresponds to "DataType" in Catalog)
        /// </summary>
        string Category { get; }

        /// <summary>
        /// Does the uri represent a catalog of one or more data sets from this service?
        /// </summary>
        /// <param name="uri">Address of the server</param>
        /// <returns>true if the uri represents a catalog of one or more data sets, otherwise false</returns>
        bool IsUriSupported(string uri);

        /// <summary>
        /// Get the catalogs accessible from this uri (non-recursive)
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The catalogs</returns>
        List<DataSetCatalog> GetCatalogs(string uri, IPermit permit);

        /// <summary>
        /// Get the data sets accessible from this uri.
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The data sets</returns>
        List<DataSet> GetDataSets(string uri, IPermit permit);

        /// <summary>
        /// Build the catalog of data sets available from this resource.
        /// </summary>
        /// <param name="uri">Address of the resource</param>
        /// <param name="permit">Provides credentials to access the resource, if required</param>
        /// <returns>The catalog</returns>
        DataSetCatalog BuildCatalog(string uri, IPermit permit = null);

        /// <summary>
        /// Create a DataImportService that can generate IProcess from DataSet and also Enrich the result GeoSource with more metadata
        /// </summary>
        /// <param name="dataSet">The data set to be opened.</param>
        /// <param name="permit">Provides credentials to access the resource, if required</param>
        /// <returns>An IDataSetImportService instance to import the dataset</returns>
        IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null);
    }

    /// <summary>
    /// IDataSetImportService interface to control how to build and GeoSource from a DataSet
    /// 
    /// this class work hand in hand with ImportGeoSourceProgress class
    /// </summary>
    public interface IDataSetImportService
    {
        IProcess_SPtr BuildPipeline(Engine engine, IImportSettingProvider settingsProvider);
        void EnrichGeoSource(Engine engine, GeoSource geoSource);
    }
}
