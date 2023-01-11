using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core.IO;
using Pyxis.Core.Services;
using Pyxis.Publishing;
using PyxNet.Pyxis;
using User = Pyxis.Publishing.User;

namespace Pyxis.Core
{
    /// <summary>
    /// A core service encapsulating all of the other services provided by the Pyxis.Core.
    /// </summary>
    /// <remarks>
    /// The Pyxis.Core.Engine service must be started before its services can be used.
    /// Once the Pyxis.Core.Engine is stopped its services can no longer be used.
    /// </remarks>
    public class Engine : ServiceBase
    {
        private EngineConfig Config { get; set; }

        private List<ICoreService> Services { get; set; }

        private PipelineService Pipelines { get; set; }

        private GeometryCacheService GeometryCache { get; set; }

        private Engine(EngineConfig config)
        {
            Config = config;

            Services = new List<ICoreService>();

            //make sure there is only one running engine
            Services.Add(new SingletonEnforcerService());

            //ensure our local presistance is working correctly
            Services.Add(new LocalPersistanceService());

            //we always use pyxlib...
            Services.Add(new PyxlibService(Config));

            if (Config.UsePyxnet)
            {
                //add pyxnet service if needed
                Services.Add(new PyxNetService(Config));
            }

            //create active pipelines cache
            Pipelines = new PipelineService(Config);
            Services.Add(Pipelines);

            //create geometry resolver cache
            GeometryCache = new GeometryCacheService(this);
            Services.Add(GeometryCache);
        }

        /// <summary>
        /// Creates an instance of Pyxis.Core.Engine configured using Pyxis.Core.EngineConfig.
        /// </summary>
        /// <param name="config">The engine configuration used to create the Engine</param>
        /// <returns>The created Engine.</returns>
        static public Engine Create(EngineConfig config)
        {
            return new Engine(config);
        }

        /// <summary>
        /// Starts all the PYXIS services the Pyxis.Core.Engine is configured to use.
        /// </summary>
        /// <exception cref="System.NotSupportedException">The engine only does not support x64 bit environments.</exception>
        /// <exception cref="System.InvalidOperationException">The license server urls do not match.</exception>
        /// <exception cref="Pyxis.Core.EngineException">One of the configured PYXIS services failed to start.</exception>
        protected override void StartService()
        {
            ValidateEnvironment();

            // ensure we are not mixing channels
            if ((Config.User != null) && (Config.User.LicenseServerUrl != Config.APIUrl))
            {
                throw new InvalidOperationException("User's license server must match config's license server.");
            }

            try
            {
                //start services in order
                foreach (var service in Services)
                {
                    service.Start();
                }
            }
            catch (Exception e)
            {
                throw new EngineException("Failed to start Pyxis.Core.Engine sub systems: " + e.Message, e);
            }
        }

        private void ValidateEnvironment()
        {
            if (IntPtr.Size != 4)
            {
                throw new NotSupportedException("Pyxis.Core.Engine doesn't support x64 environment as it depend on x86 native 3rd libraries. Please configure your solution to prefer 32-bit or compile it as x86 CPU");
            }
        }

        /// <summary>
        /// Stops the Pyxis.Core.Engine service by stopping each of the services it encapsulates.
        /// </summary>
        /// <exception cref="Pyxis.Core.EngineException">The Pyxis.Core.Engine service failed to stop</exception>
        protected override void StopService()
        {
            try
            {
                //stop services in reverse order
                foreach (var service in Services.AsEnumerable().Reverse())
                {
                    service.Stop();
                }
            }
            catch (Exception e)
            {
                throw new EngineException("Failed to stop Pxis.Core.Engine sub systems: " + e.Message, e);
            }
        }

        /// <summary>
        /// Checks whether the state of the state Pyxis.Core.Engine corresponds to the expected.
        /// </summary>
        /// <param name="state">The expected state of the Pyxis.Core.Engine.</param>
        /// <param name="message">The message to throw in case of an error.</param>
        /// <exception cref="System.InvalidOperationException">The state of the Pyxis.Core.Engine is unexpected.</exception>
        public void ValidateEngineState(CoreServiceState state, string message)
        {
            if (State != state)
            {
                throw new InvalidOperationException(message);
            }
        }

        private T GetService<T>() where T : class, ICoreService
        {
            return Services.FirstOrDefault(x => x is T) as T;
        }

        public bool PyxNetEnabled
        {
            get { return Config.UsePyxnet; }
        }

        /// <summary>
        /// Determines if a particular data source can be loaded. The algorithm will modify its
        /// behaviour based on the options selected.
        /// </summary>
        /// <param name="strPath">The potential data source.</param>
        /// <param name="options">The type of checking to perform.</param>
        /// <returns>true if the data source can be loaded, otherwise false.</returns>
        public bool IsDataSourceSupported(string strPath, IPipeBuilder.eCheckOptions options)
        {
            return PipelineConstructionUtility.IsDataSourceSupported(strPath, options);
        }
        
        /// <summary>
        /// Publish a pipeline to be reachable over pyxnet. this method does not negotiate with license server.
        /// </summary>
        /// <param name="pipeline">Pipeline to publish.</param>
        /// <returns>True if publish was successful, otherwise it return false.</returns>
        public bool PublishLocally(Pipeline pipeline)
        {
            var pyxNetService = GetService<PyxNetService>();

            if (pyxNetService != null)
            {
                var process = GetProcess(pipeline);
                return PublisherSingleton.Publisher.Publish(new ProcRef(process));
            }
            return false;
        }

        /// <summary>
        /// Unpublishing a pipeline to that was been published previously. this method does not negotiate with license server.
        /// </summary>
        /// <param name="pipeline">Pipeline to unpublish.</param>
        /// <returns>True if unpublish was successful, otherwise it return false.</returns>
        public bool UnpublishLocally(Pipeline pipeline)
        {
            var pyxNetService = GetService<PyxNetService>();

            if (pyxNetService != null)
            {
                var process = GetProcess(pipeline);
                return PublisherSingleton.Publisher.Unpublish(new ProcRef(process));
            }
            return false;
        }

        /// <summary>
        /// Gets a pointer to the process associated with a resource.
        /// If the engine has not initialized the process an attempt is made to initialize a new instance of the process.
        /// </summary>
        /// <param name="resource">A Pyxis.Contract.Publishing.ResourceReference to the resource the process associated with.</param>
        /// <returns>A pointer to the process.</returns>
        /// <exception cref="System.InvalidOperationException">The engine must be started.</exception>
        public IProcess_SPtr GetProcess(ResourceReference resource)
        {
            ValidateEngineState(CoreServiceState.Running, "Can't get process when engine is not running");

            //try to see if process is in cache
            IProcess_SPtr process = null;

            if (Pipelines.TryGetProcess(resource, out process))
            {
                return process;
            }

            //process is not in cache, try resource reference resolvers
            var pipeline =
                Config.ResourceReferenceResolvers
                    .Select(resolver => resolver(resource))
                    .FirstOrDefault(p => p != null) 
                //if all fails, try to request resource from channel.
                ?? GetChannel().GetResource(resource);

            if (pipeline == null)
            {
                throw new Exception("Failed to resolve resource " + resource);
            }

            //try to open that process. will throw exception if we fail
            return Pipelines.GetProcess(pipeline);
        }

        public Pipeline TryResolveReference(ReferenceOrExpression reference)
        {
            var pipeline =
                 Config.ReferenceResolvers
                     .Select(resolver => resolver(reference))
                     .FirstOrDefault(p => p != null);

            return pipeline;
        }

        public Pipeline ResolveReference(string reference)
        {
            return ResolveReference(new ReferenceOrExpression() { Reference = reference});
        }

        public Pipeline ResolveReference(ReferenceOrExpression referenceOrExpression)
        {
            var pipeline = TryResolveReference(referenceOrExpression);

            if (pipeline == null)
            {
                throw new Exception("Failed to resolve reference " + referenceOrExpression.Reference ?? referenceOrExpression.Expression);
            }

            return pipeline;
        }

        /// <summary>
        /// Gets a pointer to the process associated with a Pyxis.Contract.Publishing.Pipeline resource.
        /// If the engine has not initialized the process an attempt is made to initialize a new instance of the process.
        /// </summary>
        /// <param name="pipeline">The Pyxis.Contract.Publishing.Pipeline the process is associated with.</param>
        /// <returns>A pointer to the process.</returns>
        /// <exception cref="System.InvalidOperationException">The engine must be started.</exception>
        public IProcess_SPtr GetProcess(Pipeline pipeline)
        {
            ValidateEngineState(CoreServiceState.Running, "Can't get process when engine is not running");

            return Pipelines.GetProcess(pipeline);
        }

        /// <summary>
        /// Authenticate the Engine for the given User.
        ///
        /// This method can only be used once if a User for authentication has not already been chosen.
        ///
        /// This method is an alternative for using EngineConfig.User. Setting up EngineConfig.User can be used
        /// to initialize the Engine and authenticate as the given User by default.
        /// However, the user is not known while Engine is being initialized, this method can be used once the engine is running.
        /// </summary>
        /// <param name="user">User to be used for authentication</param>
        /// <exception cref="System.InvalidOperationException">thrown when this method is called at an invalid state.
        /// This exception would thrown when calling this function when an Engine User has already been set
        /// or when the Engine is not running.
        /// </exception>
        /// <exception cref="System.ArgumentException">thrown if user's and config's license servers don't match.
        /// </exception>
        public void AuthenticateAs(User user)
        {
            if (Config.User != null)
            {
                throw new InvalidOperationException("AuthenticateAs can't be used once the Engine is already authenticated.");
            }

            // ensure we are not mixing channels
            if (user.LicenseServerUrl != Config.APIUrl)
            {
                throw new ArgumentException("User's license server must match config's license server.");
            }

            ValidateEngineState(CoreServiceState.Running, "AuthenticateAs can't be used while the engine is not running");

            Config.User = user;

            var pyxNetService = GetService<PyxNetService>();

            if (pyxNetService != null)
            {
                pyxNetService.AuthenticateAs(user);
            }
        }

        /// <summary>
        /// Get the Pyxis.Contract.Publishing.UserInfo of the engine's user specified by the engine's Pyxis.Core.EngineConfig.
        /// </summary>
        /// <returns>
        /// The Pyxis.Contract.Publishing.UserInfo of the user in the engine's Pyxis.Core.EngineConfig if it is not null; otherwise null.
        /// </returns>
        public UserInfo GetUserInfo()
        {
            if (Config.User != null)
            {
                return new UserInfo(Config.User.GetUserId(), Config.User.GetProfile().Metadata.Name);
            }
            return null;
        }

        /// <summary>
        /// Gets the Pyxis.Contract.Publishing.User in the Pyxis.Core.Engine configuration.
        /// </summary>
        /// <returns>The Pyxis.Contract.Publishing.User in the Pyxis.Core.Engine configuration.</returns>
        public User GetUser()
        {
            return Config.User;
        }

        /// <summary>
        /// Gets a Pyxis.Publishing.Channel using the Pyxis.Core.Engine User in the engine configuration.
        /// If no User has been set, a Pyxis.Publishing.Channel using anonymous access is returned.
        /// </summary>
        /// <returns>A Pyxis.Publishing.Channel authenticated by the user specified by the engine's Pyxis.Core.EngineConfig; otherwise an unauthenticated Pyxis.Publishing.Channel.</returns>
        public Channel GetChannel()
        {
            if (Config.User != null)
            {
                return Channel.Authenticate(Config.User);
            }
            else
            {
                return new Channel(Config.APIUrl);
            }
        }

        /// <summary>
        /// Convert a given IGeometry to PYXGeometry objects.
        /// </summary>
        /// <param name="geometry">IGeometry object.</param>
        /// <param name="resolution">Resolution to use when converting into PYXGeometry, -1 for default</param>
        /// <returns>created PYXGeometry object (should not be modified).</returns>
        public PYXGeometry_SPtr ToPyxGeometry(IGeometry geometry, int resolution = -1)
        {
            return GeometryCache.ToPyxGeometry(geometry, resolution);
        }

        /// <summary>
        /// Return a bounding geometry for the given GeoSource
        /// </summary>
        /// <param name="geoSource">input GeoSource object</param>
        /// <returns>IGeometry object</returns>
        public IGeometry GetGeometry(GeoSource geoSource) {
            var process = GetProcess(geoSource);
            
            if (process == null)
            {
                throw new Exception("failed to initialize GeoSource");
            }

            var feature = pyxlib.QueryInterface_IFeature(process.getOutput());

            if (feature.isNull()) 
            {
                throw new Exception("failed to extract geometry from GeoSource (can't cast into IFeature interface).");
            }

            var geometry = feature.getGeometry();


            if (geometry.isNull())
            {
                throw new Exception("failed to extract geometry from GeoSource (geometry not provided).");
            }

            return IO.GeoJson.Geometry.FromPYXGeometry(geometry);
        }
    }
}