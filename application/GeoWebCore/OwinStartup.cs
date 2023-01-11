using System.Web.Http;
using Microsoft.Owin.Cors;
using Microsoft.Owin.Host.HttpListener;
using Owin;
using System.Web.Http.Cors;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Web.Http.Description;
using System.Web.Http.Filters;
using Swashbuckle.Application;
using Swashbuckle.Swagger;

namespace GeoWebCore
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
#if DEBUG
            app.UseErrorPage();
#endif

            HttpConfiguration config = new HttpConfiguration();

            //register our default error message handler
            config.Filters.Add(new WebConfig.ApiExceptionFilterAttribute());
                        
            config.MapHttpAttributeRoutes();
            config.Formatters.Remove(config.Formatters.XmlFormatter);
            
            config.EnableCors(new EnableCorsAttribute("*", "*", "*"));          
            app.UseCors(CorsOptions.AllowAll);

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

                    // add multipart file upload to methods that require it
                    c.OperationFilter<AddFileUploadParamsFilter>();
                })
                .EnableSwaggerUi();


            app.UseWebApi(config);
            app.UseWelcomePage("/");
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

    /// <summary>
    /// Adds a Choose File button to the Swagger UI for CreateFiles requests.
    /// </summary>
    public class AddFileUploadParamsFilter : IOperationFilter
    {
        /// <summary>
        /// Adds a Choose File button to the Swagger UI for multipart CreateFiles requests.
        /// </summary>
        /// <param name="operation">The operation</param>
        /// <param name="schemaRegistry">The schema registry</param>
        /// <param name="apiDescription">The api description</param>
        public void Apply(Operation operation, SchemaRegistry schemaRegistry, ApiDescription apiDescription)
        {
            if (operation.operationId == "Gallery_CreateFiles")
            {
                operation.consumes.Add("application/form-data");

                if (operation.parameters == null)
                {
                    operation.parameters = new List<Swashbuckle.Swagger.Parameter>();
                }

                operation.parameters.Add(new Swashbuckle.Swagger.Parameter
                {
                    name = "file",
                    @in = "formData",
                    description = "file to be uploaded",
                    required = true,
                    type = "file"
                });
            }
        }
    }
}