using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Web;
using PyxCrawler.Crawling;

namespace PyxCrawler.Models
{
    static class CrawlingTaskDb
    {

        private static List<CrawlingTask> _tasks = new List<CrawlingTask>();
        private static object _tasksLock = new object();

        public static List<CrawlingTask> Tasks
        {
            get
            {
                lock (_tasksLock)
                {
                    return _tasks.ToList();
                }
            }
        }

        public static void StartTask(Uri uri)
        {
            var fixedUri = new UriBuilder(uri);
            fixedUri.Query = "";
            var strippedUri = fixedUri.Uri.ToString().TrimEnd('/');
            fixedUri = new UriBuilder(strippedUri);
            var task = new CrawlingTask() { Uri = fixedUri.Uri, Service = "Detecting", Progress = 0, Status = "Detecting Server" };

            Task.Factory.StartNew(() => DoTask(task));
        }

        private static void DoTask(CrawlingTask task)
        {
            try
            {
                lock (_tasksLock)
                {
                    _tasks.Add(task);
                }

                var endPoint = OnlineGeospatialEndpointsDb.Get(task.Uri);
                
                var protocols = new List<string> { "WMS", "WCS", "WFS", "CSW", "AGS" };

                foreach (var service in protocols)
                {
                    ICrawler crawler = null;
                    switch (service)
                    {
                        case "CSW":
                            crawler = new CswCrawler();
                            break;

                        case "WMS":
                            crawler = new WmsCrawler();
                            break;

                        case "WFS":
                            crawler = new WfsCrawler();
                            break;

                        case "WCS":
                            crawler = new WcsCrawler();
                            break;

                        case "AGS":
                            crawler = new AgsCrawler();
                            break;

                        default:
                            continue;
                    }

                    crawler.Crawl(task, endPoint);
                }

                //clean empty version if we found a specific version
                foreach(var protocol in endPoint.Services.Select(x => x.Protocol).Distinct().ToList())
                {
                    var noVersion = endPoint.Services.Find(x => x.Protocol == protocol && String.IsNullOrEmpty(x.Version));
                    var withVersion = endPoint.Services.Where(x => x.Protocol == protocol && !String.IsNullOrEmpty(x.Version)).ToList();

                    if (noVersion != null & withVersion.Count > 0)
                    {
                        endPoint.Services.Remove(noVersion);
                    }
                }
            }
            finally
            {
                lock (_tasksLock)
                {
                    _tasks.Remove(task);
                }
            }
            OnlineGeospatialEndpointsDb.Save();
            OnlineGeospatialDatasetDb.Save();
        }
    }
}