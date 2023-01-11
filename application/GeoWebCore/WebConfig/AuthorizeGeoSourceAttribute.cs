using GeoWebCore.Properties;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Threading;
using System.Web.Http;
using System.Web.Http.Controllers;
using Pyxis.Contract;
using Pyxis.Utilities;
using Pyxis.Publishing;
using Pyxis.Publishing.Permits;
using Pyxis.Contract.Publishing;
using System.Web.Http.Filters;
using GeoWebCore.Services;
using GeoWebCore.Utilities;

namespace GeoWebCore.WebConfig
{
    /// <summary>
    /// Helper Attribute to authorize requests for GeoSources
    /// 
    /// [UserAuth(GeoSourceKey="id")] - specify the route parameter/query parameter/object property containing the GeoSource Id. Default value is "geoSource".
    /// e.g. [UserAuth] - searches for geosource (case-insensitive) as a route or query parameter.
    ///      [UserAuth(GeoSourceKey="styleRequest.GeoSource.Id")] - searches for the GeoSource.Id (case-sensitive) property in the styleRequest parameter.
    /// </summary>
    internal class AuthorizeGeoSourceAttribute : ActionFilterAttribute // use Action filter to have access to action arguments after media formatting
    {
        public string GeoSourceKey { get; set; }
        private static readonly string s_defaultGeoSourceKey = "geosource";

        const int GeoSourceCacheSize = 1000;

        // this cache is for auth_token -> dictionary of privateGeoSources and if the user has access
        private static readonly object s_geoSourceAuthLock = new object();
        private static readonly LimitedSizeDictionary<string, Dictionary<string, bool>> s_geoSourceAuthCache =
            new LimitedSizeDictionary<string, Dictionary<string, bool>>(GeoSourceCacheSize);

        // use shared token to create fully authorized channels per request and avoid authenticate network requests
        private static readonly IPermitRetainer<AccessToken> s_tokenRetainer;

        static AuthorizeGeoSourceAttribute()
        {
            var apiKey = new ApiKey(Settings.Default.GWCUserKey, Settings.Default.GWCUserAccount);
            var fullyAuthorizedChannel = new Channel(ApiUrl.ProductionLicenseServerRestAPI).Authenticate(apiKey);
            s_tokenRetainer = fullyAuthorizedChannel.TokenRetainer;
        }

        /// <summary>
        /// Creates an AuthorizeGeoSource Attribute 
        /// </summary>
        public AuthorizeGeoSourceAttribute()
        {
            GeoSourceKey = s_defaultGeoSourceKey;  // query string key
        }

        public override Task OnActionExecutingAsync(HttpActionContext actionContext, CancellationToken cancellationToken)
        {
            if (IsAuthorized(actionContext))
            {
                return base.OnActionExecutingAsync(actionContext, cancellationToken);
            }

            var response = actionContext.Request.CreateResponse(HttpStatusCode.Unauthorized);
            response.Headers.CacheControl = new CacheControlHeaderValue
            {
                Public = false,
                NoCache = true,
                MaxAge = TimeSpan.Zero
            };
            throw new HttpResponseException(response);
        }

        private bool IsAuthorized(HttpActionContext actionContext)
        {
            string geoSourceId = null;

            if (GeoSourceKey.Contains("."))
            {
                // attempt to read id from property of object
                try
                {
                    var firstDot = GeoSourceKey.IndexOf(".");
                    var argumentName = GeoSourceKey.Substring(0, firstDot);
                    var propertyPath = GeoSourceKey.Substring(firstDot + 1);
                    var actionArgument = actionContext.ActionArguments[argumentName];
                    var property = actionArgument.GetPropertyValue<Guid>(propertyPath);
                    if (property != Guid.Empty)
                    {
                        geoSourceId = property.ToString();
                    }
                }
                catch
                {
                }

            }
            else
            {
                // attempt to read id from route data
                object id = null;
                if(actionContext.RequestContext.RouteData.Values.TryGetValue(GeoSourceKey, out id))
                {
                    geoSourceId = id.ToString();
                }
                else
                {
                    // attempt to ready id from 
                    Dictionary<string, string> queryString = actionContext.Request.GetQueryNameValuePairs().ToDictionary(x => x.Key.ToLower(), x => x.Value);
                    queryString.TryGetValue(GeoSourceKey, out geoSourceId);
                }
            }

            if (geoSourceId == null)
            {
                return false;
            }

            var guid = Guid.Parse(geoSourceId);

            // step 1 -- is geoSource private??  check visibility cache first, otherwise fetch and cache
            bool isPrivate = true;

            var geoSource = GeoSourceInitializer.GetGeoSource(guid);

            if (geoSource != null)
            {
                isPrivate = geoSource.Metadata.Visibility == VisibilityType.Private;
            }
            else
            {
                isPrivate = false;
            }

            // step 2 - validate isPrivate with Authorization header presense
            if (geoSource != null && isPrivate != actionContext.Request.Headers.Contains("Authorization"))
            {
                //this mean we got private geosource with no authentication headers
                //or we got public geosoruce with authentication headers.

                //in this case we are going to refresh the cache every 30 sec.

                if (GeoSourceInitializer.InvalidateGeoSource(guid, TimeSpan.FromSeconds(30)))
                {
                    //we got a new version
                    geoSource = GeoSourceInitializer.GetGeoSource(guid);

                    if (geoSource != null)
                    {
                        isPrivate = geoSource.Metadata.Visibility == VisibilityType.Private;
                    }
                    else
                    {
                        isPrivate = false;
                    }
                }
            }

            // step 3 - enforce request
            //if geoSource is public - we are done.
            if (!isPrivate)
            {
                return true;
            }

            // when geoSource is private set a header to avoid caching in ApiCacheAttribute
            actionContext.Request.Headers.Add("ApiCachePrevent", "true");
            
            if (!actionContext.Request.Headers.Contains("Authorization"))
            {
                //no Authorization header present 
                return false;
            }

            try
            {
                string authToken = actionContext.Request.Headers.Authorization.Parameter;

                Dictionary<string, bool> geoSourceMap;

                lock (s_geoSourceAuthLock)
                {
                    if (s_geoSourceAuthCache.TryGetValue(authToken, out geoSourceMap))
                    {
                        bool isCachedValue = false;
                        if (geoSourceMap.TryGetValue(geoSourceId, out isCachedValue))
                        {
                            // we can safely return here with a cached true or false
                            return isCachedValue;
                        }
                    }
                    else
                    {
                        geoSourceMap = new Dictionary<string, bool>();
                        s_geoSourceAuthCache[authToken] = geoSourceMap;
                    }
                }


                // if we've gotten this far, the authorization for this geoSource has not been cached
                // so we request the geoSource with the given authorization, apply it to the cache and return

                var tokenDetails = new AccessToken.TokenDetails
                {
                    Token = authToken
                };

                // make authenticatedChannel
                var channel = new Channel(ApiUrl.ProductionLicenseServerRestAPI);
                var authenticatedChannel = channel.Authenticate(tokenDetails);

                bool isAuthorizedOnGeoSource = false;

                var geoSourceResult = authenticatedChannel.GeoSources.GetById(new Guid(geoSourceId));
                if (geoSourceResult != null)
                {
                    isAuthorizedOnGeoSource = true;
                }

                // set cache and return
                lock (s_geoSourceAuthLock)
                {
                    geoSourceMap[geoSourceId] = isAuthorizedOnGeoSource;
                }

                return isAuthorizedOnGeoSource;
            }
            catch
            {
                return false; // in case of malformed token or failure to fetch
            }
        }
    }
}
