using System;
using System.Collections.Generic;
using System.IO;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using File = System.IO.File;

namespace GeoWebCore.Services.Storage
{
    /// <summary>
    /// server stroage provides a presistence layer of all servers the user assoicate with the gallery
    /// </summary>
    internal class UserServersStorage
    {
        private const string ServerFilenameSuffix = ".server.json";

        /// <summary>
        /// Get all import requests keys for a given gallery
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <returns>list of import requests ids</returns>
        public List<DataSetCatalog> GetAllServers(string galleryId)
        {
            var rootDirectory = GetPath(galleryId);
            var result = new List<DataSetCatalog>();

            if (!Directory.Exists(rootDirectory))
            {
                return result;
            }

            foreach (var file in Directory.EnumerateFiles(rootDirectory, "*" + ServerFilenameSuffix))
            {
                result.Add(JsonConvert.DeserializeObject<DataSetCatalog>(File.ReadAllText(file)));
            }

            return result;
        }

        /// <summary>
        /// Save ImportDataSetRequest to disk with optionally a result GeoSource
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <param name="server">Server catalog to link to gallery</param>
        public void SaveServer(string galleryId, DataSetCatalog server)
        {
            //ensure directory exists
            var rootDirectory = GetPath(galleryId);
            if (!Directory.Exists(rootDirectory))
            {
                Directory.CreateDirectory(rootDirectory);
            }

            var filePath = GetPath(galleryId, server);
            AtomicFile.WriteAllText(filePath, JsonConvert.SerializeObject(server, Formatting.Indented));
        }

        /// <summary>
        /// Delete server from gallery
        /// </summary>
        /// <param name="galleryId">galery Id</param>
        /// <param name="server">Server catalog to unlink from gallery</param>
        /// <returns></returns>
        public bool DeleteServer(string galleryId, DataSetCatalog server)
        {
            var filePath = GetPath(galleryId, server);
            
            if (!File.Exists(filePath))
            {
                return false;
            }

            File.Delete(filePath);
            return true;
        }

        private string GetPath(string galleryId, DataSetCatalog catalog)
        {
            return GetPath(galleryId, UniqueHashGenerator.FromObject(catalog.Uri) + ServerFilenameSuffix);
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
                basePath = Path.Combine(basePath, galleryId, "Servers");

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