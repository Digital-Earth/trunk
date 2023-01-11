using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GeoWebCoreRunner
{
    public enum GeoWebCoreRunMode
    {
        Server,
        Import,
        ValidateChecksums,
        Download
    }

    /// <summary>
    /// Provide settings to start a GeoWebCore instance
    /// </summary>
    public class GeoWebCoreInstanceSettings
    {
        /// <summary>
        /// Mode of running the processes
        /// </summary>
        public GeoWebCoreRunMode Mode { get; set; }
       
        /// <summary>
        /// List of files to import/download 
        /// </summary>
        public String[] Files { get; set; }

        /// <summary>
        /// set to true to provide verbose mode output
        /// </summary>
        public bool Verbose { get; set; }

        /// <summary>
        /// Environment settings for GeoWebCore process
        /// </summary>
        public RunnerConfiguration.EnvironmentInfo Environment { get; set; }

        /// <summary>
        /// Information about the Node (open ports, pyxnet node id)
        /// </summary>
        public RunnerConfiguration.NodeInfo NodeInfo { get; set; }

        /// <summary>
        /// MasterUrl to connect to if not null.
        /// </summary>
        public string MasterUrl { get; set; }
    }
}
