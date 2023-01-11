using System;
using System.Collections.Generic;
using System.IO;
using GeoWebCore.Models;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using File = System.IO.File;

namespace GeoWebCore.Services.Storage
{
    /// <summary>
    /// Import stroage provides a presistence layer of import requests and results
    /// </summary>
    internal class UserImportsStorage
    {
        private const string GeoSourceFilenameSuffix = ".geosource.json";
        private const string RequestFilenameSuffix = ".request.json";
        
        private string GetGeoSourceFilePath(string galleryId, string id)
        {
            var filePath = GetPath(galleryId, id + GeoSourceFilenameSuffix);
            return filePath;
        }

        private string GetImportRequestFilePath(string galleryId, string id)
        {
            var filePath = GetPath(galleryId, id + RequestFilenameSuffix);
            return filePath;
        }

        /// <summary>
        /// Get all import requests keys for a given gallery
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <returns>list of import requests ids</returns>
        public List<ImportDataSetRequestProgress> GetAllImports(string galleryId)
        {
            var rootDirectory = GetPath(galleryId);
            var result = new List<ImportDataSetRequestProgress>();

            if (!Directory.Exists(rootDirectory))
            {
                return result;
            }

            foreach (var file in Directory.EnumerateFiles(rootDirectory, "*" + RequestFilenameSuffix))
            {
                result.Add(JsonConvert.DeserializeObject<ImportDataSetRequestProgress>(File.ReadAllText(file)));
            }

            return result;
        }

        /// <summary>
        /// Load ImportDataSetRequest object from a given request key
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <param name="request">Import request to get from</param>
        /// <returns>ImportDataSetRequest object success or null</returns>
        public ImportDataSetRequestProgress GetImportRequest(string galleryId, ImportDataSetRequest request)
        {
            return GetImportRequest(galleryId, UniqueHashGenerator.FromObject(request));
        }

        /// <summary>
        /// Load ImportDataSetRequest object from a given request key
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <param name="requestId">Import Request Id </param>
        /// <returns>ImportDataSetRequest object success or null</returns>
        public ImportDataSetRequestProgress GetImportRequest(string galleryId, string requestId)
        {
            var filePath = GetImportRequestFilePath(galleryId, requestId);

            if (File.Exists(filePath))
            {
                return JsonConvert.DeserializeObject<ImportDataSetRequestProgress>(File.ReadAllText(filePath));
            }

            return null;
        }

        /// <summary>
        /// Try to get previously created GeoSource for a given ImportDataSetRequest
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <param name="request">ImportDataSetRequest</param>
        /// <returns>imported GeoSource or null</returns>
        public GeoSource GetImportResult(string galleryId, ImportDataSetRequestProgress request)
        {
            var filePath = GetGeoSourceFilePath(galleryId, request.Id);

            if (File.Exists(filePath))
            {
                return JsonConvert.DeserializeObject<GeoSource>(File.ReadAllText(filePath));
            }

            return null;
        }

        /// <summary>
        /// Save ImportDataSetRequest to disk with optionally a result GeoSource
        /// </summary>
        /// <param name="galleryId">Gallery Id</param>
        /// <param name="request">ImportDataSetRequestProgress</param>
        /// <param name="result">result GeoSource from import Request (optional)</param>
        public void SaveImport(string galleryId, ImportDataSetRequestProgress request, GeoSource result = null)
        {
            //ensure directory exists
            var rootDirectory = GetPath(galleryId);
            if (!Directory.Exists(rootDirectory))
            {
                Directory.CreateDirectory(rootDirectory);
            }

            var requestPath = GetImportRequestFilePath(galleryId, request.Id);
            AtomicFile.WriteAllText(requestPath, JsonConvert.SerializeObject(request, Formatting.Indented));

            if (result != null)
            {
                var geoSourcePath = GetGeoSourceFilePath(galleryId, request.Id);
                AtomicFile.WriteAllText(geoSourcePath, JsonConvert.SerializeObject(result, Formatting.Indented));
            }
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
                basePath = Path.Combine(basePath, galleryId, "Imports");

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