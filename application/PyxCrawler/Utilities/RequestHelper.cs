using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Web;
using PyxCrawler.Models;

namespace PyxCrawler.Utilities
{
    public static class RequestHelper
    {
        private static string[] s_arcGisProtocols = {"AGSF", "AGSM"};

        public static Uri CreateCapabilitiesRequest(Uri uri, OnlineGeospatialService service)
        {
            var uriBuilder = new UriBuilder(uri);
            var query = HttpUtility.ParseQueryString(uriBuilder.Query);
            
            if (s_arcGisProtocols.Contains(service.Protocol))
            {
                CreateAgsCapabilitiesQuery(query);
            }
            else
            {
                CreateOgcCapabilitiesQuery(query, service);
            }

            uriBuilder.Query = query.ToString();
            return uriBuilder.Uri;
        }

        private static void CreateOgcCapabilitiesQuery(NameValueCollection query, OnlineGeospatialService service)
        {
            query["Service"] = service.Protocol;
            query["Version"] = service.Version;
            query["Request"] = "GetCapabilities";
        }

        private static void CreateAgsCapabilitiesQuery(NameValueCollection query)
        {
            query["f"] = "json";
        }
    }
}