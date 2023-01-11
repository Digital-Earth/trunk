using GeoWebCore.Properties;
using System;
using System.Threading.Tasks;
using System.Web.Http.Filters;

namespace GeoWebCore.WebConfig
{
    /// <summary>
    /// Helper Attribute to add Cache headers to Web API request.
    /// 
    /// it seems that Web API has no built in OutputCache directive. and the NuGet package that I have found was way to complex for our needs.
    /// 
    /// Usage:
    ///   decorate a controller or a method.
    ///   
    ///   [HttpGet]
    ///   [ApiCache]
    ///   public string Hello() { return "world"; }
    ///   
    /// ApiCacheAttribute add a cache to the http get requests that didn't throw an exception.
    /// 
    /// You can control how long the Cache maxAge will be in 2 methods:   
    /// 1) preferred way: [ApiCache(Profile=ApiCacheAttribute.Profiles.Static)] - will use one of the cache profile settings.
    /// 2) custom way: [ApiCache(Hours=1,Minutes=30)] - Days,Hours,Minutes,Seconds fields are available.
    /// </summary>
    internal class ApiCacheAttribute : ActionFilterAttribute
    {
        /// <summary>
        /// ApiCacheAttribute profiles
        /// </summary>
        public enum Profiles
        {
            /// <summary>
            /// No cache - this response is dynamic 
            /// </summary>
            None,
            /// <summary>
            /// Short, for request that update frequently
            /// </summary>
            Short,

            /// <summary>
            /// Default cache time - defined in configuration (consider days)
            /// </summary>
            Default,

            /// <summary>
            /// For static pages
            /// </summary>
            Static
        }

        /// <summary>
        /// Seconds to store in cache
        /// </summary>
        public int Seconds { get; set; }
        /// <summary>
        /// Minutes to store in cache
        /// </summary>
        public int Minutes { get; set; }

        /// <summary>
        /// Hours to store in cache
        /// </summary>
        public int Hours { get; set; }

        /// <summary>
        /// Days to store in cache
        /// </summary>
        public int Days { get; set; }

        /// <summary>
        /// Profile cache to use
        /// </summary>
        public Profiles Profile { get; set; }


        /// <summary>
        /// Create an attribute for adding cache to requests
        /// </summary>
        public ApiCacheAttribute()
        {
            Profile = Profiles.Default;
        }

        public override Task OnActionExecutedAsync(HttpActionExecutedContext actionExecutedContext, System.Threading.CancellationToken cancellationToken)
        {
            if (actionExecutedContext != null &&
                actionExecutedContext.Exception == null &&
                actionExecutedContext.Request.Method == System.Net.Http.HttpMethod.Get)
            {
                var maxAge = GetMaxAge();

                if (maxAge != TimeSpan.Zero)
                {
                    //add cache to request
                    actionExecutedContext.Response.AddCache(actionExecutedContext.Request, maxAge);

                    //because this goes into CDN - we need to verify that "CORS" has "*" origin.
                    actionExecutedContext.Response.Headers.Add("Access-Control-Allow-Origin", new[] { "*" });
                }
            }
            return base.OnActionExecutedAsync(actionExecutedContext, cancellationToken);
        }

        private TimeSpan GetMaxAge()
        {
            var maxAge = TimeSpan.FromSeconds(Seconds + 60 * (Minutes + 60 * (Hours + 24 * Days)));

            if (maxAge == TimeSpan.Zero)
            {
                switch (Profile)
                {
                    case Profiles.None:
                        maxAge = TimeSpan.Zero;
                        break;

                    case Profiles.Short:
                        maxAge = TimeSpan.FromMinutes(1);
                        break;

                    case Profiles.Default:
                    case Profiles.Static:
                        maxAge = TimeSpan.FromDays(Settings.Default.DefaultCacheInDays);
                        break;
                }
            }
            return maxAge;
        }
    }
}
