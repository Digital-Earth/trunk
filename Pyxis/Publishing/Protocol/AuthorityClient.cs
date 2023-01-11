using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using RestSharp;

namespace Pyxis.Publishing.Protocol
{
    class AuthorityClient : RestPublishingClient, IAuthorityClient
    {
        public AuthorityClient(string licenseServerUrl)
            : base(licenseServerUrl)
        {
        }

        public AuthorityClient(string licenseServerUrl, string licenseServerRestPrefix)
            : base(licenseServerUrl, licenseServerRestPrefix)
        {
        }

        public AuthorityClient(RestPublishingClient basedOn)
            : base(basedOn)
        {
        }

        public TrustedNodes GetTrustedNodes()
        {
            var request = new RestRequest(RestUrlBuilder.TrustedNodes(), Method.GET);
            var response = m_client.Execute(request);

            if (response.StatusCode == System.Net.HttpStatusCode.OK)
            {
                return JsonConvert.DeserializeObject<Pyxis.Contract.Publishing.TrustedNodes>(response.Content);
            }
            var exceptionMessage = "Failed to get trusted nodes from the License Server: " + m_licenseServerUrl + " due to the following error:" + response.ErrorMessage;
            Debug.WriteLine(exceptionMessage);
            throw new Exception(exceptionMessage);
        }
    }
}
