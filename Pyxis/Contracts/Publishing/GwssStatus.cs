/******************************************************************************
GwssStatus.cs

begin		: Sept. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.GeoWebStreamService;

namespace Pyxis.Contract.Publishing
{
    public class GwssStatus // : IGwssStatus cannot deserialize list of interface types with JavascriptSerializer
    {
        public string Name { get; set; }

        public IList<PipelineStatusImpl> PipelinesStatuses { get; set; }

        public IServerStatus ServerStatus { get { return m_serverStatus; } set { m_serverStatus = value as ServerStatus; } }
        private ServerStatus m_serverStatus = new ServerStatus();

        public IList<OperationStatus> OperationsStatuses { get; set; }
    }

    public class OperationStatus : IOperationStatus
    {
        public IOperation Operation { get { return m_operation; } set { m_operation = value as Operation; } }
        private Operation m_operation = new Operation();

        public string Description { get; set; }

        public DateTime EndTime { get; set; }

        public string Guid { get; set; }

        public float? Progress { get; set; }

        public DateTime StartTime { get; set; }

        [JsonConverter(typeof(StringEnumConverter))]
        public OperationStatusCode StatusCode { get; set; }

        public string ToJson()
        {
            return JsonConvert.SerializeObject(this);
        }

        public static OperationStatus FromJson(string json)
        {
            OperationStatus operationStatus = null;
            if (json != null)
            {
                operationStatus = JsonConvert.DeserializeObject<OperationStatus>(json);
            }
            return operationStatus;
        }
    }

    public class NetworkStatus : INetworkStatus
    {
        public float UploadBandWidthKbps { get; set; }

        public float DownLoadBandWidthKbps { get; set; }

        public float AverageUploadUtilizationKbps { get; set; }
    }

    public class ServerStatus : IServerStatus
    {
        public double AvailableDiskSpaceMB { get; set; }

        public float AverageCPU { get; set; }

        public string Version { get; set; }

        public INetworkStatus NetworkStatus { get { return m_networkStatus; } set { m_networkStatus = value as NetworkStatus; } }
        private NetworkStatus m_networkStatus = new NetworkStatus();
    }

    public class PipelineStatusImpl : IPipelineStatus
    {
        public string ProcRef { get; set; }

        [JsonConverter(typeof(StringEnumConverter))]
        public PipelineStatusCode StatusCode { get; set; }
    }
}