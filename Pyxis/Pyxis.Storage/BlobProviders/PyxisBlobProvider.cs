using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Web;
using Newtonsoft.Json;
using Pyxis.Storage.Properties;

namespace Pyxis.Storage.BlobProviders
{
    public class PyxisBlobProvider : AbstractBlobProvider
    {
        
        /// <summary>
        /// KnownMissingKeysCache is a helper class to reduce requests over the network.
        /// 
        /// The class keeps a small set of keys that are known not to exist on
        /// the remote server
        /// </summary>
        private class KnownMissingKeysCache
        {
            /// <summary>
            /// Default cache size to use.
            ///
            /// Under the assumption each part is 1MB -> we have a cache for 1GB of data.
            /// </summary>
            public const int DefaultCacheSize = 1000;

            /// <summary>
            /// Maximum number of keys in cache
            /// </summary>
            private readonly int m_maxSize;

            /// <summary>
            /// Locking object to be thread safe
            /// </summary>
            private readonly object m_knownMissingKeysLock = new object();

            /// <summary>
            /// Lookup table for keys
            /// </summary>
            private readonly HashSet<string> m_knownMissingKeys = new HashSet<string>();

            /// <summary>
            /// Order of keys reported to the cache (Least recently created)
            /// </summary>
            private readonly Queue<string> m_knownMissingKeysOrder = new Queue<string>();

            /// <summary>
            /// Create KnownMissingKeysCache
            /// </summary>
            /// <param name="maxSize"></param>
            public KnownMissingKeysCache(int maxSize)
            {
                m_maxSize = maxSize;
            }

            /// <summary>
            /// Add list of missing keys into cache
            /// </summary>
            /// <param name="keys">IEnumerable<string> of missing keys</param>
            public void AddMissingKeys(IEnumerable<string> keys)
            {
                lock (m_knownMissingKeysLock)
                {
                    foreach (var key in keys)
                    {
                        if (!m_knownMissingKeys.Contains(key))
                        {
                            m_knownMissingKeys.Add(key);
                            m_knownMissingKeysOrder.Enqueue(key);
                        }
                    }

                    while (m_knownMissingKeysOrder.Count > m_maxSize)
                    {
                        var oldKey = m_knownMissingKeysOrder.Dequeue();
                        m_knownMissingKeys.Remove(oldKey);
                    }
                }
            }

            /// <summary>
            /// Return true if we know the given key is missing.             
            /// </summary>
            /// <param name="key">key to check.</param>
            /// <returns>true if key is missing. false means we don't know the status of the key.</returns>
            public bool IsMissing(string key)
            {
                lock (m_knownMissingKeysLock)
                {
                    return m_knownMissingKeys.Contains(key);
                }
            }

            /// <summary>
            /// Report that a key exists.
            /// </summary>
            /// <param name="key">key that exists.</param>
            public void ReportKeyExists(string key)
            {
                lock (m_knownMissingKeysLock)
                {
                    m_knownMissingKeys.Remove(key);                    
                }
            }
        }

        private readonly URLBuilder m_urlBuilder;

        private readonly KnownMissingKeysCache m_knownMissingKeysCache = new KnownMissingKeysCache(KnownMissingKeysCache.DefaultCacheSize);

        public PyxisBlobProvider()
        {
            m_urlBuilder = new URLBuilder(Settings.Default.StorageServerURL);
        }

        public PyxisBlobProvider(string serverURL)
        {
            m_urlBuilder = new URLBuilder(serverURL);
        }

        public override bool GetBlob(string key, Stream data)
        {
            var webClient = new WebClient();

            try
            {
                webClient.Headers[HttpRequestHeader.Accept] = "*/*";
                var result = webClient.DownloadData(m_urlBuilder.PostBlobURL(key));
                data.Write(result, 0, result.Length);
                return true;
            }
            catch (Exception e)
            {
                var webException = e as WebException;
                if (webException != null)
                {
                    var response = (HttpWebResponse) webException.Response;

                    if (response != null && response.StatusCode == HttpStatusCode.NotFound)
                    {
                        Debug.WriteLine("Blob not found : " + key);
                        return false;
                    }
                }
                Trace.TraceInformation("An exception happened trying to get this blob: " + key);
                Trace.TraceInformation(e.Message + "\n" + e.StackTrace);
                return false;
            }
        }

        public override IDictionary<string, string> GetBlobs(IEnumerable<string> keys)
        {
            using (var client = new HttpClient())
            {
                var request = new HttpRequestMessage(HttpMethod.Post, m_urlBuilder.GetMultiBlobs());
                request.Content = new StringContent(JsonConvert.SerializeObject(keys));
                var response = client.SendAsync(request).Result;

                try
                {
                    response.EnsureSuccessStatusCode();
                    var responseContent = (response.Content as StreamContent).ReadAsStringAsync().Result;
                    return JsonConvert.DeserializeObject<Dictionary<string, string>>(responseContent);
                }
                catch (Exception e)
                {
                    TraceHttpError("GetBlobs", e, request, response);
                    throw new Exception("Failed to get blobs.", e);
                }
            }
        }

        public override bool AddBlob(string blobKey, Stream blob)
        {
            if (BlobExists(blobKey))
            {
                return false;
            }

            using (var client = new HttpClient())
            {
                var request = new HttpRequestMessage(HttpMethod.Post, m_urlBuilder.PostBlobURL(blobKey));
                request.Content = new StreamContent(blob);
                var response = client.SendAsync(request).Result;

                try
                {
                    response.EnsureSuccessStatusCode();
                    
                    //we know this blob exists now
                    m_knownMissingKeysCache.ReportKeyExists(blobKey);

                    return true;
                }
                catch (Exception e)
                {
                    TraceHttpError("AddBlob", e, request, response);
                    throw new Exception("Failed to upload blob.", e);
                }
            }
        }

        public override IEnumerable<string> MissingBlobs(IEnumerable<string> keys)
        {
            var allKeys = keys.ToList();

            //if no keys are requested, none are missing
            if (allKeys.Count == 0)
            {
                return new string[] {};
            }

            using (var client = new HttpClient())
            {
                client.DefaultRequestHeaders.Accept.Clear();
                client.DefaultRequestHeaders.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));

                var request = new HttpRequestMessage(HttpMethod.Post, m_urlBuilder.MissingBlobs());
                request.Content = new StringContent(JsonConvert.SerializeObject(allKeys));
                var response = client.SendAsync(request).Result;

                try
                {
                    response.EnsureSuccessStatusCode();
                    var json = response.Content.ReadAsStringAsync().Result;
                    var result = JsonConvert.DeserializeObject<string[]>(json);

                    //store information about missing keys
                    m_knownMissingKeysCache.AddMissingKeys(result);

                    return result;
                }
                catch (Exception e)
                {
                    TraceHttpError("MisingBlobs", e, request, response);
                    throw new Exception("Failed to check if the blob exists", e);
                }
            }
        }

        public override bool BlobExists(string key)
        {
            //check if we already know the given key is missing
            if (m_knownMissingKeysCache.IsMissing(key))
            {
                return false;
            }

            return !MissingBlobs(new string[] { key }).Any();
        }

        public override bool RemoveBlob(string key)
        {
            throw new NotImplementedException();
        }

        private void TraceHttpError(string methodName, Exception e, HttpRequestMessage request, HttpResponseMessage response)
        {
            Trace.TraceError(methodName + ": " + e.Message);
            Trace.TraceError("Exception Type: " + e.GetType().FullName);
            Trace.TraceError("HTTP Request: " + request);
            Trace.TraceError("HTTP Response Code: " + response.StatusCode);
            Trace.TraceError("HTTP Response: " + response);
        }

        private class URLBuilder
        {
            private string m_baseURL;

            public URLBuilder(string baseURL)
            {
                m_baseURL = baseURL + "/api/v1/storage/blobs/";
            }

            public string PostBlobURL(string key)
            {
                var codedKey = HttpUtility.UrlEncode(key);
                codedKey = HttpUtility.UrlEncode(codedKey);
                return m_baseURL + "blob/" + codedKey;
            }

            public string GetBlobURL(string key)
            {
                var codedKey = HttpUtility.UrlEncode(key);
                codedKey = HttpUtility.UrlEncode(codedKey);
                return m_baseURL + "blob/" + codedKey;
            }

            public string MissingBlobs()
            {
                return m_baseURL + "missingblobs";
            }

            public string GetMultiBlobs()
            {
                return m_baseURL + "multiblobs";
            }
        }
    }
}