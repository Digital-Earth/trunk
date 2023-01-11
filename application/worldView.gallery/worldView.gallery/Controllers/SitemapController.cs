using System;
using System.Net;
using System.Web.Mvc;
using Newtonsoft.Json;
using worldView.gallery.Services;

namespace worldView.gallery.Controllers
{
    public class SitemapController : Controller
    {        
        /// <summary>
        /// Get sitemap index that points to all galleries sitemaps.
        /// </summary>
        /// <returns></returns>
        public string Index()
        {
            var webClient = new WebClient();
            
            // get Id and Metadata.Name fields of all non-empty galleries:
            // Url parts:
            // 1) /Gallery? - get all galleries
            // 2) $expand=Resources,Metadata - make sure we get Resources and Metadata from the gallery object
            // 3) $select=Id,Metadata/Name - select only Id and Metadata
            // 4) $filter=Resources/any(q:%20q/ResourceId%20ne%20guid%2700000000-0000-0000-0000-000000000000%27) - check if gallery is not empty
            //    This filter return true if there is a resource with Guid != Guid.Empty. which is true for every resources.
            //    Therefore, this filter will return true if there is a single resource inside the gallery.
            var url = string.Format("{0}/Gallery?" +
                                    "$expand=Resources,Metadata&" +
                                    "$select=Id,Metadata/Name&" +
                                    "$filter=Resources/any(q:%20q/ResourceId%20ne%20guid%2700000000-0000-0000-0000-000000000000%27)",
                                    this.GetApiUrlFromRequest());

            dynamic json = JsonConvert.DeserializeObject(webClient.DownloadString(url));

            var sitemapIndex = new SitemapIndexFile();

            //add general pages sitemap.xml
            sitemapIndex.AddSitemap(String.Format("{0}/static-sitemap.xml", this.GetHostFromRequest()));

            //add first chunk of galleries
            AddGalleriesToSitemap(json, sitemapIndex);

            //continue fetch more sitemap.xml requests
            while (json.NextPageLink != null)
            {
                json = JsonConvert.DeserializeObject(webClient.DownloadString(json.NextPageLink));
                AddGalleriesToSitemap(json, sitemapIndex);
            }

            return sitemapIndex.ToString();
        }

        private void AddGalleriesToSitemap(dynamic json, SitemapIndexFile sitemapIndex)
        {
            var baseAddress = this.GetHostFromRequest();

            foreach (var item in json.Items)
            {
                sitemapIndex.AddSitemap(String.Format("{0}/Gallery/{1}/sitemap.xml", baseAddress, item.Id));
            }
        }

        /// <summary>
        /// Return a sitemap for our static pages
        /// </summary>
        /// <returns></returns>
        public string StaticSitemap()
        {
            var baseAddress = this.GetHostFromRequest();

            var sitemap = new SitemapFile();

            //main page is important update frequently
            sitemap.AddUrl(baseAddress, SitemapEntryRefreshRate.Daily, 1.0);
            
            var pages = new[]
            {
                "/download",
                "/signUp",
                "/info/vision",
                "/info/education-vision"
            };

            foreach (var page in pages)
            {
                sitemap.AddUrl(baseAddress + page, SitemapEntryRefreshRate.Weekly, 0.8);     
            }            

            return sitemap.ToString();
        }

        /// <summary>
        /// get urls for a gallery
        /// </summary>
        /// <param name="id">Gallery Id</param>
        /// <returns></returns>
        public string Gallery(string id)
        {
            var baseAddress = this.GetHostFromRequest();

            var webClient = new WebClient();
            var url = string.Format("{0}/Gallery/{1}/Expanded", this.GetApiUrlFromRequest(), id);

            dynamic galleryJson = JsonConvert.DeserializeObject(webClient.DownloadString(url));

            var sitemap = new SitemapFile();
            
            //add gallery page
            sitemap.AddUrl(
                String.Format("{0}/{1}", baseAddress, galleryJson.Gallery.Metadata.Name),
                SitemapEntryRefreshRate.Daily, 0.8);

            //add gallery resource pages
            foreach (var item in galleryJson.Resources)
            {
                sitemap.AddUrl(
                    String.Format("{0}/{1}/{2}", baseAddress, item.Type, item.Id),
                    SitemapEntryRefreshRate.Daily, 0.5);
            }

            return sitemap.ToString();
        }
    }
}