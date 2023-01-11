using System.Collections.Generic;
using System.Linq;
using Pyxis.Utilities;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Helper class for handling OGC WCS URLs
    /// </summary>
    public class OgcWebCoverageUrl : OgcUrl
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Address of an OGC resource on a OGC WCS server</param>
        public OgcWebCoverageUrl(string url) :
            base(url)
        {
            var query = new UriQueryBuilder(ServerUrl);

            // Read attributes
            Name = Version == "1.0.0" ?
                query.Parameters.Get("identifier") ?? query.Parameters.Get("coverage") ?? null
                : Version != null && Version.StartsWith("1.1") ?
                    query.Parameters.Get("identifiers") :
                    query.Parameters.Get("coverageId");

            EoId = query.Parameters.Get("eoId");
            RangeSubset = query.Parameters.Get("rangeSubset");
            EncodedSubset = query.Parameters.Get("subset");
            Format = query.Parameters.Get("format");

            // Remove attributes from the server URL
            query.RemoveParameter("bbox");
            query.RemoveParameter("boundingbox");
            query.RemoveParameter("coverage");
            query.RemoveParameter("coverageid");
            query.RemoveParameter("featureID");
            query.RemoveParameter("filter");
            query.RemoveParameter("format");
            query.RemoveParameter("height");
            query.RemoveParameter("identifier");
            query.RemoveParameter("identifiers");
            query.RemoveParameter("mediatype");
            query.RemoveParameter("eoId");
            query.RemoveParameter("rangeSubset");
            query.RemoveParameter("subset");


            // Save the server URL and reset the service type
            ServerUrl = query.ToString();
            Service = "WCS";
        }

        public string Name
        {
            get
            {
                return Version == "1.0.0" ?
                    GetAttribute("identifier") ?? GetAttribute("coverage") ?? null
                    : Version != null && Version.StartsWith("1.1") ?
                        GetAttribute("identifiers") :
                        GetAttribute("coverageId");
            }
            set
            {
                SetAttribute(
                    Version == "1.0.0" ?
                        "identifier"
                        : Version != null && Version.StartsWith("1.1") ?
                            "identifiers"
                            : "coverageId",
                    value
                );
            }
        }


        /// <summary>
        /// EO extenation (Earth Observation) for DescribeEOCoverageSet request
        /// </summary>
        public string Format
        {
            get { return GetAttribute("format"); }
            set { SetAttribute("format", value); }
        }

        /// <summary>
        /// EO extenation (Earth Observation) for DescribeEOCoverageSet request
        /// </summary>
        public string EoId
        {
            get { return GetAttribute("eoId"); }
            set { SetAttribute("eoId", value); }
        }

        /// <summary>
        /// Define the range subset of bands to load from the coverage
        /// </summary>
        public string RangeSubset
        {
            get { return GetAttribute("rangeSubset"); }
            set { SetAttribute("rangeSubset", value); }
        }

        public string EncodedSubset
        {
            get { return GetAttribute("subset"); }
            set { SetAttribute("subset", value); }
        }

        public IReadOnlyDictionary<string,string> Subset
        {
            get
            {
                return ParseSubset(EncodedSubset);
            }
            set
            {
                EncodedSubset = string.Join(",", value.Select(keyValue => keyValue.Key + "(" + keyValue.Value + ")"));
            }
        }

        //encoded as: key(value),key(value)
        private static IReadOnlyDictionary<string, string> ParseSubset(string encodedString)
        {
            encodedString = encodedString ?? "";

            var subsets = new Dictionary<string, string>();

            while (encodedString.Length > 0)
            {
                var startPos = encodedString[0] == ',' ? 1 : 0;
                var nameEnd = encodedString.IndexOf('(');
                var valueEnd = encodedString.IndexOf(')');
                if (nameEnd == -1 || valueEnd == -1)
                {
                    break;
                }

                var key = encodedString.Substring(startPos, nameEnd - startPos );
                var value = encodedString.Substring(nameEnd + 1, valueEnd - nameEnd - 1);

                subsets[key] = value;

                encodedString = encodedString.Substring(valueEnd + 1);
            }

            return subsets;
        }
    }
}