using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using System.Net;
using System.IO;

namespace ApplicationUtility
{
    public class ManagedHttpRequestProvider : HttpRequestProvider, IDirectorReferenceCounter
    {
        #region Members

        private Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(true);

        Dictionary<int, ManagedHttpRequest> m_requests = new Dictionary<int, ManagedHttpRequest>();

        int m_nextRequestIndex = 0;

        #endregion

        #region HttpRequestProvider

        public override int createRequest(string url, string method)
        {
            return createRequest(url, method, null);
        }

        public override int createRequest(string url, string method, IUserCredentials_SPtr userCredentials)
        {
            ManagedHttpRequest request = new ManagedHttpRequest(url, method);

            //HACK: increase timeout because WPS failed to return results in 100 sec.
            request.Request.Timeout = 200000;

            if (userCredentials != null && userCredentials.isNotNull())
            {
                var usernameAndPassword = pyxlib.QueryInterface_IUsernameAndPasswordCredentials(userCredentials);
                if (usernameAndPassword != null && usernameAndPassword.isNotNull())
                {
                    request.Request.Credentials = new NetworkCredential(usernameAndPassword.getUsername(), usernameAndPassword.getPassword());
                }
            }
            
            int requestHandle;

            //lock our manager
            lock (this)
            {
                //create an doc handle
                requestHandle = m_nextRequestIndex;
                m_nextRequestIndex++;

                //store the doc for later use
                m_requests[requestHandle] = request;
            }

            return requestHandle;
        }

        public override void destroyRequest(int requestHandle)
        {
            lock (this)
            {
                m_requests.Remove(requestHandle);
            }
        }

        private ManagedHttpRequest GetRequest(int requestHandle)
        {
            lock (this)
            {
                if (m_requests.ContainsKey(requestHandle))
                {
                    return m_requests[requestHandle];
                }
            }
            return null;
        }

        public override bool getResponse(int requestHandle)
        {
            ManagedHttpRequest request = GetRequest(requestHandle);

            if (request == null)
            {
                return false;
            }

            return request.GetResponse();
        }

        public override string getResponseBody(int requestHandle)
        {
            ManagedHttpRequest request = GetRequest(requestHandle);

            if (request == null)
            {
                return "";
            }

            return request.ResponseBody;
        }

        public override bool downloadResponse(int requestHandle, string filename)
        {
            ManagedHttpRequest request = GetRequest(requestHandle);

            return request.GetResponseAndWriteToFile(filename);
        }

        public override void addRequestBody(int requestHandle, string body)
        {
            ManagedHttpRequest request = GetRequest(requestHandle);

            var stream = (request.Request as HttpWebRequest).GetRequestStream();
            using (var writer = new StreamWriter(stream))
            {
                writer.Write(body);
            }
        }

        public override void addRequestHeader(int requestHandle, string headerName, string headerValue)
        {
            var webRequest = (HttpWebRequest)GetRequest(requestHandle).Request;

            // a quick check to speed up setting User-Agent
            if (headerName == "User-Agent")
            {
                webRequest.UserAgent = headerValue;
                return;
            }

            // Setting headers that are properties will throw an Error
            //We first check if the header name is a property of the HttpWebRequest
            var propertyInfo = webRequest.GetType().GetProperty(headerName);
            if (propertyInfo != null)
            {
                propertyInfo.SetValue(webRequest, headerValue, null);
            }
            else
            {
                // if header name is not in the properties then we set it here
                webRequest.Headers.Add(headerName, headerValue);
            }

        }

        #endregion

        #region PYXObject Lifetime Management

        #region IDirectorReferenceCounter Members

        public void setSwigCMemOwn(bool value)
        {
            swigCMemOwn = value;
        }

        public int doAddRef()
        {
            return base.addRef();
        }

        public int doRelease()
        {
            return base.release();
        }

        #endregion

        #region PYXObject

        /// <summary>
        /// Override the reference-counting addRef.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after increment).</returns>
        public override int addRef()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.addRef(this);
        }

        /// <summary>
        /// Override the reference-counting release.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after decrement).</returns>
        public override int release()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.release(this);
        }

        #endregion
        /// <summary>
        /// Default constructor.
        /// </summary>
        public ManagedHttpRequestProvider()
        {
        }

        #endregion PYXObject Lifetime Management
    }

    class ManagedHttpRequest
    {
        public WebRequest Request { get; set; }
        public WebResponse Response { get; set; }
        public bool Requested { get; set; }
        public bool GotResponseOk { get; set; }
        public string ResponseBody { get; set; }

        public ManagedHttpRequest(string url, string method)
        {
            Request = WebRequest.Create(url);
            Request.Method = method;
            Requested = false;
            GotResponseOk = false;
        }

        public bool GetResponse()
        {
            if (Requested)
                return GotResponseOk;

            Requested = true;

            try
            {
                Response = Request.GetResponse();
                ResponseBody = new StreamReader(Response.GetResponseStream()).ReadToEnd();
                GotResponseOk = true;
            }
            catch (Exception e)
            {
                Trace.error("Failed to response from WebRequest: " + e.Message);
                GotResponseOk = false;
            }

            return GotResponseOk;
        }

        public bool GetResponseAndWriteToFile(string filename)
        {
            if (Requested)
                return false;

            Requested = true;

            try
            {
                Response = Request.GetResponse();
                var x = new MemoryStream();

                var input = Response.GetResponseStream();

                using (var output = File.OpenWrite(filename))
                {
                    byte[] buffer = new byte[8 * 1024];
                    int len;
                    while ((len = input.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        output.Write(buffer, 0, len);
                    }
                    output.Flush();
                }

                GotResponseOk = true;
            }
            catch (Exception e)
            {
                Trace.error("Failed to get a response from WebRequest: " + e.Message);
                GotResponseOk = false;
            }

            return GotResponseOk;
        }
    }
}
