using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Web.Mvc;
using System.Net.Http;
using System.Web.Http;
using PyxCrawler.App_Start;
using PyxCrawler.Import;
using PyxCrawler.Indexing;
using PyxCrawler.Models;

namespace PyxCrawler.Controllers
{
    public class DatasetsController : ApiController
    {
        public OnlineGeospatialDataSet Get(int id)
        {
            return OnlineGeospatialDatasetDb.Get(id);
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(string query,int start,int limit)
        {
            return OnlineGeospatialDatasetDb.Search(query, start, limit);
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(string query, OnlineGeospatialServiceStatus status, int start, int limit)
        {
            return OnlineGeospatialDatasetDb.Search(query, null, null, status, start, limit);
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(string query, string protocol, int start, int limit)
        {
            return OnlineGeospatialDatasetDb.Search(query, protocol, null, null, start, limit);
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(string query, string protocol, string version, int start, int limit)
        {
            return OnlineGeospatialDatasetDb.Search(query, protocol, version, null, start, limit);
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(string query, string protocol, string version, OnlineGeospatialServiceStatus status, int start, int limit)
        {
            return OnlineGeospatialDatasetDb.Search(query, protocol, version, status, start, limit);
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(string query, int start, int limit, int serverId)
        {
            return OnlineGeospatialDatasetDb.SearchByServer(serverId, query, start, limit);
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(string query, string protocol, string version, int start, int limit, int serverId)
        {
            return OnlineGeospatialDatasetDb.SearchByServer(serverId, query, protocol, version, null, start, limit);
        }

        public IEnumerable<OnlineGeospatialDataSet> Get(string query, string protocol, string version, OnlineGeospatialServiceStatus status, int start, int limit, int serverId)
        {
            return OnlineGeospatialDatasetDb.SearchByServer(serverId, query, protocol, version, status, start, limit);
        }

        public HttpResponseMessage Post(OperationDTO operation)
        {
            var succeeded = false;
            if (operation.Operation == "Index")
            {
                succeeded = Indexer.Index(OnlineGeospatialDatasetDb.Datasets);
            } 
            else if (operation.Operation == "Verify")
            {
                succeeded = DataSetVerifier.Start(EngineConfig.Engine);
            }
            return Request.CreateResponse(succeeded ? HttpStatusCode.OK : HttpStatusCode.InternalServerError, succeeded);
        }

        public HttpResponseMessage Put(ChangeOrder changeOrder)
        {
            if (changeOrder.Id == null || changeOrder.Service == null)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Id and Service must be specified");
            }
            try
            {
                var dataSet = OnlineGeospatialDatasetDb.Get(changeOrder.Id.Value);
                var service = dataSet.Services.First(s => s.Protocol == changeOrder.Service.Protocol && s.Version == changeOrder.Service.Version);
                if (service.Status == changeOrder.Service.Status)
                {
                    return Request.CreateResponse(HttpStatusCode.NotModified);
                }
                service.Status = changeOrder.Service.Status;
                OnlineGeospatialDatasetDb.Save();
            }
            catch (Exception e)
            {
                return Request.CreateResponse(HttpStatusCode.BadRequest, "Specified data set with Id and Service doesn't exist: " + e.Message);
            }
            return Request.CreateResponse(HttpStatusCode.OK);
        }

        public class OperationDTO
        {
            public string Operation { get; set; }
        }

        public class ChangeOrder
        {
            public int? Id { get; set; }
            public OnlineGeospatialService Service { get; set; }
        }
    }
}
