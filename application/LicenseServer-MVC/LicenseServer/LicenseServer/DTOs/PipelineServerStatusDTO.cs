using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Services.LicenseService;

namespace LicenseServer.DTOs
{
    public class PipelineServerStatusDTO //: IPipelineServerStatus
    {
        public string ProcRef { get; set; }
        public Guid NodeId { get; set; }
        public string Status { get; set; }
        public OperationStatus OperationStatus { get; set;}
    }
}