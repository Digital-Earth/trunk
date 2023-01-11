/******************************************************************************
RestPublishingClient.cs

begin		: Oct. 22, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Configuration;
using System.Net;
using RestSharp;

namespace Pyxis.Publishing.Protocol
{
    /// <summary>
    /// Base class for REST clients of the license server
    /// </summary>
    public abstract class RestPublishingClient
    {
        protected RestClient m_client;
        protected readonly string m_licenseServerUrl;
        internal LicenseServerRestUrlBuilder RestUrlBuilder { get; set; }

        public RestPublishingClient(RestPublishingClient basedOn)
        {
            m_licenseServerUrl = basedOn.m_licenseServerUrl;
            m_client = new RestClient(m_licenseServerUrl);
            RestUrlBuilder = basedOn.RestUrlBuilder;
        }

        public RestPublishingClient(string licenseServerUrl, string licenseServerRestPrefix)
        {
            m_licenseServerUrl = licenseServerUrl;
            m_client = new RestClient(m_licenseServerUrl);
            RestUrlBuilder = new LicenseServerRestUrlBuilder(licenseServerRestPrefix);
        }

        public RestPublishingClient(string licenseServerUrl)
            : this(licenseServerUrl, Properties.Settings.Default.LicenseServerRestPrefix)
        {
        }

        /// <summary>
        /// Helper class to execute RestRequest and deserialize the result using Json.Net
        /// </summary>
        /// <typeparam name="T">Output type</typeparam>
        /// <param name="request">the request to execute</param>
        /// <returns>result object deserialized or throws an exception if the request could not be completed.</returns>
        /// <remarks>
        /// A completed request does not imply the request completed without errors.  E.g. it may complete with HTTP status code 400
        /// and as such the deserialized object returned could be that constructed using the default constructor of the class.
        /// </remarks>
        protected T Execute<T>(RestSharp.RestRequest request) where T : new()
        {
            var response = m_client.Execute(request);

            if (response.ResponseStatus == ResponseStatus.Completed)
            {
                return Newtonsoft.Json.JsonConvert.DeserializeObject<T>(response.Content);
            }
            throw response.ErrorException;
        }

        /// <summary>
        /// Helper class to execute RestRequest and deserialize the result using Json.Net.
        /// </summary>
        /// <typeparam name="T">Output type</typeparam>
        /// <param name="request">the request to execute</param>
        /// <param name="expectedHttpStatusCode">The expected HTTP status code of the response.</param>
        /// <returns>Deserialized response object or throws an exception if the response does not have the expected HTTP status code.</returns>
        protected T Execute<T>(RestSharp.RestRequest request, HttpStatusCode expectedHttpStatusCode) where T : new()
        {
            var response = m_client.Execute(request);

            if (response.ResponseStatus == ResponseStatus.Completed && response.StatusCode == expectedHttpStatusCode)
            {
                return Newtonsoft.Json.JsonConvert.DeserializeObject<T>(response.Content);
            }
            throw new Exception("Unexpected HTTP response status code.  Returned: " + response.StatusCode + " with content " + response.Content);
        }
    }
}
