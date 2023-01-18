using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Net;
using System.Text;
using System.Xml;
using ApplicationUtility;
using Newtonsoft.Json;

namespace Pyxis.IO.Sources.Remote
{
    /// <summary>
    /// Provides utility methods for making JSON and XML requests from a server. The methods support
    /// validation with user credentials if available.
    /// </summary>
    class WebRequestHelper
    {
        /// <summary>
        /// Retrieve a JSON object from a web service with network authorization.
        /// </summary>
        /// <param name="url">The web service</param>
        /// <param name="credentials">The credentials</param>
        /// <returns>The JSON object if successful, otherwise null</returns>
        public static T GetJsonWithAuthorization<T>(string url, NetworkCredential credentials, int retryCount = 2) where T: class
        {
            var requestCount = 0;
            T obj = null;
            while (obj == null) // good old-fashioned infinite loop if json deserializing fails
            {
                try
                {
                    requestCount++;
                    obj = GetJson<T>(url);
                }
                catch (WebException ex)
                {
                    var response = ex.Response as HttpWebResponse;

                    if (response == null ||
                        response.StatusCode != HttpStatusCode.Unauthorized)
                    {
                        throw;
                    }

                    if (!AskUserForNetworkCredentialsForRequest(url, credentials))
                    {
                        throw;
                    }
                }

                if (requestCount >= retryCount)
                {
                    throw new Exception("Failed to parse json as " + typeof(T).Name + " after " + retryCount + " attempts.");
                }
            }

            return obj;
        }

        /// <summary>
        /// Retrieve a JSON object from a web service.
        /// </summary>
        /// <param name="url">The url</param>
        /// <param name="accept">accept header</param>
        /// <returns>The JSON object if successful, otherwise null</returns>
        public static T GetJson<T>(String url, string accept = null) where T: class
        {
            var request = (HttpWebRequest) WebRequest.Create(url);
            request.Accept = accept ?? "application/json; charset=utf-8";
/*            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12 | SecurityProtocolType.Tls |
                                                   SecurityProtocolType.Tls11 | SecurityProtocolType.Ssl3;*/

            try
            {
                request.Credentials = FindNetworkCredentialsForRequest(url);
            }
            catch (Exception)
            {
                // credentials not found, continue
            }

            try
            {
                using (var response = (HttpWebResponse) request.GetResponse())
                {
                    if (response.StatusCode == HttpStatusCode.OK)
                    {
                        var rs = response.GetResponseStream();
                        if (rs != null)
                        {
                            using (var sr = new StreamReader(rs))
                            {
                                return JsonConvert.DeserializeObject<T>(sr.ReadToEnd());
                            }
                        }
                    }
                }
            }
            catch (WebException)
            {
                //rethrow web exceptions
                throw;
            }
            catch (Exception e)
            {
                Trace.error("A request to the server failed: " + e.Message);
            }

            return null;
        }

        public static XmlDocument GetXmlDocumentWithAuthorization(string url, NetworkCredential credentials, string body = null)
        {
            XmlDocument doc = null;
            while (doc == null)
            {
                try
                {
                    doc = GetXmlDocument(BuildRequest(url,body,credentials));
                }
                catch (WebException ex)
                {
                    var response = ex.Response as HttpWebResponse;

                    if (response == null ||
                        response.StatusCode != HttpStatusCode.Unauthorized)
                    {
                        throw;
                    }
                    if (!AskUserForNetworkCredentialsForRequest(url, credentials))
                    {
                        throw;
                    }
                }
            }
            return doc;
        }

        public static HttpWebRequest BuildRequest(string url, string body = null, NetworkCredential credentials = null)
        {
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);

/*            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12 | SecurityProtocolType.Tls |
                                                   SecurityProtocolType.Tls11 | SecurityProtocolType.Ssl3;*/

            try
            {
                request.Credentials = credentials ?? FindNetworkCredentialsForRequest(url);
            }
            catch (Exception /*e*/)
            {
                // Credentials not found, continue
            }

            if (body.HasContent())
            {
                request.ContentType = "application/xml";
                request.Method = "POST";
                var bodyUtf8 = Encoding.UTF8.GetBytes(body);
                request.GetRequestStream().Write(bodyUtf8,0,bodyUtf8.Length);
            }

            return request;
        }

        public static XmlDocument GetXmlDocument(HttpWebRequest request)
        {
            try
            {
                var response = request.GetResponse();

                Stream responseStream;
                if (response.Headers[HttpResponseHeader.ContentEncoding] != null &&
                    response.Headers[HttpResponseHeader.ContentEncoding].ToLower() == "gzip")
                {
                    responseStream = new GZipStream(response.GetResponseStream(), CompressionMode.Decompress);
                }
                else
                {
                    responseStream = response.GetResponseStream();
                }

                var reader = new XmlTextReader(responseStream);
                reader.DtdProcessing = DtdProcessing.Ignore;
                
                XmlDocument result = new XmlDocument();
                result.Load(reader);

                return result;
            }
            catch (Exception e)
            {
                Trace.error("A request to the server failed: " + e.Message);
                return null;
            }
        }

        public static XmlDocument GetXmlDocument(string url, string body = null)
        {
            return GetXmlDocument(BuildRequest(url,body));
        }

        protected static bool AskUserForNetworkCredentialsForRequest(string url, NetworkCredential credentials)
        {
            if (credentials == null)
            {
                return false;
            }

            var process = PYXCOMFactory.CreateProcess(
                PYXCOMFactory.WellKnownProcesses.UserCredentialsProvider
                );
            var cache = pyxlib.QueryInterface_IUserCredentialsProvider(process.getOutput());
            var uri = new Uri(url);
            var target = uri.Host;

            //store pyxlib credentials into user cache
            cache.confirmCredentials(target, UsernameAndPasswordCredentials.create(credentials.UserName, credentials.Password), true);

            //store the C# credentials into the cache
            lock (s_credentialsCache)
            {
                s_credentialsCache[target] = credentials;
            }

            return true;
        }

        protected static ICredentials FindNetworkCredentialsForRequest(string url)
        {
            var uri = new Uri(url);
            var target = uri.Host;

            lock (s_credentialsCache)
            {
                if (s_credentialsCache.ContainsKey(target))
                {
                    return s_credentialsCache[target];
                }
            }
            var process = PYXCOMFactory.CreateProcess(
                    PYXCOMFactory.WellKnownProcesses.UserCredentialsProvider
                    );
            var cache = pyxlib.QueryInterface_IUserCredentialsProvider(process.getOutput());
            var credentialList = cache.getCredentials(target, IUsernameAndPasswordCredentials.iid);

            if (credentialList.isNotNull() && credentialList.getCredentialsCount() > 0)
            {
                var credential = credentialList.getCredentials(0);

                if (credential.getCredentialsType() == IUsernameAndPasswordCredentials.iid)
                {
                    var usernameAndPassword = pyxlib.QueryInterface_IUsernameAndPasswordCredentials(credential);

                    if (usernameAndPassword != null && usernameAndPassword.isNotNull())
                    {
                        var networkCredential = new NetworkCredential(
                            usernameAndPassword.getUsername(),
                            usernameAndPassword.getPassword()
                            );

                        lock (s_credentialsCache)
                        {
                            s_credentialsCache[target] = networkCredential;
                            return networkCredential;
                        }
                    }
                }
            }
            return null;
        }

        private static readonly Dictionary<string, ICredentials> s_credentialsCache = new Dictionary<string, ICredentials>();
    }
}
