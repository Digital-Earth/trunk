using System;

namespace Pyxis.Contract
{
    public enum PermitType
    {
        /// <summary>
        /// Allow access to https://ls-api.globalgridsystems.com/. Required for performing authenticated requests using the API
        /// </summary>
        AccessToken,

        /// <summary>
        /// Allow access to GeoSources data stream. Required for streaming data over the geoweb
        /// </summary>
        Certificate,

        /// <summary>
        /// Allow access to external APIs. Require for performing authenticated requests and streaming data over the public web services
        /// </summary>
        ExternalApiKey,
    }

    public interface IPermit
    {
        PermitType PermitType { get; }
        DateTime Issued { get; }
        DateTime Expires { get; }
    }

    public interface IPermitRetainer<T> : IDisposable where T : IPermit
    {
        T GetPermit();
        void ReleasePermit();
    }
}
