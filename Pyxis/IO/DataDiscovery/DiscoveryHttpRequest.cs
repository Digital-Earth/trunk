using System;
using System.IO;
using System.Net;
using System.Threading.Tasks;
using Pyxis.Contract;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.DataDiscovery
{
    public class DiscoveryHttpRequest : IDiscoveryNetworkRequest
    {
        public string Uri { get; set; }

        public IPermit Permit { get; set; }

        public async Task<IDiscoveryNetworkResult> SendAsync()
        {
            var request = (HttpWebRequest) WebRequest.Create(Uri);

            request.Accept = "*/*";
            request.UserAgent = "Pyxis Crawller";
/*            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12 | SecurityProtocolType.Tls |
                                                   SecurityProtocolType.Tls11 | SecurityProtocolType.Ssl3;*/

            var networkPermit = Permit as INetworkPermit;

            if (networkPermit != null)
            {
                request.Credentials = networkPermit.Credentials;
            }

            request.Method = Method;

            if (Headers != null)
            {
                foreach (string header in Headers.Keys)
                {
                    request.Headers[header] = Headers[header];
                }
            }

            DiscoveryHttpResult result;
            using (var response = await GetResponse(request))
            {
                result = await DiscoveryHttpResult.Create(this, response);
            }

            return result;
        }

        private Task<HttpWebResponse> GetResponse(WebRequest request)
        {
            return Task.Factory.StartNew(() =>
            {
                var t = Task.Factory.FromAsync<WebResponse>(
                    request.BeginGetResponse,
                    request.EndGetResponse,
                    null);

                if (Timeout.HasValue)
                {
                    if (!t.Wait(Timeout.Value))
                    {
                        request.Abort();
                        throw new TimeoutException();
                    }
                }
                
                return t.Result as HttpWebResponse;
            });
        }

        public string Method { get; set; }

        public WebHeaderCollection Headers { get; set; }

        public TimeSpan? Timeout { get; set; }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }
            var other = obj as DiscoveryHttpRequest;
            if (other != null)
            {
                return Equals(other);
            }
            return false;
        }

        protected bool Equals(DiscoveryHttpRequest other)
        {
            return string.Equals(Uri, other.Uri) && Equals(Permit, other.Permit) && string.Equals(Method, other.Method) && ReferenceEquals(Headers,other.Headers) && Equals(Headers.ToString(), other.Headers.ToString());
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = (Uri != null ? Uri.GetHashCode() : 0);
                hashCode = (hashCode*397) ^ (Permit != null ? Permit.GetHashCode() : 0);
                hashCode = (hashCode*397) ^ (Method != null ? Method.GetHashCode() : 0);
                hashCode = (hashCode*397) ^ (Headers != null ? Headers.ToString().GetHashCode() : 0);
                return hashCode;
            }
        }
    }

    public class DiscoveryHttpResult : IDiscoveryNetworkResult
    {
        public IDiscoveryNetworkRequest Request { get; set; }

        public WebHeaderCollection Headers { get; set; }

        public string Content { get; set; }

        public byte[] Body { get; set; }

        internal static async Task<DiscoveryHttpResult> Create(DiscoveryHttpRequest request, HttpWebResponse response)
        {
            var result = new DiscoveryHttpResult() { Request = request, Headers = new WebHeaderCollection() };
            
            foreach (string header in response.Headers.Keys)
            {
                result.Headers[header] = response.Headers[header];
            }

            if (request.Method != WebRequestMethods.Http.Head)
            {
                if (ContentTypeLooksLikeText(response.ContentType))
                {
                    using (var streamReader = new StreamReader(response.GetResponseStream()))
                    {
                        result.Content = await streamReader.ReadToEndAsync();
                    }
                }
                else if (response.ContentLength < 10*1024*1024) //10MB
                {
                    using (var streamReader = new BinaryReader(response.GetResponseStream()))
                    {
                        result.Body = streamReader.ReadBytes((int)response.ContentLength);
                    }
                }
            }

            return result;
        }
        private static bool ContentTypeLooksLikeText(string contentType)
        {
            return contentType.Contains("text") || contentType.Contains("html") || contentType.Contains("json") ||
                   contentType.Contains("html") || contentType.Contains("utf");
        }
    }

}