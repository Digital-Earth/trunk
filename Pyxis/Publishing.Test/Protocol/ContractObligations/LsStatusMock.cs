/******************************************************************************
LsStatusMock.cs

begin		: Oct. 22, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.LicenseService;

namespace Publishing.Test.Protocol.ContractObligations
{
    internal class LsStatusMock : ILsStatus
    {
        public LsStatusMock()
        {
            PipelinesRequests = new List<IOperation>();
        }

        public IList<IOperation> PipelinesRequests { get; set; }
    }
}
