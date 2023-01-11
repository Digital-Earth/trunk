using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;
using Pyxis.IO.Sources.Remote;
using File = System.IO.File;

namespace Pyxis.IO.Sources.Socrata
{
    public class SocrataDataSetImportService : IDataSetImportService
    {
        public DataSet DataSet { get; set; }
        public IPermit Permit { get; set; }

        public SocrataDataSetImportService(DataSet dataSet, IPermit permit) 
        {
            DataSet = dataSet;
            Permit = permit;
        }

        public IProcess_SPtr BuildPipeline(Engine engine, IImportSettingProvider settingsProvider)
        {
            var dataSetMetadata = WebRequestHelper.GetJson<SocrataHelper.SocrataMetadata>(DataSet.Uri,"*/*");

            var localDataSet = new DataSet(DataSet);

            Action<string> postProcessingLocalFile = null;

            //convert dataset uri to the download url for this dataset.
            if (dataSetMetadata.metadata.ContainsKey("geo"))
            {
                dynamic geo = dataSetMetadata.metadata["geo"];
                string downloadUrl = geo["owsUrl"];
                if (downloadUrl.HasContent())
                {
                    downloadUrl = new Uri(DataSet.Uri).GetLeftPart(UriPartial.Authority) + downloadUrl +
                                  "?method=export&format=geojson";

                    localDataSet.Uri = downloadUrl;
                }
            }
            else
            {
                List<Action<Feature,Dictionary<string,object>>> importActions = new List<Action<Feature,Dictionary<string,object>>>();

                foreach (var column in dataSetMetadata.columns.OrderBy(column => column.position))
                {
                    var field = column.fieldName;
                    Action<Feature, Dictionary<string, object>> action ;

                    switch (SocrataHelper.ParseDataTypeName(column.dataTypeName))
                    {
                        case SocrataHelper.FieldType.Location:
                            action = (feature, row) =>
                            {
                                if (!row.ContainsKey(field))
                                {
                                    return;
                                }

                                dynamic location = row[field];
                                if (location.latitude == null || location.longitude == null)
                                {
                                    return;
                                }

                                var lat = double.Parse(location.latitude.ToString());
                                var lon = double.Parse(location.longitude.ToString());

                                var geom = new PointGeometry()
                                {
                                    Coordinates = GeographicPosition.FromWgs84LatLon(lat, lon)
                                };

                                feature.Geometry = geom;
                            };
                            break;

                        case SocrataHelper.FieldType.String:
                            action = (feature, row) =>
                            {
                                if (row.ContainsKey(field))
                                {
                                    feature.Properties[field] = row[field].ToString();
                                }
                            };
                            break;

                        case SocrataHelper.FieldType.Url:
                            //return as { url: "http://..." } -> we are extarcting the url value as string.
                            action = (feature, row) =>
                            {
                                if (row.ContainsKey(field))
                                {
                                    dynamic url = row[field];
                                    feature.Properties[field] =  url.url.ToString();
                                }
                            };
                            break;

                        case SocrataHelper.FieldType.Number:
                            action = (feature, row) =>
                            {
                                if (row.ContainsKey(field))
                                {
                                    feature.Properties[field] = double.Parse(row[field].ToString());
                                }
                            };
                            break;
                        default:
                            throw new Exception("unknown field type");
                    }

                    importActions.Add(action);
                }

                //convert into download method
                localDataSet.Uri = localDataSet.Uri.Replace("/api/views/", "/resource/") + ".json";

                postProcessingLocalFile = (file) =>
                {
                    var rows = JsonConvert.DeserializeObject<List<Dictionary<string, object>>>(File.ReadAllText(file));

                    var featuresCollection = new FeatureCollection() { Features = new List<Feature>() };

                    var id = 0;
                    foreach (var row in rows)
                    {
                        var currentRow = row;
                        var feature = new Feature()
                        {
                            Id = id.ToString()
                        };

                        importActions.ForEach(action => action(feature, currentRow));

                        if (feature.Geometry != null)
                        {
                            featuresCollection.Features.Add(feature);    
                        }
                    }

                    File.WriteAllText(file, JsonConvert.SerializeObject(featuresCollection));
                };
            }

            var downloadService = new UrlImportService(localDataSet, Permit, "json");

            if (postProcessingLocalFile != null)
            {
                downloadService.PostProcessingLocalFile += (e, file) => postProcessingLocalFile(file);
            }

            return downloadService.BuildPipeline(engine, settingsProvider);
        }

        public void EnrichGeoSource(Engine engine, GeoSource geoSource)
        {
            //make sure we have a good style setup for icons.
            if (geoSource.Specification.OutputType == PipelineSpecification.PipelineOutputType.Feature)
            {
                geoSource.Style = engine.CreateDefaultStyle(geoSource);
            }
        }
    }
}