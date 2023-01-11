using System;
using System.Collections.Generic;
using System.Configuration;
using System.Linq;
using System.Net;
using System.Net.Configuration;
using System.Net.Mail;
using System.Web;

namespace worldView.gallery.Configuration
{
    /// <summary>
    /// Helper class to read site configuration settings
    /// </summary>
    public static class SiteConfiguration
    {
        /// <summary>
        /// List of all names of connection strings in web.config 
        /// </summary>
        private static class ConnectionStringsKeys
        {
            public static string CrashDumpConnectionString = "CrashDumpsAzureConnectionString";
        }
        
        /// <summary>
        /// Settings related to crash dumps
        /// </summary>
        public static class CrashDump
        {
            public static string ConnectionString
            {
                get
                {
                    return ConfigurationManager.ConnectionStrings[ConnectionStringsKeys.CrashDumpConnectionString].ConnectionString;
                }
            }

            public static string ContainerName
            {
                get
                {
                    return "crash-dumps";
                }
            }
        }

        public static class SupportEmail
        {
            public static string Email
            {
                get
                {
                    return (ConfigurationManager.GetSection("system.net/mailSettings/smtp") as SmtpSection).From;
                }
            }
        }
    }
}