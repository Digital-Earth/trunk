using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http.OData.Query;

namespace PyxisCLI.Server
{
    internal static class ODataExtensions
    {
        public static IEnumerable<dynamic> ApplyTo<T>(this ODataQueryOptions<T> options, IEnumerable<T> items)
        {
            return options.ApplyTo(items.AsQueryable(), ODataConfig.ODataQuerySettings) as IEnumerable<dynamic>;
        }
    }
}
