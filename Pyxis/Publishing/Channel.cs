using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using Pyxis.Publishing.Permits;
using Pyxis.Publishing.Protocol;
using System;
using System.Net;

namespace Pyxis.Publishing
{
    public class Channel
    {
        public IPermitRetainer<AccessToken> TokenRetainer { get; private set; }

        private ILicenseServerClient LS { get; set; }

        public Channel(string licenseServerUrl)
        {
            LS = new LicenseServerClient(licenseServerUrl);
        }

        public Channel(string licenseServerUrl, IPermitRetainer<AccessToken> tokenRetainer)
        {
            LS = new LicenseServerClient(licenseServerUrl).Authenticate(tokenRetainer);
            TokenRetainer = tokenRetainer;
        }

        public Channel(ILicenseServerClient licenseServerClient)
        {
            LS = licenseServerClient;
        }

        public Channel(ILicenseServerClient licenseServerClient, IPermitRetainer<AccessToken> tokenRetainer)
        {
            LS = licenseServerClient.Authenticate(tokenRetainer);
            TokenRetainer = tokenRetainer;            
        }

        public QueryableResources<GeoSource> GeoSources
        {
            get { return new QueryableResources<GeoSource>(LS.GeoSources); }
        }

        public QueryableResources<Map> Maps
        {
            get { return new QueryableResources<Map>(LS.Maps); }
        }

        public QueryableResources<License> Licenses
        {
            get { return new QueryableResources<License>(LS.Licenses); }
        }

        public QueryableResources<Gallery> Galleries
        {
            get { return new QueryableResources<Gallery>(LS.Galleries); }
        }

        public LicenseTerms GetTermsOfUse()
        {
            return LS.Licensing.GetTermsOfUse();
        }

        public TrustedNodes GetTrustedAuthorities()
        {
            return LS.Authority.GetTrustedNodes();
        }

        public Channel Authenticate(ApiKey apiKey)
        {
            var token = LS.RequestAccessToken(new NetworkCredential(apiKey.ApplicationKey, apiKey.Email));
            return new Channel(LS, token);
        }

        public Channel Authenticate(AccessToken.TokenDetails tokenDetails)
        {
            return new Channel(LS, new NonRenewableAccessTokenRetainer(tokenDetails));
        }

        public Channel Authenticate(NetworkCredential credential)
        {
            var token = LS.RequestAccessToken(credential);
            return new Channel(LS, token);
        }

        public static Channel Authenticate(User user)
        {
            return new Channel(user.LS, user.TokenRetainer);
        }

        public Pipeline GetResourceByProcRef(string procRef)
        {
            return LS.Pipelines.GetPipelineResource(procRef);
        }

        public Pipeline GetResource(ResourceReference resource)
        {
            switch (resource.Type)
            {
                case ResourceType.GeoSource:
                    //allow geo-source to access old versions if needed
                    return GeoSources.GetByIdAndVersion(resource.Id, resource.Version);
                case ResourceType.Map:
                    //allow maps to access old versions if needed
                    return Maps.GetByIdAndVersion(resource.Id, resource.Version);
                default:
                    //only access latest version for all other resources
                    return LS.Pipelines.GetPipelineResource(resource.Id, resource.Version);
            }
        }

        public User AsUser()
        {
            if (TokenRetainer == null)
            {
                throw new UnauthorizedAccessException("Channel needed to be authenticated before using AsUser");
            }

            return new User(TokenRetainer, LS);
        }
    }
}