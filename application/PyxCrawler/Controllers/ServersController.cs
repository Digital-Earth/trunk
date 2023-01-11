using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using System.Web.Http;
using PyxCrawler.Import;
using PyxCrawler.Models;
using PyxCrawler.Publishing;

namespace PyxCrawler.Controllers
{
    public class ServersController : ApiController
    {
        public List<OnlineGeospatialEndpoint> Get()
        {
            return OnlineGeospatialEndpointsDb.Servers;
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(int id)
        {
            return OnlineGeospatialDatasetDb.Get(OnlineGeospatialEndpointsDb.GetById(id).Uri.ToString());
        }

        public async Task<OnlineGeospatialEndpoint> Put()
        {
            var uri = new Uri(await this.Request.Content.ReadAsStringAsync());

            return OnlineGeospatialEndpointsDb.Add(uri);
        }

        public HttpResponseMessage Post(int id)
        {
            var endpoint = OnlineGeospatialEndpointsDb.GetById(id);

            try
            {
                GallerySynchronizer.UpsertGallery(endpoint);
            }
            catch (InvalidOperationException exception)
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, exception.Message);
            }

            return Request.CreateResponse(HttpStatusCode.OK);
        }

        public HttpResponseMessage Post()
        {
            try
            {
                GallerySynchronizer.UpsertCatalog();
            }
            catch (InvalidOperationException exception)
            {
                return Request.CreateErrorResponse(HttpStatusCode.BadRequest, exception.Message);
            }

            return Request.CreateResponse(HttpStatusCode.OK);
        }
        
        public void Delete(string uri)
        {
            OnlineGeospatialEndpointsDb.Remove(uri);
        }
    }
}
