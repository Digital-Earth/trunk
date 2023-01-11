using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Reflection;
using System.Web;
using System.Web.Http.OData;
using System.Web.Http.OData.Extensions;
using System.Web.Http.OData.Query;
using LicenseServer.App_Start;
using LicenseServer.Models.Mongo;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Extensions
{
    public static class IQueryableExtensions
    {
        public static PageResult<dynamic> ToPageResult<T>(this IQueryable<T> result, HttpRequestMessage request, ODataQueryOptions<T> options) where T : class
        {
            var results = options.ApplyTo(result, ODataConfig.ODataQuerySettings);

            return new PageResult<dynamic>(results as IEnumerable<dynamic>, request.ODataProperties().NextLink, request.ODataProperties().TotalCount);
        }

        public static PageResult<dynamic> ToFormattedPageResult(this IQueryable<Resource> result, ResultFormat format, HttpRequestMessage request, ODataQueryOptions<Resource> options)
        {
            return result.FormatResources(format).ToPageResult(request, options);
        }
    }
}