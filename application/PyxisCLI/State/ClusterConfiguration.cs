using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ApplicationUtility;
using Newtonsoft.Json;

namespace PyxisCLI.State
{
    public class JobConfiguration
    {
        public class DeploymentDetails
        {
            [JsonProperty("name")]
            public string Name { get; set; }
            
            [JsonProperty("version")]
            public string Version { get; set; }
        }

        public class AuthenticationDetails        
        {
            [JsonProperty("provider")]
            public string Provider { get; set; }

            [JsonProperty("settings")]
            public Dictionary<string,object> Settings { get; set; }
        }

        [JsonProperty("deployment")]
        public DeploymentDetails Deployment { get; set; }

        [JsonProperty("authentication")]
        public AuthenticationDetails Authentication { get; set; }
    }

    /// <summary>
    /// Extract configuration from Environment vairables that sent by Cluster runner
    /// </summary>
    static class ClusterConfiguration
    {
        public static string MasterNode
        {
            get { return Environment.GetEnvironmentVariable("GGS_CLUSTER_MASTER"); }
        }

        public static string JobId
        {
            get { return Environment.GetEnvironmentVariable("GGS_JOB"); }
        }

        private static JobConfiguration s_configuration;

        public static JobConfiguration JobConfiguration
        {
            get
            {
                if (s_configuration == null)
                {
                    s_configuration =
                        JsonConvert.DeserializeObject<JobConfiguration>(
                            Environment.GetEnvironmentVariable("GGS_JOB_CONFIG") ?? "")
                        ?? new JobConfiguration();
                }

                return s_configuration;
            }
        }

        private static string s_authProvider;

        public static string AuthenticationProvider
        {
            get
            {
                if (s_authProvider.HasContent())
                {
                    return s_authProvider;
                }
                return JobConfiguration.Authentication != null ? JobConfiguration.Authentication.Provider : null;
            }
        }

        public static void ForceAuthenticationProvider(string provider)
        {
            s_authProvider = provider;
        }
    }
}
