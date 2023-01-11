using System.Collections.Generic;
using Newtonsoft.Json;

namespace GeoWebCore.Models
{
    /// <summary>
    /// Describes the files and directories in a gallery
    /// </summary>
    public class DirectoryListing
    {
        /// <summary>
        /// ID of the gallery
        /// </summary>
        public string GalleryId { get; set; }

        /// <summary>
        /// Directory path inside the gallery
        /// </summary>
        public string Path { get; set; }

        /// <summary>
        /// Names of files in this directory
        /// </summary>
        public List<string> Files { get; set; }

        /// <summary>
        /// Names of subdirectories in this directory
        /// </summary>
        public List<string> Subdirectories { get; set; }
    }
}