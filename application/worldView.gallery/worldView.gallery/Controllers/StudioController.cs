using System;
using System.Linq;
using System.Web.Mvc;
using worldView.gallery.Services;

namespace worldView.gallery.Controllers
{
    public class StudioController : Controller
    {
        public ActionResult Index(string skin, string backend, string dev)
        {
            // Determine the license server url to use. For local requests look at the "backend"
            // parameter, otherwise determine the license server url from the request
            ViewBag.BackendUrl = Request.IsLocal ? BackendResolver.ResolveBackendUrl(backend) : this.GetApiUrlFromRequest();

            //notify view to load end-to-end testing framework
            if (dev != null && dev.Split(',').Any(param => param == "test"))
            {
                ViewBag.Funcunit = true;
            }

            if (!String.IsNullOrEmpty(skin))
            {
                switch (skin.Trim().ToLower())
                {
                    case "beta-v1":
                        return View("Index");
                    case "web":
                        return View("Web");                    
                    case "calgary":
                        return View("Index.Calgary");
                }
            }            

            return View("Index");
        }
    }
}