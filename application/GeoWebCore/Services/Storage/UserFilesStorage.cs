using System;
using System.Collections.Generic;
using System.IO;
using GeoWebCore.Models;

namespace GeoWebCore.Services.Storage
{
    /// <summary>
    /// UserFilesStorage provides methods for working with the storage for files, uploaded by users for import.
    /// It is used by UserFilesController.
    /// </summary>
    internal class UserFilesStorage
    {
        /// <summary>
        /// Create a file in the given gallery/path. Creates containing directories if needed.
        /// </summary>
        /// <param name="galleryId">The gallery id</param>
        /// <param name="path">The file path</param>
        /// <returns>A file stream for the created file</returns>
        public FileStream CreateFile(string galleryId, string path)
        {
            var fullPath = GetPath(galleryId, path);

            // Create the containing directories if required
            var containingDirectory = Path.GetDirectoryName(fullPath);
            if (containingDirectory != null && !Directory.Exists(containingDirectory))
            {
                Directory.CreateDirectory(containingDirectory);
            }

            // Now create the file
            return File.Create(fullPath);
        }

        /// <summary>
        /// Create a directory in the given gallery/path
        /// </summary>
        /// <param name="galleryId">The gallery id</param>
        /// <param name="path">Optional path</param>
        /// <returns>The directory information</returns>
        public DirectoryInfo CreateDirectory(string galleryId, string path = null)
        {
            var fullPath = GetPath(galleryId, path);
            return Directory.CreateDirectory(fullPath);
        }

        /// <summary>
        /// Determine if a file exists in a gallery
        /// </summary>
        /// <param name="galleryId">The gallery id</param>
        /// <param name="path">Optional path</param>
        /// <returns>true if the file exists otherwise false</returns>
        public bool FileExists(string galleryId, string path = null)
        {
            var fullPath = GetPath(galleryId, path);
            return File.Exists(fullPath);
        }

        /// <summary>
        /// Determine if a directory exists in a gallery
        /// </summary>
        /// <param name="galleryId">The gallery id</param>
        /// <param name="path">Optional path</param>
        /// <returns>true if the directory exists (or the gallery directory exists if no path) otherwise false</returns>
        public bool DirectoryExists(string galleryId, string path = null)
        {
            var fullPath = GetPath(galleryId, path);
            return Directory.Exists(fullPath);
        }

        /// <summary>
        /// Delete a file or directory within a gallery
        /// </summary>
        /// <param name="galleryId">The gallery</param>
        /// <param name="path">The file or directory</param>
        public void Delete(string galleryId, string path = null)
        {
            var fullPath = GetPath(galleryId, path);
            if (Directory.Exists(fullPath))
            {
                Directory.Delete(fullPath, true);
                return;
            }

            if (File.Exists(fullPath))
            {
                Directory.Delete(fullPath);
            }
        }

        /// <summary>
        /// Get the files and subdirectories in a given gallery/path
        /// </summary>
        /// <param name="galleryId">The gallery id</param>
        /// <param name="path">The path</param>
        /// <returns></returns>
        public DirectoryListing GetDirectoryListing(string galleryId, string path = null)
        {
            var fullPath = GetPath(galleryId, path);
            DirectoryListing result = new DirectoryListing
            {
                Path = path,
                GalleryId = galleryId,
                Files = new List<string>(),
                Subdirectories = new List<string>()
            };

            var items = Directory.EnumerateFileSystemEntries(fullPath);
            foreach (var item in items)
            {
                if (File.Exists(item))
                {
                    result.Files.Add(Path.GetFileName(item));
                }
                else
                {
                    result.Subdirectories.Add(Path.GetFileName(item));
                }
            }

            return result;
        }

        /// <summary>
        /// Get the system path based on the gallery id and path. Relative paths are not allowed.
        /// </summary>
        /// <param name="galleryId">Optional gallery id</param>
        /// <param name="path">Optional path</param>
        /// <returns></returns>
        public string GetPath(string galleryId = null, string path = null)
        {
            // common root for gallery files
            var basePath = Program.RunInformation.GalleryFilesRoot;

            // add gallery and path if supplied
            if (!String.IsNullOrEmpty(galleryId))
            {
                basePath = Path.Combine(basePath, galleryId, "Files");

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