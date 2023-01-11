using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core.Analysis;
using Pyxis.Core.Analysis.Question;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using Pyxis.Core.Measurements;
using Pyxis.IO.Import;
using Pyxis.IO.Publish;
using Pyxis.IO.Sources.Memory;
using Pyxis.UI.Layers.Globe;
using Pyxis.WorldView.Studio.GeoTag;
using Pyxis.WorldView.Studio.Layers;
using Pyxis.WorldView.Studio.Layers.Html;
using ResourceType = Pyxis.Contract.Publishing.ResourceType;

namespace Pyxis.WorldView.Studio.JsAPI
{
    internal class EngineAPI
    {
        public ApplicationForm Form { get; private set; }

        public EngineAPI(ApplicationForm form)
        {
            Form = form;
        }

        public void Register(HtmlLayer HtmlLayer)
        {
            var engine = Form.Engine;

            HtmlLayer.RegisterProxy("engine", proxy =>
            {
                proxy.BindAsync("getSpecification", (GeoSource geoSource) =>
                {
                    return engine.GetSpecification(geoSource);
                });

                Func<GeoSource, string, FeatureCollection> safeValuePicking =
                    (geoSource, index) =>
                    {
                        switch (engine.GetSpecification(geoSource).OutputType)
                        {
                            case PipelineSpecification.PipelineOutputType.Feature:

                                //Step1: pick features that intersects the given cell/index
                                var exactPicking = engine.GetAsFeature(geoSource).GetFeatures(new CellGeometry() { Index = index }, 0, 10, FeatureExtractionFlags.Fields);

                                if (exactPicking.Features.Count > 0)
                                {
                                    return exactPicking;
                                }

                                //Step2: expand the picking area to find near by features
                                //       we create a circle geometry around the given cell, 
                                //       the radius of the circle is 5 times the cell radius.
                                var pyxIndex = new PYXIcosIndex(index);
                                var cellCircle = PYXCell.create(pyxIndex).getBoundingCircle();

                                var pickGeometry = new CircleGeometry()
                                {
                                    Coordinates = new GeographicPosition(PointLocation.fromXYZ(cellCircle.getCenter())),
                                    Radius = cellCircle.getRadius() * 5 * SphericalDistance.Radian
                                };
                                return engine.GetAsFeature(geoSource).GetFeatures(pickGeometry, 0, 10, FeatureExtractionFlags.Fields);

                            case PipelineSpecification.PipelineOutputType.Coverage:
                                return engine.GetAsCoverage(geoSource).GetValue(new CellGeometry() { Index = index });
                        }
                        return null;
                    };

                proxy.BindAsync("getFeatures", safeValuePicking);
                proxy.BindAsync("getValue", safeValuePicking);

                proxy.BindAsync("createFeatureCollection", (FeatureCollection featureCollection) =>
                {
                    return engine.CreateInMemory(featureCollection);
                });

                proxy.BindAsync("createWatershedGeometry", (string index) =>
                {
                    var watershed = Form.GlobeLayer.CalculateWatershed(new PYXIcosIndex(index));
                    return Geometry.FromPYXGeometry(watershed);
                });

                proxy.BindAsync("whereIntersection", (List<IGeometry> geometries) =>
                {
                    switch (geometries.Count)
                    {
                        case 0:
                            return null;
                        case 1:
                            return geometries[0];
                    }

                    var booleanGeometry = new BooleanGeometry()
                    {
                        Operations = geometries.Select(geometry => new BooleanGeometry.Clause()
                        {
                            Operation = BooleanGeometry.Operation.Intersection,
                            Geometry = geometry
                        }).ToList()
                    };
                    //first operation must be disjunction
                    booleanGeometry.Operations[0].Operation = BooleanGeometry.Operation.Disjunction;

                    return booleanGeometry.Calculate(engine);
                });

                proxy.BindAsync("createWatershed", (string index) =>
                {
                    var watershed = Form.GlobeLayer.CalculateWatershed(new PYXIcosIndex(index));

                    return engine.CreateInMemory(
                        new FeatureCollection()
                        {
                            Features = new List<Feature>() {
                                new Feature("1",Geometry.FromPYXGeometry(watershed),new Dictionary<string,object>())
                            }
                        });
                });

                proxy.BindAsync("getAllFeatures", (GeoSource geoSource) =>
                {
                    return engine.GetAsFeature(geoSource).GetFeatures(FeatureExtractionFlags.Fields);
                });

                proxy.BindAsync("getFieldStatistics", (GeoSource geoSource, string fieldName, int binCount) =>
                {
                    return engine.Statistics(geoSource).GetFieldStatistics(fieldName, binCount);
                });

                proxy.BindAsync("getFieldStatisticsAt", (GeoSource geoSource, string fieldName, IGeometry geometry, int binCount) =>
                {
                    return engine.Statistics(geoSource).GetFieldStatistics(geometry, fieldName, binCount);
                });

                proxy.BindAsync("getFieldValueCount", (GeoSource geoSource, string fieldName, string fieldValue) =>
                {
                    return engine.Statistics(geoSource).GetFieldStatisticsWithValue(null, fieldName, fieldValue);
                });

                proxy.BindAsync("getFieldValueCountAt", (GeoSource geoSource, string fieldName, string fieldValue, IGeometry geometry) =>
                {
                    return engine.Statistics(geoSource).GetFieldStatisticsWithValue(geometry, fieldName, fieldValue);
                });

                proxy.BindAsync("searchQuery", (FeaturesSearchQuery query) =>
                {
                    return engine.GetAsFeature(query.GeoSource).Search(query);
                });

                proxy.BindAsync("getArea", (IGeometry geometry) =>
                {
                    return geometry.GetArea(engine).InSquareMeters;
                });

                proxy.BindAsync("getDataSize", (GeoSource geoSource) =>
                {
                    var process = PipeManager.getProcess(pyxlib.strToProcRef(geoSource.ProcRef));
                    var geoPacketSource = process.GeoPacketSources().First();
                    var manifests = geoPacketSource.WalkPipelinesExcludeGeoPacketSourcesAfterParent().ExtractManifests().ToList();
                    return manifests.SelectMany(m => m.Entries).Sum(e => e.FileSize);
                });

                var jsImportSettingProvider = new JsSettingsProvider(engine);

                var importSettingRequired = proxy.Callback<DataSet, string, string>("importSettingRequired");

                List<GeoTagMethodFinder.FeatureCollectionLookup> geoTagReferences = new List<GeoTagMethodFinder.FeatureCollectionLookup>();

                jsImportSettingProvider.SettingRequested += (s, e) =>
                {
                    //notify the JS we need import settings
                    if (e.RequiredSetting == typeof(SRSImportSetting))
                    {
                        //run the callback on UI thread...
                        Form.BeginInvoke((MethodInvoker)(() =>
                        {
                            importSettingRequired(e.DataSet, "SRS", "");
                        }));
                    }

                    if (e.RequiredSetting == typeof(GeoTagImportSetting))
                    {
                        var finder = new GeoTagMethodFinder(engine, e.Args as ProvideGeoTagImportSettingArgs, geoTagReferences);
                        var geoTagOptions = finder.FindGeoTagOptions();

                        //run the callback on UI thread...
                        Form.BeginInvoke((MethodInvoker)(() =>
                        {
                            importSettingRequired(e.DataSet, "GeoTag", JsonConvert.SerializeObject(geoTagOptions));
                        }));
                    }
                };

                proxy.Bind("setGeoTagReferenceGeoSources", (List<GeoTagMethodFinder.FeatureCollectionLookup> references) =>
                {
                    geoTagReferences = references;
                    return true;
                });

                proxy.BindAsync("provideImportSetting", (DataSet dataSet, string settingType, JsonString value) =>
                {
                    if (settingType == "SRS")
                    {
                        var srs = JsonConvert.DeserializeObject<SpatialReferenceSystem>(value.ToString());

                        if (srs == null)
                        {
                            jsImportSettingProvider.SetImportSettingFailed(
                                dataSet,
                                typeof(SRSImportSetting),
                                new Exception("No SRS was provided"));
                        }
                        else
                        {
                            jsImportSettingProvider.SetImportSetting(
                                dataSet,
                                new SRSImportSetting
                                {
                                    SRS = srs
                                });
                        }
                        return true;
                    }
                    else if (settingType == "GeoTag")
                    {
                        var option = JsonConvert.DeserializeObject<GeoTagMethodFinder.GeoTagOption>(value.ToString());

                        if (option == null)
                        {
                            jsImportSettingProvider.SetImportSettingFailed(
                                dataSet,
                                typeof(GeoTagImportSetting),
                                new Exception("No GeoTag method was provided"));
                        }
                        else
                        {
                            jsImportSettingProvider.SetImportSetting(
                                dataSet,
                                new GeoTagImportSetting
                                {
                                    Method = option.CreateGeoTagMethod(engine)
                                });
                        }
                        return true;
                    }
                    else
                    {
                        throw new Exception("unsupported setting type");
                    }
                });

                proxy.BindAsync("import", (DataSet dataSet) =>
                {
                    ImportGeoSourceProgress progress = jsImportSettingProvider.GetImportAction(dataSet);
                    try
                    {
                        return progress.Task.Result;
                    }
                    catch (Exception)
                    {
                        var errorString = String.Format("Unable to import data set: {0}.", dataSet.Metadata.Name);

                        // provide detailed error message if available
                        if (progress.Task.Exception != null && progress.Task.Exception.InnerException != null)
                        {
                            errorString += " " + progress.Task.Exception.InnerException.Message;
                        }

                        throw new Exception(errorString);      
                    }
                });

                // Deprecated - for backwards compatibility with previous versions of the Studio front-end 2015-07-31
                // Use isDataSourceSupported instead.
                proxy.BindAsync("supportedImportFileFormats", () =>
                {
                    return new List<string>
                    {
                        "adf",
                        "asc",
                        "csv",
                        "ddf",
                        "dem",
                        "dt0",
                        "dt1",
                        "dt2",
                        "e00",
                        "g98",
                        "gif",
                        "gml",
                        "grb",
                        "grd",
                        "img",
                        "jpg",
                        "kml",
                        "ntf",
                        "png",
                        "ppl",
                        "shp",
                        "sid",
                        "tab",
                        "tif",
                        "tiff",
                        "tl2",
                        "toc",
                        "vrt",
                        "json",
                        "geojson"
                        // "xls",   Claimed to be supported but was broken
                        // "xlsx"   Claimed to be supported but was broken                 
                    };
                });

                proxy.BindAsync("isDataSourceSupported", (string strPath) =>
                {
                    return engine.IsDataSourceSupported(strPath, IPipeBuilder.eCheckOptions.knLenient);
                });

                proxy.BindAsync("upgradeMap", (MapWithEmbeddedResources mapResource) =>
                {
                    mapResource.Upgrade(engine);
                    return mapResource;
                });

                proxy.Bind("verifyMapSchema", (JsonString obj) =>
                {
                    var mapResource = JsonConvert.DeserializeObject<MapWithEmbeddedResources>(obj.ToString());

                    if (mapResource.Id == Guid.Empty)
                    {
                        mapResource.Id = Guid.NewGuid();
                    }
                    return mapResource;
                });

                proxy.BindAsync("publishLocally", (GeoSource geoSource) =>
                {
                    return engine.PublishLocally(geoSource);
                });

                proxy.BindAsync("unpublishLocally", (GeoSource geoSource) =>
                {
                    return engine.UnpublishLocally(geoSource);
                });

                var settingProvider = new PublishSettingProvider();
                settingProvider.Register(
                    typeof(UploadImagePublishSetting),
                    args =>
                    {
                        return Task<IPublishSetting>.Factory.StartNew(() => new UploadImagePublishSetting()
                        {
                            ImagePath = Form.ApplicationJsAPI.ImageStorage.FromResourceName((args as UploadImagePublishSettingArgs).ImageStorageUrl)
                        });
                    });

                proxy.BindAsync("publishGeoSource", (GeoSource geoSource, Style style, Gallery gallery) =>
                {
                    //apply provider
                    geoSource.Metadata.Providers = new List<Provider>
                    {
                            new Provider
                            {
                                Id = gallery.Id,
                                Name = gallery.Metadata.Name,
                                Type = ResourceType.Gallery,
                            }
                        };

                    return engine.BeginPublish(geoSource, style, settingProvider).TransactionTask.Result.PublishedGeoSource;
                });

                var publishingTasksProgress = new List<PublishGeoSourceProgress>();

                proxy.BindAsync("startPublishGeoSource", (GeoSource geoSource, Style style, Gallery gallery) =>
                {
                    //apply provider
                    geoSource.Metadata.Providers = new List<Provider>
                    {
                        new Provider
                        {
                            Id = gallery.Id,
                            Name = gallery.Metadata.Name,
                            Type = ResourceType.Gallery,
                        }
                    };

                    var publishProgress = engine.BeginPublish(geoSource, style, settingProvider);

                    publishingTasksProgress.Add(publishProgress);

                    return publishProgress.Status;
                });

                proxy.Bind("getPublishGeoSourceStatus", (GeoSource geoSource) =>
                {
                    var task = publishingTasksProgress.FirstOrDefault(p => p.ProvidedGeoSource.Id == geoSource.Id);
                    if (task != null)
                    {
                        return task.Status;
                    }
                    return null;
                });

                proxy.BindAsync("finishPublishGeoSource", (GeoSource geoSource) =>
                {
                    var task = publishingTasksProgress.First(p => p.ProvidedGeoSource.Id == geoSource.Id);

                    try
                    {
                        var result = task.TransactionTask.Result.PublishedGeoSource;

                        return result;
                    }
                    catch (Exception e)
                    {
                        var errorString = String.Format("Unable to publish.");

                        // provide detailed error message if available
                        if (e.InnerException != null)
                        {
                            errorString += " " + e.InnerException.Message;
                        }

                        throw new Exception(errorString);
                    }
                    finally
                    {
                        publishingTasksProgress.Remove(task);                       
                    }
                });

                proxy.BindAsync("publishMap", (MapWithEmbeddedResources map, Gallery gallery) =>
                {
                    //apply provider
                    map.Metadata.Providers = new List<Provider>()
                        {
                            new Provider() {
                                Id = gallery.Id,
                                Name = gallery.Metadata.Name,
                                Type = ResourceType.Gallery,
                            }
                        };

                    //create backward support before we publish the map.
                    map.Downgrade(engine);

                    return engine.BeginPublish(map, settingProvider).TransactionTask.Result.PublishedMap;
                });

                proxy.Bind("isValidQuestion", (string question) =>
                {
                    return QuestionEngine.IsValidQuestion(question);
                });

                proxy.Bind("getQuestionSuggestions", (string question) =>
                {
                    return QuestionEngine.GetQuestionSuggestions(question);
                });

                proxy.BindAsync("answerQuestion", (string question, Map map) =>
                {
                    try
                    {
                        var questionContext = QuestionContext.FromMap(map, engine);
                        var answer = new QuestionEngine(questionContext).Answer(question);
                        return answer;
                    }
                    catch (Exception e)
                    {
                        Trace.error(e.Message);
                        throw new Exception("Failed to answer the question", e);
                    }

                });

                proxy.Bind("isOgcUrl", (string url) => engine.GetImporterCategory(url) == DataSetType.OGC.ToString());

                // note: JavaScript requires that the DataSetCatalog not be null
                proxy.BindAsync("openOgcServer", (string url) => engine.BuildCatalog(url) ?? new DataSetCatalog());

                proxy.Bind("isGeoServiceUrl", (string url) => engine.GetImporterCategory(url) == DataSetType.ArcGIS.ToString());

                // note: JavaScript requires that the DataSetCatalog not be null
                proxy.BindAsync("openGeoServer", (string url) => engine.BuildCatalog(url) ?? new DataSetCatalog());

                /*
                proxy.Bind("toLocalResource", (string path) => engine.ToLocalResource(path));

                // note: JavaScript requires that the DataSetCatalog not be null
                proxy.BindAsync("openLocalResource", (string path) => engine.DiscoverCatalog(path) ?? new DataSetCatalog());

                proxy.BindAsync("scanLocalComputer", () => engine.ScanLocalComputer());
                */

                proxy.BindAsync("mosaic", (List<GeoSource> geoSources) => engine.Mosaic(geoSources));

                proxy.BindAsync("calculate", (List<GeoSource> geoSources, string expression) => engine.Calculate(geoSources, expression));                
            });
        }
    }
}
