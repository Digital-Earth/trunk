/******************************************************************************
ServerMockClient.cs

begin		: Oct. 22, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using Publishing.Test.Protocol.ContractObligations;
using Pyxis.Contract.Services.GeoWebStreamService;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Protocol;
using Pyxis.Publishing.Test.Protocol;

namespace Pyxis.Publishing.Test
{
    internal class ServerMockClient: IGwssClient
    {
        public ILsStatus UpdateStatus(IGwssStatus status)
        {
            Console.WriteLine("Status for " + status.NodeId + ": " + status);
            var lsStatus = new LsStatusMock();
            lsStatus.PipelinesRequests.Add(new OperationMock());
            return lsStatus;
        }
    }
}
