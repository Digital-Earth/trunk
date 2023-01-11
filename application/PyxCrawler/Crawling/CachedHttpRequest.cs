using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Web;
using System.Text;
using System.Security.Cryptography;
using Newtonsoft.Json;

namespace PyxCrawler.Crawling
{
    public class CachedHttpRequest
    {
        public HttpRequestDetails RequestDetails { get; set; }

        public DateTime CachedDate { get; set; }
        public TimeSpan ResponseRoundTrip { get; set; }
        public HttpStatusCode ResponseCode { get; set; }
        public byte[] ResponseBody { get; set; }
        public string ResponseContentType { get; set; }
        public string ResponseEncoding { get; set; }

        public static CachedHttpRequest GetCachedRequest(HttpRequestDetails request)
        {
            var result = CachedHttpRequestDb.Find(request);

            if (result != null)
            {
                return result;
            }

            result = new CachedHttpRequest();
            result.RequestDetails = request;

            var start = DateTime.Now;

            try
            {
                
                var webRequest = request.CreateRequest();
                var response = (HttpWebResponse)webRequest.GetResponse();

                using(var memoryStream = new MemoryStream())
                {
                    response.GetResponseStream().CopyTo(memoryStream);
                    result.ResponseBody = memoryStream.ToArray();
                }

                result.ResponseContentType = response.ContentType;
                result.ResponseEncoding = response.ContentEncoding;

                result.ResponseCode = response.StatusCode;
            }
            catch (WebException webE)
            {
                if (webE.Status == WebExceptionStatus.ConnectFailure || webE.Status == WebExceptionStatus.NameResolutionFailure || webE.Status == WebExceptionStatus.Timeout)
                {
                    //we can't connect - mark it as nofound (we need better solution for that)
                    result.ResponseCode = HttpStatusCode.NotFound;
                }
                else
                {
                    var webResponse = webE.Response as HttpWebResponse;
                    result.ResponseCode = webResponse.StatusCode;

                    using (var memoryStream = new MemoryStream())
                    {
                        webResponse.GetResponseStream().CopyTo(memoryStream);
                        result.ResponseBody = memoryStream.ToArray();
                    }
                }
            }
            catch (Exception e)
            {
                throw e;
            }

            var end = DateTime.Now;

            result.CachedDate = end;
            result.ResponseRoundTrip = end - start;

            CachedHttpRequestDb.Save(result);

            return result;
        }

        public Stream GetResponseStream()
        {
            return new MemoryStream(ResponseBody);
        }
    }

    public static class CachedHttpRequestDb
    {
        private static string _cacheDir = System.Web.HttpRuntime.AppDomainAppPath + "\\req-cache";
        private static object _lock = new object();

        static CachedHttpRequestDb()
        {
            if (!System.IO.Directory.Exists(_cacheDir))
            {
                System.IO.Directory.CreateDirectory(_cacheDir);
            }
        }

        public static CachedHttpRequest Find(HttpRequestDetails request)
        {
            var fileName = _cacheDir + "\\" + request.CreateSHA256() + ".json";

            if (System.IO.File.Exists(fileName))
            {
                return JsonConvert.DeserializeObject<CachedHttpRequest>(File.ReadAllText(fileName));
            }

            return null;
        }

        public static void Save(CachedHttpRequest result)
        {
            var fileName = _cacheDir + "\\" + result.RequestDetails.CreateSHA256() + ".json";

            if (System.IO.File.Exists(fileName))
            {
                File.Delete(fileName);
            }

            File.WriteAllText(fileName + ".temp", JsonConvert.SerializeObject(result));
            File.Move(fileName + ".temp", fileName);
        }
    }


    public class HttpRequestDetails
    {
        public Uri Url { get; set; }
        public string Method { get; set; }
        public string RequestAccept { get; set; }
        public string RequestContentType { get; set; }
        public byte[] RequestBody { get; set; }

        public static HttpRequestDetails Get(Uri url)
        {
            return new HttpRequestDetails()
            {
                Url = url,
                Method = "GET",
                RequestAccept = "application/xml"
            };
        }

        public static HttpRequestDetails PostXml(Uri url,string body)
        {
            return new HttpRequestDetails()
            {
                Url = url,
                Method = "POST",
                RequestAccept = "application/xml",
                RequestContentType = "application/xml; charset=utf-8",
                RequestBody = Encoding.UTF8.GetBytes(body),
            };
        }

        public string CreateSHA256()
        {
            using (var stream = new MemoryStream())
            {
                WriteObject(stream, Method);
                WriteObject(stream, Url);

                WriteObject(stream, RequestAccept);
                if (RequestBody != null)
                {
                    WriteObject(stream, RequestContentType);
                    WriteObject(stream, RequestBody);
                }

                SHA256Managed hashstring = new SHA256Managed();
                byte[] hash = hashstring.ComputeHash(stream.GetBuffer());
                return Convert.ToBase64String(hash).Replace("+", "-").Replace("=", "_").Replace("/", ".");
            }
        }

        public override bool Equals(object obj)
        {
            if (object.ReferenceEquals(this, obj)) return true;
            if (obj is HttpRequestDetails)
            {
                var other = obj as HttpRequestDetails;

                return this.Url == other.Url &&
                    this.Method == other.Method &&
                    this.RequestAccept == other.RequestAccept &&
                    this.RequestContentType == other.RequestContentType &&
                    (this.RequestBody == null && other.RequestBody == null ||
                     this.RequestBody.SequenceEqual(other.RequestBody));
            }

            return false;
        }

        public override int GetHashCode()
        {
            var hash = Url.GetHashCode() ^ Method.GetHashCode() ^ RequestAccept.GetHashCode();

            if (RequestBody != null)
            {
                hash ^= RequestBody.Length.GetHashCode() ^ RequestContentType.GetHashCode();
            }

            return hash;
        }

        public static bool operator==(HttpRequestDetails a, HttpRequestDetails b)
        {
            if (Object.ReferenceEquals(a, b)) return true;
            if (a == null || b == null) return false;

            return a.Equals(b);
        }

        public static bool operator !=(HttpRequestDetails a, HttpRequestDetails b)
        {
            return !(a == b);
        }

        private void WriteObject(Stream stream, object obj)
        {
            WriteObject(stream,Encoding.UTF8.GetBytes(obj.ToString()));
        }

        private void WriteObject(Stream stream, byte[] buffer)
        {        
            stream.Write(buffer, 0, buffer.Length);
        }

        public HttpWebRequest CreateRequest()
        {
            var request = (HttpWebRequest)WebRequest.Create(Url);

            request.Method = Method;
            request.Accept = RequestAccept;            
            request.ServicePoint.Expect100Continue = false;
            if (RequestBody != null && RequestBody.Length > 0)
            {
                request.ContentType = RequestContentType;
                request.ContentLength = RequestBody.Length;
                
                var stream = request.GetRequestStream();
                stream.Write(RequestBody, 0, RequestBody.Length);
                stream.Close();
            }
            return request;
        }
    }
}