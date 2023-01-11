using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Class that create a valid IOgcUrl and Catalog from an input GetCapabilities document
    /// </summary>
    public class OgcWebMapCapabilitiesParser
    {
        private readonly IOgcUrl m_url;
        private readonly DataSetCatalog m_catalog;
        private string m_fileFormat;

        /// <summary>
        /// Provide stats about parsing issues, errors or fixes been done on the xml document
        /// </summary>
        public Dictionary<string, int> Stats { get; set; }

        public class LayerContext
        {
            private readonly LayerContext m_parent;

            private string m_format;

            public string Format
            {
                get
                {
                    if (m_format.HasContent())
                    {
                        return m_format;
                    }
                    return m_parent != null ? m_parent.Format : null;
                }
                set { m_format = value != null ? value.Trim() : null; }
            }

            private string m_latLonBbox;

            public string LatLonBBox
            {
                get
                {
                    if (m_latLonBbox.HasContent())
                    {
                        return m_latLonBbox;
                    }
                    return m_parent != null ? m_parent.LatLonBBox : null;
                }
                set { m_latLonBbox = value != null ? value.Trim() : null; }
            }

            private readonly Dictionary<string, string> m_bbox = new Dictionary<string, string>();

            public string GetBBox(string crs)
            {
                if (!crs.HasContent())
                {
                    return null;
                }

                crs = crs.Trim();

                if (m_bbox.ContainsKey(crs))
                {
                    return m_bbox[crs];
                }
                return m_parent != null ? m_parent.GetBBox(crs) : null;
            }

            public string FindValidBBoxCrs()
            {
                if (m_bbox.Count > 0)
                {
                    return m_bbox.First().Key;
                }
                return m_parent != null ? m_parent.FindValidBBoxCrs() : null;
            }

            public void AddBBox(string crs, string bbox)
            {
                if (crs.HasContent())
                {
                    crs = crs.Trim();
                    m_bbox[crs] = bbox;
                }
            }

            private readonly List<string> m_styles = new List<string>();

            public IEnumerable<string> Styles
            {
                get
                {
                    if (m_parent != null)
                    {
                        return m_styles.Concat(m_parent.Styles).Distinct();
                    }
                    return m_styles;
                }
            }

            public void AddStyle(string style)
            {
                if (style.HasContent())
                {
                    style = style.Trim();
                    if (!m_styles.Contains(style))
                    {
                        m_styles.Add(style);
                    }
                }
            }

            private readonly List<string> m_crs = new List<string>();

            public IEnumerable<string> Crs
            {
                get
                {
                    if (m_parent != null)
                    {
                        return m_crs.Concat(m_parent.Crs).Distinct();
                    }
                    return m_crs;
                }
            }

            public void AddCrs(string crs)
            {
                if (crs.HasContent())
                {
                    crs = crs.Trim();
                    if (!m_crs.Contains(crs))
                    {
                        m_crs.Add(crs);
                    }
                }
            }

            private readonly List<Domain> m_domains = new List<Domain>();
            public IEnumerable<Domain> Domains
            {
                get
                {
                    if (m_parent != null)
                    {
                        return m_domains.Concat(m_parent.Domains).Distinct().Where(d => d.Values != null && d.Values.Any());
                    }
                    return m_domains.Where(d => d.Values != null && d.Values.Any());
                }
            }

            public void AddDomain(Domain domain)
            {
                if (domain != null && domain.Name != null)
                {
                    var existingDomain = m_domains.FirstOrDefault(d => d.Name == domain.Name);
                    if (existingDomain == null)
                    {
                        m_domains.Add(domain);
                    }
                    else
                    {
                        existingDomain.Value = existingDomain.Value == null ? domain.Value : existingDomain.Value;
                        existingDomain.Values = existingDomain.Values == null ? domain.Values : existingDomain.Values;
                    }
                }
            }

            public LayerContext(LayerContext parent = null)
            {
                m_parent = parent;
            }


            public LayerContext CreateChildContext()
            {
                return new LayerContext(this);
            }
        }

        public OgcWebMapCapabilitiesParser(IOgcUrl url, DataSetCatalog catalog)
        {
            m_url = url;
            m_catalog = catalog;

            Stats = new Dictionary<string, int>()
            {
                {"Layers", 0},
                {"MultiStyle", 0},
                {"NoNativeSRS", 0},
                {"ForcedDefaultSRS", 0},
                {"NoName", 0},
                {"NoNameWithChildren", 0},
                {"NoBBox", 0},
                {"NoStyle", 0},
                {"NoTitle", 0}
            };
        }

        public void Parse(XmlDocument capabilities)
        {
            if (capabilities == null)
            {
                throw new Exception("The server capabilities are not available");
            }

            // Retrieve the server's OGC version
            RetrieveVersion(capabilities);

            var pass1 = new XmlNodeExtractor()
                .On("Service", new OgcMetadataExtractor(m_catalog.Metadata))
                .On("Capability", new XmlNodeExtractor()
                        .On("Request", new XmlNodeExtractor()
                                .On("GetMap", ExtractFormat)
                                .On("Map", ExtractFormat)
                        )
                );
            pass1.Parse(capabilities.DocumentElement);

            var pass2 = new XmlNodeExtractor("Capability", new XmlNodeExtractor("Layer", ExtractLayer));
            pass2.Parse(capabilities.DocumentElement);

            /*
            // Verify the "GetTileService" elements' contents
            var tileService = capabilities.SelectNodes("//*[local-name()='GetTileService']");
            if (tileService != null)
            {
                foreach (XmlNode t in tileService)
                {
                    var formatFlag = t.SelectSingleNode("Format");
                    var format = formatFlag != null ? (formatFlag.Value ?? formatFlag.InnerText ?? "") : "";
                    if (!format.Contains("text/xml"))
                    {
                        // Issue a warning.  We expect text/xml.
                        TraceTool.GlobalTrace.WriteLine(
                            "Warning! Saw an unexpected format ({0}) while parsing WMS from {1} (ignoring)",
                            formatFlag != null ? formatFlag.Value : null,
                            m_url.ServerUrl
                            );
                    }

                    var urlText = t.SelectSingleNode("DCPType/HTTP/Get/OnlineResource");
                    var test = urlText != null ? urlText.Attributes != null ? urlText.Attributes.GetNamedItem("xlink:href") : null : null;
                    if (test == null)
                    {
                        Trace.error("An element 'DCPType/HTTP/Get/OnlineResource' with an attribute 'xlink:href' not found in a 'GetTileService' element");
                        continue;
                    }
                    TraceTool.GlobalTrace.WriteLine("Success: {0}", test.ToString());
                }
            }

            // Finally, retrieve information about the data sets on the server
            RetrieveDataSets(capabilities);
            */
        }

        private void ExtractLayer(XmlNode layer)
        {
            var context = new LayerContext()
            {
                Format = m_fileFormat,
            };

            ExtractLayerWithContext(layer, context);
        }

        private void ExtractLayerWithContext(XmlNode layer, LayerContext context)
        {
            Stats["Layers"]++;

            var dataSetUrl = new OgcWebMapUrl(m_url.ServerUrl)
            {
                Request = null
            };

            var dataSetMetadata = new SimpleMetadata();


            var metadataExtractor = new OgcMetadataExtractor(dataSetMetadata);
            metadataExtractor.On("name", node => dataSetUrl.Name = node.InnerText);
            metadataExtractor.On("scalehint", node => dataSetUrl.Scalehint = node.OuterXml);
            metadataExtractor.On("format", node => context.Format = node.InnerText);
            metadataExtractor.On("style", node => AddStyle(context, node));
            metadataExtractor.On("dimension", node => AddDimension(context, node));
            metadataExtractor.On("extent", node => AddDimension(context, node));
            metadataExtractor.On("srs", node => context.AddCrs(node.InnerText));
            metadataExtractor.On("crs", node => context.AddCrs(node.InnerText));
            metadataExtractor.On("latlonboundingbox", node => context.LatLonBBox = node.OuterXml);
            metadataExtractor.On("ex_geographicboundingbox", node => context.LatLonBBox = node.OuterXml);
            metadataExtractor.On("boundingbox", node => context.AddBBox(ExtractBBoxCRS(node), node.OuterXml));

            metadataExtractor.Parse(layer);

            dataSetUrl.Styles = String.Join(",", context.Styles);
            if (dataSetUrl.Styles == "")
            {
                dataSetUrl.Styles = "default";
            }

            //We need to extract bbox and SRS only for layer with valid name and style. 
            //all other layers are decorative and therefore bbox is waste of effort
            if (dataSetUrl.Name.HasContent() && dataSetUrl.Styles.HasContent())
            {
                ExtractBBoxAndCrs(context, dataSetUrl);
            }

            dataSetUrl.Format = context.Format;

            //if layer has a <Name> but not <Title> or <Title /> empty element, 
            //we default to layer Name (ID) as metadata name
            if (!dataSetMetadata.Name.HasContent() && dataSetUrl.Name.HasContent())
            {
                //layer names have "_" to be used as spaces. replace back to more readable name
                dataSetMetadata.Name = dataSetUrl.Name.Replace("_", " ");
            }

            // Don't add the data set if it is not complete (header case).
            if (dataSetUrl.Name.HasContent()
                && dataSetUrl.Styles.HasContent()
                && dataSetUrl.BBox.HasContent()
                && dataSetMetadata.Name.HasContent())
            {
                var dataSet = new DataSet
                {
                    Layer = dataSetUrl.Name,
                    Uri = dataSetUrl.ToString(),
                    Metadata = dataSetMetadata,
                    Domains = context.Domains.Any() ? context.Domains.ToList() : null 
                };
                m_catalog.DataSets.Add(dataSet);

                if (context.Styles.Count() > 1)
                {
                    Stats["MultiStyle"]++;
                }
            }
            else
            {
                if (string.IsNullOrEmpty(dataSetUrl.Styles))
                {
                    Stats["NoStyle"]++;
                    Stats["Layers"]--;
                }
                else
                {
                    if (string.IsNullOrEmpty(dataSetUrl.Name))
                    {
                        if (layer.ChildNodes.Cast<XmlNode>().Any(x => x.LocalName.ToLower() == "layer"))
                        {
                            Stats["Layers"]--;
                            Stats["NoNameWithChildren"]++;
                        }
                        else
                        {
                            Stats["NoName"]++;
                        }
                    }
                    if (string.IsNullOrEmpty(dataSetMetadata.Name))
                    {
                        Stats["NoTitle"]++;
                    }
                    if (string.IsNullOrEmpty(dataSetUrl.BBox))
                    {
                        Stats["NoBBox"]++;
                    }
                }
            }

            //parse child layers
            new XmlNodeExtractor("layer", (node) => ExtractLayerWithContext(node, context.CreateChildContext())).Parse(layer);
        }

        private void ExtractBBoxAndCrs(LayerContext context, OgcWebMapUrl dataSetUrl)
        {
            //match crs case sensitivity of the server. EPSG:4326 or epsg:4326
            var defaultSrs = context.Crs.FirstOrDefault(crs => crs.Equals("EPSG:4326", StringComparison.OrdinalIgnoreCase)) ??
                             "EPSG:4326";

            // 1. check if our default SRS has a bbox defined - this is the what the standard intended
            if (context.GetBBox(defaultSrs).HasContent())
            {
                dataSetUrl.Srs = defaultSrs;
                dataSetUrl.BBox = context.GetBBox(defaultSrs);
            }
            // 2. If EPSG:4326 (our default SRS) is supported (we have an <SRS>EPSG:4326</SRS> element),
            // and we have exterior latLonBoundingBox, we are good
            else if (context.Crs.Contains(defaultSrs, StringComparer.OrdinalIgnoreCase) && context.LatLonBBox.HasContent())
            {
                Stats["ForcedDefaultSRS"]++;
                dataSetUrl.Srs = defaultSrs;
                dataSetUrl.BBox = context.LatLonBBox;

                var nativeSRS = context.FindValidBBoxCrs();
                if (nativeSRS == null)
                {
                    Stats["NoNativeSRS"]++;
                }
                else if (nativeSRS == "")
                {
                    Stats["NoNativeSRS"]++;
                }
                else if (!Stats.ContainsKey(nativeSRS))
                {
                    Stats[nativeSRS] = 1;
                }
                else
                {
                    Stats[nativeSRS]++;
                }
            }
            // 3. If any <BoundingBox> found, just take any
            else if (context.FindValidBBoxCrs().HasContent())
            {
                dataSetUrl.Srs = context.FindValidBBoxCrs();
                dataSetUrl.BBox = context.GetBBox(dataSetUrl.Srs);
            }
            // 4. Finally, if we have latLonBoundingBox but no SRS specified,
            // use the value but don't set the SRS
            else if (context.LatLonBBox.HasContent() && !context.Crs.Any())
            {
                dataSetUrl.BBox = context.LatLonBBox;
            }
        }

        private string ExtractBBoxCRS(XmlNode node)
        {
            return XmlUtils.SafeAttributeValue(node.Attributes.GetNamedItem("CRS") as XmlAttribute) ??
                   XmlUtils.SafeAttributeValue(node.Attributes.GetNamedItem("SRS") as XmlAttribute);
        }

        private void AddStyle(LayerContext context, XmlNode styleNode)
        {
            string style = null;
            new XmlNodeExtractor("name", node => style = node.InnerText).Parse(styleNode);
            if (style.HasContent())
            {
                context.AddStyle(style);
            }
        }

        private void AddDimension(LayerContext context, XmlNode dimensionNode)
        {
            var dimension = XmlUtils.SafeAttributeValue(dimensionNode.Attributes.GetNamedItem("name") as XmlAttribute);
            if (dimension == null)
            {
                return;
            }

            var units = XmlUtils.SafeAttributeValue(dimensionNode.Attributes.GetNamedItem("units") as XmlAttribute);
            if (units != null && units != "ISO8601")
            {
                return;
            }

            var defaultValue = XmlUtils.SafeAttributeValue(dimensionNode.Attributes.GetNamedItem("default") as XmlAttribute);
            DateTime defaultTime = DateTime.MinValue;
            if (defaultValue != null && !DateTime.TryParse(defaultValue, out defaultTime))
            {
                return;
            }

            var domain = new Domain
            {
                Name = dimension,
                Metadata = new SimpleMetadata
                {
                    Name = dimension
                },
                Type = PipelineSpecification.FieldType.Date
            };

            if (defaultTime != DateTime.MinValue)
            {
                domain.Value = defaultTime;
            }
            SetValues(dimensionNode, domain);

            context.AddDomain(domain);
        }

        private static void SetValues(XmlNode dimensionNode, Domain domain)
        {
            var valueRange = dimensionNode.InnerText;
            if (valueRange != null)
            {
                var tokens = valueRange.Split('/');
                if (tokens.Length != 3)
                {
                    return;
                }
                DateTime start, end;
                if (!DateTime.TryParse(tokens[0], out start))
                {
                    return;
                }
                if (!DateTime.TryParse(tokens[1], out end))
                {
                    if (string.Equals(tokens[1], "current", StringComparison.InvariantCultureIgnoreCase))
                    {
                        end = DateTime.Today;
                    }
                    else
                    {
                        return;
                    }
                }
                TimeSpan step;
                try
                {
                    step = System.Xml.XmlConvert.ToTimeSpan(tokens[2]);
                }
                catch (Exception)
                {
                    return;
                }
                var values = new List<object>();
                for (var date = start; date <= end; date += step)
                {
                    values.Add(date);
                    if (values.Count > 1000)
                    {
                        // consider supporting domain value generators (start, end, step) in this case
                        break;
                    }
                }
                domain.Values = values;

                if (domain.Value == null)
                {
                    domain.Value = start;
                }
            }
        }

        private void ExtractFormat(XmlNode node)
        {
            var fileFormat = "";

            var fileFormats = new List<string>();
            new XmlNodeExtractor("Format", format => fileFormats.Add(format.InnerText.Trim())).Parse(node);

            foreach (var preferedFormat in new[] { "image/png", "image/jpeg", "image/tiff" })
            {
                if (fileFormats.Contains(preferedFormat,StringComparer.OrdinalIgnoreCase))
                {
                    fileFormat = preferedFormat;
                    break;
                }
            }

            if (string.IsNullOrEmpty(fileFormat))
            {
                fileFormat = fileFormats.FirstOrDefault() ?? "image/jpeg";
            }

            m_fileFormat = fileFormat;
        }

        protected void RetrieveVersion(XmlDocument capabilities)
        {
            try
            {
                m_url.Version = capabilities.DocumentElement.Attributes["version"].Value;
            }
            // NullReferenceException is possible
            catch (Exception)
            {
                try
                {
                    var capabilitiesUrl = new OgcWebMapUrl(capabilities.BaseURI);
                    m_url.Version = capabilitiesUrl.Version;
                }
                catch (Exception)
                {
                    // Just continue
                }
            }
            if (string.IsNullOrEmpty(m_url.Version))
            {
                m_url.Version = "1.1.1";
            }
        }
    }
}