using System.Net;
using Pyxis.Publishing.Permits;
using RestSharp;

namespace Pyxis.Publishing.Protocol
{
    public class AccessTokenGenerator : RestPublishingClient
    {
        public AccessTokenGenerator(string licenseServerUrl)
            : base(licenseServerUrl)
        {
        }

        public AccessTokenGenerator(string licenseServerUrl, string licenseServerRestPrefix)
            : base(licenseServerUrl, licenseServerRestPrefix)
        {
        }

        public AccessToken GetAccessToken(NetworkCredential credential)
        {
            var restRequest = new RestRequest(RestUrlBuilder.RequestPermit<AccessToken>(), Method.POST);
            restRequest.RequestFormat = DataFormat.Json;

            var request = new
            {
                UserName = credential.UserName,
                Password = credential.Password
            };
            restRequest.AddBody(request);

            return new AccessToken(Execute<AccessToken.TokenDetails>(restRequest, HttpStatusCode.OK));
        }
    }
}
