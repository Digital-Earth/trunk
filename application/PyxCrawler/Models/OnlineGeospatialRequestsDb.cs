using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Newtonsoft.Json;

namespace PyxCrawler.Models
{
    public static class OnlineGeospatialRequestsDb
    {
        private static string _fileName = System.Web.HttpRuntime.AppDomainAppPath + "\\requests.json";
        private static object _lock = new object();

        public static void Save()
        {
            lock (_lock)
            {
                var json = JsonConvert.SerializeObject(m_requests.Values.ToList());
                System.IO.File.WriteAllText(_fileName, json);
            }
        }

        public static void Load()
        {
            lock (_lock)
            {
                if (System.IO.File.Exists(_fileName))
                {
                    var requests = JsonConvert.DeserializeObject<List<OnlineGeospatialRequest>>(System.IO.File.ReadAllText(_fileName));

                    m_requests.Clear();

                    foreach (var request in requests)
                    {
                        m_requests[request.Uri.ToString()] = request;
                    }
                }
            }
        }

        private static Dictionary<string, OnlineGeospatialRequest> m_requests = new Dictionary<string, OnlineGeospatialRequest>();

        static OnlineGeospatialRequestsDb()
        {
            try
            {
                Load();
            }
            catch
            {
            }
        }

        public static List<OnlineGeospatialRequest> Requests
        {
            get
            {
                lock (_lock)
                {
                    return m_requests.Values.ToList();
                }
            }
        }

        public static int Count
        {
            get
            {
                lock (_lock)
                {
                    return m_requests.Count;
                }
            }
        }

        public static OnlineGeospatialRequest Add(Uri uri, int count = 1)
        {
            lock (_lock)
            {
                try
                {
                    uri = new Uri(HttpUtility.UrlDecode(uri.ToString()));
                }
                catch (Exception)
                {
                    // failed to parse Uri
                    return null;
                }
                var uriLeftPart = new Uri(uri.GetLeftPart(UriPartial.Path).ToLower());
                if (!m_requests.ContainsKey(uriLeftPart.ToString()))
                {
                    var protocol = new List<string>();
                    var service = HttpUtility.ParseQueryString(uri.Query.ToLower())["service"];
                    if (!string.IsNullOrEmpty(service))
                    {
                        protocol.Add(service);
                    }
                    var request = new OnlineGeospatialRequest
                    {
                        Id = 0,
                        Uri = uriLeftPart,
                        Count = count,
                        State = OnlineGeospatialRequestState.Pending,
                        Protocols = protocol
                    };
                    if (m_requests.Count > 0)
                    {
                        request.Id = m_requests.Max(x => x.Value.Id) + 1;
                    }
                    m_requests[uriLeftPart.ToString()] = request;
                }
                else
                {
                    m_requests[uriLeftPart.ToString()].Count += count;
                }
                return m_requests[uriLeftPart.ToString()];
            }
        }

        public static OnlineGeospatialRequest Get(string uri)
        {
            lock (_lock)
            {
                if (m_requests.ContainsKey(uri))
                {
                    return m_requests[uri];
                }
                return Add(new Uri(uri));
            }
        }

        public static OnlineGeospatialRequest Get(Uri uri)
        {
            return Get(uri.ToString());
        }

        public static OnlineGeospatialRequest Update(int id, OnlineGeospatialRequestState state)
        {
            lock (_lock)
            {
                var request = GetById(id);
                request.State = state;
                return request;
            }
        }

        public static void Remove(string uri)
        {
            lock (_lock)
            {
                m_requests.Remove(uri);
            }
        }

        public static OnlineGeospatialRequest GetById(int id)
        {
            lock (_lock)
            {
                return m_requests.Values.FirstOrDefault(x => x.Id == id);
            }
        }
    }
}