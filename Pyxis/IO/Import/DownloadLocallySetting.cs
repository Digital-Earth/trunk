namespace Pyxis.IO.Import
{
    /// <summary>
    /// DownloadLocallySetting is provided by the import request to download the remote file to the specificed folder
    /// </summary>
    public class DownloadLocallySetting : IImportSetting
    {
        /// <summary>
        /// A local path to download the remote files to
        /// </summary>
        public string Path { get; set; }
    }
}