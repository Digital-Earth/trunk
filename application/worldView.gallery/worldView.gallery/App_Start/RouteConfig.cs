using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Web.Routing;

namespace worldView.gallery
{
    public class RouteConfig
    {
        public static void RegisterRoutes(RouteCollection routes)
        {
            routes.IgnoreRoute("{resource}.axd/{*pathInfo}");

            //Add sitemap.xml support
            //host/sitemap.xml - will return a sitemapindex to point to all galleries
            routes.MapRoute(
                name: "sitemap",
                url: "sitemap.xml",
                defaults: new { controller = "Sitemap", action = "Index" }
            );

            //host/static-sitemap.xml - point to download pages and alike
            routes.MapRoute(
                name: "static sitemap",
                url: "static-sitemap.xml",
                defaults: new { controller = "Sitemap", action = "StaticSitemap" }
            );

            //host/Gallery/id/sitemap.xml - return a sub sitemap to all resources in gallery
            routes.MapRoute(
                name: "gallery sitemap",
                url: "Gallery/{id}/sitemap.xml",
                defaults: new { controller = "Sitemap", action = "Gallery" }
            );

            routes.MapRoute(
                name: "requestNews",
                url: "requestNews",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "signUp",
                url: "signUp",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "forgotPassword",
                url: "forgotPassword",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "resetPassword",
                url: "resetPassword",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "updatedPassword",
                url: "updatedPassword",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "confirmEmail",
                url: "confirmEmail",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "GeoSource",
                url: "GeoSource/{id}",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "Globe",
                url: "Globe/{id}",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "Gallery",
                url: "Gallery/{id}",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "Browse",
                url: "browse",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "User",
                url: "User/{id}",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "info",
                url: "info/{id}",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "admin",
                url: "admin/{id}",
                defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "create",
                url: "create/{type}",
                defaults: new { controller = "Home", action = "Index", type = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "studioSkin",
                url: "studio/{skin}",
                defaults: new { controller = "Studio", action = "Index", skin = UrlParameter.Optional }
            );


            // single route for the demo

            /* Disable unless working on */
            routes.MapRoute(
                name: "Demo",
                url: "demo",
                defaults: new { controller = "Demo", action = "Index" }
            );

            // Routes for the website
            routes.MapRoute(
                name: "Site",
                url: "landing",
                defaults: new { controller = "Site", action = "Index" }
            );

            routes.MapRoute(
                name: "SiteFeatures",
                url: "features",
                defaults: new { controller = "Site", action = "Index" }
            );

            routes.MapRoute(
                name: "SiteNews",
                url: "news",
                defaults: new { controller = "Site", action = "Index" }
            );

            routes.MapRoute(
                name: "SiteNewsPost",
                url: "news/{year}/{post}",
                defaults: new { controller = "Site", action = "Index", year = UrlParameter.Optional, post = UrlParameter.Optional }
            );

            routes.MapRoute(
                name: "SitePrivacy",
                url: "privacy",
                defaults: new { controller = "Site", action = "Index" }
            );

            routes.MapRoute(
                name: "SiteTerms",
                url: "terms",
                defaults: new { controller = "Site", action = "Index" }
            );

            routes.MapRoute(
                name: "SiteContact",
                url: "contact",
                defaults: new { controller = "Site", action = "Index" }
            );

            routes.MapRoute(
                name: "NamedGallery",
                url: "{id}",
                defaults: new { controller = "Home", action = "Index" }
            );

            // Default catch all
            routes.MapRoute(
                 name: "Default",
                 url: "{controller}/{action}/{id}",
                 defaults: new { controller = "Home", action = "Index", id = UrlParameter.Optional }
             );
        }
    }
}
