using System;
using System.Collections.Generic;
using System.Globalization;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Web.UI.WebControls;
using System.Xml;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces.Domains;
using Style = Pyxis.Contract.Publishing.Style;

namespace Pyxis.Contract.Workspaces
{
    /// <summary>
    /// Import url template based on domain values
    /// </summary>
    public class ImportTemplate : IImport
    {
        public string Type
        {
            get { return "DataSetTemplate"; }
        }

        public string Uri { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string InternalPath { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Layer { get; set; }
        
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public Style Style { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public PipelineSpecification Specification { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Srs { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public GeoTagMethods GeoTag { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string Sampler { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public Dictionary<string, Domain> Domains { get; set; }

        public class Domain
        {
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public string Type { get; set; }

            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public List<string> Values { get; set; }

            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public RangeDetails Range { get; set; }

            public IDomain AsIDomain()
            {
                if (Values != null)
                {
                    return new ValuesDomain(Values);
                }
                if (Range != null)
                {
                    switch (Type)
                    {
                        case "time":
                        case "date":
                            return new DateRangeDomain(
                                (DateTime)Range.Start,
                                (DateTime)Range.End,
                                Range.Step.ToString()
                                );

                        case "number":
                            return new NumericDomainRange(double.Parse(Range.Start.ToString()), double.Parse(Range.End.ToString()), double.Parse(Range.Step.ToString()));

                        default:
                            throw new Exception("Unsupported range type: " + Type);
                    }
                }
                throw new Exception("Can't create IDomain for this domain. please provide Range or Values");
            }
        }

        public class RangeDetails
        {
            public object Start { get; set; }
            public object End { get; set; }
            public object Step { get; set; }
        }

        public string ApplyTemplate(string value, Dictionary<string, string> domainsValues)
        {
            var finalValue = value;
            
            var regex = new Regex(@"\$\{(?<name>\w+)(?<format>|:\w+)\}");
            
            var match = regex.Match(finalValue);

            while (match.Success)
            {
                var name = match.Groups["name"].Value;
                var format = match.Groups["format"].Value.TrimStart(':');
                var formatedValue = "";
                if (Domains.ContainsKey(name))
                {
                    formatedValue = Domains[name].AsIDomain().FormatValue(domainsValues[name], format);
                }                
                
                finalValue = finalValue.Replace(match.Value, formatedValue);
                match = regex.Match(finalValue);
            }

            return finalValue;
        }

        public bool IsDomainValueValid(string name, string value)
        {
            var domain = Domains[name];

            return domain.AsIDomain().Contains(value);
        }

        private string DomainPossibleValues(string name)
        {
            var domain = Domains[name];

            if (domain.Range != null)
            {
                return string.Format("{0} .. {1} (step : {2})", domain.Range.Start, domain.Range.End, domain.Range.Step);
            }
            if (domain.Values != null)
            {
                return string.Join(", ", domain.Values);
            }
            return "";
        }

        public DataSet Resolve(Dictionary<string, string> domainsValues)
        {
            if (domainsValues == null)
            {
                throw new Exception("domains value are required to resolve import template");
            }

            var finalUri = Uri;
            var finalInternalPath = InternalPath;

            foreach(var domain in Domains)
            {
                var name = domain.Key;
                
                if (!domainsValues.ContainsKey(name))
                {
                    throw new Exception("Missing domain " + name + ", possible values: " + DomainPossibleValues(name));
                }

                if (!IsDomainValueValid(name,domainsValues[name]))
                {
                    throw new Exception("Domain " + name + " value of " + domainsValues[name] + " is not allowed");
                }                
            }

            finalUri = ApplyTemplate(Uri,domainsValues);

            if (InternalPath != null)
            {
                finalInternalPath = ApplyTemplate(InternalPath, domainsValues);
            }

            return new DataSet()
            {
                Uri = finalUri,
                InternalPath = finalInternalPath,
                Layer = Layer,
                Specification = Specification
            };
        }
    }
}
