using Pyxis.IO.Settings;

namespace Pyxis.IO.Import
{
    /// <summary>
    /// Base interface to decorate import settings
    /// </summary>
    public interface IImportSetting : ISetting
    {
    }

    /// <summary>
    /// Base class used to decorate additional arguments for ProvideSetting function
    /// </summary>
    public class ProvideImportSettingArgs : ISettingArgs
    {
    }

    /// <summary>
    /// This interface is used by ImportGeoSourceProgress class to retrieve additional import settings when required.
    /// <see cref="ImportSettingProvider"/> for default implementation.
    /// </summary>
    public interface IImportSettingProvider : ISettingProvider<IImportSetting, ProvideImportSettingArgs>
    {
    }

    /// <summary>
    /// Default implementation of IImportSettingProvider.
    /// </summary>
    public class ImportSettingProvider : SettingProvider<IImportSetting, ProvideImportSettingArgs>, IImportSettingProvider
    {
        public ImportSettingProvider()
        {
            
        }

        public ImportSettingProvider(IImportSettingProvider defaults) : base(defaults)
        {
            
        }
    }
}
