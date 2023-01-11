using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Core;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Sources.ArcGIS;
using Pyxis.IO.Sources.Local;
using Pyxis.IO.Sources.OGC;
using Pyxis.IO.Sources.Socrata;
using Pyxis.IO.Sources.UCAR;
using Pyxis.IO.Sources.Microsoft;

namespace Pyxis.IO.Import
{
    /// <summary>
    /// Extension methods for Pyxis.Core.Engine.
    /// </summary>
    public static class EngineExtensions
    {
        /// <summary>
        /// Register a DataSet Discovery Service into the Engine.
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object.</param>
        /// <param name="service">IDataSetDiscoveryService to register.</param>
        public static void RegisterDiscoveryService(this Engine engine, IDataSetDiscoveryService service)
        {
            if (!s_geoSourceImporters.Contains(service))
            {
                s_geoSourceImporters.Add(service);
            }
        }

        /// <summary>
        /// Register multiple objects responsible for importing GeoSources to enable importing of all kinds of GeoSources currently supported
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        public static void EnableAllImports(this Engine engine)
        {
            if (!s_geoSourceImporters.Any())
            {
                engine.RegisterDiscoveryService(new LocalDataSetDiscoveryService());
                engine.RegisterDiscoveryService(new OgcWebMapService());
                engine.RegisterDiscoveryService(new OgcWebCoverageService());
                engine.RegisterDiscoveryService(new OgcWebFeatureService());
                engine.RegisterDiscoveryService(new ArcGISGeoServices());
                engine.RegisterDiscoveryService(new OgcCatalogService(engine));

                DiscoveryTask.RegisterDiscoverService(new LocalDiscoveryService());
                DiscoveryTask.RegisterDiscoverService(new OgcWebMapDiscoveryService());
                DiscoveryTask.RegisterDiscoverService(new OgcWebFeatrureDiscoveryService());
                DiscoveryTask.RegisterDiscoverService(new OgcWebCoverageDiscoveryService());
                DiscoveryTask.RegisterDiscoverService(new ArcGISDiscoveryService());
                DiscoveryTask.RegisterDiscoverService(new SocrataDiscoveryService());
                DiscoveryTask.RegisterDiscoverService(new BingDiscoveryService());
                DiscoveryTask.RegisterDiscoverService(new UcarDiscoveryService());
            }
        }

        public static Task<DiscoveryResult> Discover(this Engine engine, string uri)
        {
            var task = new DiscoveryTask(engine,uri);

            return task.GetResult();
        }

        /// <summary>
        /// Get the category of importer that can read this Uri.
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="uri">Address of the server</param>
        /// <returns>The importer category if the Uri can be read, otherwise the empty string</returns>
        public static string GetImporterCategory(this Engine engine, string uri)
        {
            // Try all services to find a match for the URL and open the server
            foreach (var importer in s_geoSourceImporters)
            {
                if (importer.IsUriSupported(uri))
                {
                    return importer.Category;
                }
            }

            return "";
        }

        /// <summary>
        /// Does the uri represent a catalog of one or more data sets from this service?
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="uri">Address of the server</param>
        /// <returns>true if the uri represents a catalog of one or more data sets, otherwise false</returns>
        public static bool IsUriSupported(this Engine engine, string uri)
        {
            // check if this URI is supported by any importer
            return s_geoSourceImporters.Any(importer => importer.IsUriSupported(uri));
        }

        /// <summary>
        /// Get the catalogs available from this resource (non-recursive)
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="uri">Address of the resource</param>
        /// <param name="permit">Provides credentials to access the resource, if required</param>
        /// <returns>The catalogs</returns>
        public static List<DataSetCatalog> GetCatalogs(this Engine engine, string uri, IPermit permit = null)
        {
            // try all services to find a match for the uri and open the server
            foreach (var importer in s_geoSourceImporters)
            {
                try
                {
                    if (importer.IsUriSupported(uri))
                    {
                        return importer.GetCatalogs(uri, permit);
                    }
                }
                catch (Exception e)
                {
                    // A wide range of exceptions may be expected at this level; just write an info log
                    Trace.info(e.Message);
                }
            }

            return new List<DataSetCatalog>();
        }

        /// <summary>
        /// Get the data sets available from this resource
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="uri">Address of the resource</param>
        /// <param name="permit">Provides credentials to access the resource, if required</param>
        /// <returns>The data sets</returns>
        public static List<DataSet> GetDataSets(this Engine engine, string uri, IPermit permit = null)
        {
            // try all services to find a match for the uri and open the server
            foreach (var importer in s_geoSourceImporters)
            {
                try
                {
                    if (importer.IsUriSupported(uri))
                    {
                        return importer.GetDataSets(uri, permit);
                    }
                }
                catch (Exception e)
                {
                    // A wide range of exceptions may be expected at this level; just write an info log
                    Trace.info(e.Message);
                }
            }

            return new List<DataSet>();
        }

        /// <summary>
        /// Build the catalog of data sets available from this resource.
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="uri">Address of the resource</param>
        /// <param name="permit">Provides credentials to access the resource, if required</param>
        /// <returns>The catalog</returns>
        public static DataSetCatalog BuildCatalog(this Engine engine, string uri, IPermit permit = null)
        {
            // Try all services to find a match for the URL and open the server
            foreach (var importer in s_geoSourceImporters)
            {
                try
                {
                    var catalog = importer.BuildCatalog(uri, permit);
                    if (catalog != null)
                    {
                        return catalog;
                    }
                }
                catch (Exception e)
                {
                    // A wide range of exceptions may be expected at this level; just write an info log
                    Trace.info(e.Message);
                }
            }

            return null;
        }

        /// <summary>
        /// Import a GeoSource from a URI into PYXIS DERM. Default implementation.
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="dataSet">The data set to be imported</param>
        /// <returns>ImportGeoSourceProgress object</returns>
        public static ImportGeoSourceProgress BeginImport(this Engine engine, DataSet dataSet)
        {
            return BeginImport(engine, dataSet, new ImportSettingProvider());
        }

        /// <summary>
        /// Import a URI into PYXIS DERM using a specific setting provider.
        /// </summary>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="dataSet">Data set to import data from</param>
        /// <param name="settingsProvider">An IImportSettingProvider to use to provide additional settings</param>
        /// <returns>ImportGeoSourceProgress object (null if the URI is valid for none of the registered data importers)</returns>
        public static ImportGeoSourceProgress BeginImport(
            this Engine engine,
            DataSet dataSet,
            IImportSettingProvider settingsProvider
            )
        {
            if (s_geoSourceImporters.Count == 0)
            {
                throw new InvalidOperationException("No Discovery Services were registered, please call Engine.EnableAllImports() or Engine.RegisterDiscoveryService()");
            }

            var discoveryService = DiscoveryTask.FindDiscoveryServiceForUri(dataSet.Uri);

            if (discoveryService != null)
            {
                return new ImportGeoSourceProgress(engine, dataSet, discoveryService.GetDataSetImportService(dataSet), settingsProvider);
            }

            // TODO: remove this when we can...

            // Find an importer that will handle the URI
            // (unless no GeoSource importer has been registered that can process the URI)
            return (from importer in s_geoSourceImporters
                    where importer.IsUriSupported(dataSet.Uri)
                    select new ImportGeoSourceProgress(engine, dataSet, importer.GetDataSetImportService(dataSet), settingsProvider))
                    .FirstOrDefault();
        }

        /// <summary>
        /// List of supported GeoSource importers
        /// </summary>
        private static readonly List<IDataSetDiscoveryService> s_geoSourceImporters = new List<IDataSetDiscoveryService>();
    }
}
