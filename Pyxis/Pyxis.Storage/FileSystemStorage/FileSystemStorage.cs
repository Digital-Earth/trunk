using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Microsoft.Practices.Unity;
using Newtonsoft.Json;
using System;

namespace Pyxis.Storage.FileSystemStorage
{
    /// <summary>
    /// Implementation of FileSystemStorage that downloads and uploads files and directories
    /// from storage to the client computer file system.
    /// </summary>
    public class FileSystemStorage : IFileSystemStorage
    {
        /// <summary>
        /// Create directory structure and files in temporary directory on client computer
        /// </summary>
        protected static string s_tmpPath = Path.Combine(Path.GetTempPath(), "PyxisStorage");

        public static string TmpPath { get { return s_tmpPath; } }

        [Dependency]
        public IBlobProvider BlobProvider
        {
            get
            {
                return m_blobProvider;
            }
            set
            {
                m_blobProvider = value;
                m_fileBlobClient = new FileBlobClient(m_blobProvider);
                m_directoryBlobClient = new DirectoryBlobClient(m_blobProvider);
            }
        }

        private IBlobProvider m_blobProvider;

        private FileBlobClient m_fileBlobClient;
        private DirectoryBlobClient m_directoryBlobClient;

        // empty ctor for dependency injection
        public FileSystemStorage()
        {
        }

        public FileSystemStorage(IBlobProvider blobProvider)
        {
            BlobProvider = blobProvider;
        }

        public virtual bool DownloadDirectory(string directoryKey, string target)
        {
            return DownloadDirectory(directoryKey, target, null);
        }

        public ProgressTracker<bool> DownloadDirectoryAsync(string directoryKey, string target)
        {
            var directoryBlob = BlobProvider.GetBlob<DirectoryBlob>(directoryKey);
            return ProgressTracker<bool>.FromFunction(directoryBlob != null ? directoryBlob.DirectorySize : 0, (tracker) => DownloadDirectory(directoryKey, target, tracker));
        }

        public virtual bool DownloadFile(string fileKey, string targetFile)
        {
            return DownloadFile(fileKey, targetFile, null);
        }

        public ProgressTracker<bool> DownloadFileAsync(string fileKey, string targetFile)
        {
            FileBlob fileBlob = BlobProvider.GetBlob<FileBlob>(fileKey);
            return ProgressTracker<bool>.FromFunction(fileBlob != null ? fileBlob.FileSize : 0, (tracker) => DownloadFile(fileKey, targetFile, tracker));
        }

        public virtual string UploadDirectory(string directoryPath)
        {
            return UploadDirectory(directoryPath, null);
        }

        public ProgressTracker<string> UploadDirectoryAsync(string directoryPath)
        {
            long dirSize = FileSystemUtilities.GetDirectorySize(directoryPath);
            return ProgressTracker<string>.FromFunction(dirSize, (tracker) => UploadDirectory(directoryPath, tracker));
        }

        public virtual string UploadFile(string fileName)
        {
            return UploadFile(fileName, null);
        }

        public ProgressTracker<string> UploadFileWithKeyAsyncWithDelayedStart(string fileName, string key, Task after)
        {
            long fileSize;
            FileSystemUtilities.TryGetFileSize(fileName, out fileSize);
            
            return ProgressTracker<string>.FromFunctionWithDelayedStart(
                fileSize,
                (tracker) => UploadFileWithKey(fileName, key, tracker), 
                after);
        }

        public ProgressTracker<string> UploadFileWithKeyAsync(string fileName, string key)
        {
            long fileSize;
            FileSystemUtilities.TryGetFileSize(fileName, out fileSize);
            return ProgressTracker<string>.FromFunction(fileSize, (tracker) => UploadFileWithKey(fileName, key, tracker));
        }

        public ProgressTracker<string> UploadFileAsync(string fileName)
        {
            long fileSize;
            FileSystemUtilities.TryGetFileSize(fileName, out fileSize);
            return ProgressTracker<string>.FromFunction(fileSize, (tracker) => UploadFile(fileName, tracker));
        }

        public virtual string UploadFileWithKey(string fileName, string key)
        {
            return UploadFileWithKey(fileName, key, null);
        }

        // Prevent callers from supplying their own tracker
        private bool DownloadDirectory(string directoryKey, string target, ProgressTracker<bool> tracker)
        {
            var directoryBlob = BlobProvider.GetBlob<DirectoryBlob>(directoryKey);
            if (directoryBlob == null)
            {
                return false;
            }

            var tmpDir = Path.Combine(s_tmpPath, Path.GetRandomFileName());
            var result = m_directoryBlobClient.DownloadDirectoryBlob(directoryBlob, tmpDir, tracker);


            result &= FileSystemUtilities.Retry(() => FileSystemUtilities.TryMoveDirectory(tmpDir, target + " tmp", true), 4, 50);
            if (result)
            {
                result &= FileSystemUtilities.Retry(() => FileSystemUtilities.TryRenameDirectory(target + " tmp", new DirectoryInfo(target).Name, true), 4, 50);
            }

            return result;
        }

        // Prevent callers from supplying their own tracker
        private bool DownloadFile(string fileKey, string targetFile, ProgressTracker<bool> tracker)
        {
            FileBlob fileBlob = BlobProvider.GetBlob<FileBlob>(fileKey);
            if (fileBlob == null)
            {
                return false;
            }
            return m_fileBlobClient.DownloadFileBlob(fileBlob, targetFile, tracker);
        }

        // Prevent callers from supplying their own tracker
        private string UploadDirectory(string directoryPath, ProgressTracker<string> progressTracker)
        {
            var directoryBlob = m_directoryBlobClient.UploadDirectoryBlob(directoryPath, progressTracker);
            return BlobProvider.AddBlob(directoryBlob);
        }

        // Prevent callers from supplying their own tracker
        private string UploadFile(string fileName, ProgressTracker<string> progressTracker)
        {
            var fileBlob = m_fileBlobClient.UploadFileBlob(fileName, progressTracker);
            return BlobProvider.AddBlob(fileBlob);
        }

        // Prevent callers from supplying their own tracker
        private string UploadFileWithKey(string fileName, string key, ProgressTracker<string> progressTracker)
        {
            using (var stream = new MemoryStream())
            {
                var fileBlob = m_fileBlobClient.UploadFileBlob(fileName, progressTracker);
                var streamWriter = new StreamWriter(stream);
                streamWriter.Write(JsonConvert.SerializeObject(fileBlob));
                streamWriter.Flush();
                stream.Position = 0;
                BlobProvider.AddBlob(key, stream);
                return key;
            }
        }
        /// <summary>
        /// Checking to see if a file with following key exists on the storage
        /// </summary>
        /// <param name="key">key to check the existance of</param>
        /// <returns>where the file exists on the storage</returns>
        public bool FileExists(string key)
        {
            return m_blobProvider.BlobExists(key);
        }
    }

    //[JsonConverter(typeof(FileStorageVersionConverter))]
    public enum FileStorageVersion
    {
        BasicFileBlobV1,
        ZippedFileBlobV1
    }

    public class FileStorageVersionConverter : JsonConverter
    {
        /// <summary>
        /// Determines if the specified System.Type can be converted into json object by this converter.
        /// </summary>
        /// <param name="objectType">The System.Type to determine if it can be converted.</param>
        /// <returns>true if <paramref name="objectType"/> can be converted; otherwise, false.</returns>
        public override bool CanConvert(System.Type objectType)
        {
            return objectType == typeof(FileStorageVersion);
        }

        /// <summary>
        /// Reads the JSON representation of the object.
        /// </summary>
        /// <param name="reader">The JSON Reader to read from.</param>
        /// <param name="objectType">The System.Type of the object.</param>
        /// <param name="existingValue">The existing value of object being read.</param>
        /// <param name="serializer">The calling serializer.</param>
        /// <returns>The object value.</returns>
        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var value = serializer.Deserialize<object>(reader);

            //legacy support
            if (value.Equals("V1"))
            {
                return FileStorageVersion.BasicFileBlobV1;
            } 
            else 
            {
                return (FileStorageVersion)(int)value;
            }
        }

        /// <summary>
        /// Writes the JSON representation of the object.
        /// </summary>
        /// <param name="writer">The JSON writer to write to.</param>
        /// <param name="value">The System.Object.</param>
        /// <param name="serializer">The calling serializer.</param>
        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            var version = (FileStorageVersion)value;
            serializer.Serialize(writer, (int)version);
        }
    }



    public enum DirectoryStorageVersion
    {
        BasicDirectoryBlobV1
    }

    /// <summary>
    /// Describes a recursive directory structure with files.
    /// </summary>
    public class DirectoryBlob
    {
        public List<DirectoryBlob> Directories { get; set; }
        public long DirectorySize { get; set; }
        public List<ManifestEntry> Files { get; set; }
        public string Name { get; set; }
        public DirectoryStorageVersion Version { get; set; }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="directoryName"> the directory name</param>
        /// <param name="Directory Storage Version">blob version</param>
        public DirectoryBlob(string directoryName, DirectoryStorageVersion version)
        {
            Name = directoryName;
            Files = new List<ManifestEntry>();
            Directories = new List<DirectoryBlob>();
            Version = version;
        }

        /// <summary>
        /// Describes a file entry in a directory.
        /// </summary>
        public struct ManifestEntry
        {
            public string Name { get; set; }
            public long Size { get; set; }
            public string Key { get; set; }
        }
    }

    /// <summary>
    /// Represents a multi-part file.
    /// </summary>
    public class FileBlob
    {
        /// <summary>
        /// The first block of the file.
        /// </summary>
        public string Data { get; set; }

        /// <summary>
        /// The size of the file - meaning is provider dependent
        /// </summary>
        public long FileSize { get; set; }

        /// <summary>
        /// For large files, an ordered list of keys that reference the remaining parts of the file
        /// in the BlobProvider
        /// </summary>
        public List<string> Parts { get; set; }

        /// <summary>
        /// The version for backwards compatibility
        /// </summary>
        public FileStorageVersion Version { get; set; }

        /// <summary>
        /// Constructor
        /// </summary>
        public FileBlob(FileStorageVersion version)
        {
            Version = version;
            Parts = new List<string>();
        }
    }
}