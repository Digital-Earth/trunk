using System.Collections.Generic;
using LicenseServer.Models;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Services.GeoWebStreamService;

namespace LicenseServer.DTOs
{
    public class GeoSourceStatusDTO
    {
        public string Status { get; set; }
        public float? Progress { get; set; }
        [JsonConverter(typeof(StringEnumConverter))]
        public OperationStatusCode? StatusCode;

        static private readonly string s_initialStatus = "Waiting";
        static private readonly Dictionary<string, int> s_pipelineStatusOrder = new Dictionary<string, int> {
                {s_initialStatus, 0}, 
                {PipelineStatusCode.Removing.ToString(), 1},
                {PipelineStatusCode.Initializing.ToString(), 2},
                {PipelineStatusCode.Downloading.ToString(), 3},
                {PipelineStatusCode.Processing.ToString(), 4},
                {PipelineStatusCode.Publishing.ToString(), 5},
                {PipelineStatusCode.Published.ToString(), 6}
            };
        static private readonly Dictionary<string, string> s_pipelineLabels = new Dictionary<string, string> {
                {PipelineStatusCode.Removing.ToString(), "Removing"},
                {PipelineStatusCode.Initializing.ToString(), "Initializing"},
                {PipelineStatusCode.Downloading.ToString(), "Uploading"},
                {PipelineStatusCode.Processing.ToString(), "Processing Data"},
                {PipelineStatusCode.Publishing.ToString(), "Publishing"},
                {PipelineStatusCode.Published.ToString(), "Published"}
            };
        static private readonly Dictionary<OperationStatusCode, int> s_operationStatusOrder = new Dictionary<OperationStatusCode, int> {
                {OperationStatusCode.Cancelled, 0},
                {OperationStatusCode.Failed, 1},
                {OperationStatusCode.Running, 2},
                {OperationStatusCode.Completed, 3}
            };

        public static GeoSourceStatusDTO WebServiceGeoSourceStatus()
        {
            return new GeoSourceStatusDTO { Status = s_pipelineLabels[PipelineStatusCode.Published.ToString()] };
        }

        public static GeoSourceStatusDTO RemovedGeoSourceStatus()
        {
            return new GeoSourceStatusDTO { Status = PipelineDefinitionState.Removed.ToString() };
        }

        public static GeoSourceStatusDTO FromPublishedPipelineDetails(PublishedPipelineDetailsDTO pipelineDetails)
        {
            var status = new GeoSourceStatusDTO { Status = s_initialStatus };
            if (pipelineDetails.PublishingServerStatus == null)
            {
                return status;
            }
            foreach (var detail in pipelineDetails.PublishingServerStatus)
            {
                if (s_pipelineStatusOrder[detail.Status] > s_pipelineStatusOrder[status.Status])
                {
                    status.CopyStatus(detail);
                }
                else if (s_pipelineStatusOrder[detail.Status] == s_pipelineStatusOrder[status.Status])
                {
                    // take the most advanced operation status
                    if (detail.OperationStatus != null)
                    {
                        if (status.StatusCode == null 
                            || s_operationStatusOrder[detail.OperationStatus.StatusCode] > s_operationStatusOrder[status.StatusCode.Value]
                            || (s_operationStatusOrder[detail.OperationStatus.StatusCode] == s_operationStatusOrder[status.StatusCode.Value]
                                && detail.OperationStatus.Progress != null 
                                    && (status.Progress == null || detail.OperationStatus.Progress.Value > status.Progress.Value )
                                )
                            )
                        {
                            status.CopyStatus(detail);
                        }
                    }
                }
            }
            status.Status = s_pipelineLabels[status.Status];
            if (status.Status == s_pipelineLabels["Published"] && status.Progress != null)
            {
                status.Status += ", Creating Cache";
            }
            return status;
        }

        private void CopyStatus(PipelineServerStatusDTO detail)
        {
            Status = detail.Status;
            if (detail.OperationStatus != null)
            {
                Progress = detail.OperationStatus.Progress;
                StatusCode = detail.OperationStatus.StatusCode;
            }
        }
    }
}