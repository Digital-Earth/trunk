using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Threading.Tasks;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.DataDiscovery;

namespace Pyxis.IO.Sources.Socrata
{
    public class SocrataDiscoveryService : IDiscoveryService
    {
        private const string DiscoverUrl = "http://api.us.socrata.com/api/catalog/v1/domains";

        public string ServiceIdentifier
        {
            get { return "Socrata"; }
        }

        public bool IsUriSupported(string uri)
        {
            Uri parsedUri;
            if (Uri.TryCreate(uri, UriKind.Absolute, out parsedUri))
            {
                return parsedUri.IsPath("**/api/views/**") || uri == DiscoverUrl;
            }
            return false;
        }

        public async Task<DiscoveryResult> DiscoverAsync(IDiscoveryContext context)
        {
            if (!IsUriSupported(context.Request.Uri))
            {
                return null;
            }

            //TODO: fix the https issue: https://stackoverflow.com/questions/40921890/socrata-an-existing-connection-was-forcibly-closed-by-the-remote-host
            var uri = new Uri(context.Request.Uri.Replace("https://","http://"));

            var response = (DiscoveryHttpResult) await context.SendRequestAsync(new DiscoveryHttpRequest
            {
                Uri = uri.ToString(),
                Permit = context.Request.Permit,
                Method = WebRequestMethods.Http.Get,
                Timeout = TimeSpan.FromMinutes(5)
            });

            //verify we have a the socrata header
            if (!response.Headers.Get("X-Socrata-Region").HasContent())
            {
                return null;
            }

            if (context.Request.Uri == DiscoverUrl)
            {
                return DiscoverServersUsingDiscoveryAPI(context, response);
            }
            else if (uri.IsPath("**/api/views"))
            {
                return DiscoverServerAsync(context, response);
            }

            return await DiscoverSingleDataSetAsync(context, response);
        }

        private DiscoveryResult DiscoverServersUsingDiscoveryAPI(IDiscoveryContext context, DiscoveryHttpResult response)
        {
            var result = JsonConvert.DeserializeObject<SocrataHelper.SocrataDiscoveryAPI>(response.Content);
            var domainList = result.results;

            if (domainList == null)
            {
                return null;
            }

            return new DiscoveryResult
            {
                Uri = context.Request.Uri,
                LastUpdated = DateTime.Now,
                AdditionalRoots = domainList.Select(domain => new DiscoveryRequest() { Uri = CreateSocrataUriFromDomain(domain) }).ToList(),
                ServiceIdentifier = ServiceIdentifier
            };
        }

        private string CreateSocrataUriFromDomain(SocrataHelper.DiscoveryAPICountBy domain)
        {
            return "http://" + domain.domain + "/api/views/";
        }

        private async Task<DiscoveryResult> DiscoverSingleDataSetAsync(IDiscoveryContext context, DiscoveryHttpResult response)
        {
            var dataSetMetadata = JsonConvert.DeserializeObject<SocrataHelper.SocrataMetadata>(response.Content);

            if (dataSetMetadata == null)
            {
                return null;
            }

            if (dataSetMetadata.viewType == "geo")
            {
                var dataSet = new DataSet
                {
                    Uri = context.Request.Uri,
                    Metadata = new SimpleMetadata
                    {
                        Name = dataSetMetadata.name,
                        Description = dataSetMetadata.description
                    }
                };

                if (dataSetMetadata.childViews != null)
                {
                    var columnsResponse = (DiscoveryHttpResult)await context.SendRequestAsync(new DiscoveryHttpRequest
                    {
                        Uri = new Uri(context.Request.Uri).GetLeftPart(UriPartial.Authority) + "/api/views/" + dataSetMetadata.childViews[0],
                        Permit = context.Request.Permit,
                        Method = WebRequestMethods.Http.Get,
                        Timeout = TimeSpan.FromMinutes(5)
                    });

                    var columnsMetadata = JsonConvert.DeserializeObject<SocrataHelper.SocrataMetadata>(columnsResponse.Content);

                    if (columnsMetadata != null && columnsMetadata.columns != null)
                    {
                        var spec = SocrataHelper.CreateSpecification(columnsMetadata);

                        dataSet.Specification = spec;
                        dataSet.Fields = spec.Fields.Select(field => field.Name).ToList();
                    }
                }

                if (dataSetMetadata.metadata.ContainsKey("geo"))
                {
                    dynamic bboxData = dataSetMetadata.metadata["geo"];
                    string bbox = bboxData.bbox;
                    string srs = bboxData.bboxCrs;

                    if (srs.StartsWith("EPSG:"))
                    {
                        srs = srs.Replace("EPSG:", "");
                    }

                    var latlons = bbox.Split(',').Select(double.Parse).ToList();

                    dataSet.BBox = new List<BoundingBox>
                    {
                        new BoundingBox()
                        {
                            LowerLeft = new BoundingBoxCorner(latlons[0],latlons[1]),
                            UpperRight = new BoundingBoxCorner(latlons[2],latlons[3]),
                            Srs = srs
                        }
                    };
                }

                return new DiscoveryResult
                {
                    Uri = context.Request.Uri,
                    Metadata = new SimpleMetadata
                    {
                        Name = dataSetMetadata.name,
                        Description = dataSetMetadata.description
                    },
                    DataSet = dataSet,
                    LastUpdated = SocrataHelper.FromUnixTime(dataSetMetadata.rowsUpdatedAt),
                    ServiceIdentifier = ServiceIdentifier
                };
            }
            if (dataSetMetadata.viewType == "tabular" && dataSetMetadata.columns != null && dataSetMetadata.columns.Any(x => SocrataHelper.ParseDataTypeName(x.dataTypeName) == SocrataHelper.FieldType.Location))
            {
                var dataSet = new DataSet
                {
                    Uri = context.Request.Uri,
                    Metadata = new SimpleMetadata
                    {
                        Name = dataSetMetadata.name,
                        Description = dataSetMetadata.description
                    }
                };

                var spec = SocrataHelper.CreateSpecification(dataSetMetadata);

                dataSet.Specification = spec;
                dataSet.Fields = spec.Fields.Select(field => field.Name).ToList();

                return new DiscoveryResult
                {
                    Uri = context.Request.Uri,
                    Metadata = new SimpleMetadata
                    {
                        Name = dataSetMetadata.name,
                        Description = dataSetMetadata.description
                    },
                    DataSet = dataSet,
                    LastUpdated = SocrataHelper.FromUnixTime(dataSetMetadata.rowsUpdatedAt),
                    ServiceIdentifier = ServiceIdentifier
                };
            }

            return null;
        }

        private DiscoveryResult DiscoverServerAsync(IDiscoveryContext context, DiscoveryHttpResult response)
        {
            var dataSets = JsonConvert.DeserializeObject<List<SocrataHelper.SocrataMetadata>>(response.Content);

            var rootUri = context.Request.Uri.TrimEnd('/');

            return new DiscoveryResult
            {
                Uri = context.Request.Uri,
                Metadata = new SimpleMetadata
                {
                    Name = "Socrata Server",
                    Description = context.Request.Uri
                },
                Leads = dataSets.Where(x => x.viewType == "geo" || x.viewType == "tabular").Select(x => new DiscoveryRequest
                {
                    Uri = rootUri + "/" + x.id,
                    Permit = context.Request.Permit,
                    ServiceIdentifier = ServiceIdentifier
                }).ToList(),
                ServiceIdentifier = ServiceIdentifier
            };
        }

        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            return new SocrataDataSetImportService(dataSet,permit);
        }
    }
}