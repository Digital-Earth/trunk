using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GeoWebCore.Services.Cluster;

namespace GeoWebCore
{
    /// <summary>
    /// Provide Information about the current run environment of GeoWebCore instance
    /// </summary>
    public class GeoWebCoreRunInformation
    {
        /// <summary>
        /// Run mode for the GeoWebCore instance
        /// </summary>
        public enum RunMode
        {
            /// <summary>
            /// Server mode, will listen to incomeing http requests
            /// </summary>
            Server,

            /// <summary>
            /// Validate checksum for local user storage
            /// </summary>
            ValidateChecksum,

            /// <summary>
            /// Perform a discover for local files
            /// </summary>
            LocalDiscovery,

            /// <summary>
            /// Perform an import of a local files
            /// </summary>
            LocalImport,

            /// <summary>
            /// Perform an import of a published GeoSource
            /// </summary>
            GalleryImport,

            /// <summary>
            /// Perform a download for raw supporting files of a published GeoSource
            /// </summary>
            GalleryDownload,

            /// <summary>
            /// Display New Geo Source Status
            /// </summary>
            GalleryStatus
        }

        /// <summary>
        /// Run evnironment for this instance
        /// </summary>
        public enum RunEnvironment
        {
            /// <summary>
            /// Local dev instance
            /// </summary>
            Dev,

            /// <summary>
            /// Public Test instance
            /// </summary>
            Test,

            /// <summary>
            /// Production instance
            /// </summary>
            Production
        }

        /// <summary>
        /// Run mode for the GeoWebCore instance
        /// </summary>
        public RunMode Mode { get; set; }

        /// <summary>
        /// Run environment for the GeoWebCore instance
        /// </summary>
        public RunEnvironment Environment { get; set; }

        /// <summary>
        /// local cache folder
        /// </summary>
        public string CacheFolder { get; set; }

        /// <summary>
        /// local user galleries folder.
        /// </summary>
        public string GalleryFilesRoot { get; set; }

        /// <summary>
        /// Represtation of the Cluster of GWC
        /// </summary>
        public Cluster Cluster { get; set; }
    }
}
