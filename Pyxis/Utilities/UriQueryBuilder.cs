using System;
using System.Collections.Generic;
using System.Text;
using System.Collections.Specialized;
using System.Text.RegularExpressions;

namespace Pyxis.Utilities
{
    public class UriQueryBuilder
    {
        /// <summary>
        /// Gets or sets the parameters.  Allows clients to manipulate params.
        /// </summary>
        /// <value>The parameters.</value>
        public System.Collections.Specialized.NameValueCollection Parameters { get; set; }

        private string m_baseUri;

        public string ServerUri
        {
            get
            {
                return m_baseUri;
            }
            set
            {
                m_baseUri = value;
            }
        }

        public UriQueryBuilder(string baseUri)
        {
            
            Uri inputUri = new Uri(baseUri);
            Parameters = HttpUtility.ParseQueryString(inputUri.Query);
            m_baseUri = inputUri.GetLeftPart(UriPartial.Path);
        }

        /// <summary>
        /// Sets the value of the given parameter to the given default, UNLESS it is already set.
        /// </summary>
        /// <param name="parameterName">Name of the parameter.</param>
        /// <param name="defaultValue">The default value.</param>
        public void SetDefaultParameter(string parameterName, string defaultValue)
        {
            if ((Parameters.GetValues(parameterName) == null) ||
                (Parameters.GetValues(parameterName).Length == 0))
            {
                Parameters[parameterName] = defaultValue;
            }
        }

        public void OverwriteParameter(string parameterName, string value)
        {
            //clean all paramters...
            RemoveParameter(parameterName);
            //set the new value...
            Parameters[parameterName] = value;
        }

        public void RemoveParameter(string parameterName)
        {
            Parameters.Remove(parameterName);
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.  Note that this is 
        /// the essential output of this class.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString()
        {
            UriBuilder baseUri = new UriBuilder(m_baseUri);
            StringBuilder queryBuilder = new StringBuilder();
            foreach (var key in Parameters.Keys)
            {
                //this happen when the query look like "url?param=value&&param3=value3"
                //when there are two "&" one after the other the key value is null
                if (key == null)
                {
                    //so we skip it
                    continue;
                }
                
                foreach (var value in Parameters.GetValues(key as string))
                {
                    if (queryBuilder.Length > 0)
                        queryBuilder.Append("&");

                    queryBuilder.AppendFormat("{0}={1}", key, System.Uri.EscapeDataString(value));                    
                }
            }            
            baseUri.Query = queryBuilder.ToString();
            
            return new Uri(baseUri.ToString()).AbsoluteUri; //clean the uri. aka - remove default port
        }
    }
    
    internal static class HttpUtility
    {
        public static NameValueCollection ParseQueryString(string queryString)
        {
            NameValueCollection nvc = new NameValueCollection();

            // A regular expression to remove unexpected leading characters that are not a part of the uri 
            var regex = new Regex("^[^a-zA-Z0-9]+");
            // Split out the uri to receive a list of parameters
            foreach (string vp in Regex.Split(regex.Replace(queryString, ""), "&"))
            {
                if (String.IsNullOrWhiteSpace(vp))
                {
                    continue;
                }

                string[] singlePair = Regex.Split(vp, "=");
                if (singlePair.Length == 2)
                {
                    nvc.Add(Uri.UnescapeDataString(singlePair[0]), Uri.UnescapeDataString(singlePair[1]));
                }
                else
                {
                    // only one key with no value specified in query string
                    nvc.Add(Uri.UnescapeDataString(singlePair[0]), string.Empty);
                }
            }
            return nvc;
        }
    }

}
