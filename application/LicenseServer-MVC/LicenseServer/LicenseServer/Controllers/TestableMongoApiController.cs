using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Sockets;
using System.Web.Http;
using System.Web.Http.Hosting;
using System.Web.Http.Routing;
using LicenseServer.Models.Mongo;
using MongoDB.Driver;
using Polly;

namespace LicenseServer.Controllers
{
    public class TestableMongoApiController : ApiController
    {
        protected IMongoDBEntities db;

        private static readonly PolicyBuilder s_policyBuilder = Policy
            .Handle<IOException>() // timeout, inner SocketException
            .Or<SocketException>() // timeout
            .Or<MongoCommandException>() // occurs when current primary node is unelected to secondary during command
            .Or<MongoConnectionException>() // occurs when db is inaccessible
            .Or<AggregateException>(); // occurs when updating an identity (UserManager wraps exception in AggregateException)

        private static Action<Exception, TimeSpan> OnRetry = (exception, retrySleep) =>
        {
            var log = new Exception("Retrying at " + DateTime.UtcNow + " sleep: " + retrySleep, exception);
            Elmah.ErrorSignal.FromCurrentContext().Raise(log);
        };

        private static readonly Policy s_retryPolicy = s_policyBuilder
            .WaitAndRetry(3, retryAttempt => TimeSpan.FromSeconds(Math.Pow(2, retryAttempt)), OnRetry);

        private static Policy s_retryAsyncPolicy = s_policyBuilder
            .WaitAndRetryAsync(3, retryAttempt => TimeSpan.FromSeconds(Math.Pow(2, retryAttempt)), OnRetry);

        protected Policy Retry
        {
            get { return s_retryPolicy; }
        }

        protected Policy RetryAsync
        {
            get { return s_retryAsyncPolicy; }
        }

        public TestableMongoApiController()
        {
            db = new MongoDBEntities();
        }

        public TestableMongoApiController(TestMongoSetup setup)
        {
            db = setup.Context;
            
            HttpRequestMessage request;
            if (setup.Method != null)
            {
                if (setup.BaseUri != null)
                {
                    request = new HttpRequestMessage(setup.Method, setup.BaseUri);
                }
                else
                {
                    request = new HttpRequestMessage(setup.Method, "http://localhost");
                }
            }
            else
            {
                // Controllers can't create responses with a null request (which is the case for unit testing)
                request = new HttpRequestMessage();
            }

            if (setup.Configuration != null)
            {
                if (setup.RouteName != null && setup.RouteTemplate != null)
                {
                    var route = setup.Configuration.Routes.MapHttpRoute(name: setup.RouteName, 
                        routeTemplate: setup.RouteTemplate,
                        defaults: new { id = RouteParameter.Optional });
                    var routeData = new HttpRouteData(route);
                    request.Properties[HttpPropertyKeys.HttpRouteDataKey] = routeData;
                }
                request.Properties[HttpPropertyKeys.HttpConfigurationKey] = setup.Configuration;
            }
            else
            {
                request.Properties[HttpPropertyKeys.HttpConfigurationKey] = new HttpConfiguration();
            }
            Request = request;
        }

        protected override void Dispose(bool disposing)
        {
            if (db is IDisposable)
            {
                ((IDisposable)db).Dispose();
            }
            base.Dispose(disposing);
        }
    }

    // For unit testing, to configure a TestableMongoApiController with its non-default contructor
    // The Context must be initialized to an instance of a db interface
    // All other properties may be null
    // When not specified the default HTTP GET method is inferred
    // both RouteName and RouteTemplate must be specified in order to take effect
    public class TestMongoSetup
    {
        public IMongoDBEntities Context { get; set; }
        public HttpMethod Method { get; set; }
        public string BaseUri { get; set; }
        public HttpConfiguration Configuration { get; set; }
        public string RouteName { get; set; }
        public string RouteTemplate { get; set; }
        public PyxisIdentityUser CurrentUserIdentity { get; set; }
    }
}
