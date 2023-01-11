using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Web;
using Newtonsoft.Json;

namespace PyxCrawler.Models
{
    public class OnlineGeospatialDatasetDb
    {
        private static string _fileName = System.Web.HttpRuntime.AppDomainAppPath + "\\datasets.json";
        private static object _lock = new object();

        public static void Save()
        {
            lock (_lock)
            {
                var json = JsonConvert.SerializeObject(_datasets);
                System.IO.File.WriteAllText(_fileName, json);
            }
        }

        public static void Load()
        {
            lock (_lock)
            {
                if (System.IO.File.Exists(_fileName))
                {
                    _datasets = JsonConvert.DeserializeObject<List<OnlineGeospatialDataSet>>(System.IO.File.ReadAllText(_fileName));                    
                }
            }
        }

        private static List<OnlineGeospatialDataSet> _datasets = new List<OnlineGeospatialDataSet>();

        static OnlineGeospatialDatasetDb()
        {
            try
            {
                Load();
            }
            catch
            {
            }
        }

        public static int Count
        {
            get
            {
                lock (_lock)
                {
                    return _datasets.Count;
                }
            }
        }

        public static List<OnlineGeospatialDataSet> Datasets
        {
            get
            {
                lock (_lock)
                {
                    return _datasets.ToList();
                }
            }
        }

        public static List<OnlineGeospatialDataSet> SearchByServer(int serverId, string searchTerm, int start, int limit)
        {
            lock (_lock)
            {
                var server = OnlineGeospatialEndpointsDb.GetById(serverId);

                var terms = (searchTerm??"").Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                if (terms.Length == 0)
                {
                    return _datasets.Where(x=>x.Server == server.Uri).Skip(start).Take(limit).ToList();
                }
                return _datasets.Where(x => x.Server == server.Uri).Where(x => x.Match(terms)).Skip(start).Take(limit).ToList();
            }
        }

        public static List<OnlineGeospatialDataSet> SearchByServer(int serverId, string searchTerm, string protocol, string version, OnlineGeospatialServiceStatus? status, int start, int limit)
        {
            lock (_lock)
            {
                var server = OnlineGeospatialEndpointsDb.GetById(serverId);

                var terms = (searchTerm ?? "").Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

                return _datasets.Where(x => x.Server == server.Uri)
                    .Where(d => terms.Length == 0 || d.Match(terms))
                    .Where(d => String.IsNullOrEmpty(protocol) || String.IsNullOrEmpty(version) || !status.HasValue || d.Services.Any(s => s.Protocol == protocol && s.Version == version && status.Value == status))
                    .Where(d => String.IsNullOrEmpty(protocol) || String.IsNullOrEmpty(version) || d.Services.Any(s => s.Protocol == protocol && s.Version == version))
                    .Where(d => String.IsNullOrEmpty(protocol) || !status.HasValue || d.Services.Any(s => s.Protocol == protocol && status.Value == status))
                    .Where(d => String.IsNullOrEmpty(version) || !status.HasValue || d.Services.Any(s => s.Version == version && status.Value == status))
                    .Where(d => (String.IsNullOrEmpty(protocol) || d.Services.Any(s => s.Protocol == protocol)))
                    .Where(d => String.IsNullOrEmpty(version) || d.Services.Any(s => s.Version == version))
                    .Where(d => !status.HasValue || d.Services.Any(s => s.Status == status.Value))
                    .Skip(start)
                    .Take(limit)
                    .ToList();
            }
        }

        public static List<OnlineGeospatialDataSet> Search(string searchTerm, int start, int limit)
        {
            lock (_lock)
            {
                var terms = (searchTerm??"").Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                if (terms.Length == 0)
                {
                    return _datasets.Skip(start).Take(limit).ToList();
                }
                return _datasets.Where(x=>x.Match(terms)).Skip(start).Take(limit).ToList();
            }
        }

        public static List<OnlineGeospatialDataSet> Search(string searchTerm, string protocol, string version, OnlineGeospatialServiceStatus? status, int start, int limit)
        {
            lock (_lock)
            {
                var terms = (searchTerm??"").Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

                return _datasets
                    .Where(d => terms.Length == 0 || d.Match(terms))
                    .Where(d => String.IsNullOrEmpty(protocol) || String.IsNullOrEmpty(version) || !status.HasValue || d.Services.Any(s => s.Protocol == protocol && s.Version == version && status.Value == status))
                    .Where(d => String.IsNullOrEmpty(protocol) || String.IsNullOrEmpty(version) || d.Services.Any(s => s.Protocol == protocol && s.Version == version))
                    .Where(d => String.IsNullOrEmpty(protocol) || !status.HasValue || d.Services.Any(s => s.Protocol == protocol && status.Value == status))
                    .Where(d => String.IsNullOrEmpty(version) || !status.HasValue || d.Services.Any(s => s.Version == version && status.Value == status))
                    .Where(d => (String.IsNullOrEmpty(protocol) || d.Services.Any(s => s.Protocol == protocol)))
                    .Where(d => String.IsNullOrEmpty(version) || d.Services.Any(s => s.Version == version))
                    .Where(d => !status.HasValue || d.Services.Any(s => s.Status == status.Value))
                    .Skip(start)
                    .Take(limit)
                    .ToList();
            }
        }

        public static List<ErrorSummary> SummarizeErrorsByServer(int serverId, string searchTerm, string protocol, string version, OnlineGeospatialServiceStatus? status)
        {
            lock (_lock)
            {
                var server = OnlineGeospatialEndpointsDb.GetById(serverId);

                var terms = (searchTerm ?? "").Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

                return _datasets
                    .Where(d => d.Server == server.Uri)
                    .Where(d => terms.Length == 0 || d.Match(terms))
                    .SelectMany(d => d.Services)
                    .Where(s => String.IsNullOrEmpty(protocol) || String.IsNullOrEmpty(version) || !status.HasValue || (s.Protocol == protocol && s.Version == version && status.Value == status))
                    .Where(s => String.IsNullOrEmpty(protocol) || String.IsNullOrEmpty(version) || (s.Protocol == protocol && s.Version == version))
                    .Where(s => String.IsNullOrEmpty(protocol) || !status.HasValue || (s.Protocol == protocol && status.Value == status))
                    .Where(s => String.IsNullOrEmpty(version) || !status.HasValue || (s.Version == version && status.Value == status))
                    .Where(s => (String.IsNullOrEmpty(protocol) || (s.Protocol == protocol)))
                    .Where(s => String.IsNullOrEmpty(version) || (s.Version == version))
                    .Where(s => !status.HasValue || (s.Status == status.Value))
                    .Where(s => s.Error != null)
                    .GroupBy(s => s.Error)
                    .Select(g => new ErrorSummary { Count = g.Count(), Error = g.Key })
                    .OrderByDescending(g => g.Count)
                    .Take(10)
                    .ToList();
            }
        }

        public static List<ErrorSummary> SummarizeErrors(string searchTerm, string protocol, string version, OnlineGeospatialServiceStatus? status)
        {
            lock (_lock)
            {
                var terms = (searchTerm ?? "").Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                return _datasets
                    .Where(d => terms.Length == 0 || d.Match(terms))
                    .SelectMany(d => d.Services)
                    .Where(s => String.IsNullOrEmpty(protocol) || String.IsNullOrEmpty(version) || !status.HasValue || (s.Protocol == protocol && s.Version == version && status.Value == status))
                    .Where(s => String.IsNullOrEmpty(protocol) || String.IsNullOrEmpty(version) || (s.Protocol == protocol && s.Version == version))
                    .Where(s => String.IsNullOrEmpty(protocol) || !status.HasValue || (s.Protocol == protocol && status.Value == status))
                    .Where(s => String.IsNullOrEmpty(version) || !status.HasValue || (s.Version == version && status.Value == status))
                    .Where(s => (String.IsNullOrEmpty(protocol) || (s.Protocol == protocol)))
                    .Where(s => String.IsNullOrEmpty(version) || (s.Version == version))
                    .Where(s => !status.HasValue || (s.Status == status.Value))
                    .Where(s => s.Error != null)
                    .GroupBy(s => s.Error)
                    .Select(g => new ErrorSummary { Count = g.Count(), Error = g.Key })
                    .OrderByDescending(g => g.Count)
                    .Take(10)
                    .ToList();
            }
        }

        public static OnlineGeospatialDataSet Find(Uri server, string datasetId)
        {
            lock (_lock)
            {
                return _datasets.FirstOrDefault(x => x.Server == server && x.DatasetId == datasetId);
            }
        }
       
        public static OnlineGeospatialDataSet GetOrCreate(Uri server, OnlineGeospatialService service, string datasetId)
        {
            lock(_lock)
            {
                var dataset = GetOrCreateNoModify(server, service, datasetId);

                dataset.GetService(service.Protocol, service.Version).Status = service.Status;

                return dataset;
            }
        }

        public static OnlineGeospatialDataSet GetOrCreateNoModify(Uri server, OnlineGeospatialService service, string datasetId)
        {
            lock (_lock)
            {
                var dataset = _datasets.FirstOrDefault(x => x.Server == server && x.DatasetId == datasetId);

                if (dataset == null)
                {
                    dataset = new OnlineGeospatialDataSet()
                    {
                        Id = 0,
                        Server = server,
                        DatasetId = datasetId,
                    };

                    if (_datasets.Count > 0)
                    {
                        dataset.Id = _datasets.Max(x => x.Id) + 1;
                    }

                    dataset.GetService(service.Protocol, service.Version).Status = service.Status;

                    _datasets.Add(dataset);
                }
                return dataset;
            }
        }

        public static List<OnlineGeospatialDataSet> Get(string uri)
        {
            lock (_lock)
            {
                return _datasets.Where(x=>x.Server.ToString() == uri).ToList();
            }
        }

        public static OnlineGeospatialDataSet Get(int id)
        {
            return _datasets.SingleOrDefault(x=>x.Id == id);
        }

        public static void Remove(OnlineGeospatialDataSet dataSet)
        {
            lock (_lock)
            {
                _datasets.Remove(dataSet);
            }
        }
    }
}