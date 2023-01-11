using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.Models
{
    /// <summary>
    /// Return information about Gallery useeage
    /// </summary>
    public class GalleryUsageResponse
    {
        /// <summary>
        /// Storage used in bytes
        /// </summary>
        public long StorageUsed { get; set; }

        /// <summary>
        /// Number of sessions
        /// </summary>
        public long Sessions { get; set; }

        /// <summary>
        /// Date time of last session
        /// </summary>
        public DateTime? LastSessionUploaded { get; set; }
    }
}
