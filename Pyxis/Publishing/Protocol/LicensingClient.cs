using System;
using System.Diagnostics;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using RestSharp;

namespace Pyxis.Publishing.Protocol
{
    public class LicensingClient : RestPublishingClient, ILicensingClient
    {
        public LicensingClient(string licenseServerUrl)
            : base(licenseServerUrl)
        {
        }

        public LicensingClient(string licenseServerUrl, string licenseServerRestPrefix)
            : base(licenseServerUrl, licenseServerRestPrefix)
        {
        }

        public LicensingClient(RestPublishingClient basedOn)
            : base(basedOn)
        {
        }

        public LicenseTerms GetTermsOfUse()
        {
            var request = new RestRequest(RestUrlBuilder.TermsOfUse(), Method.GET);
            var response = m_client.Execute(request);

            if (response.StatusCode == System.Net.HttpStatusCode.OK)
            {
                return JsonConvert.DeserializeObject<LicenseTerms>(response.Content);
            }
            var exceptionMessage = "Failed to get terms of use from the License Server: " + m_licenseServerUrl + " due to the following error:" + response.ErrorMessage;
            Debug.WriteLine(exceptionMessage);
            throw new Exception(exceptionMessage);
        }
    }
}