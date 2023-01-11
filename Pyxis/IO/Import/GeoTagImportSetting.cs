namespace Pyxis.IO.Import
{
    /// <summary>
    /// GeoTagImportSetting is requested when a GeoSource is IRecordCollection, and needed to be geo-tagged.
    /// </summary>
    public class GeoTagImportSetting : IImportSetting
    {
        /// <summary>
        /// Gets or sets the geo tag method to use.
        /// </summary>
        public GeoTagging.IGeoTagMethod Method { get; set; }
    }

    /// <summary>
    /// ProvideGeoTagImportSettingArgs allow the settingProvider access to the RecordCollection process.
    /// </summary>    
    public class ProvideGeoTagImportSettingArgs : ProvideImportSettingArgs
    {
        /// <summary>
        /// Gets or sets the record collection to be geo tagged.
        /// </summary>
        public IProcess_SPtr RecordCollection { get; set; }
    }
}
