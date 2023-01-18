using System;
using System.Threading.Tasks;

namespace Pyxis.Core
{
    /// <summary>
    /// Represents a Pyxis.Core service
    /// </summary>
    /// <see cref="Pyxis.Core.Services.ServiceBase"/>
    interface ICoreService
    {
        CoreServiceState State { get; }

        /// <summary>
        /// Execute a System.Action when the service is running.
        /// Actions are executed immediately if the service is running; otherwise they are stored to be executed when the core service starts.
        /// </summary>
        /// <param name="action">The System.Action to execute.</param>
        void WhenReady(Action action);
        /// <summary>
        /// Execute a System.Action when the service is stopped.
        /// Actions are stored until the service begins to shutdown.
        /// </summary>
        /// <param name="action">The System.Action to execute.</param>
        void BeforeStopping(Action action);

        /// <summary>
        /// Execute a System.Action when the service is running.
        /// Actions are executed immediately if the service is running; otherwise they are stored to be executed when the core service starts.
        /// </summary>
        /// <param name="action">>The System.Action to execute.</param>
        /// <param name="scheduler">The System.Threading.Tasks.TaskScheduler to use to handle scheduling.</param>
        /// <remarks>
        /// The scheduler can be used to ensure the System.Action is run on the UI thread.
        /// </remarks>
        void WhenReady(Action action, TaskScheduler scheduler);
        /// <summary>
        /// Execute a System.Action when the core service is stopped.
        /// Actions are stored until the core service begins to shutdown.
        /// </summary>
        /// <param name="action">>The System.Action to execute.</param>
        /// <param name="scheduler">The System.Threading.Tasks.TaskScheduler to use to handle scheduling.</param>
        void BeforeStopping(Action action, TaskScheduler scheduler);

        /// <summary>
        /// Start the service.
        /// </summary>
        void Start();
        /// <summary>
        /// Stop the service.
        /// </summary>
        void Stop();
    }
}
