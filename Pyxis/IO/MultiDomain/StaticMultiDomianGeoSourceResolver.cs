using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.MultiDomain
{
    /// <summary>
    /// StaticMultiDomianGeoSourceResolver is a simple resolver that can be easily serialized int json object
    /// 
    /// This class build a search tree based on domain values.
    /// 
    /// json schema looks like:
    /// {
    ///     Domains: [ { domains information }, ... ],
    ///     Lookup: {
    ///         Name: "domain-name",
    ///         Values: [
    ///             {
    ///                 Value: "v1",
    ///                 Resource: { resolved resource 
    ///             },
    ///             {
    ///                 Value: "v2"
    ///                 Lookup: {
    ///                     Name: "second-domain-name",
    ///                     Values: [ next domain lookup values ... ]
    ///                 }
    ///             },
    ///             ...
    ///         ]
    ///     }
    /// }
    /// </summary>
    public class StaticMultiDomianGeoSourceResolver : IMultiDomainGeoSourceResolver
    {
        /// <summary>
        /// DomainLookupNodeValue represents the resolved value.
        /// This node can resolve into a Resource or to a next domain lookup for multiple domain resolving.
        /// </summary>
        public class DomainLookupNodeValue
        {
            /// <summary>
            /// Value for the domain
            /// </summary>
            public object Value { get; set; }

            /// <summary>
            /// result Resource if this node value can be resolved.
            /// </summary>
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public ResourceReference Resource { get; set; }

            /// <summary>
            /// next lookup domain more domain lookup is required.
            /// </summary>
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public DomainLookupNode LookupNode { get; set; }
        }

        /// <summary>
        /// DomainLookupNodeValue represents all information for resolving a sigle domain.
        /// </summary>
        public class DomainLookupNode
        {
            public string Domain { get; set; }

            public List<DomainLookupNodeValue> Values { get; set; }
        }

        /// <summary>
        /// Get list of available domains
        /// </summary>
        public List<Domain> Domains { get; set; }

        /// <summary>
        /// Get the Root of the lookup tree
        /// </summary>
        public DomainLookupNode Lookup { get; set; }

        /// <summary>
        /// Register a new entry into lookup tree
        /// </summary>
        /// <param name="domains">domain values</param>
        /// <param name="resource">Resovled reference</param>
        public void Register(IEnumerable<KeyValuePair<string, object>> domains, ResourceReference resource)
        {
            if (Lookup == null)
            {
                Lookup = new DomainLookupNode();
            }

            var node = Lookup;
            var depth = 1;

            foreach (var domain in domains)
            {
                if (!node.Domain.HasContent())
                {
                    node.Domain = domain.Key;
                }

                if (node.Values == null)
                {
                    node.Values = new List<DomainLookupNodeValue>();
                }

                var entry = node.Values.FirstOrDefault(e => e.Value.Equals(domain.Value));

                if (entry == null)
                {
                    entry = new DomainLookupNodeValue()
                    {
                        Value = domain.Value
                    };

                    node.Values.Add(entry);
                }

                if (depth == Domains.Count)
                {
                    entry.Resource = resource;
                }
                else
                {
                    //contiune to next depth;
                    if (entry.LookupNode == null)
                    {
                        entry.LookupNode = new DomainLookupNode();
                    }

                    depth++;
                    node = entry.LookupNode;
                }
            }
        }

        /// <summary>
        /// Resolve a given set of domain and values into a geoSource object
        /// </summary>
        /// <param name="domains">Dictionary of domain name and domain valu.</param>
        /// <returns>resolve GeoSource object or throw an exception</returns>
        public ResourceReference Resolve(Dictionary<string, object> domains)
        {
            if (domains == null)
            {
                throw new ArgumentNullException("domains");
            }

            return Resolve(Lookup,domains);
        }

        private ResourceReference Resolve(DomainLookupNode node,Dictionary<string, object> domains)
        {
            if (!domains.ContainsKey(node.Domain))
            {
                throw new Exception("Domain " + node.Domain + " value was not provided.");
            }

            var value = domains[node.Domain];

            foreach (var entry in node.Values)
            {
                if (entry.Value.Equals(value))
                {
                    if (entry.Resource != null)
                    {
                        return entry.Resource;
                    }
                    else if (entry.LookupNode != null)
                    {
                        return Resolve(entry.LookupNode,domains);
                    }
                    else
                    {
                        throw new Exception("invalid domain lookup entry. both Resource and Loopup field where null");
                    }
                }
            }

            //if we got here, we fail to resolve.
            return null;
        }
    }
}