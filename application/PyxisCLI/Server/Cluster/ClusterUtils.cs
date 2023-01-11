using System;
using System.Linq;
using System.Threading.Tasks;
using Pyxis.IO.DataDiscovery;

namespace PyxisCLI.Server.Cluster
{
    internal static class ClusterUtils
    {
        public static UriBuilder AddPath(this UriBuilder builder, params string[] path)
        {
            if (builder.Path != String.Empty && !builder.Path.EndsWith("/"))
            {
                builder.Path += "/";
            }
            builder.Path += String.Join("/",path.Select(part=>part.Trim('/')));
            return builder;
        }

        public static UriBuilder AddQuery(this UriBuilder builder, string key,object value)
        {
            //var parameters = HttpUtility.ParseQueryString(builder.Query);
            //parameters[key] = value;
            //builder.Query = parameters.ToString();

            var query = builder.Query;
            if (query != String.Empty && !query.EndsWith("&"))
            {
                query += "&";
            }
            builder.Query = query.TrimStart('?') + key + "=" + Uri.EscapeDataString(value.ToString());
            
            return builder;
        }

        public static async Task<DiscoveryHttpResult> SendAsync(DiscoveryHttpRequest request)
        {
            return await request.SendAsync() as DiscoveryHttpResult;
        }

        public static async Task<DiscoveryHttpResult> SendAsync(string uri, string method, TimeSpan? timeout = null)
        {
            return await SendAsync(new DiscoveryHttpRequest()
            {
                Uri = uri,
                Method = method,
                Timeout = timeout
            });
        }
    }
}