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
using Pyxis.Core;
using Pyxis.IO.DataDiscovery;

namespace Pyxis.IO.Sources.ArcGIS
{
    public class ArcGISDiscoveryService : IDiscoveryService
    {
        private static TimeSpan TimeoutForShortRequests = TimeSpan.FromMinutes(2);
        private static TimeSpan TimeoutForLongRequests = TimeSpan.FromMinutes(5);

        public string ServiceIdentifier
        {
            get { return "ArcGIS"; }
        }

        public bool IsUriSupported(string uri)
        {
            return ArcGISGeoServicesHelper.IsUriSupported(uri);
        }

        public async Task<DiscoveryResult> DiscoverAsync(IDiscoveryContext context)
        {
            if (!IsUriSupported(context.Request.Uri))
            {
                return null;
            }

            var leads = new List<DiscoveryRequest>();

            var uri = new Uri(context.Request.Uri);

            if (uri.IsPath("**/rest/services/**/MapServer|ImageServer|FeatureServer/*"))
            {
                return await DiscoverLayerAsync(context, uri);
            }
            else if (uri.IsPath("**/rest/services/**/MapServer|ImageServer|FeatureServer"))
            {
                return await DiscoverServiceAsync(context, uri);
            }
            else if (uri.IsPath("**/rest/services/**"))
            {
                var respone = (DiscoveryHttpResult)await context.SendRequestAsync(new DiscoveryHttpRequest()
                {
                    Uri = new Uri(uri, "?f=json").ToString(),
                    Permit = context.Request.Permit,
                    Method = WebRequestMethods.Http.Get,
                    Timeout = TimeoutForShortRequests
                });

                var catalogDescription =
                    JsonConvert.DeserializeObject<ArcGISGeoServicesHelper.ArcGISCatalog>(respone.Content);

                var rootUrl = uri.ToString();
                var rootEnd = "/rest/services";
                var rootLength = rootUrl.IndexOf(rootEnd) + rootEnd.Length;
                rootUrl = rootUrl.Substring(0, rootLength);


                if (catalogDescription.services != null)
                {
                    foreach (var service in catalogDescription.services)
                    {
                        leads.Add(new DiscoveryRequest()
                        {
                            Uri = rootUrl + "/" + service.name + "/" + service.type,
                            Permit = context.Request.Permit,
                            ServiceIdentifier = ServiceIdentifier
                        });
                    }
                }

                if (catalogDescription.folders != null)
                {
                    foreach (var folder in catalogDescription.folders)
                    {
                        leads.Add(new DiscoveryRequest()
                        {
                            Uri = rootUrl + "/" + folder,
                            Permit = context.Request.Permit,
                            ServiceIdentifier = ServiceIdentifier
                        });
                    }
                }

                return new DiscoveryResult()
                {
                    Uri = uri.ToString(),
                    Metadata = new SimpleMetadata()
                    {
                        Name = "ArcGIS GeoServices",
                        Description = uri.ToString()
                    },
                    Leads = leads.Count > 0 ? leads : null,
                    ServiceIdentifier = ServiceIdentifier
                };
            }
            return null;
        }

        private async Task<DiscoveryResult> DiscoverLayerAsync(IDiscoveryContext context, Uri uri)
        {
            var respone = (DiscoveryHttpResult)await context.SendRequestAsync(new DiscoveryHttpRequest()
            {
                Uri = new Uri(uri, "?f=json").ToString(),
                Permit = context.Request.Permit,
                Method = WebRequestMethods.Http.Get,
                Timeout = TimeoutForLongRequests
            });

            var layerDescription = JsonConvert.DeserializeObject<ArcGISGeoServicesHelper.ArcGISLayerDescription>(respone.Content);

            // exclude non-spatial layers
            if (layerDescription.type != null && layerDescription.type.ToUpper() == "TABLE")
            {
                return null;
            }

            // TODO: we need to fix the importing for ESRI large geojson results by avoding pagination
            // exclude layers that do not support pagination (required by GDAL GeoJSON driver)
            //if (ArcGISGeoServicesHelper.NoPaginationSupport(layerDescription))
            //{
            //    return null;
            //}

            var serviceType = ArcGISGeoServicesHelper.GetServiceTypeFromUrl(uri);

            // exclude non-queryable layers from non-dynamic MapServers
            if (layerDescription.capabilities == null ||
                !layerDescription.capabilities.ToUpper().Contains("QUERY"))
            {
                return null;
            }

            var specification = CreateSpecification(layerDescription);

            if (specification == null)
            {
                return null;
            }

            var dataSet = new DataSet
            {
                Layer = layerDescription.id.ToString(),
                Metadata =
                {
                    // <layer name> : <folder> / <service name>
                    Name = layerDescription.name,
                    Description = layerDescription.description ?? ""
                },
                Fields = layerDescription.fields != null ? layerDescription.fields.Select(x => x.name).ToList() : null,
                Specification = specification,
            };

            var bbox = ArcGISGeoServicesHelper.GetBoundingBox(layerDescription.extent);
            if (bbox != null)
            {
                dataSet.BBox = new List<BoundingBox>() { bbox };
            }

            if (serviceType == "FeatureServer" ||
                (layerDescription.type != null &&
                 layerDescription.type.ToUpper() == "FEATURE LAYER" &&
                 layerDescription.capabilities != null &&
                 layerDescription.capabilities.ToUpper().Contains("QUERY")))
            {
                var fieldId =
                    layerDescription.fields.Where(
                            field => field.type == ArcGISGeoServicesHelper.ArcGISFieldTypes.esriFieldTypeOID.ToString())
                        .Select(x => x.name)
                        .FirstOrDefault();

                if (!fieldId.HasContent())
                {
                    return null;
                }

                // a FeatureServer layer or a MapServer feature layer with query capabilities
                dataSet.Uri = ArcGISGeoServicesHelper.AsJsonFeatureRequest(uri.ToString(), fieldId);
                var responeCount = (DiscoveryHttpResult)await context.SendRequestAsync(new DiscoveryHttpRequest()
                {
                    Uri = ArcGISGeoServicesHelper.AsFeatureCountRequest(uri.ToString(), fieldId),
                    Permit = context.Request.Permit,
                    Method = WebRequestMethods.Http.Get,
                    Timeout = TimeoutForLongRequests
                });

                var count = JsonConvert.DeserializeObject<ArcGISGeoServicesHelper.ArcGISCount>(responeCount.Content);

                dataSet.DiscoveryReport = new DataSetDiscoveryReport()
                {
                    FeaturesCount = count.count,
                };

                ArcGISGeoServicesHelper.ArcGISGeometryType geometryType;
                if (Enum.TryParse(layerDescription.geometryType, true, out geometryType))
                {
                    switch (geometryType)
                    {
                        case ArcGISGeoServicesHelper.ArcGISGeometryType.esriGeometryPoint:
                            dataSet.DiscoveryReport.GeometryType = "Point";
                            break;
                        case ArcGISGeoServicesHelper.ArcGISGeometryType.esriGeometryPolyline:
                            dataSet.DiscoveryReport.GeometryType = "Line";
                            break;

                        case ArcGISGeoServicesHelper.ArcGISGeometryType.esriGeometryPolygon:
                            dataSet.DiscoveryReport.GeometryType = "Polygon";
                            break;
                    }
                }

                dataSet.Layer = layerDescription.id.ToString();
            }
            else if (serviceType == "MapServer")
            {
                dataSet.Uri = uri.ToString();
                dataSet.Layer = layerDescription.id.ToString();
            }

            return new DiscoveryResult(context.Request)
            {
                DataSet = dataSet,
                ServiceIdentifier = ServiceIdentifier
            };
        }

        private PipelineSpecification CreateSpecification(ArcGISGeoServicesHelper.ArcGISLayerDescription layerDescription)
        {
            if (layerDescription.type.ToUpper() == "FEATURE LAYER")
            {
                var result = new PipelineSpecification()
                {
                    OutputType = PipelineSpecification.PipelineOutputType.Feature,
                };

                if (layerDescription.fields != null)
                {
                    result.Fields = layerDescription.fields.Select(ConvertToFieldSpec).ToList();
                }
                return result;
            }
            else if (layerDescription.type.ToUpper() == "RASTER LAYER")
            {
                var result = new PipelineSpecification()
                {
                    OutputType = PipelineSpecification.PipelineOutputType.Coverage,
                };

                if (layerDescription.fields != null)
                {
                    result.Fields = layerDescription.fields.Select(ConvertToFieldSpec).ToList();
                }
                else
                {
                    result.Fields = new List<PipelineSpecification.FieldSpecification> { new PipelineSpecification.FieldSpecification()
                    {
                        Name = "RGB",
                        FieldType = PipelineSpecification.FieldType.Color,
                        Metadata = new SimpleMetadata()
                        {
                            Name = "RGB"
                        }
                    }};
                }
                return result;
            }
            else if (layerDescription.type.ToUpper() == "GROUP LAYER")
            {

            }
            return null;
        }

        private PipelineSpecification.FieldSpecification ConvertToFieldSpec(ArcGISGeoServicesHelper.ArcGISField arcGISField)
        {


            var field = new PipelineSpecification.FieldSpecification()
            {
                Name = arcGISField.name,
                Metadata = new SimpleMetadata()
                {
                    Name = arcGISField.alias ?? arcGISField.name,
                    Description = ""
                },
                FieldType = PipelineSpecification.FieldType.String
            };

            ArcGISGeoServicesHelper.ArcGISFieldTypes fieldType;

            if (Enum.TryParse(arcGISField.type, true, out fieldType))
            {
                switch (fieldType)
                {
                    case ArcGISGeoServicesHelper.ArcGISFieldTypes.esriFieldTypeDouble:
                    case ArcGISGeoServicesHelper.ArcGISFieldTypes.esriFieldTypeSingle:
                    case ArcGISGeoServicesHelper.ArcGISFieldTypes.esriFieldTypeInteger:
                    case ArcGISGeoServicesHelper.ArcGISFieldTypes.esriFieldTypeSmallInteger:
                        field.FieldType = PipelineSpecification.FieldType.Number;
                        break;

                    case ArcGISGeoServicesHelper.ArcGISFieldTypes.esriFieldTypeRaster:
                        field.FieldType = PipelineSpecification.FieldType.Color;
                        break;
                }
            }

            if (arcGISField.domain != null)
            {
                if (arcGISField.domain.codedValues != null)
                {
                    field.ValueTranslation = arcGISField.domain.codedValues.ToDictionary(x => x.code.ToString(), x => x.name);
                }
            }

            return field;
        }

        private async Task<DiscoveryResult> DiscoverServiceAsync(IDiscoveryContext context, Uri uri)
        {
            var respone = (DiscoveryHttpResult)await context.SendRequestAsync(new DiscoveryHttpRequest()
            {
                Uri = new Uri(uri, "?f=json").ToString(),
                Permit = context.Request.Permit,
                Method = WebRequestMethods.Http.Get,
                Timeout = TimeoutForShortRequests
            });

            var serviceDescription =
                JsonConvert.DeserializeObject<ArcGISGeoServicesHelper.ArcGISServiceDescription>(respone.Content);

            var serviceType = ArcGISGeoServicesHelper.GetServiceTypeFromUrl(uri);
            var serviceName = ArcGISGeoServicesHelper.GetServiceNameFromUrl(uri);

            if (serviceType == "ImageServer")
            {
                // the MapServer does not support dynamic layers, we have no control over the layers that are displayed
                var dataSet = new DataSet
                {
                    Uri = uri.ToString(),
                    Layer = "",
                    Metadata =
                    {
                        // <layer name> : <folder> / <service name>
                        Name = serviceName,
                        Description = serviceDescription.serviceDescription
                    },
                    BBox = new List<BoundingBox>() { ArcGISGeoServicesHelper.GetBoundingBox(serviceDescription.initialExtent) },
                    Specification = new PipelineSpecification()
                    {
                        OutputType = PipelineSpecification.PipelineOutputType.Coverage,
                        Fields = new List<PipelineSpecification.FieldSpecification>()
                        {
                            new PipelineSpecification.FieldSpecification()
                            {
                                FieldType = PipelineSpecification.FieldType.Color,
                                Name = "RGB",
                                Metadata = new SimpleMetadata()
                                {
                                    Name = "RGB"
                                }
                            }
                        }
                    },
                    Fields = new List<string>() { "RGB" }
                };

                return new DiscoveryResult(context.Request)
                {
                    DataSet = dataSet,
                    ServiceIdentifier = ServiceIdentifier
                };
            }

            if (serviceDescription.layers != null)
            {
                var leads = 
                    serviceDescription.layers
                        .Select(layer => new DiscoveryRequest()
                        {
                            Uri = uri.ToString() + "/" + layer.id,
                            Permit = context.Request.Permit,
                            ServiceIdentifier = ServiceIdentifier
                        }).ToList();

                return new DiscoveryResult()
                {
                    ServiceIdentifier = ServiceIdentifier,
                    Metadata = new SimpleMetadata()
                    {
                        Name = serviceName,
                        Description = serviceDescription.serviceDescription,
                    },
                    Leads = leads.Count > 0 ? leads : null
                };
            }
            else if (serviceType == "MapServer")
            {
                // the MapServer does not support dynamic layers, we have no control over the layers that are displayed
                var dataSet = new DataSet
                {
                    Uri = uri.ToString(),
                    Layer = "",
                    Metadata =
                    {
                        // <layer name> : <folder> / <service name>
                        Name = serviceName,
                        Description = serviceDescription.serviceDescription
                    },
                    BBox = new List<BoundingBox>() { ArcGISGeoServicesHelper.GetBoundingBox(serviceDescription.initialExtent) },
                    Specification = new PipelineSpecification()
                    {
                        OutputType = PipelineSpecification.PipelineOutputType.Coverage,
                        Fields = new List<PipelineSpecification.FieldSpecification>()
                        {
                            new PipelineSpecification.FieldSpecification()
                            {
                                FieldType = PipelineSpecification.FieldType.Color,
                                Name = "RGB",
                                Metadata = new SimpleMetadata()
                                {
                                    Name = "RGB"
                                }
                            }
                        }
                    },
                    Fields = new List<string>() { "RGB" }
                };

                return new DiscoveryResult(context.Request)
                {
                    DataSet = dataSet,
                    ServiceIdentifier = ServiceIdentifier
                };
            }
            return null;
        }

        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            return new ArcGISDataSetImportService(dataSet, permit);
        }
    }
}