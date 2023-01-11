using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http.Formatting;
using System.Reflection;
using System.Web.Http;
using System.Web.Http.Cors;
using System.Web.Http.Description;
using System.Web.Http.Filters;
using ApplicationUtility;
using Microsoft.Owin.Cors;
using Microsoft.Owin.Host.HttpListener;
using Owin;
using PyxisCLI.Server.WebConfig;
using PyxisCLI.Server.WebConfig.AuthenticationProvider;
using PyxisCLI.State;
using Swashbuckle.Application;
using Swashbuckle.Swagger;

namespace PyxisCLI.Server
{
    /// <summary>
    /// Configure Owin envrionment
    /// </summary>
    public class OwinStartup
    {
        // This method exists so if we do "remove unused reference the OwinHttpListener is not removed from the project"
        // ReSharper disable once UnusedMember.Local
        private void KeepReferenceToDlls()
        {
            // ReSharper disable once UnusedVariable
            var type = typeof (OwinHttpListener);
        }

        /// <summary>
        /// Owin configuration logic. enable HttpFilters and Swagger
        /// </summary>
        /// <param name="app">AppBuilder instance</param>
        public void Configuration(IAppBuilder app)
        {
//#if DEBUG
//            app.UseErrorPage();
//#endif

            HttpConfiguration config = new HttpConfiguration();

            var authProvider = ClusterConfiguration.AuthenticationProvider;
            if (authProvider.HasContent() && authProvider.ToLower() == "auth0")
            {
                Console.WriteLine("Authentication Provider: Auth0");
                config.Filters.Add(new PlugableAuthenticationAttribute(new Auth0AuthenticationProvider()));
            }
            else
            {
                Console.WriteLine("Authentication Provider: Guest");
                config.Filters.Add(new PlugableAuthenticationAttribute(new GuestAuthenticationProvider()));
            }
            
            //register our default error message handler
            config.Filters.Add(new ApiExceptionFilterAttribute());
                        
            config.MapHttpAttributeRoutes();

            ConfigureFormatters(config);            
            ConfigureCors(app, config);
            ConfigureSwagger(config);


            config.EnsureInitialized();

            app.UseWebApi(config);
            //app.UseWelcomePage("/");
        }

        private static void ConfigureCors(IAppBuilder app, HttpConfiguration config)
        {
            config.EnableCors(new EnableCorsAttribute("*", "*", "*"));
            app.UseCors(CorsOptions.AllowAll);
        }

        private static void ConfigureFormatters(HttpConfiguration config)
        {
            //TODO: enable this soon, this will require complete change on UI code

            //var defaultSettings = new JsonSerializerSettings
            //{
            //    Formatting = Formatting.Indented,
            //    ContractResolver = new CamelCasePropertyNamesContractResolver(),
            //    Converters = new List<JsonConverter>
            //    {
            //        new StringEnumConverter {CamelCaseText = true},
            //    }
            //};

            //JsonConvert.DefaultSettings = () => defaultSettings;

            config.Formatters.Clear();
            config.Formatters.Add(new JsonMediaTypeFormatter());
            
            //config.Formatters.JsonFormatter.SerializerSettings = defaultSettings;
        }

        private static void ConfigureSwagger(HttpConfiguration config)
        {
            // enable Swagger to provide documentation for GeoWebCore REST API
            config
                .EnableSwagger(c =>
                {
                    c.SingleApiVersion("v1", "GeoWebCore API");

                    // handle methods with similar signatures
                    c.ResolveConflictingActions(apiDescriptions => apiDescriptions.First());

                    // include XML comments in Swagger documentation
                    var baseDirectory = AppDomain.CurrentDomain.BaseDirectory;
                    var commentsFileName = Assembly.GetExecutingAssembly().GetName().Name + ".XML";
                    var commentsFile = Path.Combine(baseDirectory, commentsFileName);

                    c.IncludeXmlComments(commentsFile);

                    // add bearer authentication to methods that require it
                    c.OperationFilter<AddAuthorizationHeaderParameterOperationFilter>();
                })
                .EnableSwaggerUi();
        }
    }

    /// <summary>
    /// Adds an Authorize parameter to the Swagger UI for methods decorated with an Authorize attribute or derivative.
    /// </summary>
    public class AddAuthorizationHeaderParameterOperationFilter : IOperationFilter
    {
        /// <summary>
        /// Adds an Authorize parameter to the Swagger UI for methods decorated with an Authorize attribute or derivative.
        /// </summary>
        /// <param name="operation">The operation</param>
        /// <param name="schemaRegistry">The schema registry</param>
        /// <param name="apiDescription">The api description</param>
        public void Apply(Operation operation, SchemaRegistry schemaRegistry, ApiDescription apiDescription)
        {
            var filterPipeline = apiDescription.ActionDescriptor.GetFilterPipeline();
            var isAuthorized = filterPipeline
                                             .Select(filterInfo => filterInfo.Instance)
                                             .Any(filter => filter is IAuthorizationFilter);

            var allowAnonymous = apiDescription.ActionDescriptor.GetCustomAttributes<AllowAnonymousAttribute>().Any();

            if (isAuthorized && !allowAnonymous)
            {
                if (operation.parameters == null)
                {
                    operation.parameters = new List<Swashbuckle.Swagger.Parameter>();
                }

                operation.parameters.Add(new Swashbuckle.Swagger.Parameter
                {
                    name = "Authorization",
                    @in = "header",
                    description = "authorization token",
                    required = true,
                    type = "string"
                });
            }
        }
    }
}