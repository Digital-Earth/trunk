/******************************************************************************
LicenseServerRestClient.cs

begin		: Oct. 8, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Net;
using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using Pyxis.Publishing.Permits;

namespace Pyxis.Publishing.Protocol
{
    internal class LicenseServerClient : ILicenseServerClient
    {
        public string LicenseServerUrl { get; protected set; }

        public IPipelineClient Pipelines { get; protected set; }

        public IGwssClient Servers { get; protected set; }

        public IResourcesClient<GeoSource> GeoSources { get; protected set; }

        public IResourcesClient<Map> Maps { get; protected set; }

        public IResourcesClient<License> Licenses { get; protected set; }

        public IResourcesClient<Gallery> Galleries { get; protected set; }

        public IAuthorityClient Authority { get; protected set; }

        public ILicensingClient Licensing { get; protected set; }

        protected AccessTokenGenerator AccessTokenGenerator { get; set; }

        public LicenseServerClient(string licenseServerUrl)
        {
            LicenseServerUrl = licenseServerUrl;
            Pipelines = new PipelineClient(licenseServerUrl);
            Servers = new GwssClient(licenseServerUrl);
            GeoSources = new ResourcesClient<GeoSource>(licenseServerUrl);
            Maps = new ResourcesClient<Map>(licenseServerUrl);
            Licenses = new ResourcesClient<License>(licenseServerUrl);
            Galleries = new ResourcesClient<Gallery>(licenseServerUrl);
            Authority = new AuthorityClient(licenseServerUrl);
            AccessTokenGenerator = new AccessTokenGenerator(licenseServerUrl);
            Licensing = new LicensingClient(licenseServerUrl);
        }

        public LicenseServerClient(string licenseServerUrl, string licenseServerRestPrefix)
        {
            LicenseServerUrl = licenseServerUrl;
            Pipelines = new PipelineClient(licenseServerUrl, licenseServerRestPrefix);
            Servers = new GwssClient(licenseServerUrl, licenseServerRestPrefix);
            GeoSources = new ResourcesClient<GeoSource>(licenseServerUrl, licenseServerRestPrefix);
            Maps = new ResourcesClient<Map>(licenseServerUrl, licenseServerRestPrefix);
            Licenses = new ResourcesClient<License>(licenseServerUrl, licenseServerRestPrefix);
            Galleries = new ResourcesClient<Gallery>(licenseServerUrl, licenseServerRestPrefix);
            Authority = new AuthorityClient(licenseServerUrl, licenseServerRestPrefix);
            AccessTokenGenerator = new AccessTokenGenerator(licenseServerUrl, licenseServerRestPrefix);
            Licensing = new LicensingClient(licenseServerUrl, licenseServerRestPrefix);
        }

        public IPermitRetainer<AccessToken> RequestAccessToken(NetworkCredential credential)
        {
            return new AccessTokenRetainer(AccessTokenGenerator,credential);
        }

        public IAuthenticatedLicenseServerClient Authenticate(IPermitRetainer<AccessToken> token)
        {
            if (token.GetPermit() == null)
            {
                throw new Exception("Failed to obtain access token");
            }
            return new AuthenticatedLicenseServerClient(this, token);
        }
    }

    internal class AuthenticatedLicenseServerClient : LicenseServerClient, IAuthenticatedLicenseServerClient
    {
        public IPermitRetainer<AccessToken> TokenRetainer { get; private set; }

        public UserClient User { get; private set; }

        public AuthenticatedLicenseServerClient(LicenseServerClient basedOn, IPermitRetainer<AccessToken> token)
            : base(basedOn.LicenseServerUrl)
        {
            LicenseServerUrl = basedOn.LicenseServerUrl;
            TokenRetainer = token;

            Pipelines = new PipelineClient((PipelineClient)basedOn.Pipelines,token);
            Servers = basedOn.Servers;

            User = new UserClient((ResourcesClient<GeoSource>)basedOn.GeoSources, token);

            GeoSources = new ResourcesClient<GeoSource>((ResourcesClient<GeoSource>)basedOn.GeoSources, token);
            Maps = new ResourcesClient<Map>((ResourcesClient<Map>)basedOn.Maps, token);
            Licenses = new ResourcesClient<License>((ResourcesClient<License>)basedOn.Licenses, token);
            Galleries = new ResourcesClient<Gallery>((ResourcesClient<Gallery>)basedOn.Galleries, token);
            Authority = new AuthorityClient((RestPublishingClient)basedOn.Authority);
        }

        public UserProfile GetUserProfile()
        {
            return User.GetUserProfile();
        }

        public AccessToken.TokenDetails GetTokenDetails()
        {
            return User.GetTokenDetails();
        }
    }
}