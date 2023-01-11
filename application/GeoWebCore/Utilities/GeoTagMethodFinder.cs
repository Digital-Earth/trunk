using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.IO.Import;
using GeoWebCore.Models;
using GeoWebCore.Services;
using Pyxis.Contract.DataDiscovery;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// This class is using some heuristics to give the user suggestion how geo-tag the record collection
    /// </summary>
    class GeoTagMethodFinder
    {
        public Engine Engine { get; set; }
        public ProvideGeoTagImportSettingArgs Args { get; set; }
        public List<ImportDataSetRequest.GeoTagBasedOnSettings> ReferenceGeoSources { get; set; }

        public GeoTagMethodFinder(ProvideGeoTagImportSettingArgs args, List<ImportDataSetRequest.GeoTagBasedOnSettings> referenceGeoSources)
        {
            Args = args;
            ReferenceGeoSources = referenceGeoSources ?? new List<ImportDataSetRequest.GeoTagBasedOnSettings>();
        }

        public List<ImportDataSetRequest.GeoTagRequest> FindGeoTagOptions()
        {
            var recordCollection = pyxlib.QueryInterface_IRecordCollection(Args.RecordCollection.getOutput());

            var sample = RecordCollectionSample.Create(recordCollection);

            var result = new List<ImportDataSetRequest.GeoTagRequest>();

            result.AddRange(
                FindLatLonFields(recordCollection, sample));

            result.AddRange(
                FindLookupFields(recordCollection, sample));

            return result;
        }

        private IEnumerable<ImportDataSetRequest.GeoTagRequest> FindLookupFields(IRecordCollection_SPtr recordCollection, RecordCollectionSample sample)
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

                        yield return new ImportDataSetRequest.GeoTagRequest()
                            {
                                BasedOn = new ImportDataSetRequest.GeoTagBasedOnSettings()
                                {
                                    TargetFieldName = field.FieldName,
                                    ReferenceGeoSource = refGeoSource.ReferenceGeoSource,
                                    ReferenceFieldName = refGeoSource.ReferenceFieldName
                                }
                            };
                    }
                }
            }
        }

        private int MatchingCount(ImportDataSetRequest.GeoTagBasedOnSettings refGeoSource, IEnumerable<object> values)
        {
            var process = GeoSourceInitializer.Initialize(refGeoSource.ReferenceGeoSource.Id);
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

        private IEnumerable<ImportDataSetRequest.GeoTagRequest> FindLatLonFields(IRecordCollection_SPtr recordCollection, RecordCollectionSample sample)
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

        private IEnumerable<ImportDataSetRequest.GeoTagRequest> FindLatLonFields(List<string> fieldNames, string latitudeName, string longitudeName)
        {            
            var prefixes = 
                FindPossiblePrefixes(fieldNames, latitudeName)
                    .Intersect(FindPossiblePrefixes(fieldNames, longitudeName))
                    .ToList();

            foreach (var prefix in prefixes)
            {
                yield return new ImportDataSetRequest.GeoTagRequest()
                {
                    LatLon = new ImportDataSetRequest.GeoTagLatLonSettings()
                    {
                        LatitudeFieldName = fieldNames.First(x=>x.ToLower() == prefix + latitudeName),
                        LongitudeFieldName = fieldNames.First(x => x.ToLower() == prefix + longitudeName),
                        ReferenceSystem = SpatialReferenceSystem.WGS84,
                        Resolution = 24
                    }
                };
            }
        }
    }
}
