/******************************************************************************
GwssStatus.cs

begin		: July 9, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.GeoWebStreamService;
using System;
using System.Collections.Generic;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    public class GwssStatus : IGwssStatus
    {
        public GwssStatus()
        {
            PipelinesStatuses = new List<IPipelineStatus>();
            ServerStatus = new ServerStatus();
            OperationsStatuses = new List<IOperationStatus>();
        }

        public string Name { get; set; }

        public string ServerType { get; set; }
      
        public string NodeId { get; set; }

        public IList<IOperationStatus> OperationsStatuses { get; set; }

        public IList<IPipelineStatus> PipelinesStatuses { get; set; }

        public IServerStatus ServerStatus { get; set; }

        public void AddPipelineStatus(IList<String> procRefs, PipelineStatusCode status)
        {
            foreach (var procref in procRefs)
            {
                IPipelineStatus pipelineStatus = new PipelineStatus();
                pipelineStatus.ProcRef = procref;
                pipelineStatus.StatusCode = status;
                PipelinesStatuses.Add(pipelineStatus);
            }
        }

        public override string ToString()
        {
            string msg = Name + ":\n";
            msg += "Server's Status: \n Available disk space: " + ServerStatus.AvailableDiskSpaceMB +
                "\n CPU :" + ServerStatus.AverageCPU +
                "\n version:" + ServerStatus.Version;
            msg += "\nPipelines:\n";
            foreach (var pipe in PipelinesStatuses)
            {
                msg += pipe.StatusCode + " :" + pipe.ProcRef + "\n";
            }

            msg += "\nCurrent Jobs:\n";
            foreach (var job in OperationsStatuses)
            {
                msg += job.Description +" " + job.StatusCode + "\n";
            }

            msg += "\n--------------------------------------------";

            return msg;
        }
    }
}