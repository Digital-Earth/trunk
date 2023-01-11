using GeoWebCore.Models;
using GeoWebCore.Services;
using GeoWebCore.Utilities;
using GeoWebCore.WebConfig;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.IO.Import;
using System;
using System.Web.Http;
using GeoWebCore.Services.Cache;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Sources.Memory;


namespace GeoWebCore.Controllers
{
    /// <summary>
    /// ImportController allow GeoWebCore Clients to import datasets and get back a GeoSource to make future calls against that dataset/geojson
    /// </summary>
    [RoutePrefix("api/v1/Local/Import")]
    public class ImportController : ApiController
    {
        /// <summary>
        /// Creates a temporary in memory GeoSource from a GeoJson.FeatureCollection
        /// </summary>
        [HttpPost]
        [Route("GeoJson")]
        [TimeTrace()]
        public GeoSource ImportGeoJson(FeatureCollection featureCollection)
        {
            if (featureCollection == null || featureCollection.Features == null || featureCollection.Features.Count == 0)
            {
                throw new InvalidOperationException("Can't import empty feature collection");
            }

            //optimiziation to reuse geosources created for the same feature collection
            GeoSource geoSource;
            if (ImportedGeoSourceCache.TryGet(featureCollection, out geoSource))
            {
                return geoSource;
            }

            //create a new geoSource
            geoSource = GeoSourceInitializer.Engine.CreateInMemory(featureCollection);
            ImportedGeoSourceCache.Add(featureCollection, geoSource);

            //register our newly created GeoSource in the GeoSourceInitializer cache for later use
            GeoSourceInitializer.InitializeLocalGeoSource(geoSource);
            
            return geoSource;
        }

        /// <summary>
        /// Initialize the given geoSource on the local GeoWebCore for future requests.
        /// </summary>
        /// <param name="geoSource">GeoSource to initialize.</param>
        /// <returns>True if successes.</returns>
        [HttpPost]
        [Route("GeoSource")]
        [TimeTrace("geoSource")]
        public bool ImportGeoSource(GeoSource geoSource)
        {
            return GeoSourceInitializer.Initialize(geoSource) != null;
        }

        /// <summary>
        /// Import a Dataset and returns a GeoSource or FailureResponse if more information is needed (like SRS).
        /// </summary>
        /// <param name="request">ImportDataSetRequest or DataSet object</param>
        /// <returns>imported GeoSource for future use</returns>
        [HttpPost]
        [Route("DataSet")]
        [Obsolete()]
        [TimeTrace()]
        public GeoSource ImportDataSet(ImportDataSetRequest request)
        {
            if (!Request.RequestUri.IsLoopback)
            {
                throw new ApiException(FailureResponse.ErrorCodes.BadRequest,"Request is only allowed on local computer");
            }

            var settingProvider = new CustomImportSettingProvider(request);

            try
            {
                var geoSource = GeoSourceInitializer.Engine.BeginImport(request, settingProvider).Task.Result;

                //register inside the GeoWebCore for later use (by id)
                GeoSourceInitializer.InitializeLocalGeoSource(geoSource);

                return geoSource;
            }
            catch (Exception ex)
            {
                var errorResult = new FailureResponse(FailureResponse.ErrorCodes.ServerError,"Failed to import dataset due to: " + ex.Message);

                if (settingProvider.GetPossibleGetTagRequests() != null)
                {
                    errorResult.ErrorCode = FailureResponse.ErrorCodes.BadRequest;
                    errorResult.AddRequiredInformation(
                        "GeoTag",
                        "GeoTag options are required to import the requested dataset.",
                        settingProvider.GetPossibleGetTagRequests());
                    throw new ApiException(errorResult);
                }

                if (settingProvider.WasSpatialReferenceSystemMissing()) {
                    errorResult.ErrorCode = FailureResponse.ErrorCodes.BadRequest;
                    errorResult.AddRequiredInformation(
                        "SRS", 
                        "Spatial Reference System is required to import the requested dataset.",
                        new [] { SpatialReferenceSystem.CreateGeographical(SpatialReferenceSystem.Datums.WGS84) });
                    throw new ApiException(errorResult);
                }

                //pass the exception details up
                throw new ApiException(errorResult, ex);
            }
        }
    }    
}
