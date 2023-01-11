using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Web.Mvc;
using worldView.gallery.Services;

namespace worldView.gallery.Controllers
{
    public class SiteController : Controller
    {
        public ActionResult Index(string backend)
        {
            // Determine the license server url to use. For local requests look at the "backend"
            // parameter, otherwise determine the license server url from the request
            ViewBag.BackendUrl = Request.IsLocal ? BackendResolver.ResolveBackendUrl(backend) : this.GetApiUrlFromRequest();

            return View();
        }
    }
}