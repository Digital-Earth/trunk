using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Denotes the success or failure of specific portions of the 
    /// application. These values are often 'OR'ed together to produce
    /// a composite return value.
    /// </summary>
    public enum ApplicationResult
    {
        /// <summary>
        /// The applicaiton ran to completion without any significant errors.
        /// </summary>
        Success = 0x00,

        /// <summary>
        /// An error occured during initialization of the applicaiton or 
        /// libraries and assemblies.
        /// </summary>
        InitializationFailure = 0x01,

        /// <summary>
        /// An error occured during unit testing.
        /// </summary>
        UnitTestFailure = 0x02,

        /// <summary>
        /// An unexpected and unhandled error occured during program
        /// execution.
        /// </summary>
        ExecutionFailure = 0x08,

        /// <summary>
        /// An error occured while the application was closing and cleaning
        /// up resources.
        /// </summary>
        ShutdownFailure = 0x10,

        /// <summary>
        /// An instance of the application is already running.
        /// </summary>
        DuplicateInstance = 0x04,

        /// <summary>
        /// The application is shutting down because someone (the launcher) 
        /// requested a restart.
        /// </summary>
        RestartRequested = 0x20
    };
}
