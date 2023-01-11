using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Import
{
    /// <summary>
    /// NetworkCredentialImportSetting is requested when credentials are required in order to access a GeoSource.
    /// </summary>
    public class NetworkCredentialImportSetting : IImportSetting
    {
        /// <summary>
        /// Gets or sets the permit for accessing the resource.
        /// </summary>
        public INetworkPermit Permit { get; set; }
    }

    /// <summary>
    /// Provide NetworkCredentialImportSettingArgs to the settingProvider to receive a NetworkCredentialImportSetting callback.
    /// </summary>    
    public class NetworkCredentialImportSettingArgs : ProvideImportSettingArgs
    {
        /// <summary>
        /// Gets or sets the URL that the credentials are required for.
        /// </summary>
        public string Url { get; set; }
    }  
}
