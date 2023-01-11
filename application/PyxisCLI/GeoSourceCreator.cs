using System;
using System.IO.Compression;
using System.Linq;
using System.Threading.Tasks;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core;
using Pyxis.IO.Import;
using Pyxis.IO.Import.GeoTagging;
using Pyxis.IO.Sources.Remote;
using Pyxis.Utilities;
using PyxisCLI.State;
using PyxisCLI.Utilities;

namespace PyxisCLI
{
    static class GeoSourceCreator
    {
        public static GeoSource CreateFromArgs(Engine engine, string[] args)
        {
            bool save = false;
            string srs = null;
            string layer = null;
            string fields = null;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("save", (name) => save = true),
                new ArgsParser.Option("srs", (value) => srs = value),
                new ArgsParser.Option("layer", (value) => layer = value),
                new ArgsParser.Option("fields", (value) => fields = value));

            return CreateFromUrl(engine, args[0], srs, layer, fields);
        }

        public static GeoSource CreateFromUrl(Engine engine, string url, string srs, string layer, string fields)
        {
            Guid guid;

            if (Guid.TryParse(url, out guid))
            {
                return engine.GetChannel().GeoSources.GetById(guid);
            }

            var dataSet = new DataSet()
            {
                Uri = url,
                Layer = layer,
            };

            if (fields.HasContent())
            {
                dataSet.Fields = fields.Split(',').ToList();
            }

            return CreateFromDataSet(engine,dataSet,srs);
        }

        public static GeoSource CreateFromDataSet(Engine engine, DataSet dataSet, string srs = null, GeoTagMethods geoTag = null, string sampler = null)
        {
            var importSettingsProvider = new ImportSettingProvider();

            if (srs.HasContent())
            {
                importSettingsProvider.SRS(srs);
            }
            else if (dataSet.Uri.EndsWith(".nc"))
            {
                importSettingsProvider.SRS("+proj=latlong +datum=WGS84 +to +proj=latlong +datum=WGS84 +lon_wrap");
            }

            importSettingsProvider.Register(typeof(DownloadLocallySetting), new DownloadLocallySetting()
            {
                Path = LocalGazetteer.GetLocalDir(dataSet)
            });

            importSettingsProvider.Register(typeof(GeoTagImportSetting), (ProvideImportSettingArgs args) =>
            {
                return Task<IImportSetting>.Factory.StartNew(() =>
                {
                    var geoTagArgs = args as ProvideGeoTagImportSettingArgs;
                    if (geoTagArgs == null)
                    {
                        throw new Exception("No geo tag method were provided");
                    }

                    if (geoTag != null)
                    {
                        if (geoTag.LatLon != null)
                        {
                            return new GeoTagImportSetting()
                            {
                                Method = new GeoTagByLatLonPoint()
                                {
                                    LatitudeFieldName = geoTag.LatLon.Latitude,
                                    LongitudeFieldName = geoTag.LatLon.Longitude,
                                    ReferenceSystem = SpatialReferenceSystem.CreateFromWKT(geoTag.LatLon.Srs),
                                    Resolution = geoTag.LatLon.Resolution
                                }
                            };
                        }
                        if (geoTag.Lookup != null)
                        {
                            return new GeoTagImportSetting()
                            {
                                Method = new GeoTagByFeatureCollectionLookup(Program.Engine)
                                {
                                    ReferenceGeoSource = Program.Workspaces.ResolveGeoSource(geoTag.Lookup.Reference),
                                    ReferenceFieldName = geoTag.Lookup.SourceField,
                                    RecordCollectionFieldName = geoTag.Lookup.DestinationField,
                                }
                            };
                        }
                    }

                    var finder = new GeoTagMethodFinder(geoTagArgs, srs);
                    var options = finder.FindGeoTagOptions();

                    if (options.Count == 1)
                    {
                        return new GeoTagImportSetting() {Method = options[0]};
                    }
                    else
                    {

                    }
                    throw new Exception("No geo tag method were provided");
                });
            });

            if (sampler.HasContent())
            {
                switch (sampler)
                {
                    case "nearest":
                        importSettingsProvider.Sampler(PYXCOMFactory.WellKnownProcesses.NearestNeighbourSampler);
                        break;

                    case "bilinear":
                        importSettingsProvider.Sampler(PYXCOMFactory.WellKnownProcesses.BilinearSampler);
                        break;

                    case "bicubic":
                        importSettingsProvider.Sampler(PYXCOMFactory.WellKnownProcesses.BicubicSampler);
                        break;

                    case "bicubic_with_nulls":
                        importSettingsProvider.Sampler(PYXCOMFactory.WellKnownProcesses.BicubicSamplerWithNulls);
                        break;
                }
            }

            return CreateFromDataSet(engine, dataSet, importSettingsProvider);
        }

        private static GeoSource CreateFromDataSet(Engine engine, DataSet dataSet, ImportSettingProvider importSettingsProvider)
        {
            var progress = engine.BeginImport(dataSet, importSettingsProvider);

            if (progress == null)
            {
                if (dataSet.Uri.StartsWith("http") || dataSet.Uri.StartsWith("ftp"))
                {
                    var downloadService = new UrlImportService(dataSet, null,
                        dataSet.Uri.Substring(dataSet.Uri.LastIndexOf('.') + 1));
                    var localFile = downloadService.DownloadToLocalFile(importSettingsProvider);

                    if (localFile.HasContent())
                    {
                        if (dataSet.InternalPath.HasContent() && localFile.ToLower().EndsWith(".zip"))
                        {
                            var directory = System.IO.Path.Combine(
                                System.IO.Path.GetDirectoryName(localFile),
                                System.IO.Path.GetFileNameWithoutExtension(localFile));
                                
                            if (!System.IO.Directory.Exists(directory))
                            {
                                ZipFile.ExtractToDirectory(localFile, directory);
                            }
                            localFile = System.IO.Path.Combine(directory, dataSet.InternalPath);
                        }

                        var localDataSet = new DataSet(dataSet)
                        {
                            Uri = localFile,
                            InternalPath = null
                        };

                        progress = engine.BeginImport(localDataSet, importSettingsProvider);
                    }
                }
            }

            if (progress == null)
            {
                throw new Exception(dataSet.Uri + " is not supported");
            }

            return progress.Task.Result;
        }
    }
}
