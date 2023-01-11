using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

namespace GeoWebCore.Services.Cluster
{
    /// <summary>
    /// HashRing is used to balance load requested based on string key.
    /// 
    /// Usage: 
    /// <code lang="cs">
    /// var ring = new HashRing("search"); //created ring with default settings using name "search"
    /// //get the active endpoint that server hello world. this will use ClustEndpoints to get active nodes
    /// var endpoint = ring.GetEndpoint("hello world");
    /// </code>
    ///   
    /// </summary>
    public class HashRing
    {
        public int StopPointsPerNode { get; private set; }
        
        public int Count {
            get { return m_endpoints.Count; } 
        }

        public IEnumerable<string> Endpoints
        {
            get
            {
                return m_endpoints;
            }
        }

        private List<string> m_endpoints;

        private int[] m_stopPointsHash;
        private string[] m_stopPointsEndpoint;
        private MD5CryptoServiceProvider m_hasher;

        public HashRing(int stopPointsPerNode = 64)
        {
            m_hasher = new System.Security.Cryptography.MD5CryptoServiceProvider();
            StopPointsPerNode = stopPointsPerNode;
            m_endpoints = new List<string>();
        }

        public void SetEndpoints(List<string> endpoints)
        {
            m_endpoints = endpoints;
            CalculateStopPoints();
        }

        private void CalculateStopPoints()
        {
            var stopPoints = new SortedList<int, string>();
            foreach (var endpoint in m_endpoints)
            {
                var lastHash = 0;
                for (var point = 0; point < StopPointsPerNode; point++)
                {
                    var key = endpoint + "(" + point + "," + lastHash + ")";
                    var hash = Hash(key);
                    stopPoints.Add(hash, endpoint);
                    lastHash = hash;
                }
            }
            m_stopPointsHash = stopPoints.Keys.ToArray();
            m_stopPointsEndpoint = stopPoints.Values.ToArray();
        }

        public string GetEndpoint(string key)
        {
            return GetEndpoint(Hash(key));
        }

        public string GetEndpoint(int hash)
        {
            //return null for empty endpoint
            if (m_endpoints.Count == 0) return null;

            var index = Array.BinarySearch(m_stopPointsHash, hash);
            if (index >= 0)
            {
                return m_stopPointsEndpoint[index];
            }
            index = ~index;
            if (index == m_stopPointsEndpoint.Length)
            {
                index = 0;
            }
            return m_stopPointsEndpoint[index];
        }

        public IEnumerable<KeyValuePair<int,string>> GetStopPoints()
        {
            return m_stopPointsHash.Select((key, i) => new KeyValuePair<int, string>(key, m_stopPointsEndpoint[i]));
        }

        public int Hash(string key)
        {
            var bytes = m_hasher.ComputeHash(Encoding.UTF8.GetBytes(key));
            return (bytes[12] << 24) + (bytes[13] << 16) + (bytes[14] << 8) + bytes[15];
        }

        public bool ContainsEndpoint(string url)
        {
            return this.m_endpoints.Contains(url);
        }
    }
}