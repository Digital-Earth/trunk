using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core
{
    /// <summary>
    /// Defines the operating state of an core service.
    /// </summary>
    public enum CoreServiceState
    {
        /// <summary>
        /// Specifies an core service has only been created.
        /// </summary>
        Created,
        /// <summary>
        /// Specifies a core service is starting.
        /// </summary>
        Starting,
        /// <summary>
        /// Specifies a core service has finished starting.
        /// </summary>
        Running,
        /// <summary>
        /// Specifies an core service is stopping.
        /// </summary>
        ShuttingDown,
        /// <summary>
        /// Specifies a core has stopped and is shutdown.
        /// </summary>
        Stopped,
        /// <summary>
        /// Specifies a core service has encountered an unrecoverable error.
        /// </summary>
        Broken
    }
}
