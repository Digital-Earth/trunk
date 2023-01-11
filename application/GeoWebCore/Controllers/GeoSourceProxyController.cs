using ApplicationUtility;
using System;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using System.Web.Http;
using Pyxis.Contract.Publishing;
using GeoWebCore.WebConfig;
using GeoWebCore.Services;
using Pyxis.Utilities;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// The GeoSourceController provides client access to data from the GeoSource.
    /// </summary>
    [RoutePrefix("api/v1/GeoSource")]
    public class GeoSourceProxyController : ApiController
    {
        /// <summary>
        /// forward a GET HTTP request to GeoSource server
        /// </summary>
        /// <param name="geoSourceId">GeoSource Id</param>
        /// <param name="pathInfo">Sub path that should be passed to the target proxy</param>
        /// <returns>HttpResponse</returns>
        [HttpGet]
        [AuthorizeGeoSource(GeoSourceKey = "geoSourceId")]
        [Route("{geoSourceId}/Proxy/{*pathInfo}")]
        [TimeTrace("geoSourceId")]
        public async Task<HttpResponseMessage> Get(Guid geoSourceId,string pathInfo)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSourceId);

            var geoSource = await state.GetGeoSource();

            var externalUrl = "";
            if (geoSource.Metadata.ExternalUrls != null)
            {
                externalUrl = geoSource.Metadata.ExternalUrls.Where(u => u.Type == ExternalUrlType.Reference).Select(u=>u.Url).FirstOrDefault();
            }

            if (!externalUrl.HasContent())
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadGateway, "GeoSource has no Proxy.");
            }

            var builder = new UriQueryBuilder(externalUrl);

            if (pathInfo.HasContent())
            {
                if (!builder.ServerUri.EndsWith("/"))
                {
                    builder.ServerUri += "/";
                }
                builder.ServerUri += pathInfo;
            }
            

            foreach (var pair in Request.GetQueryNameValuePairs())
            {
                builder.OverwriteParameter(pair.Key,pair.Value);
            }

            //use HttpClient to forward the request.
            
            HttpResponseMessage response;
            
            using (var client = new HttpClient())
            {
                client.Timeout = TimeSpan.FromSeconds(120);
                response = await client.GetAsync(builder.ToString());
            }

            //remove CORS from response so the app CORS middleware can enable it correcty.
            response.Headers.Remove("Access-Control-Allow-Origin");            

            return response;
        }
    }
}
