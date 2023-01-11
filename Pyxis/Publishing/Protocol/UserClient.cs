using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http.Headers;
using System.Web.Http.OData;
using Newtonsoft.Json;
using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using Pyxis.Publishing.Permits;
using RestSharp;
using CertificatePermit = Pyxis.Publishing.Permits.CertificatePermit;

namespace Pyxis.Publishing.Protocol
{
    public class UserClient : RestPublishingClient, IPermissionClient
    {
        protected IPermitRetainer<AccessToken> TokenRetainer { get; set; }
        protected AccessToken Token { get { return TokenRetainer != null ? TokenRetainer.GetPermit() : null; } }

        public UserClient(RestPublishingClient basedOn, IPermitRetainer<AccessToken> tokenRetainer)
            : base(basedOn)
        {
            TokenRetainer = tokenRetainer;
        }

        /// <summary>
        /// Construct a user client based on a licenseServerUrl and authentication header with user token.
        /// </summary>
        /// <param name="licenseServerUrl">The license server url.</param>
        /// <param name="headers">Http headers containing user authentication token.</param>
        public UserClient(string licenseServerUrl, HttpRequestHeaders headers)
            : base(licenseServerUrl)
        {
            var userToken = "";
            
            // extract the authorization header from the request
            IEnumerable<string> values;
            if (headers.TryGetValues("Authorization", out values))
            {
                var prefix = "Bearer ";
                if (values.First().StartsWith(prefix))
                {
                    userToken = values.First().Remove(0, prefix.Length);
                }
            }

            var tokenDetails = new AccessToken.TokenDetails()
            {
                Token = userToken
            };

            TokenRetainer = new NonRenewableAccessTokenRetainer(tokenDetails);      
        }

        public UserProfile GetUserProfile()
        {
            var request = new RestRequest(RestUrlBuilder.GetUserProfile(), Method.GET);
            request.RequestFormat = DataFormat.Json;
            if (Token != null)
            {
                Token.Authenticate(request);
            }

            return Execute<UserProfile>(request);
        }

        public AccessToken.TokenDetails GetTokenDetails()
        {
            var request = new RestRequest(RestUrlBuilder.GetAccessToken(), Method.GET);
            request.RequestFormat = DataFormat.Json;
            if (Token != null)
            {
                Token.Authenticate(request);
            }

            var response = Execute<Newtonsoft.Json.Linq.JObject>(request);
            var tokenDetails = JsonConvert.DeserializeObject<AccessToken.TokenDetails>(response["Properties"]["Dictionary"].ToString());
            tokenDetails.Token = Token.Token;
            tokenDetails.ExpiresIn = Math.Max((long)(tokenDetails.Expires - DateTime.UtcNow).TotalSeconds, 0);
            tokenDetails.TokenType = "bearer";
            return tokenDetails;
        }

        // Handle OData
        public class OData<T>
        {
            public List<T> Items { get; set; }
        }

        /// <summary>
        /// Get the galleries accessible by this user.
        /// </summary>
        /// <returns>The galleries</returns>
        public List<Gallery> GetGalleries()
        {
            var request = new RestRequest(RestUrlBuilder.GetUserGalleries(GetUserId()), Method.GET);
            request.RequestFormat = DataFormat.Json;
            if (Token != null)
            {
                Token.Authenticate(request);
            }

            return Execute<OData<Gallery>>(request).Items;
        }

        private Guid m_userId = Guid.Empty;

        /// <summary>
        /// Get the Id for this user.
        /// </summary>
        /// <returns>The userID or Guid.Empty if the userId could not be retrieved.</returns>
        public Guid GetUserId()
        {
            if (m_userId == Guid.Empty)
            {
                m_userId = GetUserProfile().Id;
            }
            return m_userId;
        }

        public CertificatePermissionGrant GetPermission(PyxNetPermissionRequest request)
        {
            RestRequest restRequest = new RestRequest(RestUrlBuilder.RequestPermit<CertificatePermit>(), Method.POST);
            restRequest.RequestFormat = DataFormat.Json;
            restRequest.AddBody(request);
            if (Token != null)
            {
                Token.Authenticate(restRequest);
            }

            var response = m_client.Execute(restRequest);
            var permissionGrant = JsonConvert.DeserializeObject<CertificatePermissionGrant>(response.Content);

            if(permissionGrant != null)
            {
                return permissionGrant;
            }
            throw new Exception(string.Format("Permission grant for resources {0} failed from {1} with response: {2}", 
                string.Join(", ", request.ResourceIds.Select(r => r.ToString())), m_licenseServerUrl, response.ErrorMessage));
        }

        public ExternalApiKeyPermit RequestKeyPermit(Guid keyGroup)
        {
            RestRequest restRequest = new RestRequest(RestUrlBuilder.RequestPermit<ExternalApiKeyPermit>(keyGroup), Method.POST);
            if (Token != null)
            {
                Token.Authenticate(restRequest);
            }
            var keyGrant = this.Execute<KeyPermissionGrant>(restRequest);

            if (keyGrant.Permits.Any())
            {
                return new ExternalApiKeyPermit(keyGrant.Permits[0].Key, keyGrant.Permits[0].Expires, keyGrant.Permits[0].Issued);
            }
            throw new Exception("Failed to obtain API key permit for group " + keyGroup.ToString() + ": " 
                + string.Join(", ", keyGrant.NotGranted.Select(x => "(" + x.ResourceId.ToString() + " - " + x.Message + ")")));
        }

        public void ReleaseKeyPermit(Guid keyGroup,ExternalApiKeyPermit keyPermit)
        {
            RestRequest restRequest = new RestRequest(RestUrlBuilder.ReleasePermit<ExternalApiKeyPermit>(keyGroup), Method.POST);
            restRequest.RequestFormat = DataFormat.Json;
            restRequest.AddBody(keyPermit);
            if (Token != null)
            {
                Token.Authenticate(restRequest);
            }
            var response = m_client.Execute(restRequest);
            if (response.StatusCode != HttpStatusCode.OK)
            {
                throw new Exception("Failed to Release KeyPermit for group " + keyGroup);
            }
        }
    }
}
