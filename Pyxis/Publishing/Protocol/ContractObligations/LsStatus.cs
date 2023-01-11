/******************************************************************************
LsStatus.cs

begin		: Oct. 21, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.LicenseService;

namespace Pyxis.Publishing.Protocol.ContractObligations
{
    internal class LsStatus : ILsStatus
    {
        public LsStatus()
        {
            PipelinesRequests = new List<IOperation>();
        }

        public IList<IOperation> PipelinesRequests { get; set; }
    }
}
