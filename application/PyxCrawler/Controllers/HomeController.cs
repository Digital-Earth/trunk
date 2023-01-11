using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using PyxCrawler.Crawling;
using PyxCrawler.Models;

namespace PyxCrawler.Controllers
{
    public class HomeController : Controller
    {
        public ActionResult Index()
        {
            ViewBag.DatasetCount = OnlineGeospatialDatasetDb.Count;
            ViewBag.EndpointsCount = OnlineGeospatialEndpointsDb.Count;


            ViewBag.WMSDatasets = OnlineGeospatialDatasetDb.Datasets.Count(x => x.Services.Any(y => y.Protocol == "WMS"));
            ViewBag.WFSDatasets = OnlineGeospatialDatasetDb.Datasets.Count(x => x.Services.Any(y => y.Protocol == "WFS"));
            ViewBag.WCSDatasets = OnlineGeospatialDatasetDb.Datasets.Count(x => x.Services.Any(y => y.Protocol == "WCS"));
            ViewBag.AGSMDatasets = OnlineGeospatialDatasetDb.Datasets.Count(x => x.Services.Any(y => y.Protocol == "AGSM"));
            ViewBag.AGSFDatasets = OnlineGeospatialDatasetDb.Datasets.Count(x => x.Services.Any(y => y.Protocol == "AGSF"));


            ViewBag.WMSEndpoints = OnlineGeospatialEndpointsDb.Servers.Count(x => x.Services.Any(y => y.Protocol == "WMS"));
            ViewBag.WFSEndpoints = OnlineGeospatialEndpointsDb.Servers.Count(x => x.Services.Any(y => y.Protocol == "WFS"));
            ViewBag.WCSEndpoints = OnlineGeospatialEndpointsDb.Servers.Count(x => x.Services.Any(y => y.Protocol == "WCS"));
            ViewBag.CSWEndpoints = OnlineGeospatialEndpointsDb.Servers.Count(x => x.Services.Any(y => y.Protocol == "CSW"));
            ViewBag.AGSEndpoints = OnlineGeospatialEndpointsDb.Servers.Count(x => x.Services.Any(y => y.Protocol == "AGS"));
            ViewBag.AGSMEndpoints = OnlineGeospatialEndpointsDb.Servers.Count(x => x.Services.Any(y => y.Protocol == "AGSM"));
            ViewBag.AGSFEndpoints = OnlineGeospatialEndpointsDb.Servers.Count(x => x.Services.Any(y => y.Protocol == "AGSF"));
            return View();
        }

        public ActionResult Datasets(string q,int s = -1)
        {
            ViewBag.DatasetCount = OnlineGeospatialDatasetDb.Count;
            ViewBag.Query = q ?? "";
            OnlineGeospatialEndpoint endpoint = null;
            if (s >= 0)
            {
                endpoint = OnlineGeospatialEndpointsDb.GetById(s);
                ViewBag.GalleryOperation = endpoint.Provider == null ? "Create" : "Update";
            }
            ViewBag.Server = endpoint;
            ViewBag.CanMakeCatalogGallery = s == 0
                                            && endpoint.Services.FirstOrDefault(se => se.Protocol == "CSW") != null;
            ViewBag.CanMakeEndpointGallery = s >= 0 
                                             && !ViewBag.CanMakeCatalogGallery;
            return View();
        }

        public ActionResult Endpoints()
        {            
            return View();
        }

        public ActionResult About()
        {
            return View();
        }

        public ActionResult Contact()
        {
            return View();
        }

        public ActionResult Requests()
        {
            return View();
        } 
    }
}
