using System;
using System.Collections.Generic;
using System.IO;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using File = System.IO.File;

namespace GeoWebCore.Services.Storage
{
    public enum SyncInterval
    {
        None,
        Hourly,
        Daily,
        Weekly,
        Monthly,
        Yearly
    };

    public enum SyncStatus
    {
        Created,
        Syncing,
        Success,
        Failure
    }

    /// <summary>
    /// SyncDetails represnt a sync connection details.
    /// </summary>
    public class SyncDetails
    {
        public Guid Id { get; set; }

        public string Uri { get; set; }

        public SyncInterval? Interval { get; set; }

        public SimpleMetadata Metadata { get; set; }

        public DateTime Created { get; set; }

        public DateTime Updated { get; set; }

        public SyncStatus? Status { get; set; }

        public DateTime? LastSynced { get; set; }

        public string ETag { get; set; }

        public string Checksum { get; set; }

        public string ErrorMessage { get; set; }
    }

    /// <summary>
    /// sync stroage provides a presistence layer of all servers the user assoicate with the gallery
    /// </summary>
    internal class UserSyncStorage
    {
        private const string SyncFilenameSuffix = ".sync.json";

        /// <summary>
        /// Get all import requests keys for a given gallery
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <returns>list of import requests ids</returns>
        public List<SyncDetails> GetAllSync(string galleryId)
        {
            var rootDirectory = GetPath(galleryId);
            var result = new List<SyncDetails>();

            if (!Directory.Exists(rootDirectory))
            {
                return result;
            }

            foreach (var file in Directory.EnumerateFiles(rootDirectory, "*" + SyncFilenameSuffix))
            {
                result.Add(JsonConvert.DeserializeObject<SyncDetails>(File.ReadAllText(file)));
            }

            return result;
        }

        /// <summary>
        /// Get all import requests keys for a given gallery
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <returns>list of import requests ids</returns>
        public SyncDetails GetSync(string galleryId, Guid syncId)
        {
            var filePath = GetPath(galleryId, syncId);

            if (File.Exists(filePath))
            {
                return JsonConvert.DeserializeObject<SyncDetails>(File.ReadAllText(filePath));
            }
            return null;
        }

        /// <summary>
        /// Save ImportDataSetRequest to disk with optionally a result GeoSource
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <param name="sync">Server catalog to link to gallery</param>
        public void SaveSync(string galleryId, SyncDetails sync)
        {
            //ensure directory exists
            var rootDirectory = GetPath(galleryId);
            if (!Directory.Exists(rootDirectory))
            {
                Directory.CreateDirectory(rootDirectory);
            }

            var filePath = GetPath(galleryId, sync);
            AtomicFile.WriteAllText(filePath, JsonConvert.SerializeObject(sync, Formatting.Indented));
        }

        /// <summary>
        /// Delete sync from gallery
        /// </summary>
        /// <param name="galleryId">galery Id</param>
        /// <param name="sync">Server catalog to unlink from gallery</param>
        /// <returns></returns>
        public bool DeleteSync(string galleryId, SyncDetails sync)
        {
            var filePath = GetPath(galleryId, sync);
            
            if (!File.Exists(filePath))
            {
                return false;
            }

            File.Delete(filePath);
            return true;
        }

        private string GetPath(string galleryId, SyncDetails sync)
        {
            return GetPath(galleryId, sync.Id);
        }

        private string GetPath(string galleryId, Guid syncId)
        {
            return GetPath(galleryId, syncId + SyncFilenameSuffix);
        }

        /// <summary>
        /// Get the system path based on the gallery id and path. Relative paths are not allowed.
        /// </summary>
        /// <param name="galleryId">Optional gallery id</param>
        /// <param name="path">Optional path</param>
        /// <returns></returns>
        public string GetPath(string galleryId, string path = null)
        {
            // common root for gallery files
            var basePath = Program.RunInformation.GalleryFilesRoot;

            // add gallery and path if supplied
            if (!String.IsNullOrEmpty(galleryId))
            {
                basePath = Path.Combine(basePath, galleryId, "Sync");

                if (!String.IsNullOrEmpty(path))
                {
                    // strip double quotes from path
                    basePath = Path.Combine(basePath, path);
                }
            }

            // security check - make sure path is within gallery files directory
            var fullPath = Path.GetFullPath(basePath);
            if (!fullPath.StartsWith(Program.RunInformation.GalleryFilesRoot))
            {
                throw new Exception("Invalid gallery id: " + (galleryId ?? "") + " or path: " + (path ?? ""));
            }

            return fullPath;
        }
    }
}