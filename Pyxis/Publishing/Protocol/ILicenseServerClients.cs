/******************************************************************************
ILicenseServerClients.cs

begin		: Oct. 22, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using Pyxis.Contract;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Services.GeoWebStreamService;
using Pyxis.Contract.Services.LicenseService;
using Pyxis.Publishing.Permits;
using System;
using System.Collections.Generic;
using System.Net;

namespace Pyxis.Publishing.Protocol
{
    public interface ILicenseServerClient
    {
        string LicenseServerUrl { get; }

        IPipelineClient Pipelines { get; }

        IGwssClient Servers { get; }

        IResourcesClient<GeoSource> GeoSources { get; }

        IResourcesClient<Map> Maps { get; }

        IResourcesClient<License> Licenses { get; }

        IResourcesClient<Gallery> Galleries { get; }

        IAuthorityClient Authority { get; }

        ILicensingClient Licensing { get; }

        IPermitRetainer<AccessToken> RequestAccessToken(NetworkCredential credential);

        IAuthenticatedLicenseServerClient Authenticate(IPermitRetainer<AccessToken> token);
    }

    public interface IAuthenticatedLicenseServerClient : ILicenseServerClient
    {
        IPermitRetainer<AccessToken> TokenRetainer { get; }

        UserClient User { get; }

        UserProfile GetUserProfile();

        AccessToken.TokenDetails GetTokenDetails();
    }

    public interface IPipelineClient
    {
        IPipelineMetaData GetPipelineMetaData(string procRef);

        IList<IPipelineServerStatus> GetPipelineServerStatuses(string procRef);

        IEnumerable<IPipelineProcRef> GetAllPipelines();

        Pipeline GetPipelineResource(string procRef);

        Pipeline GetPipelineResource(Guid id, Guid version);
    }

    public interface IResourcesClient<T> where T : Pyxis.Contract.Publishing.Resource
    {
        IEnumerable<T> GetResources(ResourceFilter filter);

        T GetResourceById(Guid id);

        IEnumerable<T> GetResourceVersions(Guid id);

        T GetResourceByIdAndVerison(Guid id, Guid version);
    }

    public interface IGwssClient
    {
        ILsStatus UpdateStatus(IGwssStatus status);
    }

    public interface IPermissionClient
    {
        CertificatePermissionGrant GetPermission(PyxNetPermissionRequest request);
    }

    public interface IAuthorityClient
    {
        TrustedNodes GetTrustedNodes();
    }

    public interface ILicensingClient
    {
        LicenseTerms GetTermsOfUse();
    }
}