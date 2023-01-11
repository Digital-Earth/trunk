using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Publishing;
using User = Pyxis.Publishing.User;

namespace Pyxis.Core
{
    /// <summary>
    /// Object used to configure an instance of Pyxis.Core.Engine.
    /// </summary>
    public class EngineConfig
    {
        /// <summary>
        /// Gets or sets the User of the created Pyxis.Core.Engine.  
        /// The User is used for sending authenticated messages with globalgridsystems.com API.
        /// </summary>
        public User User { get; set;}

        /// <summary>
        /// Gets or sets whether the created instance of Pyxis.Core.Engine is configured to use the PYXIS innovation inc. network PyxNet.
        /// </summary>
        public bool UsePyxnet { get; set; }
        
        /// <summary>
        /// Gets or sets the name to associate with the instance of Pyxis.Core.Engine.
        /// </summary>
        public string EngineName { get; set; }

        /// <summary>
        /// Gets or sets the license server API url to use with the Pyxis.Core.Engine.
        /// </summary>
        public string APIUrl { get; set; }

        /// <summary>
        /// if set, overwrite default Cache directory location
        /// </summary>
        public string CacheDirectory { get; set; }

        /// <summary>
        /// Gets or sets whether to clear the cache directory when the Pyxis.Core.Engine is started.
        /// </summary>
        public bool ClearCache { get; set; }

        /// <summary>
        /// Set a constant Node Id. use it only if you know what you are doing.
        /// </summary>
        public Guid PyxNetNodeId { get; set; }

        /// <summary>
        /// Set trace level from Pyxis.Core into console
        /// </summary>
        public TraceLevels TraceLevel { get; set; }

        private string m_definedWorkingDirectory;
        /// <summary>
        /// Gets or sets the working directory of the created Pyxis.Core.Engine.
        /// </summary>
        public string WorkingDirectory
        {
            get
            {
                return m_definedWorkingDirectory ?? GenerateDefaultWorkingDirectory();
            }
            set
            {
                m_definedWorkingDirectory = value;
            }
        }

        private string m_definedApplicationDirectory;
        /// <summary>
        /// Gets or sets the application directory of the created Pyxis.Core.Engine.
        /// </summary>
        public string ApplicationDirectory
        {
            get
            {
                return m_definedApplicationDirectory ?? GenerateDefaultApplicationDirectory();
            }
            set
            {
                m_definedApplicationDirectory = value;
            }
        }

        /// <summary>
        /// Allow the user to register custom resource reference resolvers. 
        /// Engine will default to use Engine.GetChannel.GetResource() to locate the resoruce.
        /// Add more resolvers to enable local GeoSources created by scripts and alike.
        /// </summary>
        public List<Func<ResourceReference, Pipeline>> ResourceReferenceResolvers { get; private set; }


        /// <summary>
        /// Allow the user to register custom resource reference resolvers. 
        /// Add more resolvers to enable local GeoSources created by scripts and alike.
        /// </summary>
        public List<Func<ReferenceOrExpression, Pipeline>> ReferenceResolvers { get; private set; }

        public EngineConfig()
        {
            TraceLevel = TraceLevels.Errors;
            ResourceReferenceResolvers = new List<Func<ResourceReference, Pipeline>>();
            ReferenceResolvers = new List<Func<ReferenceOrExpression, Pipeline>>();
        }

        /// <summary>
        /// A default Pyxis.Core.EngineConfig for general-purpose applications
        /// </summary>
        public static EngineConfig WorldViewDefault
        {
            get
            {
                return new EngineConfig()
                {
                    UsePyxnet = true,
                    EngineName = "WorldView",
                    APIUrl = Pyxis.Publishing.ApiUrl.ProductionLicenseServerRestAPI
                };
            }
        }

        /// <summary>
        /// Generate a default working directory for the Pyxis.Core.EngineConfig
        /// </summary>
        /// <returns>The generated working directory</returns>
        public string GenerateDefaultWorkingDirectory()
        {
            return 
                Environment.GetFolderPath(
                Environment.SpecialFolder.LocalApplicationData) +
                Path.DirectorySeparatorChar + 
                "PYXIS" + 
                Path.DirectorySeparatorChar + 
                EngineName;
        }

        /// <summary>
        /// Generate a default working directory for the Pyxis.Core.EngineConfig
        /// </summary>
        /// <returns>The generated application directory</returns>
        public string GenerateDefaultApplicationDirectory()
        { 
            return 
                Path.GetDirectoryName(
                    Assembly.GetExecutingAssembly().Location);
        }

        /// <summary>
        /// Create a Pyxis.Core.EngineConfig using a Pyxis.Publishing.ApiKey.
        /// The created Pyxis.Core.EngineConfig sets UsePyxnet to false.
        /// </summary>
        /// <param name="apiKey">Defines the api key used to initialize the configuration's User</param>
        /// <returns>The created Pyxis.Core.EngineConfig</returns>
        public static EngineConfig FromApiKey(ApiKey apiKey)
        {
            var channel = new Channel(Pyxis.Publishing.ApiUrl.ProductionLicenseServerRestAPI).Authenticate(apiKey);

            //todo - we need some backend logic to generate different config based apiKey

            return new EngineConfig()
            {
                User = channel.AsUser(),
                UsePyxnet = false,
                EngineName = "GeoWebCore",
                APIUrl = Pyxis.Publishing.ApiUrl.ProductionLicenseServerRestAPI
            };
        }
    }
}
