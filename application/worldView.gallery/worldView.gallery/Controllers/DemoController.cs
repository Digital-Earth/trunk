using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Web.Mvc;
using worldView.gallery.Services;

namespace worldView.gallery.Controllers
{
    public class DemoController : Controller
    {
        public ActionResult Index(string backend, string dev)
        {
            // Determine the license server url to use. For local requests look at the "backend"
            // parameter, otherwise determine the license server url from the request
            ViewBag.BackendUrl = Request.IsLocal ? BackendResolver.ResolveBackendUrl(backend) : this.GetApiUrlFromRequest();

            //notify view to load end-to-end testing framework
            if (dev != null && dev.Split(',').Any(param => param == "test"))
            {
                ViewBag.Funcunit = true;
            }

            return View();
        }
    }
}