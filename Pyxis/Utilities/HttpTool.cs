using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.Text;
using System.Net;
using System.Text.RegularExpressions;

namespace Pyxis.Utilities
{
    public static class HttpTool
    {
        /// <summary>
        /// Verifies that the URL is reachable. 
        /// </summary>
        /// <param name="url">The URL.</param>
        /// <param name="exactMatch">
        /// if set to <c>true</c> then a redirected response will be considered a failure.
        /// </param>
        /// <returns></returns>
        public static bool UrlIsReachable(string url, bool exactMatch)
        {
            var urlCheck = new Uri(url);

            var request = WebRequest.Create(urlCheck);
            request.Timeout = 35000;

            WebResponse response;
            try
            {
                //get URL web response
                response = request.GetResponse();
            }
            catch (Exception)
            {
                return false; //url does not exist
            }

            if (exactMatch)
            {
                var responseURL = response.ResponseUri.ToString();

                return string.Compare(responseURL, url, true) == 0;
            }
            else
            {
                return true;
            }
        }

        // Adapted from http://stackoverflow.com/questions/566462/upload-files-with-httpwebrequest-multipart-form-data
        /// <summary>
        /// Upload a file via HTTP.
        /// </summary>
        /// <param name="url">URL to upload the file to.</param>
        /// <param name="file">Path to the file.</param>
        /// <param name="paramName">Field name to use for the underlying form.</param>
        /// <param name="contentType">MIME Type of the file.</param>
        /// <param name="nvc">Any additional form items included in the content disposition.</param>
        public static void UploadFile(string url, string file, string paramName, string contentType, NameValueCollection nvc)
        {
            var boundary = "---------------------------" + DateTime.Now.Ticks.ToString("x");
            var boundaryBytes = Encoding.ASCII.GetBytes("\r\n--" + boundary + "\r\n");

            var wr = CreateRequest(url, boundary);
            var rs = wr.GetRequestStream();

            WriteFormData(nvc, rs, boundaryBytes);

            WriteFormDataHeader(file, paramName, contentType, rs);

            using (var fileStream = new FileStream(file, FileMode.Open, FileAccess.Read))
            {
                var buffer = new byte[4096];
                var bytesRead = 0;
                while ((bytesRead = fileStream.Read(buffer, 0, buffer.Length)) != 0)
                {
                    rs.Write(buffer, 0, bytesRead);
                }
            }

            WriteFooter(rs, boundary);
            rs.Close();

            SendRequest(wr);
        }

        /// <summary>
        /// Upload a file via HTTP.
        /// </summary>
        /// <param name="url">URL to upload the file to.</param>
        /// <param name="data">Data URI.</param>
        /// <param name="paramName">Field name to use for the underlying form.</param>
        /// <param name="contentType">MIME Type of the file.</param>
        /// <param name="nvc">Any additional form items included in the content disposition.</param>
        public static void UploadDataUrl(string url, string dataUri, string paramName, NameValueCollection nvc)
        {
            var boundary = "---------------------------" + DateTime.Now.Ticks.ToString("x");
            var boundaryBytes = Encoding.ASCII.GetBytes("\r\n--" + boundary + "\r\n");

            var wr = CreateRequest(url, boundary);
            var rs = wr.GetRequestStream();

            WriteFormData(nvc, rs, boundaryBytes);

            var regex = new Regex(@"data:(?<mime>[\w/\-\.]+);(?<encoding>\w+),(?<data>.*)", RegexOptions.Compiled);

            var match = regex.Match(dataUri);

            var contentType = match.Groups["mime"].Value;
            var encoding = match.Groups["encoding"].Value;
            var data = match.Groups["data"].Value;
            var binData = Convert.FromBase64String(data);

            WriteFormDataHeader("data-uri", paramName, contentType, rs);

            using (var memoryStream = new MemoryStream(binData))
            {
                var buffer = new byte[4096];
                var bytesRead = 0;
                while ((bytesRead = memoryStream.Read(buffer, 0, buffer.Length)) != 0)
                {
                    rs.Write(buffer, 0, bytesRead);
                }
            }

            WriteFooter(rs, boundary);
            rs.Close();

            SendRequest(wr);
        }

        private static HttpWebRequest CreateRequest(string url, string boundary)
        {
            var wr = (HttpWebRequest)WebRequest.Create(url);
            wr.ContentType = "multipart/form-data; boundary=" + boundary;
            wr.Method = "POST";
            wr.KeepAlive = true;
            wr.Credentials = CredentialCache.DefaultCredentials;
            return wr;
        }

        private static void WriteFormData(NameValueCollection nvc, Stream rs, byte[] boundarybytes)
        {
            var formdataTemplate = "Content-Disposition: form-data; name=\"{0}\"\r\n\r\n{1}";
            foreach (string key in nvc.Keys)
            {
                rs.Write(boundarybytes, 0, boundarybytes.Length);
                var formitem = string.Format(formdataTemplate, key, nvc[key]);
                var formitembytes = Encoding.UTF8.GetBytes(formitem);
                rs.Write(formitembytes, 0, formitembytes.Length);
            }
            rs.Write(boundarybytes, 0, boundarybytes.Length);
        }

        private static void WriteFormDataHeader(string file, string paramName, string contentType, Stream rs)
        {
            var headerTemplate = "Content-Disposition: form-data; name=\"{0}\"; filename=\"{1}\"\r\nContent-Type: {2}\r\n\r\n";
            var header = string.Format(headerTemplate, paramName, file, contentType);
            var headerbytes = Encoding.UTF8.GetBytes(header);
            rs.Write(headerbytes, 0, headerbytes.Length);
        }

        private static void WriteFooter(Stream rs, string boundary)
        {
            var trailer = Encoding.ASCII.GetBytes("\r\n--" + boundary + "--\r\n");
            rs.Write(trailer, 0, trailer.Length);
        }

        private static void SendRequest(HttpWebRequest wr)
        {
            using (var wresp = wr.GetResponse())
            {
                var responseString =
                    (new StreamReader(wresp.GetResponseStream()))
                        .ReadToEnd();
                if (responseString != "200")
                {
                    throw new Exception("Error uploading image: " + responseString);
                }
            }
        }
    }
}
