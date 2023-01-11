using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;
using Pyxis.IO.Sources.Local;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Sources.ArcGIS
{
    internal class ArcGISDataSetImportService : IDataSetImportService
    {
        public DataSet DataSet { get; set; }
        public IPermit Permit { get; set; }

        public ArcGISDataSetImportService(DataSet dataSet, IPermit permit)
        {
            DataSet = dataSet;
            Permit = permit;
        }

        /// <summary>
        /// ValueTransfromTable instance that is detect while import the dataset.
        /// </summary>
        RasterValueTransformTable m_valueTable = null;

        /// <summary>
        /// Create a pipeline for importing a GeoSource from a data set
        /// </summary>
        /// <param name="engine">Pyxis engine to use</param>
        /// <param name="settingsProvider">Settings provider for building the pipeline</param>
        /// <returns>IProcess_SPtr object</returns>
        public IProcess_SPtr BuildPipeline(Engine engine, IImportSettingProvider settingsProvider)
        {
            if (!ArcGISGeoServicesHelper.IsUriSupported(DataSet.Uri))
            {
                return null;
            }

            switch (GetServiceFromUrl(DataSet.Uri))
            {
                case "AGSM":
                case "AGSI":
                    return BuildMapServerPipeline();
                case "AGSF":
                    return BuildFeatureServerPipeline(engine, settingsProvider);
                default:
                    // server type not recognized
                    return null;
            }
        }

        private string GetServiceFromUrl(string url)
        {
            var urlNormalized = url.ToUpper();

            // build the appropriate pipeline for the server
            if (urlNormalized.Contains("FEATURESERVER"))
            {
                return "AGSF";
            }
            else if (urlNormalized.Contains("IMAGESERVER"))
            {
                return "AGSI";
            }
            else if (urlNormalized.Contains("MAPSERVER"))
            {
                if (urlNormalized.Contains("QUERY?WHERE="))
                {
                    // this is a dynamic layer, treat as a feature server
                    return "AGSF";
                }

                return "AGSM";
            }
            return "";
        }

        private void DownloadFeaturesFromServer(string path)
        {
            var downloader = new ArcGISFeatureDownloader(this);

            var result = downloader.GetFeatureChunk(2);
            

            if (result.error != null)
            {
                throw new Exception(String.Format("Remote server error {0}: {1}", result.error.code, result.error.message));
            }

            WriteHeader(path + ".part-header", result);

            var featureCount = WriteFeatures(path + ".part-features", result);

            while (!downloader.DownloadCompleted)
            {
                Console.WriteLine("Downloaded {0} features... {1}", featureCount, CalculateDownloadProgress(featureCount));


                result = downloader.GetFeatureChunk(2);
                    
                if (result.error != null)
                {
                    throw new Exception(String.Format("Remote server error {0}: {1}", result.error.code, result.error.message));
                }

                //merge features
                featureCount += WriteFeatures(path + ".part-features", result);
            }

            MergeResults(path, path + ".part-header", path + ".part-features");

            Console.WriteLine("Downloaded {0} features, file size: {1} bytes", featureCount, (new System.IO.FileInfo(path)).Length);

            if (featureCount == 0)
            {
                throw new Exception("No features where downloaded, skipping import");
            }
        }

        private string CalculateDownloadProgress(int featureCount)
        {
            if (DataSet.DiscoveryReport == null || DataSet.DiscoveryReport.FeaturesCount == 0)
            {
                return "";
            }
            return String.Format("({0:0}%)", 100 * featureCount / DataSet.DiscoveryReport.FeaturesCount);
        }

        private void WriteHeader(string path, ArcGISGeoServicesHelper.ArcGISJsonFeatureCollection result)
        {
            var features = result.features;
            result.features = new List<ArcGISGeoServicesHelper.ArcGISJsonFeature>();
            try
            {
                System.IO.File.WriteAllText(path, JsonConvert.SerializeObject(result));
            }
            finally
            {
                result.features = features;    
            }            
        }

        private int WriteFeatures(string path, ArcGISGeoServicesHelper.ArcGISJsonFeatureCollection result)
        {
            if (result == null || result.features == null || result.features.Count == 0)
            {
                Console.WriteLine("ArcGIS respons contained no features.");
                return 0;
            }

            var feautres = result.features.Where(EnsureFeatureHasGeometry).ToList();

            var str = JsonConvert.SerializeObject(feautres);
            str = str.Substring(1, str.Length - 2); //remove leading and trailing ',';

            if (System.IO.File.Exists(path))
            {
                System.IO.File.AppendAllText(path, "," + str);
            }
            else
            {
                System.IO.File.WriteAllText(path, str);
            }

            return feautres.Count;
        }

        private void MergeResults(string path, string headerPath, string featuresPath)
        {
            var key = "\"features\"";

            using (var writeStream = new StreamWriter(System.IO.File.OpenWrite(path)))
            {
                var header = System.IO.File.ReadAllText(headerPath);

                var pos = header.IndexOf(key+":[]");

                pos += (key + ":[").Length;

                //write everything before features
                writeStream.Write(header.Substring(0,pos));

                using (var readStream = new StreamReader(System.IO.File.OpenRead(featuresPath)))
                {
                    var readCount = 0;
                    var buffer = new char[1024*1024];
                    while ((readCount = readStream.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        writeStream.Write(buffer,0,readCount);
                    }
                }

                //write the end of the header
                writeStream.Write(header.Substring(pos));
            }

            System.IO.File.Delete(headerPath);
            System.IO.File.Delete(featuresPath);
        }

        private bool EnsureFeatureHasGeometry(ArcGISGeoServicesHelper.ArcGISJsonFeature feature)
        {
            if (feature == null || feature.geometry == null)
            {
                return false;
            }

            if (feature.geometry.ContainsKey("rings"))
            {
                var rings = feature.geometry["rings"] as Newtonsoft.Json.Linq.JArray;
                if (rings != null && rings.Count == 0)
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// Create a pipeline for importing a GeoSource from a FeatureServer data set
        /// </summary>
        /// <returns>IProcess_SPtr object</returns>
        private IProcess_SPtr BuildFeatureServerPipeline(Engine engine, IImportSettingProvider settingsProvider)
        {
            var downloadLocallyTask = settingsProvider.ProvideSetting(typeof(DownloadLocallySetting),
                new ProvideImportSettingArgs());
            
            var downloadLocally = downloadLocallyTask != null
                ? downloadLocallyTask.Result as DownloadLocallySetting
                : null;

            if (downloadLocally != null)
            {
                var localFile = new Uri(DataSet.Uri).Host + "." + DateTime.Now.Ticks + ".json";

                localFile = Path.GetFullPath(Path.Combine(downloadLocally.Path, localFile));

                if (!Directory.Exists(downloadLocally.Path))
                {
                    Directory.CreateDirectory(downloadLocally.Path);
                }

                DownloadFeaturesFromServer(localFile);

                var localService = new LocalDataSetDiscoveryService();
                var datasets = localService.GetDataSets(localFile, Permit);

                if (datasets == null || datasets.Count == 0)
                {
                    throw new Exception("Failed to load downloaded features. It could be because the file is more than 100MB or it has bad content.");
                }

                var localPipeline = localService.GetDataSetImportService(datasets[0], Permit).BuildPipeline(engine, settingsProvider);

                return localPipeline;    
            }

            // Create the FeatureServer process.
            var procName = DataSet.Metadata != null ? DataSet.Metadata.Name : "unknown name";
            var spProc = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(pyxlib.strToGuid("{88C80CEE-5EBE-4472-8E5F-FF0D6DD0A935}"))
                    .AddAttribute("uri", DataSet.Uri)
                    .SetName(procName)
            );
            if (spProc.isNull())
            {
                throw new Exception(
                    "Failed to create a feature server process from GUID {88C80CEE-5EBE-4472-8E5F-FF0D6DD0A935}");
            }

            // TODO: this is only needed when the logic handles exchanging credentials with the application level
            // this is going to be covered by an upcoming task
            //
            //add credentials provider to notify the WFS to ask for credentials for that url
            //var credentialsProvider =
            //    PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.UserCredentialsProvider);
            //
            //spProc.getParameter(1).addValue(credentialsProvider);

            // Create and return the features summary
            return PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.FeaturesSummary)
                    .AddInput(0, spProc)
                    .SetName(procName)
                    .SetDescription(DataSet.Metadata != null
                        ? DataSet.Metadata.Description
                        : "GeoServices Feature Server data source")
            );
        }

        /// <summary>
        /// Create a pipeline for importing a GeoSource from a MapServer data set
        /// </summary>
        /// <returns>IProcess_SPtr object</returns>
        private IProcess_SPtr BuildMapServerPipeline()
        {
            // get information about the data set from the server
            var credentials = ArcGISGeoServicesHelper.RetrieveCredentials(Permit);

            var serviceType = GetServiceFromUrl(DataSet.Uri);

            var serviceUri = ArcGISGeoServicesHelper.ExtractServiceUrl(new Uri(DataSet.Uri));

            var serviceDescription =
                WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISServiceDescription>(
                    ArcGISGeoServicesHelper.AsJsonRequest(serviceUri), credentials);

            var layerUrl = serviceUri + "/" + DataSet.Layer;
            var layerDescription =
                WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISLayerDescription>(
                    ArcGISGeoServicesHelper.AsJsonRequest(layerUrl), credentials);

            // Create the WS process.
            var wmsProcessGuid = pyxlib.strToGuid("{F5E595F7-B58B-4d23-AC2B-865829306E10}");
            var spProc = PYXCOMFactory.CreateProcess(wmsProcessGuid);
            if (spProc.isNull())
            {
                throw new Exception("Failed to create a WS process from GUID " + wmsProcessGuid);
            }

            // Set the Process 'Metadata'
            var procName = DataSet.Metadata != null && DataSet.Metadata.Name.HasContent() ? DataSet.Metadata.Name : (layerDescription.name ?? "unknown name");
            var procDescription = DataSet.Metadata != null && DataSet.Metadata.Description.HasContent() ? DataSet.Metadata.Description : (layerDescription.description ?? "GeoServices Map Server data source");
            spProc.setProcName(procName);
            spProc.setProcDescription(procDescription);

            // TODO: this is only needed when the logic handles exchanging credentials with the application level
            // this is going to be covered by an upcoming task
            //
            //add credentials provider to notify the WMS to ask for credentials for that url
            //var credentialsProvider =
            //    PYXCOMFactory.CreateProcess(PYXCOMFactory.WellKnownProcesses.UserCredentialsProvider);
            //spProc.getParameter(0).addValue(credentialsProvider);

            // Make attribute map
            var map = new Attribute_Map();


            switch (serviceType)
            {
                case "AGSI":
                    map.set("serviceName", "AGSI");
                    break;
                default:
                    map.set("serviceName", "AGS");
                    break;
            }

            map.set("server", serviceUri);

            // for servers that don't support dynamic layers, we do not specify the layer
            if (!String.IsNullOrEmpty(DataSet.Layer))
            {
                map.set("layer", "show:" + DataSet.Layer);
            }

            if (layerDescription.drawingInfo != null &&
                layerDescription.drawingInfo.renderer != null &&
                layerDescription.drawingInfo.renderer.type != null &&
                layerDescription.drawingInfo.renderer.type == "uniqueValue" ||
                //TODO: better detect when to use PNG
                serviceType == "AGSI")
            {
                // this is a chloropleth map, retrieve in lossless format
                map.set("format", "PNG");
            }
            else
            {
                map.set("format", "JPG");
            }

            var extent = layerDescription.extent ?? serviceDescription.initialExtent;

            if (extent == null)
            {
                throw new Exception("Bounding box is not specificed for map layer");
            }

            string srs;
            srs = ArcGISGeoServicesHelper.GetSrs(extent);
            map.set("srs", srs);

            // set the bounding box
            /*
            Note: longitude should correspond to x and latitude to y but
            the underlying implementation has reversed this, so I have maintained it here.
            */
            map.set("minLat", Convert.ToString(extent.xmin));
            map.set("minLon", Convert.ToString(extent.ymin));
            map.set("maxLat", Convert.ToString(extent.xmax));
            map.set("maxLon", Convert.ToString(extent.ymax));

            // get the minimum and maximum resolution
            double minResolution = double.MaxValue;
            double maxResolution = 0.0;
            if (serviceDescription.tileInfo != null)
            {
                var lods = serviceDescription.tileInfo.lods;
                if (lods != null)
                {
                    foreach (var lod in lods)
                    {
                        if (lod.resolution > 0.0)
                        {
                            minResolution = Math.Min(minResolution, lod.resolution);
                            maxResolution = Math.Max(maxResolution, lod.resolution);
                        }
                    }

                }
            }

            if (minResolution < double.MaxValue)
            {
                map.set("minRes", Convert.ToString(minResolution));
            }

            if (maxResolution > 0.0)
            {
                map.set("maxRes", Convert.ToString(maxResolution));
            }

            // Initialize the process
            spProc.setAttributes(map);
            if (spProc.initProc() != IProcess.eInitStatus.knInitialized)
            {
                throw new Exception("Failed to initialize WMS MapServer with error: " + spProc.getInitError().getError());
            }

            // Create and initialize the nearest neighbour sampler.
            var nnSamplerGuid = pyxlib.strToGuid("{D612004E-FC51-449a-B0D6-1860E59F3B0D}");
            var spSampler = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(nnSamplerGuid)
                    .SetName("Nearest Neighbour Sampler - " + procName)
                    .SetDescription(procDescription)
                    .AddInput(0, spProc)
            );
            if (spSampler.isNull())
            {
                throw new Exception("Failed to create a sampler process (GUID " + nnSamplerGuid + ")");
            }
            spSampler.initProc();

            if (serviceType == "AGSI")
            {
                var rasterAttributeTableUrl = layerUrl + "/rasterAttributeTable";

                var rasterTable =
                    WebRequestHelper.GetJsonWithAuthorization<ArcGISGeoServicesHelper.ArcGISServiceRasterAttributeTable>(
                        ArcGISGeoServicesHelper.AsJsonRequest(rasterAttributeTableUrl), credentials);

                if (rasterTable != null &&
                    rasterTable.fields != null &&
                    rasterTable.fields.Count > 0 &&
                    rasterTable.features != null &&
                    rasterTable.features.Count > 0
                    )
                {
                    m_valueTable = ParseRasterTable(rasterTable);
                }
            }

            //add value sampled infront of the process
            if (m_valueTable != null)
            {
                spSampler = AttachValueTableTransform(spSampler, m_valueTable);
            }

            // Create the coverage cache and return it (without initialization)
            var process = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageCache)
                    .SetName(procName)
                    .SetDescription(procDescription)
                    .AddInput(0, spSampler)
            );

            //add value sampled infront of the process
            if (m_valueTable != null)
            {
                //make sure things get initialized so we can get the spec
                process.initProc(true);

                var style = new Style()
                {
                    ShowAsElevation = false,
                    Fill = new FieldStyle()
                    {
                        Style = FieldStyleOptions.Palette,
                        PaletteExpression = process.CreatePipelineSpecification().Fields[0].Name,
                        Palette = new Palette()
                        {
                            Steps =
                                m_valueTable.Transform.OrderBy(x => x.Value)
                                    .Select(x => new Palette.Step() { Value = x.Value, Color = x.Color })
                                    .ToList()
                        }
                    }
                };

                //add a style to match everything process 
                process = style.ApplyStyle(process);
            }

            return process;
        }

        private IProcess_SPtr AttachValueTableTransform(IProcess_SPtr spSampler, RasterValueTransformTable valueTable)
        {
            var outputType = "int";
            if (valueTable.Transform.All(x => x.Value >= 0 && x.Value < 256))
            {
                //we can compress the data into bytes as all values between 0 and 255
                outputType = "byte";
            }

            var transformString = new StringWriter();
            transformString.Write("{0}", valueTable.Transform.Count);

            foreach (var entry in valueTable.Transform)
            {
                //add key PYXValue
                transformString.Write(" uint8_t[3] {0} {1} {2}", entry.Color.R, entry.Color.G, entry.Color.B);

                //add value PYXValue
                transformString.Write(outputType == "byte" ? " uint8_t {0}" : " int32_t {0}", entry.Value);
            }

            var rgbToValueProcessInfo = new PYXCOMProcessCreateInfo("{E23B6008-7F32-4BA7-BA37-D4038B30FB6D}");

            rgbToValueProcessInfo.AddAttribute("Output", outputType);
            rgbToValueProcessInfo.AddAttribute("Transform", transformString.ToString());
            rgbToValueProcessInfo.AddInput(0, spSampler);

            return PYXCOMFactory.CreateProcess(rgbToValueProcessInfo);
        }

        private RasterValueTransformTable ParseRasterTable(ArcGISGeoServicesHelper.ArcGISServiceRasterAttributeTable rasterTable)
        {

            var expectedFields =
                new[] { "OBJECTID", "Value", "Red", "Green", "Blue" }.Select(x => x.ToLower()).ToList();

            //not all exected fields are present in the rasterTable
            if (!expectedFields.All(x => rasterTable.fields.Any(f => f.name.ToLower() == x)))
            {
                return null;
            }

            var result = new RasterValueTransformTable();

            var labelField =
                rasterTable.fields.Where(x => !expectedFields.Contains(x.name))
                    .Where(x => x.type.ToLower().Contains("string"))
                    .Select(x => x.name)
                    .FirstOrDefault();

            if (!labelField.HasContent())
            {
                return null;
            }

            result.FieldName = labelField;

            result.Transform = new List<RasterValueTransformTable.ColorToValue>();

            foreach (var feature in rasterTable.features)
            {
                var color = Color.FromArgb(
                    (int)Convert.ChangeType(feature.attributes["Red"], typeof(int)),
                    (int)Convert.ChangeType(feature.attributes["Green"], typeof(int)),
                    (int)Convert.ChangeType(feature.attributes["Blue"], typeof(int)));

                var value = (int)Convert.ChangeType(feature.attributes["Value"], typeof(int));

                var label = (string)Convert.ChangeType(feature.attributes[labelField], typeof(string));

                result.Transform.Add(new RasterValueTransformTable.ColorToValue()
                {
                    Color = color,
                    Value = value,
                    Label = label
                });
            }

            return result;
        }

        public void EnrichGeoSource(Engine engine, GeoSource geoSource)
        {
            if (m_valueTable != null)
            {
                //populate value translation for this field
                var field = geoSource.Specification.Fields[0];
                field.ValueTranslation = m_valueTable.Transform.ToDictionary(x => x.Value.ToString(), y => y.Label);
            }

            //make sure we have a good style setup for icons.
            if (geoSource.Specification.OutputType == PipelineSpecification.PipelineOutputType.Feature)
            {
                geoSource.Style = engine.CreateDefaultStyle(geoSource);
            }
        }
    }
}