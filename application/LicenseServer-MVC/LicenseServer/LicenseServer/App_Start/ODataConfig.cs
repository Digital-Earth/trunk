using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Http.OData.Query;

namespace LicenseServer.App_Start
{
    public static class ODataConfig
    {
        private static int? s_ODataPageSize = 50;
        private static ODataQuerySettings s_ODataSettings = new ODataQuerySettings() { PageSize = s_ODataPageSize, EnsureStableOrdering = false };

        public static ODataQuerySettings ODataQuerySettings
        {
            get
            {
                return s_ODataSettings;
            }
        }

        public static int PageSize
        {
            get
            {
                return s_ODataPageSize.Value;
            }
        }
    }
}