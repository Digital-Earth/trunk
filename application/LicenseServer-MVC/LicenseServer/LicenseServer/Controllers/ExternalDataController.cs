using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web;
using System.Web.Http;
using System.Web.Http.ModelBinding;
using System.Web.Http.OData;
using System.Web.Http.OData.Query;
using Elmah;
using LicenseServer.App_Start;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;

namespace LicenseServer.Controllers
{
    [RoutePrefix("api/v1/ExternalData")]
    public class ExternalDataController : CORSMongoApiController
    {
        // GET api/v1/ExternalData?search={search string}
        public PageResult<dynamic> Get(string search, ODataQueryOptions<ExternalData> options)
        {
            return Get(search, null, null, options);
        }

        // GET api/v1/ExternalData?centerLat={lat}&centerLon={lon}
        public PageResult<dynamic> Get(double centerLat, double centerLon, ODataQueryOptions<ExternalData> options)
        {
            return Get(null, new List<double>{centerLat, centerLon}, null, options);
        }

        // GET api/v1/ExternalData?upperLat={lat}&leftLon={lon}&lowerLat={lat}&rightLon={lon}
        public PageResult<dynamic> Get(double upperLat, double leftLon, double lowerLat, double rightLon, ODataQueryOptions<ExternalData> options)
        {
            return Get(null, null, new Envelope(upperLat, leftLon, lowerLat, rightLon), options);
        }

        // GET api/v1/ExternalData?search={search string}&centerLat={lat}&centerLon={lon}
        public PageResult<dynamic> Get(string search, double centerLat, double centerLon, ODataQueryOptions<ExternalData> options)
        {
            return Get(search, new List<double> { centerLat, centerLon }, null, options);
        }

        // GET api/v1/ExternalData?search={search string}&upperLat={lat}&leftLon={lon}&lowerLat={lat}&rightLon={lon}
        public PageResult<dynamic> Get(string search, double upperLat, double leftLon, double lowerLat, double rightLon, ODataQueryOptions<ExternalData> options)
        {
            return Get(search, null, new Envelope(upperLat, leftLon, lowerLat, rightLon), options);
        }

        // GET api/v1/ExternalData?centerLat={lat}&centerLon={lon}&upperLat={lat}&leftLon={lon}&lowerLat={lat}&rightLon={lon}
        public PageResult<dynamic> Get(double centerLat, double centerLon, double upperLat, double leftLon, double lowerLat, double rightLon, ODataQueryOptions<ExternalData> options)
        {
            return Get(null, new List<double> { centerLat, centerLon }, new Envelope(upperLat, leftLon, lowerLat, rightLon), options);
        }

        // GET api/v1/ExternalData?search={search string}&centerLat={lat}&centerLon={lon}&upperLat={lat}&leftLon={lon}&lowerLat={lat}&rightLon={lon}
        public PageResult<dynamic> Get(string search,
            double centerLat, double centerLon,
            double upperLat, double leftLon, double lowerLat, double rightLon,
            ODataQueryOptions<ExternalData> options)
        {
            return Get(search, new List<double> { centerLat, centerLon }, new Envelope(upperLat, leftLon, lowerLat, rightLon), options);
        }

        private PageResult<dynamic> Get(string search, List<double> center, Envelope bbox, ODataQueryOptions<ExternalData> options)
        {
            IQueryable<ExternalData> result;
            if (search == null && center == null && bbox == null)
            {
                result = new List<ExternalData>().AsQueryable();
            }
            else
            {
                var skip = options.Skip != null ? options.Skip.Value : 0;
                var top = options.Top != null ? options.Top.Value : ODataConfig.PageSize;
                result = db.SearchExternal(search, center, bbox, skip, top);
            }

            return result.ToPageResult(Request, options);
        }
    }
}