using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.Sources.OGC
{
    public class OgcWebFeatureCapabilitiesParser
    {
        private readonly IOgcUrl m_url;
        private readonly DataSetCatalog m_catalog;

        /// <summary>
        /// Provide stats about parsing issues, errors or fixes been done on the xml document
        /// </summary>
        public Dictionary<string, int> Stats { get; set; }

        public OgcWebFeatureCapabilitiesParser(IOgcUrl url, DataSetCatalog catalog)
        {
            m_url = url;
            m_catalog = catalog;

            Stats = new Dictionary<string, int>()
            {
                {"Layers", 0},
                {"NoName", 0},
                {"NoTitle", 0},
                {"NoDefaultCRS", 0},
                {"NoneGeospatial", 0}
            };
        }

        public void Parse(XmlDocument capabilities)
        {
            if (capabilities == null)
            {
                throw new Exception("The server capabilities are not available");
            }

            // Retrieve the Metadata
            RetrieveMetadata(capabilities);

            m_url.Version = RetrieveVersion(capabilities.DocumentElement);

            // Retrieve information about the data sets on the server
            RetrieveDataSets(capabilities);
        }


        protected void RetrieveMetadata(XmlDocument capabilities)
        {
            var metadataParser = new OgcMetadataExtractor(m_catalog.Metadata);
            var parser = new XmlNodeExtractor()
                .On("ServiceIdentification", metadataParser)
                .On("Service", metadataParser);

            parser.Parse(capabilities.DocumentElement);
        }

        private string RetrieveVersion(XmlNode capabilities)
        {
            if (capabilities == null)
            {
                return m_url.Version;
            }
            var version = capabilities.Attributes != null ? capabilities.Attributes["version"] : null;
            return version != null ? version.Value : m_url.Version;
        }

        /// <summary>
        /// Retrieve the data sets on this server.
        /// </summary>
        /// <returns>A list of data sets.</returns>
        protected void RetrieveDataSets(XmlDocument capabilities)
        {
            var features = capabilities.SelectNodes("//*[local-name()='FeatureType']");
            if (features == null)
            {
                return;
            }

            // Build our list of data sets being offered, along with their properties. 
            foreach (XmlNode feature in features)
            {
                // Each data set is represented by its URL and Metadata
                var dataSetUrl = new OgcWebFeatureUrl(m_url.ServerUrl)
                {
                    Request = null
                };

                bool GeospatialDataset = true;
                string defaultCRS = null;
                var otherCRS = new List<string>();
                BoundingBox dataSetBoundingBox = null;

                var dataSetMetadata = new SimpleMetadata();
                var metadataExtractor = new OgcMetadataExtractor(dataSetMetadata);
                metadataExtractor.On("name", (node) => dataSetUrl.Name = node.InnerText);
                metadataExtractor.On("SRS", (node) => defaultCRS = node.InnerText);
                metadataExtractor.On("DefaultSRS", (node) => defaultCRS = node.InnerText);
                metadataExtractor.On("DefaultCRS", (node) => defaultCRS = node.InnerText);
                metadataExtractor.On("OtherCRS", (node) => otherCRS.Add(node.InnerText));
                metadataExtractor.On("OtherSRS", (node) => otherCRS.Add(node.InnerText));
                metadataExtractor.On("NoCRS", (node) => GeospatialDataset = false);
                metadataExtractor.On("WGS84BoundingBox", (node) => dataSetBoundingBox = ExtractWgs84BoundingBox(node));
                metadataExtractor.On("LatLongBoundingBox", (node) => dataSetBoundingBox = ExtractLatLongBoundingBox(node));
                metadataExtractor.Parse(feature);

                Stats["Layers"]++;

                if (!defaultCRS.HasContent())
                {
                    Stats["NoDefaultCRS"]++;
                }
                if (!GeospatialDataset)
                {
                    Stats["NoneGeospatial"]++;
                }

                //try to use laer id as name
                if (dataSetUrl.Name.HasContent() && !dataSetMetadata.Name.HasContent())
                {
                    dataSetMetadata.Name = dataSetUrl.Name.Replace("_", " ");
                }

                // Verify that the data set is complete (header case)
                if (GeospatialDataset && dataSetUrl.Name.HasContent() && dataSetMetadata.Name.HasContent())
                {
                    var dataSet = new DataSet
                    {
                        Layer = dataSetUrl.Name,
                        Uri = dataSetUrl.ToString(),
                        Metadata = dataSetMetadata,
                        BBox = new List<BoundingBox>() { dataSetBoundingBox }
                    };
                    m_catalog.DataSets.Add(dataSet);
                }
                else
                {
                    if (!dataSetUrl.Name.HasContent())
                    {
                        Stats["NoName"]++;
                    }
                    else if (!dataSetMetadata.Name.HasContent())
                    {
                        Stats["NoTitle"]++;
                    }
                }
            }
        }

        private BoundingBox ExtractLatLongBoundingBox(XmlNode node)
        {
            try
            {
                var minx = XmlUtils.SafeAttributeValue(node.Attributes.GetNamedItem("minx") as XmlAttribute);
                var miny = XmlUtils.SafeAttributeValue(node.Attributes.GetNamedItem("miny") as XmlAttribute);
                var maxx = XmlUtils.SafeAttributeValue(node.Attributes.GetNamedItem("maxx") as XmlAttribute);
                var maxy = XmlUtils.SafeAttributeValue(node.Attributes.GetNamedItem("maxy") as XmlAttribute);
                return new BoundingBox()
                {
                    LowerLeft = new BoundingBoxCorner
                    {
                        X = double.Parse(minx),
                        Y = double.Parse(miny)
                    },
                    UpperRight = new BoundingBoxCorner
                    {
                        X = double.Parse(maxx),
                        Y = double.Parse(maxy)
                    }
                };
            }
            catch (Exception)
            {
                return null;
            }
        }

        private BoundingBox ExtractWgs84BoundingBox(XmlNode node)
        {
            try
            {
                var lowerCornerNode = node.ChildNodes.Cast<XmlNode>().FirstOrDefault(x => x.LocalName.ToLower() == "lowercorner");
                var upperCornerNode = node.ChildNodes.Cast<XmlNode>().FirstOrDefault(x => x.LocalName.ToLower() == "uppercorner");
                var lowerCorner = lowerCornerNode.InnerText.Split(new[] {' '}, StringSplitOptions.RemoveEmptyEntries);
                var upperCorner = upperCornerNode.InnerText.Split(new[] {' '}, StringSplitOptions.RemoveEmptyEntries);

                return new BoundingBox()
                {
                    LowerLeft = new BoundingBoxCorner
                    {
                        X = double.Parse(lowerCorner[0]),
                        Y = double.Parse(lowerCorner[1])
                    },
                    UpperRight = new BoundingBoxCorner
                    {
                        X = double.Parse(upperCorner[0]),
                        Y = double.Parse(upperCorner[1])
                    }
                };
            }
            catch (Exception)
            {
                return null;
            }
        }
    }
}