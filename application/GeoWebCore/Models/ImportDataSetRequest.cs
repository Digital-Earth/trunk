using GeoWebCore.Services;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.Models
{
    /// <summary>
    /// Represent a request to import a DataSet.
    /// 
    /// This object is a DataSet object by itself that allow users to send DataSets directly to the GeoWebCore.
    /// However, ImportDataSetRequest allow users to add additional information about how to import the dataset.
    /// </summary>
    public class ImportDataSetRequest : DataSet
    {
        /// <summary>
        /// Provides SpatialReferenceSystem to be used when importing the DataSet.
        /// </summary>
        public SpatialReferenceSystem SRS { get; set; }

        /// <summary>
        /// Provide a GeoTag method to be used when importing the DataSet
        /// </summary>
        public GeoTagRequest GeoTag { get; set; }

        /// <summary>
        /// Provide a List of GeoTag method to be considered while importing the dataset.
        /// </summary>
        public List<GeoTagBasedOnSettings> GeoTagReferences { get; set; }

        /// <summary>
        /// Represents a GeoTag method.
        /// </summary>
        public class GeoTagRequest
        {
            /// <summary>
            /// Represents a GeoTag method based on a spatial join with a GeoSource.
            /// </summary>
            public GeoTagBasedOnSettings BasedOn { get; set; }

            /// <summary>
            /// Represents a GeoTag method as point based on latitude and longitude fields
            /// </summary>
            public GeoTagLatLonSettings LatLon { get; set; }
        }

        /// <summary>
        /// Represents a GeoTag method as point based on latitude and longitude fields
        /// </summary>
        public class GeoTagLatLonSettings
        {
            /// <summary>
            /// The field name to be used for latitude
            /// </summary>
            public string LatitudeFieldName { get; set; }

            /// <summary>
            /// The field name to be used for longitude
            /// </summary>
            public string LongitudeFieldName { get; set; }

            /// <summary>
            /// Logan Rakai
            /// Pyxis resolutionto be used for setting the accuracy level of the data.  
            /// </summary>
            public int? Resolution { get; set; }

            /// <summary>
            /// SpatialReferenceSystem used to parse latitude and longitude values.
            /// </summary>
            public SpatialReferenceSystem ReferenceSystem { get; set; }
        }

        /// <summary>
        /// Represents a GeoTag method based on a spatial join with a GeoSource.
        /// </summary>
        public class GeoTagBasedOnSettings
        {
            /// <summary>
            /// ResourceReference of the GeoSource to be used as source of geometries
            /// </summary>
            public ResourceReference ReferenceGeoSource { get; set; }


            /// <summary>
            /// Optional metadata for the referenceGeoSource.
            /// </summary>
            public Metadata ReferenceGeoSourceMetadata { get; set; }

            /// <summary>
            /// Reference field name to be used for the spatial join with the given reference GeoSource
            /// </summary>
            public string ReferenceFieldName { get; set; }

            /// <summary>
            /// field name in the given Dataset to be used for the spatial join operation.
            /// </summary>
            public string TargetFieldName { get; set; }
        }

        /// <summary>
        /// Create an empty ImpotDataSetRquest.
        /// </summary>
        public ImportDataSetRequest()
        {
        }

        /// <summary>
        /// Create an ImpotDataSetRquest with default setting based on import request.
        /// </summary>
        public ImportDataSetRequest(DataSet other) : base(other)
        {
        }

        /// <summary>
        /// Copy ImportDataSetRequest.
        /// </summary>
        /// <param name="other">ImportDataRequet to copy</param>
        public ImportDataSetRequest(ImportDataSetRequest other)
            : base(other)
        {
            if (other.SRS != null)
            {
                SRS = other.SRS.Clone();
            }

            if (other.GeoTag != null)
            {
                GeoTag = new GeoTagRequest();

                if (other.GeoTag.BasedOn != null)
                {
                    GeoTag.BasedOn = new GeoTagBasedOnSettings()
                    {
                        ReferenceFieldName = other.GeoTag.BasedOn.ReferenceFieldName,
                        TargetFieldName = other.GeoTag.BasedOn.TargetFieldName,
                        ReferenceGeoSource = other.GeoTag.BasedOn.ReferenceGeoSource.Clone()
                    };
                }

                if (other.GeoTag.LatLon != null)
                {
                    GeoTag.LatLon = new GeoTagLatLonSettings()
                    {
                        LatitudeFieldName = other.GeoTag.LatLon.LatitudeFieldName,
                        LongitudeFieldName = other.GeoTag.LatLon.LongitudeFieldName,
                        ReferenceSystem = other.GeoTag.LatLon.ReferenceSystem != null ? other.GeoTag.LatLon.ReferenceSystem.Clone() : null,
                        Resolution = other.GeoTag.LatLon.Resolution
                    };
                }
            }

            if (other.GeoTagReferences != null)
            {
                GeoTagReferences = other.GeoTagReferences
                    .Select(x => new GeoTagBasedOnSettings()
                    {
                        ReferenceFieldName = x.ReferenceFieldName,
                        TargetFieldName = x.TargetFieldName,
                        ReferenceGeoSource = x.ReferenceGeoSource.Clone()
                    }).ToList();
            }
        }
    }

    /// <summary>
    /// ImportDataSetRequest with additional progress information
    /// </summary>
    public class ImportDataSetRequestProgress : ImportDataSetRequest
    {
        /// <summary>
        /// Status of the request.
        /// 
        /// TODO: consider align with GwssPipelineStatus
        /// </summary>
        [JsonConverter(typeof(StringEnumConverter))]
        public enum RequestStatus
        {
            /// <summary>
            /// Import request is waiting in queue
            /// </summary>
            Waiting,

            /// <summary>
            /// Pipeline is been initialized and processed
            /// </summary>
            Processing,

            /// <summary>
            /// Pipeline failed to initialzied due to an error: missing settings or other erors
            /// </summary>
            Failed,

            /// <summary>
            /// Pipeline has been imported succefuly and not been published. This step will only happen if user didn't provide PublishingRequest
            /// </summary>
            Imported,

            /// <summary>
            /// Pipeline is been published to the the requested Gallery
            /// </summary>
            Publishing,

            /// <summary>
            /// Pipeline has been published succefully to the requested Gallery
            /// </summary>
            Published,

            /// <summary>
            /// Pipeline failed to be published to the Gallery
            /// </summary>
            PublishingFailed
        }

        /// <summary>
        /// A unique Id for this import request
        /// </summary>
        public string Id { get; set; }

        /// <summary>
        /// Status of the current import request
        /// </summary>
        public RequestStatus Status { get; set; }

        /// <summary>
        /// User that requestedthe import request
        /// </summary>
        public Guid User { get; set; }

        /// <summary>
        /// Destination gallery to publish the imported GeoSource into 
        /// </summary>
        public Guid Gallery { get; set; }

        /// <summary>
        /// ResourceReference to the GeoSourcethat been created
        /// </summary>
        public ResourceReference Result { get; set; }

        /// <summary>
        /// FailureDetails if import operation failed
        /// </summary>
        public FailureResponse FailureDetails { get; set; }

        /// <summary>
        /// Import request time. this is when the import request been queued.
        /// </summary>
        public DateTime RequestedTime { get; set; }

        /// <summary>
        /// Import Request start time. this is when the import request started to be processed
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public DateTime StartTime { get; set; }

        /// <summary>
        /// Import Reqest end time. this is when the import request has finished (success or failure)
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public DateTime? EndTime { get; set; }

        /// <summary>
        /// Creates a new ImportDataSetRequestProgress instance
        /// </summary>
        public ImportDataSetRequestProgress()
        {
        }

        private ImportDataSetRequestProgress(ImportDataSetRequest request)
            : base(request)
        {
            Id = UniqueHashGenerator.FromObject(request);
            Status = RequestStatus.Waiting;
            RequestedTime = DateTime.Now.ToUniversalTime();
        }

        /// <summary>
        /// Create ImportDataSetRequestProgress from a given ImportDataSetRequest 
        /// </summary>
        /// <param name="request">ImportDataSetRequest object.</param>
        /// <returns>ImportDataSetRequestProgress object.</returns>
        public static ImportDataSetRequestProgress FromRequest(ImportDataSetRequest request)
        {
            return new ImportDataSetRequestProgress(request);
        }
    }

    /// <summary>
    /// Short modal for sending progress on import request 
    /// </summary>
    public class ImportDataSetRequestProgressDetails
    {
        /// <summary>
        /// Id of the request
        /// </summary>
        public string Id { get; set; }

        /// <summary>
        /// Status of the request
        /// </summary>
        public ImportDataSetRequestProgress.RequestStatus Status { get; set; }

        /// <summary>
        /// Complition progress. number between 0 and 100
        /// </summary>
        public double Progress { get; set; }
    }
}