using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.LicenseService;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    internal class PipelineServerStatus : IPipelineServerStatus
    {
        public PipelineServerStatus()
        {
            OperationStatus = new OperationStatus();
        }

        public string ProcRef { get; set; }
    
        public string NodeId { get; set; }
        
        public string Status { get; set; }
        
        public IOperationStatus OperationStatus { get; set; }
    }
}
