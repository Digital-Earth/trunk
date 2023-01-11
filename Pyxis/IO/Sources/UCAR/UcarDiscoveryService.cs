using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Web;
using ApplicationUtility;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;
using Pyxis.IO.DataDiscovery;
using Pyxis.IO.Import;
using Pyxis.IO.Search;
using Pyxis.IO.Sources.Remote;
using Pyxis.Utilities;

namespace Pyxis.IO.Sources.UCAR
{
    class UcarDiscoveryService : IDiscoveryService
    {
        public string ServiceIdentifier
        {
            get { return "UCAR"; }
        }

        public bool IsUriSupported(string uri)
        {
            Uri parsedUri;
            if (Uri.TryCreate(uri, UriKind.Absolute, out parsedUri))
            {
                return parsedUri.GetLeftPart(UriPartial.Path) ==
                       "http://thredds.ucar.edu/thredds/ncss/grib/NCEP/GFS/Global_0p25deg/best";
            }
            return false;
        }

        public async Task<DiscoveryResult> DiscoverAsync(IDiscoveryContext context)
        {
            return null;
        }

        public IDataSetImportService GetDataSetImportService(DataSet dataSet, IPermit permit = null)
        {
            return new UcarImportService(dataSet, permit);
        }
    }


    public class UcarGazetteer : IGazetteer
    {
        public Engine Engine { get; set; }

        public UcarGazetteer(Engine engine)
        {
            Engine = engine;
        }

        private Gazetteer CurrentGazetteer { get; set; }

        public IEnumerable<GazetteerEntryScore> SearchEntries(IGazetteerScorer filter, IGazetteerScorer scorer = null, float filterThreshold = 0)
        {
            if (CurrentGazetteer == null)
            {
                BuildCurrentGazetteer();
            }

            return CurrentGazetteer.SearchEntries(filter, scorer, filterThreshold);
        }

        private void BuildCurrentGazetteer()
        {
            var now = DateTime.UtcNow;

            var hour = (now.Hour % 3) * 3;
            var time = new DateTime(now.Year, now.Month, now.Day, hour, 0, 0, DateTimeKind.Utc);

            var DateSet = new DataSet()
            {
                Uri =
                    String.Format(
                        "http://thredds.ucar.edu/thredds/ncss/grib/NCEP/GFS/Global_0p25deg/best?var=Temperature_surface&disableLLSubset=on&disableProjSubset=on&horizStride=1&time={0:s}Z&vertCoord=&accept=netcdf",
                        time),
                Layer = "",
                Fields = new List<string> { "Value" },
                Metadata = new SimpleMetadata()
                {
                    Name = "Surface Temperature " + time.ToString("dd/MM/yyyy HH:mm"),
                    Description = "UCAR surface temperature THREDDS service, updated every 3 hours.",
                },
                BBox = new List<BoundingBox>
                {
                    new BoundingBox()
                    {
                        Srs = "4326",
                        LowerLeft = new BoundingBoxCorner(-180,-90),
                        UpperRight= new BoundingBoxCorner(180,90)
                    }
                },
                Specification = new PipelineSpecification()
                {
                    OutputType = PipelineSpecification.PipelineOutputType.Coverage,
                    Fields = new List<PipelineSpecification.FieldSpecification>
                    {
                        new PipelineSpecification.FieldSpecification()
                        {
                            FieldType = PipelineSpecification.FieldType.Number,
                            Name = "Value",
                            FieldUnit = new PipelineSpecification.FieldUnit()
                            {
                                Name = "c"
                            },
                            Metadata = new SimpleMetadata()
                            {
                                Name = "Temperature"
                            }
                        }
                    }
                }
            };

            CurrentGazetteer = new Gazetteer();

            CurrentGazetteer.Add(DateSet);
        }

        public long? Count { get { return 1; } }
    }

    internal class UcarImportService : UrlImportService
    {
        private class ImportConfiguration
        {
            public string Name { get; set; }
            public string Description { get; set; }
            public string Calculation { get; set; }
            public string FieldName { get; set; }
            public string FieldUnit { get; set; }
            public Style Style { get; set; }
        }

        private static readonly Dictionary<string, ImportConfiguration> s_configurations = new Dictionary
            <string, ImportConfiguration>
            {
                {
                    "Temperature_surface", new ImportConfiguration()
                    {
                        Name = "Surface Temperature",
                        Description = "Surface Temperature",
                        FieldName = "Temperature",
                        FieldUnit = "c",
                        Calculation = "{0}-273.15",
                        Style = new Style
                        {
                            Fill = new Palette()
                                .Add("#0039E5", -50)
                                .Add("#008FE7", -40)
                                .Add("#00E6EA", -30)
                                .Add("#01EC9B", -20)
                                .Add("#01EF46", -10)
                                .Add("#14F202", 0)
                                .Add("#6DF402", 10)
                                .Add("#C7F703", 20)
                                .Add("#F9CF03", 30)
                                .Add("#FC7704", 40)
                                .Add("#FF1E05", 50)
                                .AsFieldStyle("Value")
                        }
                    }
                }
            };

        private ImportConfiguration Configruation { get; set; }

        public UcarImportService(DataSet dataSet, IPermit permit)
            : base(dataSet, permit)
        {
            //exptected url format: 
            //http://thredds.ucar.edu/thredds/ncss/grib/NCEP/GFS/Global_0p25deg/best?var=Temperature_surface&disableLLSubset=on&disableProjSubset=on&horizStride=1&time=2017-09-24T06%3A00%3A00Z&vertCoord=&accept=netcdf
            //
            //host: thredds.ucar.edu
            //path: /thredds/ncss/grib/NCEP/GFS/Global_0p25deg/best
            //query:
            //  var=Temperature_surface
            //  disableLLSubset=on
            //  disableProjSubset=on
            //  horizStride=1
            //  time=2017-09-24T06:00:00Z
            //  vertCoord=
            //  accept=netcdf

            var uri = new Uri(dataSet.Uri);
            var query = HttpUtility.ParseQueryString(uri.Query);

            if (s_configurations.ContainsKey(query["var"]))
            {
                Configruation = s_configurations[query["var"]];
            }
        }


        public override IProcess_SPtr BuildPipeline(Engine engine, IImportSettingProvider settingsProvider)
        {
            var importSettingsProvider = new ImportSettingProvider(settingsProvider);

            //add aditional SRS settings to make sure the NetCDF is working
            importSettingsProvider.SRS("+proj=latlong +datum=WGS84 +to +proj=latlong +datum=WGS84 +lon_wrap");

            LocalFileExtension = ".nc";

            var process = base.BuildPipeline(engine, importSettingsProvider);

            while (process != null && process.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                process = ProcessErrorFixer.Fix(process, importSettingsProvider).Result;
            }

            if (Configruation != null && Configruation.Calculation.HasContent())
            {
                //make sure we can read this process


                PipeUtils.waitUntilPipelineIdentityStable(process);

                var geoPacketSource = process.GeoPacketSources().First();
                var manifests = new List<Manifest>(geoPacketSource.WalkPipelinesExcludeGeoPacketSourcesAfterParent().ExtractManifests());
                long datasize = manifests.SelectMany(m => m.Entries).Sum(e => e.FileSize);

                var geoSource = new GeoSource
                {
                    Id = Guid.NewGuid(),
                    Version = Guid.NewGuid(),
                    Metadata = new Metadata()
                    {
                        Name = DataSet.Metadata.Name,
                        Description = DataSet.Metadata.Description
                    },
                    ProcRef = pyxlib.procRefToStr(new ProcRef(process)),
                    Definition = PipeManager.writePipelineToNewString(process),
                    DataSize = datasize,
                    Specification = process.CreatePipelineSpecification(),
                    Style = process.ExtractStyle()
                };

                var name = geoSource.Metadata.Name;

                Func<string, GeoSource> resolver = (reference) =>
                {
                    if (reference == name) return geoSource;
                    return engine.ResolveReference(reference) as GeoSource;
                };

                var resultGeoSource = engine.Calculate(resolver,
                    string.Format(Configruation.Calculation, "[" + geoSource.Metadata.Name + "]"),
                    typeof(double));

                return engine.GetProcess(resultGeoSource);
            }

            return process;
        }

        public override void EnrichGeoSource(Engine engine, GeoSource geoSource)
        {
            if (Configruation != null)
            {
                if (Configruation.FieldName.HasContent())
                {
                    geoSource.Specification.Fields[0].Metadata.Name = Configruation.FieldName;
                }

                if (Configruation.FieldUnit.HasContent())
                {
                    geoSource.Specification.Fields[0].FieldUnit = new PipelineSpecification.FieldUnit()
                    {
                        Name = Configruation.FieldUnit
                    };
                }

                if (Configruation.Style != null)
                {
                    geoSource.Style = Configruation.Style;
                }
            }
        }
    }
}
