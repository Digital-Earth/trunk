using GeoWebCore.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using GeoWebCore.Services.Storage;
using Pyxis.IO.Import;
using GeoWebCore.Utilities;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using GeoWebCore.WebConfig;
using Pyxis.Contract.DataDiscovery;

namespace GeoWebCore.Services
{
    /// <summary>
    /// Service ImportRequestsQueue provide a queue for all import requests. 
    /// This service imports requests one by one.
    /// </summary>
    public static class ImportRequestsQueue
    {
        /// <summary>
        /// ImportRequestProgress represent the progress of an import dataset request.
        /// </summary>
        public class ImportRequestProgress
        {
            /// <summary>
            /// string Key (unique) represting this import request
            /// </summary>
            public string Key { get; private set; }

            /// <summary>
            /// ImportDataSetRequest settings
            /// </summary>
            public ImportDataSetRequest Request { get; private set; }

            /// <summary>
            /// PublishRequest if we require to publush the resulting GeoSource into the user gallery
            /// </summary>
            public PublishRequest PublishRequest { get; private set; }
            
            /// <summary>
            /// Resulting GeoSource or null if request is still in progress.
            /// </summary>
            public GeoSource GeoSource { get; private set; } 

            /// <summary>
            /// FailureDetails if ImportDataSetRequest has failed
            /// </summary>
            public FailureResponse FailureDetails { get; private set; }

            /// <summary>
            /// Exception that can be used to return to the user descring the error
            /// </summary>
            public Exception Exception { get; private set; }

            /// <summary>
            /// The current status of the import Request
            /// </summary>
            public ImportDataSetRequestProgress.RequestStatus Status { get; private set; }

            /// <summary>
            /// Import request time. this is when the import request been queued.
            /// </summary>
            public DateTime RequestedTime { get; set; }

            /// <summary>
            /// Import Request start time. this is when the import request started to be processed
            /// </summary>
            public DateTime StartTime { get; set; }

            /// <summary>
            /// Import Reqest end time. this is when the import request has finished (success or failure)
            /// </summary>
            public DateTime? EndTime { get; set; }

            /// <summary>
            /// Return true if the ImportRequestProgress is still in progress
            /// </summary>
            public bool InProgress
            {
                get
                {
                    switch (Status)
                    {
                        case ImportDataSetRequestProgress.RequestStatus.Processing:
                        case ImportDataSetRequestProgress.RequestStatus.Publishing:
                            return true;

                        default:
                            return false;
                    }
                }
            }

            /// <summary>
            /// Return true if the ImportRequestProgress has completed (success or failure)
            /// </summary>
            public bool IsCompleted
            {
                get
                {
                    switch (Status)
                    {
                        case ImportDataSetRequestProgress.RequestStatus.Failed:
                        case ImportDataSetRequestProgress.RequestStatus.Imported:
                        case ImportDataSetRequestProgress.RequestStatus.Published:
                        case ImportDataSetRequestProgress.RequestStatus.PublishingFailed:
                            return true;

                        default:
                            return false;
                    }
                }
            }

            private ImportGeoSourceProgress Progress { get; set; }

            private Task ImportTask { get; set; }


            /// <summary>
            /// Creating an ImportRequestProgress for a given ImportDataSetRequest and optional PublishRequest
            /// </summary>
            /// <param name="request">ImportDataSetRequest object</param>
            /// <param name="publishRequest">optional PublishRequest object</param>
            public ImportRequestProgress(ImportDataSetRequest request, PublishRequest publishRequest = null)
            {
                Request = request;
                Key = UniqueHashGenerator.FromObject(request);
                Status = ImportDataSetRequestProgress.RequestStatus.Waiting;
                PublishRequest = publishRequest;
                RequestedTime = DateTime.Now.ToUniversalTime();
            }

            /// <summary>
            /// Oncompleted event will be triggered once the operation completed (success or failure states)
            /// </summary>
            public event EventHandler OnCompleted;

            /// <summary>
            /// Starts the import requests.
            /// </summary>
            public void Start()
            {
                if (Status != ImportDataSetRequestProgress.RequestStatus.Waiting)
                {
                    return;
                }

                Status = ImportDataSetRequestProgress.RequestStatus.Processing;

                ImportTask = Task.Factory.StartNew(PerformRequest).ContinueWith((task) =>
                    {
                        EndTime = DateTime.Now.ToUniversalTime();
                        if (OnCompleted != null)
                        {
                            OnCompleted.Invoke(this, new EventArgs());
                        }
                    });
            }

            /// <summary>
            /// Waits for the ImportRequestProrges to complete execution within a specified time interval.
            /// </summary>
            /// <param name="timeout">time to wait before this function would return</param>
            /// <returns>true if task as been completed while wating</returns>
            public bool WaitForCompletion(TimeSpan timeout)
            {
                return ImportTask.Wait(timeout);
            }

            /// <summary>
            /// Waits for the ImportRequestProrges to complete execution. will throw exception on error.
            /// </summary>
            public void WaitForCompletion()
            {
                ImportTask.Wait();
            }

            private void PerformRequest()
            {
                StartTime = DateTime.Now.ToUniversalTime();

                DoImport();

                if (Status == ImportDataSetRequestProgress.RequestStatus.Failed)
                {
                    return;
                }
                else if (PublishRequest == null || !PublishRequest.Enabled)
                {
                    //no publish request - finish the request.
                    Status = ImportDataSetRequestProgress.RequestStatus.Imported;
                    return;
                }

                DoPublish();
            }

            private void DoImport()
            {
                var settingProvider = new CustomImportSettingProvider(Request);

                //TODO: move this to settings
                foreach (var reference in new[]
                {
                    //canadian provinces (good for canada stats)
                    new {Id = Guid.Parse("13d43d37-fcb2-435a-beef-7617142e4e7c"), Field = "NAME"},
                    //world political boundaries
                    new {Id = Guid.Parse("7664e383-9d2c-494d-b3ec-5ae708623565"), Field = "LEVEL_4_NA"},
                    //populated places from natural earth
                    new {Id = Guid.Parse("1d6c092e-38be-47fa-b196-a40292934499"), Field = "NAME"}
                })
                {
                    var geoSource = GeoSourceInitializer.GetGeoSource(reference.Id);

                    if (geoSource != null)
                    {
                        settingProvider.AddGeoTagReference(new ImportDataSetRequest.GeoTagBasedOnSettings
                        {
                            ReferenceGeoSource = ResourceReference.FromResource(geoSource),
                            ReferenceFieldName = reference.Field
                        });    
                    }
                }

                settingProvider.Register(typeof(DownloadLocallySetting),new DownloadLocallySetting()
                {
                    Path = new UserUrlsStorage().GetPathForLocalDownload(PublishRequest.GalleryId.ToString(),Request.Uri)
                });

                try
                {
                    Progress = GeoSourceInitializer.Engine.BeginImport(Request, settingProvider);
                    GeoSource = Progress.Task.Result;

                    //if not style was created by import request
                    if (GeoSource.Style == null)
                    {
                        //no publish request - lets create some random style to this GeoSource
                        var styledGeoSource = Pyxis.UI.Layers.Globe.StyledGeoSource.Create(GeoSourceInitializer.Engine, GeoSource);
                        GeoSource.Style = styledGeoSource.CreateDefaultStyle(RandomStyleGenerator.Create(GeoSource.Id));
                    }
                }
                catch (Exception ex)
                {
                    //strip aggregated execption from message
                    if (ex is AggregateException)
                    {
                        ex = ex.InnerException;
                    }

                    Status = ImportDataSetRequestProgress.RequestStatus.Failed;

                    FailureDetails = new FailureResponse(FailureResponse.ErrorCodes.ServerError, string.Format("Failed to import dataset due to: {0}", ex.Message));

                    if (settingProvider.GetPossibleGetTagRequests() != null && settingProvider.GetPossibleGetTagRequests().Count > 0)
                    {
                        FailureDetails.ErrorCode = FailureResponse.ErrorCodes.BadRequest;
                        FailureDetails.AddRequiredInformation(
                            "GeoTag",
                            "GeoTag options are required to import the requested dataset.",
                            settingProvider.GetPossibleGetTagRequests());

                        Exception = new ApiException(FailureDetails);
                    }
                    else if (settingProvider.WasSpatialReferenceSystemMissing())
                    {
                        FailureDetails.ErrorCode = FailureResponse.ErrorCodes.BadRequest;
                        FailureDetails.AddRequiredInformation(
                            "SRS",
                            "Spatial Reference System is required to import the requested dataset.",
                            new[] { SpatialReferenceSystem.CreateGeographical(SpatialReferenceSystem.Datums.WGS84) });
                        Exception = new ApiException(FailureDetails);
                    }
                    else
                    {
                        Exception = new ApiException(FailureDetails, ex);
                    }
                }
            }

            private void DoPublish()
            {
                Status = ImportDataSetRequestProgress.RequestStatus.Publishing;

                try
                {
                    PublishRequest.SetupGeoSource(GeoSource);
                    GeoSource = PublishGeoSourceHelper.PublishGeoSourceForUser(PublishRequest.Token, GeoSource, null);

                    Status = ImportDataSetRequestProgress.RequestStatus.Published;
                }
                catch (Exception ex)
                {
                    GeoSource = null;
                    Status = ImportDataSetRequestProgress.RequestStatus.PublishingFailed;
                    Exception = new ApiException(new FailureResponse(FailureResponse.ErrorCodes.ServerError, "Failed to publish resource"), ex);
                }
            }

            /// <summary>
            /// Convert into ImportDataSetRequestProgress object
            /// </summary>
            /// <returns>new instance of ImportDataSetRequestProgress represent the current status</returns>
            public ImportDataSetRequestProgress AsImportDataSetRequestProgress()
            {
                var result = ImportDataSetRequestProgress.FromRequest(Request);

                result.Status = Status;
                result.RequestedTime = RequestedTime;
                result.StartTime = StartTime;
                result.EndTime = EndTime;
                if (GeoSource != null)
                {
                    result.Result = ResourceReference.FromResource(GeoSource);
                }
                if (PublishRequest != null)
                {
                    result.User = PublishRequest.UserId;
                    result.Gallery = PublishRequest.GalleryId;
                }
                result.FailureDetails = FailureDetails;

                return result;
            }
        }

        private static readonly List<ImportRequestProgress> s_queue = new List<ImportRequestProgress>();
        private static readonly object s_queueLock = new object();

        /// <summary>
        /// Get a ImportRequestProgress from a given key
        /// </summary>
        /// <param name="key">ImportDataSetRequest key</param>
        /// <returns>ImportRequestProgress object or null if not been found</returns>
        public static ImportRequestProgress Get(string key)
        {
            lock (s_queueLock)
            {
                return s_queue.FirstOrDefault(x => x.Key == key);
            }
        }

        /// <summary>
        /// Get a ImportRequestProgress from a given ImportDataSetRequest
        /// </summary>
        /// <param name="request">ImportDataSetRequest object</param>
        /// <returns>ImportRequestProgress object or null if not been found</returns>
        public static ImportRequestProgress Get(ImportDataSetRequest request)
        {
            return Get(UniqueHashGenerator.FromObject(request));
        }

        /// <summary>
        /// Start a new ImportRequest or get a preview importing request if still in queue.
        /// </summary>
        /// <param name="request">ImportDataSetRequest object</param>
        /// <param name="publishRequest">Optional PublishRequest object</param>
        /// <param name="onRequestCompleted">Optional action to be invoked once operation has been completed</param>
        /// <returns></returns>
        public static ImportRequestProgress StartOrGetInProgress(ImportDataSetRequest request, PublishRequest publishRequest = null, Action<ImportRequestProgress> onRequestCompleted = null)
        {
            var key = UniqueHashGenerator.FromObject(request);

            lock (s_queueLock)
            {
                var requestInQueue = s_queue.FirstOrDefault(x => x.Key == key);

                //remove old completd requests
                if (requestInQueue != null && requestInQueue.IsCompleted)
                {
                    s_queue.Remove(requestInQueue);
                    requestInQueue = null;
                }

                //start a new request
                if (requestInQueue == null)
                {
                    requestInQueue = new ImportRequestProgress(request, publishRequest);
                    s_queue.Add(requestInQueue);

                    requestInQueue.OnCompleted += Request_OnCompleted;

                    if (onRequestCompleted != null)
                    {
                        requestInQueue.OnCompleted += (s,e) => onRequestCompleted(requestInQueue);
                    }

                    StartNextRequestIfNeeded();
                }

                return requestInQueue;
            }
        }

        /// <summary>
        /// Starts the next import request.
        /// </summary>
        private static void StartNextRequestIfNeeded()
        {
            lock (s_queueLock)
            {
                if (!s_queue.Any(x=>x.InProgress))
                {
                    var nextRequest = s_queue.FirstOrDefault(x => x.Status == ImportDataSetRequestProgress.RequestStatus.Waiting);
                    if (nextRequest != null)
                    {
                        nextRequest.Start();
                    }
                }
            }
        }

        static void Request_OnCompleted(object sender, EventArgs e)
        {
            StartNextRequestIfNeeded();
        }

        /// <summary>
        /// Get all ImportKeys in queue (also including old completed import requests)
        /// </summary>
        /// <returns>list of ImportDataSetRequestProgress</returns>
        public static List<ImportDataSetRequestProgress> GetAllImportRequestsProgress(Guid gallery)
        {
            lock (s_queueLock)
            {
                return s_queue
                    .Where(x => x.PublishRequest != null && x.PublishRequest.GalleryId == gallery)
                    .Select(x => x.AsImportDataSetRequestProgress()).ToList();
            }
        }
    }
}
