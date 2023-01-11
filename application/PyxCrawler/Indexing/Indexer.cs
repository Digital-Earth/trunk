using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Elasticsearch.Net;
using Nest;
using PyxCrawler.Models;

namespace PyxCrawler.Indexing
{
    /// <summary>
    /// Responsible for updating the index of crawled data sets.
    /// </summary>
    public static class Indexer
    {
        private static readonly ElasticClient s_client;
        private static readonly int s_batchSize = 100;

        static Indexer()
        {
            string[] connections = new string[Properties.Settings.Default.ElasticSearchUrls.Count];
            Properties.Settings.Default.ElasticSearchUrls.CopyTo(connections, 0);
            var connectionPool = new StaticConnectionPool(connections.Select(c => new Uri(c)));
            var settings = new ConnectionSettings(connectionPool)
                // .DisableDirectStreaming() // uncomment to view raw queries in responses
                .DefaultIndex(Properties.Settings.Default.ExternalDataIndex)
                .MapDefaultTypeIndices(d => d.Add(typeof(OnlineGeospatialIndexModel), Properties.Settings.Default.ExternalDataIndex))
                .MapDefaultTypeNames(d => d.Add(typeof (OnlineGeospatialIndexModel), "Ogc"));
            s_client = new ElasticClient(settings);
        }
        
        public static bool Index(List<OnlineGeospatialDataSet> datasets)
        {
            var models = new List<OnlineGeospatialIndexModel>(s_batchSize);
            var i = 0;
            for (; i < datasets.Count; i++)
            {
                var dataset = datasets[i];
                models.Add(OnlineGeospatialIndexModelFactory.Create(dataset));
                if ((i + 1)%s_batchSize == 0)
                {
                    IndexBatch(models);
                    models.Clear();
                }
            }
            if (i + 1%s_batchSize != 0)
            {
                IndexBatch(models);
            }
            return true;
        }

        private static void IndexBatch(IEnumerable<OnlineGeospatialIndexModel> batch)
        {
            var result = s_client.Bulk(b => b.IndexMany(batch));
            if (result.Errors)
            {
                throw new Exception(result.Errors.ToString());
            }
        }
    }
}