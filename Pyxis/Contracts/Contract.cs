/******************************************************************************
Contracts.cs

begin      : July 16 , 2013
copyright  : (c) 2013 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using Pyxis.Contract.Operations;
/// <summary>
/// These Interfaces define the contract between the License server and the Geo-Web Stream Service
/// Each GWSS reports to one LS that is in charge of managing the published pipelines.
/// LS provides GWSS with pipelines to publish.
/// </summary>

namespace Pyxis.Contract
{
    namespace Operations
    {
        public enum OperationStatusCode
        {
            Running,
            Failed,
            Completed,
            Cancelled
        }

        public enum OperationType
        {
            Import,
            Publish,
            Remove,
            Download,
            Process
        }

        public interface IOperation
        {
            OperationType OperationType { get; set; }

            Dictionary<string, string> Parameters { get; set; }
        }

        public interface IOperationStatus
        {
            string Description { get; set; }

            DateTime EndTime { get; set; }

            string Guid { get; set; }

            IOperation Operation { get; set; }

            float? Progress { get; set; }

            DateTime StartTime { get; set; }

            OperationStatusCode StatusCode { get; set; }
        }
    }

    namespace Services
    {
        namespace GeoWebStreamService
        {
            public enum PipelineStatusCode
            {
                Initializing,
                Downloading,
                Processing,
                Publishing,
                Removing,
                Published
            }

            public interface IGeoWebStreamService
            {
                IGwssStatus GetStatus();
            }

            public interface IGwssStatus
            {
                string Name { get; set; }

                string ServerType { get; set; }

                string NodeId { get; set; }

                IList<IOperationStatus> OperationsStatuses { get; set; }

                IList<IPipelineStatus> PipelinesStatuses { get; set; }

                IServerStatus ServerStatus { get; set; }
            }

            public interface INetworkStatus
            {
                float AverageUploadUtilizationKbps { get; set; }

                float DownLoadBandWidthKbps { get; set; }

                float UploadBandWidthKbps { get; set; }
            }

            public interface IPipelineStatus
            {
                string ProcRef { get; set; }

                PipelineStatusCode StatusCode { get; set; }
            }

            public interface IServerStatus
            {
                double AvailableDiskSpaceMB { get; set; }

                float AverageCPU { get; set; }

                INetworkStatus NetworkStatus { get; set; }

                string Version { get; set; }
            }
        }

        namespace LicenseService
        {
            public interface ILsStatus
            {
                IList<IOperation> PipelinesRequests { get; set; }
            }

            public interface IPipelineMetaData
            {
                string Definition { get; set; }

                string ProcRef { get; set; }
            }

            public interface IPipelineProcRef
            {
                string ProcRef { get; set; }
            }

            public interface IPipelineServerStatus
            {
                string NodeId { get; set; }

                IOperationStatus OperationStatus { get; set; }

                string ProcRef { get; set; }

                string Status { get; set; }
            }

            public interface IPublishedResponse
            {
                string Url { get; set; }
            }
        }
    }
}