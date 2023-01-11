using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.Import;
using Pyxis.IO.Import.GeoTagging;

namespace Pyxis.WorldView.Studio.GeoTag
{
    /// <summary>
    /// This class is using some heuristics to give the user suggestion how geo-tag the record collection
    /// </summary>
    class GeoTagMethodFinder
    {
        public Engine Engine { get; set; }
        public ProvideGeoTagImportSettingArgs Args { get; set; }
        public List<FeatureCollectionLookup> ReferenceGeoSources { get; set; }

        public GeoTagMethodFinder(Engine engine, ProvideGeoTagImportSettingArgs args, List<FeatureCollectionLookup> referenceGeoSources)
        {
            Engine = engine;
            Args = args;
            ReferenceGeoSources = referenceGeoSources;
        }

        public List<GeoTagOption> FindGeoTagOptions()
        {
            var recordCollection = pyxlib.QueryInterface_IRecordCollection(Args.RecordCollection.getOutput());

            var sample = RecordCollectionSample.Create(recordCollection);

            var result = new List<GeoTagOption>();

            result.AddRange(
                FindLatLonFields(recordCollection, sample));

            result.AddRange(
                FindLookupFields(recordCollection, sample));

            return result;
        }

        private IEnumerable<GeoTagOption> FindLookupFields(IRecordCollection_SPtr recordCollection, RecordCollectionSample sample)
        {
            
            foreach (var field in sample.Fields.Values)
            {
                //make sure the values are distinct
                if (field.DistinctValues.Count > sample.SampleSize / 2)
                {
                    foreach (var refGeoSource in ReferenceGeoSources)
                    {
                        try
                        {
                            if (MatchingCount(refGeoSource, field.DistinctValues) <= field.DistinctValues.Count * 0.9)
                            {
                                continue;
                            }
                        }
                        catch (Exception ex)
                        {
                            Trace.error("Error while finding a geo-tag method: " + ex.Message);
                            continue;
                        }

                        yield return new GeoTagOption()
                            {
                                Lookup = new FeatureCollectionLookupSettings()
                                {
                                    RecordCollectionFieldName = field.FieldName,
                                    ReferenceGeoSource = refGeoSource.ReferenceGeoSource,
                                    ReferenceFieldName = refGeoSource.ReferenceFieldName
                                }
                            };
                    }
                }
            }
        }

        private int MatchingCount(FeatureCollectionLookup refGeoSource, IEnumerable<object> values)
        {          
            var process = Engine.GetProcess(refGeoSource.ReferenceGeoSource);
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());
            var fieldIndex = featureCollection.getFeatureDefinition().getFieldIndex(refGeoSource.ReferenceFieldName);

            var indexProcess = PYXCOMFactory.CreateProcess(new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.FeatureCollectionIndex)
                    .AddInput(0, process)
                    .AddAttribute("FieldsIndices", fieldIndex.ToString()));

            if (indexProcess.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                return 0;
            }

            var matchCount = 0;
            var index = pyxlib.QueryInterface_IFeatureCollectionIndex(indexProcess.getOutput());

            foreach(var value in values)
            {

                //null has no geotag method working
                if (value == null)
                {
                    continue;
                }

                var iterator = index.getIterator(new PYXValue(value.ToString()));
                
                while (!iterator.end())
                {
                    if (iterator.getFeature()
                        .getFieldValueByName(refGeoSource.ReferenceFieldName)
                        .ToDotNetObject().ToString() == value.ToString())
                    {
                        matchCount++;
                        break;
                    }
                    iterator.next();
                }
            }

            return matchCount;
        }

        private IEnumerable<GeoTagOption> FindLatLonFields(IRecordCollection_SPtr recordCollection, RecordCollectionSample sample)
        {
            var numericFields = sample.Fields.Values.Where(x => x.IsNumeric).Select(x => x.FieldName).ToList();

            foreach (var namePairing in new[] { 
                new [] { "latitude", "longitude"},
                new [] { "lat", "lon"},
                new [] { "lat", "long"},
                new [] { "lat", "lng"},
                new [] { "x", "y"} })
            {
                foreach (var option in FindLatLonFields(numericFields, namePairing[0], namePairing[1])) 
                {                
                    yield return option;
                }
            }
        }

        private static List<string> FindPossiblePrefixes(List<string> fieldNames, string postfix)
        {
            return fieldNames.Select(x => x.ToLower()).Where(x => x.EndsWith(postfix)).Select(x => x.Replace(postfix, "")).ToList();
        }

        private IEnumerable<GeoTagOption> FindLatLonFields(List<string> fieldNames, string latitudeName, string longitudeName)
        {            
            var prefixes = 
                FindPossiblePrefixes(fieldNames, latitudeName)
                    .Intersect(FindPossiblePrefixes(fieldNames, longitudeName))
                    .ToList();

            foreach (var prefix in prefixes)
            {
                yield return new GeoTagOption()
                {
                    Point = new LatLonPointSettings()
                    {
                        LatitudeFieldName = fieldNames.First(x=>x.ToLower() == prefix + latitudeName),
                        LongitudeFieldName = fieldNames.First(x => x.ToLower() == prefix + longitudeName),
                        ReferenceSystem = SpatialReferenceSystem.WGS84,
                        Resolution = 24
                    }
                };
            }
        }

        public class LatLonPointSettings
        {
            public SpatialReferenceSystem ReferenceSystem { get; set; }
            public string LatitudeFieldName { get; set; }
            public string LongitudeFieldName { get; set; }
            public int Resolution { get; set; }
        }

        public class FeatureCollectionLookup
        {
            public GeoSource ReferenceGeoSource { get; set; }
            public string ReferenceFieldName { get; set; }
        }

        public class FeatureCollectionLookupSettings : FeatureCollectionLookup
        {            
            public string RecordCollectionFieldName { get; set; }
        }

        /// <summary>
        /// This class represent an option of how to GeoTag the given record collection
        /// </summary>
        public class GeoTagOption
        {
            /// <summary>
            /// If Lookup is not null, this geo-tag option represent a feature collection geo-tag method.
            /// </summary>
            public FeatureCollectionLookupSettings Lookup { get; set; }

            /// <summary>
            /// If Point is not null, this geo-tag option represent a lat lon point geo-tag method.
            /// </summary>
            public LatLonPointSettings Point { get; set; }

            /// <summary>
            /// Creates a Core.IO.Import.GeoTagging.IGeoTagMethod from current settings.
            /// </summary>
            /// <param name="engine">Pyxis.Core.Engine to be used.</param>
            /// <returns>a Core.IO.Import.GeoTagging.IGeoTagMethod.</returns>
            public IGeoTagMethod CreateGeoTagMethod(Engine engine)
            {
                if (Point != null)
                {
                    return new GeoTagByLatLonPoint()
                    {
                        LatitudeFieldName = Point.LatitudeFieldName,
                        LongitudeFieldName = Point.LongitudeFieldName,
                        ReferenceSystem = Point.ReferenceSystem,
                        Resolution = Point.Resolution
                    };
                }
                if (Lookup != null)
                {
                    return new GeoTagByFeatureCollectionLookup(engine)
                    {
                        RecordCollectionFieldName = Lookup.RecordCollectionFieldName,
                        ReferenceGeoSource = Lookup.ReferenceGeoSource,
                        ReferenceFieldName = Lookup.ReferenceFieldName
                    };

                }
                throw new Exception("No GeoTag method was provided");
            }
        }
    }
}
