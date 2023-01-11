/******************************************************************************
LicenseServerMockClient.cs

begin		: Oct. 8, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using Pyxis.Publishing.Permits;
using Pyxis.Publishing.Protocol;
using System;
using System.Net;

namespace Pyxis.Publishing.Test.Protocol
{
    internal class LicenseServerMockClient : ILicenseServerClient
    {
        public string LicenseServerUrl { get; set; }

        public IPipelineClient Pipelines { get; set; }

        public IGwssClient Servers { get; set; }

        public IResourcesClient<GeoSource> GeoSources { get; set; }

        public IResourcesClient<Map> Maps { get; set; }

        public IResourcesClient<License> Licenses { get; set; }

        public IResourcesClient<Gallery> Galleries { get; set; }

        public IAuthorityClient Authority { get; set; }

        public ILicensingClient Licensing { get; set; }

        public IPermitRetainer<AccessToken> RequestAccessToken(NetworkCredential credential)
        {
            throw new NotImplementedException();
        }

        public IAuthenticatedLicenseServerClient Authenticate(IPermitRetainer<AccessToken> token)
        {
            throw new NotImplementedException();
        }
    }
}