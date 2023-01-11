/******************************************************************************
User.cs

begin		: Oct. 10, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Permits;
using Pyxis.Publishing.Protocol;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Net;
using Newtonsoft.Json;
using RestSharp;

namespace Pyxis.Publishing
{
    public class User
    {
        private readonly string m_userName;

        public string UserName { get { return m_userName; } }

        internal ILicenseServerClient LS { get; private set; }

        internal IAuthenticatedLicenseServerClient AuthLS { get; private set; }

        public User(IPermitRetainer<AccessToken> tokenRetainer, ILicenseServerClient licenseServerClient)
        {
            m_userName = tokenRetainer.GetPermit().UserName;
            LS = AuthLS = licenseServerClient.Authenticate(tokenRetainer);
        }

        public UserProfile GetProfile()
        {
            return AuthLS.GetUserProfile();
        }

        public IList<IPipelineServerStatus> GetPipelineDetails(string procRef)
        {
            return LS.Pipelines.GetPipelineServerStatuses(procRef);
        }

        public CertificatePermissionGrant GetPyxNetCertificate(List<Guid> resources, PyxNetNodeId nodeId)
        {
            var request = new PyxNetPermissionRequest() { Format = PermissionFormats.PyxNetV1, ResourceIds = resources, NodeId = nodeId };
            return AuthLS.User.GetPermission(request);
        }
 
        public Pyxis.Contract.IPermitRetainer<T> RequestPermit<T>(Guid resource) where T : Pyxis.Contract.IPermit
        {
            if (typeof(T) == typeof(Permits.ExternalApiKeyPermit))
            {
                return (Pyxis.Contract.IPermitRetainer<T>)new KeyPermitRetainer(AuthLS.User, resource);
            }

            if (typeof(T) == typeof(Permits.CertificatePermit))
            {
                throw new NotImplementedException();
            }
            
            throw new NotSupportedException("RequestPermit doesn't support Permit of type " + typeof(T).Name);
        }

        public List<Pyxis.Contract.IPermitRetainer<T>> RequestPermit<T>(List<Guid> resources) where T : Pyxis.Contract.IPermit
        {
            return resources.Select(resource => RequestPermit<T>(resource)).ToList();
        }

        private Guid m_userId = Guid.Empty;

        public Guid GetUserId()
        {
            if (m_userId == Guid.Empty)
            {
                m_userId = GetProfile().Id;
            }
            return m_userId;
        }

        public IPermitRetainer<AccessToken> TokenRetainer
        {
            get
            {
                return AuthLS.TokenRetainer;
            }
        }

        public string LicenseServerUrl
        {
            get
            {
                return AuthLS.LicenseServerUrl;
            }
        }

        /// <summary>
        /// Post a resource to the license server.
        /// </summary>
        /// <typeparam name="T">The resource type</typeparam>
        /// <param name="resource">The resource</param>
        public T PostResource<T>(T resource) where T : Resource
        {
            var authenticatedPipelineClient = new AuthenticatedPipelineClient(LicenseServerUrl, TokenRetainer);
            return authenticatedPipelineClient.PostResource(resource);
        }

        /// <summary>
        /// Send changes request for a resource to the license server.
        /// </summary>
        /// <typeparam name="T">The resource type</typeparam>
        /// <param name="resource">The resource</param>
        public void PutResource<T>(T resource) where T : Resource
        {
            var authenticatedPipelineClient = new AuthenticatedPipelineClient(LicenseServerUrl, TokenRetainer);
            authenticatedPipelineClient.PutResource(resource.Id,resource.Version,resource);
        }

        /// <summary>
        /// Get the galleries that are accessible to this user.
        /// </summary>
        /// <returns>The galleries</returns>
        public List<Gallery> GetGalleries()
        {
            return AuthLS.User.GetGalleries();
        }

        /// <summary>
        /// Get the access token details of the token authorizing the user requests.
        /// </summary>
        /// <returns>The TokenDetails object.</returns>
        public AccessToken.TokenDetails GetTokenDetails()
        {
            return AuthLS.User.GetTokenDetails();
        }
    }
}
