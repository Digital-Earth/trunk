using System.Collections.Generic;
using System.Linq;
using Pyxis.Core.IO;

namespace PyxisCLI.Server.Utilities
{
    /// <summary>
    /// Utility class to extract selected fields from a feature (or feature group).
    /// 
    /// This class is optimized for speed and robustness:
    /// 1) FeatureFieldsExtractor is created using a PYXTableDefinition used to extract information about the required field
    /// 2) The class ignores field names that are not found in the given table definition.
    /// </summary>
    class FeatureFieldsExtractor
    {
        /// <summary>
        /// Stores extraction information about a given field
        /// </summary>
        private class FieldExtractionInformation
        {
            public FieldExtractionInformation(string name, int index, bool isNumeric)
            {
                FieldName = name;
                FieldIndex = index;
                IsNumeric = isNumeric;
            }

            public bool IsNumeric { get; private set; }
            public int FieldIndex { get; private set; }
            public string FieldName { get; private set; }
        }

        private class SimpleHistogramCount
        {
            public object Min { get; set; }
            public object Max { get; set; }
            public int Count { get; set; }
        }

        private readonly List<FieldExtractionInformation> m_fieldsInfo;

        private FeatureFieldsExtractor(List<FieldExtractionInformation> fieldsInfo)
        {            
            m_fieldsInfo = fieldsInfo;
        }

        /// <summary>
        /// Create a FeatureFieldsExtractor that extracts all fields in a given table definition.
        /// </summary>
        /// <param name="definition">Table definition to use when extracting fields from features.</param>
        /// <returns>FeatureFieldsExtractor class.</returns>
        public static FeatureFieldsExtractor Create(PYXTableDefinition_CSPtr definition)
        {
            return Create(definition, 
                //select all fields
                definition.FieldDefinitions.Select(x => x.getName()));
        }

        /// <summary>
        /// Create a FeatureFieldsExtractor that extracts a set of fields using a given table definition
        /// </summary>
        /// <param name="definition">Table definition to use when extracting fields from features.</param>
        /// <param name="fields">set of field names to extract</param>
        /// <returns>FeatureFieldsExtractor class.</returns>
        public static FeatureFieldsExtractor Create(PYXTableDefinition_CSPtr definition, IEnumerable<string> fields)
        {
            var fieldsInfo = new List<FieldExtractionInformation>();

            foreach (var field in fields)
            {
                var fieldIndex = definition.getFieldIndex(field);

                if (fieldIndex != -1)
                {
                    var isFieldNumeric = definition.getFieldDefinition(fieldIndex).isNumeric();
                    fieldsInfo.Add(new FieldExtractionInformation(field,fieldIndex,isFieldNumeric));
                }
            }

            return new FeatureFieldsExtractor(fieldsInfo);
        }

        /// <summary>
        /// Extract fields from a given feature.
        /// 
        /// This function assume that the given feature has same feature definition.
        /// If that assumption is false an unpredictable value will be returned.
        /// </summary>
        /// <param name="feature">A Given feature to extract.</param>
        /// <returns>Dictionary with extracted fields</returns>
        public Dictionary<string, object> ExtractFields(IFeature_SPtr feature)
        {
            var group = pyxlib.QueryInterface_IFeatureGroup(feature);

            if (group.isNotNull())
            {
                return ExtractFields(group);
            }

            var result = new Dictionary<string, object>();

            foreach (var extractionInfo in m_fieldsInfo)
            {
                var value = feature.getFieldValue(extractionInfo.FieldIndex);
                result[extractionInfo.FieldName] = value.ToDotNetObject();
            }

            return result;
        }

        /// <summary>
        /// Extract fields from a given feature group.
        /// 
        /// This function assume that the given feature has same feature definition.
        /// If that assumption is false an unpredictable value will be returned.
        /// 
        /// The extracted value for each field for a feature group is an object:
        /// {
        ///    Min: min-value
        ///    Max: max-value
        ///    Average: average-value (only for numeric fields)
        ///    Histogram: [ { Min: value, Max: value, Count: number }, ... ]
        /// }
        /// </summary>
        /// <param name="featureGroup">A Given feature group to extract fields from.</param>
        /// <returns>Dictionary with extract fields</returns>
        public Dictionary<string, object> ExtractFields(IFeatureGroup_SPtr featureGroup)
        {
            var result = new Dictionary<string, object>();

            foreach (var extractionInfo in m_fieldsInfo)
            {
                var histogram = featureGroup.getFieldHistogram(extractionInfo.FieldIndex);

                var bins = histogram.getNormalizedBins(PYXHistogram.Normalization.knNormalizedBin, 10);

                var simpleHistogram = (
                    from PYXHistogramBin bin in bins
                    let min = bin.range.min.ToDotNetObject()
                    let max = bin.range.max.ToDotNetObject()
                    let count = bin.count.max
                    select new SimpleHistogramCount() {Min = min, Max = max, Count = count}
                    ).ToList();

                var boundaries = histogram.getBoundaries();

                var stats = new Dictionary<string, object>();
                stats["Min"] = boundaries.min.ToDotNetObject();
                stats["Max"] = boundaries.max.ToDotNetObject();
                stats["Histogram"] = simpleHistogram;

                if (extractionInfo.IsNumeric)
                {
                    stats["Average"] = histogram.getAverage().ToDotNetObject();
                }
                
                result[extractionInfo.FieldName] = stats;
            }

            return result;
        }
    }
}
