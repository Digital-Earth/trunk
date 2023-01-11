using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Web.Optimization;
using System.Web.Routing;


namespace worldView.gallery
{
    public class MvcApplication : System.Web.HttpApplication
    {
        protected void Application_Start()
        {
            AreaRegistration.RegisterAllAreas();
            RouteConfig.RegisterRoutes(RouteTable.Routes);
            BundleConfig.RegisterBundles(BundleTable.Bundles);

            //BundleTable.EnableOptimizations = true;
        }

        protected void Application_BeginRequest()
        {
            if (!Context.Request.IsSecureConnection)
            {
                if (Context.Request.IsLocal)
                {
                    return;
                }

                //This should be removed or changed to return error when WV is updated.
                if (Context.Request.RequestType == "POST")
                {
                    return;
                }

                Response.Redirect(Context.Request.Url.ToString().Replace("http:", "https:"));
            }
        }
    }
}
