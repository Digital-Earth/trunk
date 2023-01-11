using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Web;
using System.Web.Http;
using PyxCrawler.Models;

namespace PyxCrawler.Controllers
{
    public class RequestsController : ApiController
    {
        public IEnumerable<OnlineGeospatialRequest> Get()
        {
            return OnlineGeospatialRequestsDb.Requests;
        }

        public IEnumerable<OnlineGeospatialRequest> Get(OnlineGeospatialRequestState state)
        {
            return OnlineGeospatialRequestsDb.Requests.Where(r => r.State == state);
        }

        public OnlineGeospatialRequest Get(int id)
        {
            return OnlineGeospatialRequestsDb.GetById(id);
        }

        public OnlineGeospatialRequest Get(Uri uri)
        {
            return OnlineGeospatialRequestsDb.Get(uri);
        }

        public HttpResponseMessage Post(List<RequestDto> requests)
        {
            foreach (var request in requests)
            {
                try
                {
                    var uri = new Uri(request.Uri);
                    OnlineGeospatialRequestsDb.Add(uri, request.Count);
                }
                catch (UriFormatException e)
                {
                    // ignore invalid uri
                }
            }
            OnlineGeospatialRequestsDb.Save();

            return Request.CreateResponse(HttpStatusCode.OK);
        }
        
        public HttpResponseMessage Put(StateChangeDTO request)
        {
            var updatedRequest = OnlineGeospatialRequestsDb.Update(request.Id, request.State);
            if (updatedRequest.State == OnlineGeospatialRequestState.Crawled)
            {
                var requestUrl = HttpContext.Current.Request.Url.ToString();
                var crawlServiceUrl = requestUrl.Substring(0, requestUrl.IndexOf("/Requests", StringComparison.CurrentCultureIgnoreCase)) + "/Crawl";
                using (var client = new HttpClient())
                {
                    var requestContent = new StringContent(updatedRequest.Uri.ToString());
                    var crawlServiceResponse = client.PostAsync(crawlServiceUrl, requestContent).Result;
                    var responseString = crawlServiceResponse.Content.ReadAsStringAsync().Result;
                    var responseCode = crawlServiceResponse.StatusCode;
                    if (responseCode != HttpStatusCode.OK)
                    {
                        throw new HttpException((int)responseCode, "Failed to add to crawler");
                    }
                }
            }
            OnlineGeospatialRequestsDb.Save();

            return Request.CreateResponse(HttpStatusCode.OK);
        }
        
        public void Delete(string uri)
        {
            OnlineGeospatialRequestsDb.Remove(uri);
            OnlineGeospatialRequestsDb.Save();
        }

        public class RequestDto
        {
            public string Uri { get; set; }
            public int Count { get; set; }
        }

        public class StateChangeDTO
        {
            public int Id { get; set; }
            public OnlineGeospatialRequestState State { get; set; }
        }
    }
}