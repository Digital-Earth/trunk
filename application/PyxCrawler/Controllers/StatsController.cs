using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Web.Http;
using PyxCrawler.Models;

namespace PyxCrawler.Controllers
{
    public class StatsController : ApiController
    {
        public List<ItemCount<string>> Get()
        {
            return Get(150);
        }

        public List<ItemCount<string>> Get(int id)
        {
            var skipWords = new HashSet<string>() {
                "of","the","from","by","the","a","-","are","as","with","0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","were","was","and","an","it","in","for","is","to","at","on","that","each","all","which","these","this","should","no","yes","if","thus","such","than","#1","be","or","one","two","three","not","http",
                "i","ii","can","using","s","other","over","row","have",":","based","used","map","maps","minimum","maximum","atlas","mean","total","per","set","rates","updated","derived","gis","mapping","units","average","layer","band","bands",
                "count","www","short","format","dataset","like","between","min","max","final","column","through","every","part","more","has","just","type","has","across","written",
            };
            return OnlineGeospatialDatasetDb.Datasets.
                SelectMany(x => (x.Description ?? "").Split(new char[] { ' ', '.', '_', ',',':','(',')','\'','/','"','\n','\t'}, StringSplitOptions.RemoveEmptyEntries)
                             .Concat(x.Name.Split(new char[] { ' ', '.', '_', ',', ':', '(', ')'}, StringSplitOptions.RemoveEmptyEntries))
                             .Distinct())
                .Select(x=>x.ToLower())
                .Where(x=>!skipWords.Contains(x))
                .Where(x => x.Length>1)
                .GroupBy(x => x)
                .Select(x => new ItemCount<string> { Value = x.Key, Count = x.Count() })
                .OrderByDescending(x=>x.Count)
                .Take(id)
                .ToList();
        }

    }

    public class CswStatsController : ApiController
    {
        public List<ItemCount<string>> Get(int id)
        {
            var endPoint = OnlineGeospatialEndpointsDb.GetById(id);

            return OnlineGeospatialEndpointsDb.Servers
                .Where(x => x.Tags != null && x.Tags.Contains(endPoint.Name))
                .Select(x => FindBestStatus(x))
                .GroupBy(x => x)
                .Select(x => new ItemCount<string>() { Value = x.Key.ToString(), Count = x.Count() })
                .ToList();
        }

        private OnlineGeospatialServiceStatus FindBestStatus(OnlineGeospatialEndpoint endpoint)
        {
            if (endpoint.Services.Any(x => x.Status == OnlineGeospatialServiceStatus.Working))
            {
                return OnlineGeospatialServiceStatus.Working;
            }
            else if (endpoint.Services.Any(x => x.Status == OnlineGeospatialServiceStatus.Broken))
            {
                return OnlineGeospatialServiceStatus.Broken;
            }
            else if (endpoint.Services.Any(x => x.Status == OnlineGeospatialServiceStatus.Offline))
            {
                return OnlineGeospatialServiceStatus.Offline;
            }
            return OnlineGeospatialServiceStatus.Unknown;
        }
    }

    public class ItemCount<T>
    {
        public T Value { get; set; }
        public int Count { get; set; }
    }
}
