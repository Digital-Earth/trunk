using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace Pyxis.Core.Services
{
    /// <summary>
    /// An abstract base class for a core service.
    /// </summary>
    public abstract class ServiceBase : ICoreService
    {        
        /// <summary>
        /// Gets the Pyxis.Core.CoreServiceState of the core service.
        /// </summary>
        public CoreServiceState State { get; private set;}

        private readonly object m_serviceLock = new object();
        private readonly List<Action> m_whenReadyQueue = new List<Action>();
        private readonly List<Action> m_beforeStoppingQueue = new List<Action>();

        /// <summary>
        /// Initializes a new instance of Pyxis.Core.Services.ServiceBase.
        /// </summary>
        protected ServiceBase()
        {
            State = CoreServiceState.Created;
        }

        /// <summary>
        /// Execute a System.Action when the core service is running.
        /// Actions are executed immediately if the core service is running; otherwise they are stored.
        /// </summary>
        /// <param name="action">The System.Action to execute.</param>
        public void WhenReady(Action action)
        {
            lock (m_serviceLock)
            {
                if (State == CoreServiceState.Running)
                {
                    Task.Factory.StartNew(action);
                }
                else
                {
                    m_whenReadyQueue.Add(action);
                }
            }
        }

        /// <summary>
        /// Execute a System.Action when the core service is stopped.
        /// Actions are stored until the core service begins to shutdown.
        /// </summary>
        /// <param name="action">The System.Action to execute.</param>
        /// <exception cref="System.InvalidOperationException">The core service is already shutting down or has been disposed.</exception>
        public void BeforeStopping(Action action)
        {
            lock (m_serviceLock)
            {
                if (State == CoreServiceState.ShuttingDown)
                {
                    throw new InvalidOperationException("Service has already started the shut down process");
                } 
                else if (State == CoreServiceState.Stopped)
                {
                    throw new InvalidOperationException("Service has already been shut down");
                }
                else
                {
                    m_beforeStoppingQueue.Add(action);
                }
            }
        }

        /// <summary>
        /// Execute a System.Action when the core service is running.
        /// Actions are executed immediately if the core service is running; otherwise they are stored.
        /// </summary>
        /// <param name="action">>The System.Action to execute.</param>
        /// <param name="scheduler">The System.Threading.Tasks.TaskScheduler to use to handle scheduling.</param>
        public void WhenReady(Action action, TaskScheduler scheduler)
        {
            WhenReady(() =>
            {
                Task.Factory.StartNew(action, CancellationToken.None, TaskCreationOptions.None, scheduler).Wait();
            });
        }

        /// <summary>
        /// Execute a System.Action when the core service is stopped.
        /// Actions are stored until the core service begins to shutdown.
        /// </summary>
        /// <param name="action">>The System.Action to execute.</param>
        /// <param name="scheduler">The System.Threading.Tasks.TaskScheduler to use to handle scheduling.</param>
        /// <exception cref="System.InvalidOperationException">The core service is already shutting down or has been disposed.</exception>
        public void BeforeStopping(Action action, TaskScheduler scheduler)
        {
            BeforeStopping(() =>
            {
                Task.Factory.StartNew(action, CancellationToken.None, TaskCreationOptions.None, scheduler).Wait();
            });
        }

        /// <summary>
        /// Start the core service.
        /// </summary>
        /// <exception cref="System.InvalidOperationException">The core service has already been started.</exception>
        public void Start()
        {
            lock (m_serviceLock)
            {
                if (State != CoreServiceState.Created)
                {
                    throw new InvalidOperationException("Service can't be started twice");
                }

                State = CoreServiceState.Starting;
            }

            try
            {
                StartService();

                lock (m_serviceLock)
                {
                    State = CoreServiceState.Running;
                }

                m_whenReadyQueue.ForEach(action => Task.Factory.StartNew(action));
                m_whenReadyQueue.Clear();
            }
            catch (Exception e)
            {
                lock (m_serviceLock)
                {
                    State = CoreServiceState.Broken;
                }

                throw e;
            }
        }
        
        /// <summary>
        /// Stop the core service.
        /// </summary>
        /// <exception cref="System.InvalidOperationException">The core service has already been stopped or is shutting down; or the core service is still starting.</exception>
        public void Stop()
        {
            lock (m_serviceLock)
            {
                switch (State)
                {
                    case CoreServiceState.Created:
                        //we never initialize - do nothing...
                        return;

                    case CoreServiceState.Starting:
                        throw new InvalidOperationException("Engine cannot be stopped before it completely loads");
                        
                    case CoreServiceState.ShuttingDown:
                    case CoreServiceState.Stopped:
                        throw new InvalidOperationException("Engine cannot be stopped twice");
                        
                    case CoreServiceState.Running:
                    case CoreServiceState.Broken:
                        State = CoreServiceState.ShuttingDown;
                        break;
                }                
            }

            try
            {
                m_beforeStoppingQueue.ForEach(action => action());

                StopService();

                lock (m_serviceLock)
                {
                    State = CoreServiceState.Stopped;
                }
            }
            catch (Exception)
            {
                lock (m_serviceLock)
                {
                    State = CoreServiceState.Broken;
                }

                throw;
            }
        }

        /// <summary>
        /// Called when the core service is about to start. This call is protected by Pyxis.Core.Services.ServiceBase class.
        /// </summary>
        protected abstract void StartService();

        /// <summary>
        /// Called when the core service is about to stop. This call is protected by Pyxis.Core.Services.ServiceBase class.
        /// </summary>
        protected abstract void StopService();
    }
}
