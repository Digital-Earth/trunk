namespace Pyxis.Storage
{
    /// <summary>
    /// Provides methods for moving directories and files to and from storage.
    /// Directories and files are referenced by unique keys.
    /// </summary>
    public interface IFileSystemStorage
    {
        /// <summary>
        /// Download a directory structure, including subdirectories and files from storage.
        /// </summary>
        /// <param name="directoryKey"> directory key</param>
        /// <param name="target"> target location for root directory</param>
        /// <returns> true if the download was successful</returns>
        bool DownloadDirectory(string directoryKey, string target);

        /// <summary>
        /// Download a directory structure, including subdirectories and files from storage.
        /// </summary>
        /// <param name="directoryKey"> directory key</param>
        /// <param name="target"> target location for root directory</param>
        /// <returns> a progress tracker that returns true if the download was successful</returns>
        ProgressTracker<bool> DownloadDirectoryAsync(string directoryKey, string target);

        /// <summary>
        /// Download a file from storage.
        /// </summary>
        /// <param name="fileKey"> file key</param>
        /// <param name="targetFile"> target filename</param>
        /// <returns> true if the download was successful</returns>
        bool DownloadFile(string fileKey, string targetFile);

        /// <summary>
        /// Download a file from storage.
        /// </summary>
        /// <param name="fileKey"> file key</param>
        /// <param name="targetFile"> target filename</param>
        /// <returns> a progress tracker that returns true if the download was successful</returns>
        ProgressTracker<bool> DownloadFileAsync(string fileKey, string targetFile);

        /// <summary>
        /// Upload a file to storage.
        /// </summary>
        /// <param name="fileName"> the file name</param>
        /// <returns> the file key or an empty string on error</returns>
        string UploadFile(string fileName);

        /// <summary>
        /// Upload a file to storage.
        /// </summary>
        /// <param name="fileName"> the file name</param>
        /// <param name="key"> the file key</param>
        /// <returns> the file key or an empty string on error</returns>
        string UploadFileWithKey(string fileName, string key);

        /// <summary>
        /// Upload a file to storage.
        /// </summary>
        /// <param name="fileName"> the file name</param>
        /// <returns> a progress tracker that returns the file key</returns>
        ProgressTracker<string> UploadFileAsync(string fileName);

        /// <summary>
        /// Upload a directory structure including subdirectories and files to storage.
        /// </summary>
        /// <param name="directoryPath"> the directory path</param>
        /// <returns> the directory key</returns>
        string UploadDirectory(string directoryPath);

        /// <summary>
        /// Upload a directory structure including subdirectories and files to storage.
        /// </summary>
        /// <param name="directoryPath"> the directory path</param>
        /// <returns> a progress tracker that returns the directory key</returns>
        ProgressTracker<string> UploadDirectoryAsync(string directoryPath);
    }
}