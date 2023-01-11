using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Newtonsoft.Json;

namespace PyxCrawler.Models
{
    public static class OnlineGeospatialEndpointsDb
    {
        private static string _fileName = System.Web.HttpRuntime.AppDomainAppPath + "\\endpoints.json";
        private static object _lock = new object();

        public static void Save()
        {
            lock (_lock)
            {                
                var json = JsonConvert.SerializeObject(_endpoints.Values.ToList());
                System.IO.File.WriteAllText(_fileName, json);
            }
        }

        public static void Load()
        {
            lock (_lock)
            {
                if (System.IO.File.Exists(_fileName))
                {
                    var endpoints = JsonConvert.DeserializeObject<List<OnlineGeospatialEndpoint>>(System.IO.File.ReadAllText(_fileName));

                    _endpoints.Clear();

                    foreach (var e in endpoints)
                    {
                        _endpoints[e.Uri.ToString()] = e;
                    }
                }
            }
        }

        private static Dictionary<string, OnlineGeospatialEndpoint> _endpoints = new Dictionary<string, OnlineGeospatialEndpoint>();

        static OnlineGeospatialEndpointsDb()
        {
            try
            {
                Load();                   
            }
            catch
            {
            }

            if (_endpoints.Count == 0)            
            {                
                foreach(var cswNameUri in new Dictionary<string,string>
                            {
                                {"GeoDab","http://184.73.174.89/gi-cat-StP/services/cswiso"},
                                {"Geoss","http://geossregistries.info/geonetwork/srv/en/csw"},
                                {"ESRI Geoss","http://geoss.esri.com/geoportal/csw/discovery"}
                            })
                {
                    var csw = Add(new Uri(cswNameUri.Value));
                    csw.Name = cswNameUri.Key;
                    csw.GetService("CSW","2.0.2");
                }
            }
        }

        public static List<OnlineGeospatialEndpoint> Servers
        {
            get
            {
                lock (_lock)
                {
                    return _endpoints.Values.ToList();
                }
            }
        }

        public static int Count
        {
            get
            {
                lock (_lock)
                {
                    return _endpoints.Count;
                }
            }
        }

        public static OnlineGeospatialEndpoint Add(Uri uri)
        {
            lock (_lock)
            {
                if (!_endpoints.ContainsKey(uri.ToString()))
                {
                    var endpoint = new OnlineGeospatialEndpoint(uri) { Id = 0};
                    if (_endpoints.Count > 0)
                    {
                        endpoint.Id = _endpoints.Max(x => x.Value.Id) + 1;
                    }
                    _endpoints[uri.ToString()] = endpoint;
                }
                return _endpoints[uri.ToString()];
            }
        }

        public static OnlineGeospatialEndpoint Get(string uri)
        {
            lock (_lock)
            {
                if (_endpoints.ContainsKey(uri))
                {
                    return _endpoints[uri];
                }
                return Add(new Uri(uri));
            }
        }

        public static OnlineGeospatialEndpoint Get(Uri uri)
        {
            return Get(uri.ToString());
        }

        public static void Remove(string uri)
        {
            lock (_lock)
            {
                _endpoints.Remove(uri);
            }
        }

        public static OnlineGeospatialEndpoint GetById(int id)
        {
            lock (_lock)
            {
                return _endpoints.Values.FirstOrDefault(x => x.Id == id);
            }
        }
    }
}