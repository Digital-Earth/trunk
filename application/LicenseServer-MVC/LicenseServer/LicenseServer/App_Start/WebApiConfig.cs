/******************************************************************************
WebApiConfig.cs

begin		: Sept. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Web.Http;
using System.Web.Http.Controllers;
using System.Web.Http.OData.Extensions;
using System.Web.Http.Routing;

namespace LicenseServer
{
    public static class WebApiConfig
    {
        public static void Register(HttpConfiguration config)
        {
            // Cause browsers to return JSON over xml
            config.Formatters.JsonFormatter.SupportedMediaTypes.Add(new MediaTypeHeaderValue("text/html"));

            config.MapHttpAttributeRoutes(new CustomDirectRouteProvider());

            config.Routes.MapHttpRoute(
                name: "DefaultApi",
                routeTemplate: "api/v1/{controller}/{id}",
                defaults: new { id = RouteParameter.Optional },
                constraints: new { controller = @"^(?!(LegacyUser|CORS)).*$" }
            );
            
            config.Routes.MapHttpRoute(
                name: "VersionsApi",
                routeTemplate: "api/v1/{controller}/{resourceId}/{action}",
                defaults: new { },
                constraints: new { controller = @"^(?!(LegacyUser|CORS)).*$", action = "Versions" }
            );

            config.Routes.MapHttpRoute(
                name: "PipelineDetailApi",
                routeTemplate: "api/v1/Pipeline/Details/{procRef}",
                defaults: new { controller = "Pipeline", action = "Details" },
                constraints: new { controller = @"^(?!(LegacyUser|CORS)).*$" }
            );
            
            // Uncomment the following line of code to enable query support for actions with an IQueryable or IQueryable<T> return type.
            // To avoid processing unexpected or malicious queries, use the validation settings on QueryableAttribute to validate incoming queries.
            // For more information, visit http://go.microsoft.com/fwlink/?LinkId=279712.
            config.AddODataQueryFilter();

            // To disable tracing in your application, please comment out or remove the following line of code
            // For more information, refer to: http://www.asp.net/web-api
            config.EnableSystemDiagnosticsTracing();
        }
    }

    public class CustomDirectRouteProvider : DefaultDirectRouteProvider
    {
        protected override IReadOnlyList<IDirectRouteFactory>
        GetActionRouteFactories(HttpActionDescriptor actionDescriptor)
        {
            return actionDescriptor.GetCustomAttributes<IDirectRouteFactory>(inherit: true);
        }
    }
}
