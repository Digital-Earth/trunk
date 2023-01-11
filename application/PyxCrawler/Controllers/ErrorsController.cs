using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Http;
using PyxCrawler.Models;

namespace PyxCrawler.Controllers
{
    public class ErrorsController : ApiController
    {
        public IEnumerable<ErrorSummary> Get(string query)
        {
            return OnlineGeospatialDatasetDb.SummarizeErrors(query, null, null, null);
        }

        public IEnumerable<ErrorSummary> Get(string query, string protocol, string version)
        {
            return OnlineGeospatialDatasetDb.SummarizeErrors(query, protocol, version, null);
        }

        public IEnumerable<ErrorSummary> Get(string query, string protocol, string version, OnlineGeospatialServiceStatus status)
        {
            return OnlineGeospatialDatasetDb.SummarizeErrors(query, protocol, version, status);
        }

        public IEnumerable<ErrorSummary> Get(string query, int serverId)
        {
            return OnlineGeospatialDatasetDb.SummarizeErrorsByServer(serverId, query, null, null, null);
        }

        public IEnumerable<ErrorSummary> Get(string query, string protocol, string version, int serverId)
        {
            return OnlineGeospatialDatasetDb.SummarizeErrorsByServer(serverId, query, protocol, version, null);
        }

        public IEnumerable<ErrorSummary> Get(string query, string protocol, string version, OnlineGeospatialServiceStatus status, int serverId)
        {
            return OnlineGeospatialDatasetDb.SummarizeErrorsByServer(serverId, query, protocol, version, status);
        }
    }
}
