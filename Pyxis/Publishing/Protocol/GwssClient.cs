/******************************************************************************
ServerClient.cs

begin		: Oct. 21, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;
using System.Diagnostics;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Services.GeoWebStreamService;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Protocol.ContractObligations;
using RestSharp;

namespace Pyxis.Publishing.Protocol
{
    public class GwssClient : RestPublishingClient, IGwssClient
    {
        public GwssClient(string licenseServerUrl)
            : base(licenseServerUrl)
        {
        }

        public GwssClient(string licenseServerUrl, string licenseServerRestPrefix)
            : base(licenseServerUrl, licenseServerRestPrefix)
        {
        }

        public ILsStatus UpdateStatus(IGwssStatus status)
        {
            RestRequest request = new RestRequest(RestUrlBuilder.GwssNotification(status.NodeId), Method.POST);
            request.RequestFormat = DataFormat.Json;
            request.AddBody(status);
            var response = m_client.Execute<LicenseServerStatusDTO>(request);

            if (response.ResponseStatus == ResponseStatus.Completed)
            {
                return response.Data.GetILsStatus();
            }

            Debug.WriteLine("Failed to report to License Server: " + m_licenseServerUrl + " due to the following error: " + response.ErrorMessage);
            return null;
        }

        private class LicenseServerStatusDTO
        {
            public LicenseServerStatusDTO()
            {
                PipelinesRequests = new List<Operation>();
            }

            public List<Operation> PipelinesRequests { get; set; }

            public ILsStatus GetILsStatus()
            {
                var status = new LsStatus();
                foreach (var pipeRequest in PipelinesRequests)
                {
                    status.PipelinesRequests.Add(pipeRequest);
                }
                return status;
            }
        }
    }
}
