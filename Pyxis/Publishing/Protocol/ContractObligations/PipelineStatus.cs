/******************************************************************************
PipelineStatus.cs

begin		: Oct. 8, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract.Services.GeoWebStreamService;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    internal class PipelineStatus : IPipelineStatus
    {
        public PipelineStatusCode StatusCode { get; set; }

        public string ProcRef { get; set; }
    }
}
