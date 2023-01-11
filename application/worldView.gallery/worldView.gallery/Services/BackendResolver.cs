using System;
using System.Linq;
using System.Web.Mvc;

namespace worldView.gallery.Services
{
    public static class BackendResolver
    {
        public static readonly string ProductionBackend = "https://api.pyxis.worldview.gallery/api/v1";
        public static readonly string TestBackend = "https://api.test.pyxis.worldview.gallery/api/v1";
        public static readonly string DevelopmentBackend = "https://api.dev.pyxis.worldview.gallery/api/v1";

        public static readonly string[] ProductionFrontEnds =
        {
            "https://worldview.gallery",
            "https://www.worldview.gallery",
            "https://staging.worldview.gallery",
            "http://worldview.gallery",
            "http://www.worldview.gallery",
            "http://staging.worldview.gallery"
        };

        public static readonly string[] TestFrontEnds =
        {
            "http://test.worldview.gallery",
            "https://test.worldview.gallery"
        };

        public static readonly string[] DevelopmentFrontEnds =
        {
            "http://dev.worldview.gallery",
            "https://dev.worldview.gallery"
        };

        /// <summary>
        /// Determine the license server url to use for the given run mode. Defaults to the
        /// development license server url.
        /// </summary>
        /// <param name="mode">One of "live", "production", "test" or "dev" or null.</param>
        /// <returns></returns>
        public static string ResolveBackendUrl(string mode)
        {
            switch (mode)
            {
                case "live":
                case "production":
                    return BackendResolver.ProductionBackend;

                case "test":
                    return BackendResolver.TestBackend;

                case "dev":
                    return BackendResolver.DevelopmentBackend;

                default:
                    // in future, should get this from a config file
                    return BackendResolver.DevelopmentBackend;
            }
        }

        /// <summary>
        /// Get LS REST API Url based on the request.host (production/test LS)
        /// </summary>
        /// <param name="controller">The controller.</param>
        /// <returns>Api url as string (including the api/version at the end)</returns>
        public static string GetApiUrlFromRequest(this Controller controller)
        {
            if (IsTestEnvironment(controller))
            {
                return TestBackend;
            }

            if (IsDevelopmentEnvironment(controller))
            {
                return DevelopmentBackend;
            }

            return ProductionBackend;
        }
        
        /// <summary>
        /// Check if the requesting host is acting as a test environment.
        /// </summary>
        /// <param name="controller">The controller.</param>
        /// <returns>True if test environment.</returns>
        private static bool IsTestEnvironment(this Controller controller)
        {
            return TestFrontEnds.Contains(controller.GetHostFromRequest().ToLower());
        }

        /// <summary>
        /// Check if the requesting host is acting as a development environment.
        /// </summary>
        /// <param name="controller">The controller.</param>
        /// <returns>True if development environment.</returns>
        private static bool IsDevelopmentEnvironment(this Controller controller)
        {
            return DevelopmentFrontEnds.Contains(controller.GetHostFromRequest().ToLower());
        }

        /// <summary>
        /// Get the host from the request.Url. will look like https://worldview.gallery or http://localhost:12345
        /// </summary>
        /// <param name="controller">The controller.</param>
        /// <returns>the server url in the current http request</returns>
        public static string GetHostFromRequest(this Controller controller)
        {
            return controller.Request.Url != null ? controller.Request.Url.GetLeftPart(UriPartial.Authority) : ProductionBackend;
        }
    }
}