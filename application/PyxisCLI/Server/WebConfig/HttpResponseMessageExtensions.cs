using System;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using PyxisCLI.Properties;

namespace PyxisCLI.Server.WebConfig
{
    /// <summary>
    /// HttpResponseMessage helper extensions to add cache control headers
    /// </summary>
    public static class HttpResponseMessageExtensions
    {
        /// <summary>
        /// Create a http response with default cache headers for a given result object.
        /// </summary>
        /// <typeparam name="T">Type of Result object</typeparam>
        /// <param name="request">request to be used for generating a response</param>
        /// <param name="code">response code to return</param>
        /// <param name="result">result object to return</param>
        /// <returns>A HttpResponseMessage</returns>
        public static HttpResponseMessage CreateResponseWithCache<T>(this HttpRequestMessage request, HttpStatusCode code, T result)
        {
            var response = request.CreateResponse(code, result);
            response.AddCache(request);
            return response;
        }

        /// <summary>
        /// Create a 200 response with default cache headers for a given result object.
        /// </summary>
        /// <typeparam name="T">Type of Result object</typeparam>
        /// <param name="request">request to be used for generating a response</param>
        /// <param name="result">result object to return</param>
        /// <returns>A HttpResponseMessage</returns>
        public static HttpResponseMessage CreateResponseWithCache<T>(this HttpRequestMessage request, T result)
        {
            return request.CreateResponseWithCache(HttpStatusCode.OK, result);
        }

        /// <summary>
        /// Adding default cache control headers for an http response
        /// </summary>
        /// <param name="response">response to modify http cache headers</param>
        /// <param name="request">Request to add cache control headers</param>
        public static void AddCache(this HttpResponseMessage response, HttpRequestMessage request)
        {
            response.AddCache(request, TimeSpan.FromDays(Settings.Default.DefaultCacheInDays));
        }

        /// <summary>
        /// Adding default cache control headers for an http response
        /// </summary>
        /// <param name="response">response to modify http cache headers</param>
        /// <param name="request">Request to add cache control headers</param>
        /// <param name="maxAge">maxAge for the cache control header</param>
        public static void AddCache(this HttpResponseMessage response, HttpRequestMessage request, TimeSpan maxAge)
        {
            if (request == null)
            {
                //this probably a unit a test if now request object was provided
                return;
            }

            if (!request.Headers.Contains("ApiCachePrevent"))
            {
                response.Headers.CacheControl = new CacheControlHeaderValue
                {
                    Public = true,
                    MaxAge = maxAge
                };
            }
            else
            {
                response.Headers.CacheControl = new CacheControlHeaderValue
                {
                    Public = false,
                    NoCache = true,
                    MaxAge = TimeSpan.Zero
                };
            }
        }
    }
}
