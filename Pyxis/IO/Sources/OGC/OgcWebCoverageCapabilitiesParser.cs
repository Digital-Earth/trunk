using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.Sources.OGC
{
    public class OgcWebCoverageCapabilitiesParser
    {
        private readonly IOgcUrl m_url;
        private readonly DataSetCatalog m_catalog;

        /// <summary>
        /// Provide stats about parsing issues, errors or fixes been done on the xml document
        /// </summary>
        public Dictionary<string, int> Stats { get; set; }

        public OgcWebCoverageCapabilitiesParser(IOgcUrl url, DataSetCatalog catalog)
        {
            m_url = url;
            m_catalog = catalog;

            Stats = new Dictionary<string, int>()
            {
                {"Layers", 0},
                {"NoName", 0},
                {"NoTitle", 0},
                {"EO", 0},
                {"FieldsWithoutNames", 0},
                {"MultiDomains", 0}
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

            Stats[m_url.Version] = 1;

            var coverageParser = new XmlNodeExtractor()
                .On("Contents", new XmlNodeExtractor()
                    .On("CoverageSummary", ExtractCoverage)
                    .On("Extension", new XmlNodeExtractor("DatasetSeriesSummary", ExtractDatasetSeriesSummary)))
                .On("ContentMetadata", new XmlNodeExtractor("CoverageOfferingBrief", ExtractCoverageOfferingBrief))
                //DescribeCoverages results
                .On("CoverageDescription", ExtractCoverage)
                //DescribeEoCoverageSet
                .On("CoverageDescriptions", new XmlNodeExtractor("CoverageDescription", ExtractCoverage));
            coverageParser.Parse(capabilities.DocumentElement);
        }

        private void ExtractDatasetSeriesSummary(XmlNode dataSeries)
        {
            Stats["EO"]++;

            string id = null;
            new XmlNodeExtractor("DatasetSeriesId", (node) => id = node.InnerText).Parse(dataSeries);
            if (id.HasContent())
            {
                var url = new OgcWebCoverageUrl(m_url.ToString())
                {
                    Request = "DescribeEOCoverageSet",
                    EoId = id
                };

                var subCatalog = new DataSetCatalog()
                {
                    DataType = DataSetType.OGC.ToString(),
                    Metadata = new SimpleMetadata()
                    {
                        Name = id
                    },
                    Uri = url.ToString()
                };

                m_catalog.SubCatalogs.Add(subCatalog);
            }

        }


        private void RetrieveMetadata(XmlDocument capabilities)
        {
            var metadataParser = new OgcMetadataExtractor(m_catalog.Metadata);
            var parser = new XmlNodeExtractor()
                .On("ServiceIdentification", metadataParser)
                .On("Service", new XmlNodeExtractor()
                        .On("Label", (node) => m_catalog.Metadata.Name = node.InnerText)
                        .On("Name", (node) =>
                        {
                            if (!m_catalog.Metadata.Name.HasContent())
                            {
                                m_catalog.Metadata.Name = node.InnerText;
                            }
                        })
                        .On("Description", (node) => m_catalog.Metadata.Description = node.InnerText)
                        .On("Keywords", metadataParser.PopulateTags)
                );

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

        private void ExtractCoverageOfferingBrief(XmlNode coverage)
        {
            Stats["Layers"]++;

            var dataSetUrl = new OgcWebCoverageUrl(m_url.ServerUrl)
            {
                Request = null,
                Version = m_url.Version,
                //ensure we remove EoId from datasetUrl
                EoId = null
            };

            var dataSetMetadata = new SimpleMetadata();
            BoundingBox dataSetBoundingBox = null;

            var metadataExtractor = new XmlNodeExtractor()
                .On("name", node => dataSetUrl.Name = node.InnerText)
                .On("label", node => dataSetMetadata.Name = node.InnerText)
                .On("description", node => dataSetMetadata.Description = node.InnerText)
                .On("lonLatEnvelope", node => dataSetBoundingBox = ExtractLonLatBoundingBox(node))
                .On("keywords", (node) =>
                {
                    new OgcMetadataExtractor(dataSetMetadata).PopulateTags(node);
                });


            metadataExtractor.Parse(coverage);

            //if layer has a <Name> but not <Title> or <Title /> empty element, 
            //we default to layer Name (ID) as metadata name
            if (!dataSetMetadata.Name.HasContent() && dataSetUrl.Name.HasContent())
            {
                Stats["NoTitle"]++;
                //layer names have "_" to be used as spaces. replace back to more readable name
                dataSetMetadata.Name = dataSetUrl.Name.Replace("_", " ");
            }

            // Don't add the data set if it is not complete (header case).
            if (dataSetUrl.Name.HasContent()
                && dataSetMetadata.Name.HasContent())
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
        }

        private BoundingBox ExtractLonLatBoundingBox(XmlNode node)
        {
            if (node == null)
            {
                return null;
            }

            try
            {
                if (!node.Attributes.Cast<XmlNode>().Any(n => n.Value.EndsWith("CRS84")) && !node.Attributes.Cast<XmlNode>().Any(n => n.Value.EndsWith(":4326")))
                {
                    return null;
                }

                var corners = node.ChildNodes.Cast<XmlNode>().Where(x => x.LocalName.ToLower() == "pos").ToList();
                var corner1 = corners[0].InnerText.Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries).Select(s => double.Parse(s)).ToArray();
                var corner2 = corners[1].InnerText.Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries).Select(s => double.Parse(s)).ToArray();
                return new BoundingBox
                {
                    LowerLeft = new BoundingBoxCorner
                    {
                        X = Math.Min(corner1[0], corner2[0]),
                        Y = Math.Min(corner1[1], corner2[1])
                    },
                    UpperRight = new BoundingBoxCorner
                    {
                        X = Math.Max(corner1[0], corner2[0]),
                        Y = Math.Max(corner1[1], corner2[1])
                    }
                };
            }
            catch
            {
                return null;
            }
        }

        private void ExtractCoverage(XmlNode coverage)
        {
            Stats["Layers"]++;

            var dataSetUrl = new OgcWebCoverageUrl(m_url.ServerUrl)
            {
                Request = null,
                Version = m_url.Version,
                //ensure we remove EoId from datasetUrl
                EoId = null
            };

            var dataSetMetadata = new SimpleMetadata();
            BoundingBox dataSetBoundingBox = null;

            var fields = new List<string>();
            List<Domain> domains = null;

            var metadataExtractor = new OgcMetadataExtractor(dataSetMetadata);
            metadataExtractor.On("CoverageId", node => dataSetUrl.Name = node.InnerText);
            metadataExtractor.On("Identifier", node => dataSetUrl.Name = node.InnerText);
            metadataExtractor.On("WGS84BoundingBox", (node) => dataSetBoundingBox = ExtractWgs84BoundingBox(node));
            metadataExtractor.On(new[] {"rangeType", "DataRecord", "field"},
                node => fields.Add(node.FindAttributeWithLocalName("name").SafeAttributeValue()));

            metadataExtractor.On(
                new[]
                {
                    "metadata", "Extension", "EOMetadata", "EarthObservation", "resultTime", "TimeInstant", "timePosition"
                }, 
                (node) =>
                {
                    if (domains == null)
                    {
                        domains = new List<Domain>();
                    }

                    domains.Add(new Domain
                    {
                        Name = "Time",
                        Type = PipelineSpecification.FieldType.Date,
                        Metadata = new SimpleMetadata
                        {
                            Name = "Time",
                            Description = "Result time"
                        },
                        Value = node.InnerText.Trim()
                    });    
                });

            DomainReducer reducer = null;

            metadataExtractor.On("domainSet", 
                new XmlNodeExtractor()
                    .On("Grid", (node) => reducer = ReduceTo2dIfNeeded(reducer, node))
                    .On("RectifiedGrid", (node) => reducer = ReduceTo2dIfNeeded(reducer, node)));
                
            metadataExtractor.Parse(coverage);
         
            //if layer has a <Name> but not <Title> or <Title /> empty element, 
            //we default to layer Name (ID) as metadata name
            if (!dataSetMetadata.Name.HasContent() && dataSetUrl.Name.HasContent())
            {
                Stats["NoTitle"]++;
                //layer names have "_" to be used as spaces. replace back to more readable name
                dataSetMetadata.Name = dataSetUrl.Name.Replace("_", " ");
            }

            if (fields.Any(value => !value.HasContent()))
            {
                Stats["FieldsWithoutNames"]++;

                fields = fields.Where(value => value.HasContent()).ToList();
            }

            if (fields.Count > 1)
            {
                reducer = new FieldsDomainReducer(fields, reducer);
            }

            // Don't add the data set if it is not complete (header case).
            if (dataSetUrl.Name.HasContent()
                && dataSetMetadata.Name.HasContent())
            {
                if (reducer != null)
                {
                    Stats["MultiDomains"]++;

                    var baseDataSet = new DataSet()
                    {
                        Domains = domains,
                        Uri = dataSetUrl.ToString(),
                        Layer = dataSetUrl.Name,
                        Metadata = dataSetMetadata
                    };

                    foreach (var dataSet in reducer.ReduceDataSets(new DataSet[] {baseDataSet}))
                    {
                        m_catalog.DataSets.Add(dataSet);                        
                    }
                }
                else
                {
                    var dataSet = new DataSet
                    {
                        Layer = dataSetUrl.Name,
                        Uri = dataSetUrl.ToString(),
                        Metadata = dataSetMetadata,
                        Domains = domains,
                        BBox = new List<BoundingBox>() { dataSetBoundingBox }
                    };
                    m_catalog.DataSets.Add(dataSet);
                }
            }
        }

        private BoundingBox ExtractWgs84BoundingBox(XmlNode node)
        {
            try
            {
                var lowerCornerNode = node.ChildNodes.Cast<XmlNode>().FirstOrDefault(x => x.LocalName.ToLower() == "lowercorner");
                var upperCornerNode = node.ChildNodes.Cast<XmlNode>().FirstOrDefault(x => x.LocalName.ToLower() == "uppercorner");
                var lowerCorner = lowerCornerNode.InnerText.Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                var upperCorner = upperCornerNode.InnerText.Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

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

        /// <summary>
        /// Reduce coverage dimensions by subset any axes that is not latitude longitude
        /// </summary>
        /// <param name="reducer">reducer to be chanied to this reducer</param>
        /// <param name="gridNode">Gird/RectifiedGrid XML gridNode element</param>
        private DomainReducer ReduceTo2dIfNeeded(DomainReducer reducer, XmlNode gridNode)
        {
            var low = new string[0];
            var high = new string[0];
            var labels = new string[0];
            var parser = new XmlNodeExtractor()
                .On(new[] {"limits", "GridEnvelope"}, new XmlNodeExtractor()
                    .On("low", (node) => low = node.InnerText.Trim().Split(' '))
                    .On("high", (node) => high = node.InnerText.Trim().Split(' ')))
                .On("axisLabels", (node) => labels = node.InnerText.Trim().Split(' '));

            parser.Parse(gridNode);

            var dimensions = gridNode.FindAttributeWithLocalName("dimension").SafeAttributeValue();

            if (dimensions.HasContent())
            {
                var d = int.Parse(dimensions);

                if (low.Length != d || high.Length != d || labels.Length != d)
                {
                    throw new Exception("grid domains count doesn't match gridEnvelope low,high or axisLabels count.");
                }
            }
            else
            {
                if (low.Length != high.Length || high.Length != labels.Length)
                {
                    throw new Exception("grid domains count doesn't match gridEnvelope low,high or axisLabels count.");
                }
            }

            var subsets = new Dictionary<string,string>();

            //we already 2d
            if (labels.Length == 2)
            {
                return null;
            }

            DomainReducer result = reducer;

            for(var i=0;i<labels.Length;i++)
            {
                var axis = labels[i].Trim();

                switch (axis.ToLower())
                {
                    case "longitude":
                    case "latitude":
                    case "long":
                    case "lon":
                    case "lat":
                    case "x":
                    case "y":
                        break;
                    default:
                        result = new SubsetDomainReducer(axis, int.Parse(low[i]), int.Parse(high[i]), result);
                        break;
                }
            }

            return result;
        }

        abstract class DomainReducer
        {
            protected DomainReducer(DomainReducer after = null)
            {
                ReducerSource = after;
            }

            public DomainReducer ReducerSource { get; private set; }

            public IEnumerable<DataSet> ReduceDataSets(IEnumerable<DataSet> datasets)
            {
                //reduce the previous domain
                if (ReducerSource != null)
                {
                    datasets = ReducerSource.ReduceDataSets(datasets);
                }

                foreach (var dataset in datasets)
                {
                    foreach (var value in PossibleValues)
                    {
                        var datasetCopy = new DataSet(dataset);

                        ExtendDataSet(datasetCopy, value);

                        yield return datasetCopy;
                    }
                }
            }

            protected abstract IEnumerable<string> PossibleValues { get; }

            protected abstract void ExtendDataSet(DataSet dataset, string value);
        }

        class SubsetDomainReducer : DomainReducer
        {
            public string Axis { get; set; }
            public int Low { get; set; }
            public int High { get; set; }
            public PipelineSpecification.FieldType Type { get; set; }

            public SubsetDomainReducer(string axis, int low, int high, DomainReducer after = null) : base(after)
            {
                Axis = axis;
                Low = low;
                High = high;
                Type = PipelineSpecification.FieldType.String;
            }

            protected override IEnumerable<string> PossibleValues
            {
                get { return Enumerable.Range(Low,High-Low+1).Select(x=>x.ToString()); }
            }

            protected override void ExtendDataSet(DataSet dataset, string value)
            {
                var url = new OgcWebCoverageUrl(dataset.Uri);
                
                var subset = url.Subset.ToDictionary(x => x.Key, x => x.Value);
                subset[Axis] = value;
                
                url.Subset = subset;

                dataset.Uri = url.ToString();
                if (dataset.Domains == null)
                {
                    dataset.Domains = new List<Domain>();
                }

                dataset.Domains.Add(new Domain()
                {
                   Name = Axis,
                   Value = value,
                   Values = PossibleValues.Cast<object>().ToList(),
                   Type = Type,
                   Metadata = new SimpleMetadata
                   {
                       Name = Axis,
                   }
                });
                dataset.Metadata.Name += string.Format(" ({0}={1})", Axis, value);
            }
        }


        class FieldsDomainReducer : DomainReducer
        {
            private readonly string[] m_fields;
            
            public FieldsDomainReducer(IEnumerable<string> fields, DomainReducer after = null)
                : base(after)
            {
                m_fields = fields.ToArray();
            }

            protected override IEnumerable<string> PossibleValues
            {
                get { return m_fields; }
            }

            protected override void ExtendDataSet(DataSet dataset, string value)
            {
                var url = new OgcWebCoverageUrl(dataset.Uri);

                url.RangeSubset = value;
                dataset.Uri = url.ToString();
                if (dataset.Domains == null)
                {
                    dataset.Domains = new List<Domain>();
                }

                dataset.Domains.Add(new Domain()
                {
                    Name = "Band",
                    Value = value,
                    Values = PossibleValues.Cast<object>().ToList(),
                    Type = PipelineSpecification.FieldType.String,
                    Metadata = new SimpleMetadata
                    {
                        Name = "Band",
                    }
                });

                dataset.Metadata.Name += string.Format(" ({0})", value);
            }
        }
    }
}