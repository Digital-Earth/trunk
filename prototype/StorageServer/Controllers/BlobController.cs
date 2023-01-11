using Newtonsoft.Json;
using System;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Web;
using System.Web.Http;
using Microsoft.Practices.Unity;
using Pyxis.Storage;
using System.Threading.Tasks;
using System.Net;

namespace StorageServer.Controllers
{
    [RoutePrefix("api/v1/storage/blobs")]
    public class BlobController : ApiController
    {

        [Dependency]
        public IBlobProvider BlobProvider { get; set; }

        [HttpGet()]
        [Route("blob/{key}")]
        public async Task<HttpResponseMessage> GetBlob([FromUri]string key)
        {
            HttpResponseMessage response;
            try
            {
                key = HttpUtility.UrlDecode(key);
                var stream = new MemoryStream();
                var found = await Task.Factory.StartNew<bool>(() => BlobProvider.GetBlob(key, stream));
                if (found)
                {
                    response = Request.CreateResponse(System.Net.HttpStatusCode.OK);
                    stream.Position = 0;
                    response.Content = new StreamContent(stream);
                    return response;
                }
                else
                {
                    return Request.CreateErrorResponse(System.Net.HttpStatusCode.NotFound, "Blob not found");
                }
            }
            catch (Exception e)
            {
                response = Request.CreateErrorResponse(System.Net.HttpStatusCode.InternalServerError, e);
            }
            return response;
        }

        [HttpPost()]
        [Route("blob/{key}")]
        public async Task<HttpResponseMessage> PostBlob([FromUri]string key)
        {
            HttpResponseMessage response;
            try
            {
                key = HttpUtility.UrlDecode(key);
                var stream = await Request.Content.ReadAsStreamAsync();
                var result = await Task.Factory.StartNew<bool>(() => BlobProvider.AddBlob(key, stream));
                response = Request.CreateResponse((result) ? HttpStatusCode.Created : HttpStatusCode.NotModified);
            }
            catch
            {
                response = Request.CreateResponse(HttpStatusCode.InternalServerError);
            }
            return response;
        }

        [HttpPost()]
        [Route("missingblobs")]
        public async Task<HttpResponseMessage> GetMissingBlobs()
        {
            HttpResponseMessage response;
            try
            {
                var keysStr = await Request.Content.ReadAsStringAsync();
                var keys = JsonConvert.DeserializeObject<string[]>(keysStr);
                var missingKeys = keys.Where(x => !BlobProvider.BlobExists(x)).ToArray();
                response = Request.CreateResponse(System.Net.HttpStatusCode.OK);
                response.Content = new StringContent(JsonConvert.SerializeObject(missingKeys));
            }
            catch (Exception e)
            {
                response = Request.CreateErrorResponse(System.Net.HttpStatusCode.InternalServerError, e);
            }
            return response;
        }
        [HttpPost()]
        [Route("multiBlobs")]
        public async Task<HttpResponseMessage> GetMultiBlobs()
        {
            HttpResponseMessage response;
            try
            {
                var keysStr = await Request.Content.ReadAsStringAsync();
                var keys = JsonConvert.DeserializeObject<string[]>(keysStr);
                var blobs = BlobProvider.GetBlobs(keys);
                response = Request.CreateResponse(System.Net.HttpStatusCode.OK);
                response.Content = new StringContent(JsonConvert.SerializeObject(blobs));
            }
            catch (Exception e)
            {
                response = Request.CreateErrorResponse(System.Net.HttpStatusCode.InternalServerError, e);
            }
            return response;
        }

    }
}