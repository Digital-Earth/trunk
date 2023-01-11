using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RestSharp;
using Pyxis.Publishing.Permits;
using Pyxis.Contract;

namespace Pyxis.Publishing.Protocol
{
    class ResourcesClient<T> : RestPublishingClient, IResourcesClient<T> where T : Pyxis.Contract.Publishing.Resource, new()
    {
        protected IPermitRetainer<AccessToken> TokenRetainer { get; set; }
        protected AccessToken Token { get { return TokenRetainer != null ? TokenRetainer.GetPermit() : null; } }

        public ResourcesClient(string licenseServerUrl)
            : base(licenseServerUrl)
        {
        }

        public ResourcesClient(string licenseServerUrl, string licenseServerRestPrefix)
            : base(licenseServerUrl, licenseServerRestPrefix)
        {
        }

        public ResourcesClient(ResourcesClient<T> basedOn, IPermitRetainer<AccessToken> tokenRetainer)
            : base(basedOn)
        {
            TokenRetainer = tokenRetainer;
        }


        private class PagedResult
        {
            public List<T> Items { get; set; }
            public string NextPageLink { get; set; }
        }


        public IEnumerable<T> GetResources(ResourceFilter filter)
        {
            var request = new RestRequest(RestUrlBuilder.GetResources<T>(filter), Method.GET);
            request.RequestFormat = DataFormat.Json;
            if (Token != null)
            {
                Token.Authenticate(request);
            }

            return Execute<PagedResult>(request).Items;
        }

        public T GetResourceById(Guid id)
        {
            var request = new RestRequest(RestUrlBuilder.GetResourceById<T>(id), Method.GET);
            request.RequestFormat = DataFormat.Json;
            if (Token != null)
            {
                Token.Authenticate(request);
            }

            return Execute<T>(request);
        }

        public IEnumerable<T> GetResourceVersions(Guid id)
        {
            var request = new RestRequest(RestUrlBuilder.GetResourceVersions<T>(id), Method.GET);
            request.RequestFormat = DataFormat.Json;
            if (Token != null)
            {
                Token.Authenticate(request);
            }

            return Execute<List<T>>(request);
        }

        public T GetResourceByIdAndVerison(Guid id, Guid version)
        {
            var request = new RestRequest(RestUrlBuilder.GetResourceByIdAndVersion<T>(id,version), Method.GET);
            request.RequestFormat = DataFormat.Json;
            if (Token != null)
            {
                Token.Authenticate(request);
            }

            return Execute<T>(request);
        }
    }
}
