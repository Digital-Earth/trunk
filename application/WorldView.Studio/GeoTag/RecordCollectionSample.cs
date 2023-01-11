using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using ApplicationUtility;
using Pyxis.Core.IO;

namespace Pyxis.WorldView.Studio.GeoTag
{
    /// <summary>
    /// FieldValuesSample contains statistics about a single field in a RecordCollection.
    /// </summary>
    public class FieldValuesSample
    {
        /// <summary>
        /// Index of the field in the RecordCollection TableDefinition.
        /// </summary>
        public int FieldIndex { get; set; }

        /// <summary>
        /// True if the field type is numeric.
        /// </summary>
        public bool IsNumeric { get; set; }

        /// <summary>
        /// Name of the field.
        /// </summary>
        public string FieldName { get; set; }

        /// <summary>
        /// Collection of all distinct values.
        /// </summary>
        public SortedSet<object> DistinctValues { get; set; }
    }

    /// <summary>
    /// RecordCollectionSample contains statistics about the values inside the record collection using sample of first few records.
    /// </summary>
    public class RecordCollectionSample
    {
        /// <summary>
        /// Default size of sample if no size was specified.
        /// </summary>
        const int DefaultSampleSize = 100;

        /// <summary>
        /// Number of records used to create this sample.
        /// </summary>
        public int SampleSize { get; private set; }

        /// <summary>
        /// Statistics about each field in the RecordCollection
        /// </summary>
        public Dictionary<string, FieldValuesSample> Fields { get; private set; }

        /// <summary>
        /// Protect from creating a sample not using the static Create method.
        /// </summary>
        private RecordCollectionSample()
        {
        }

        /// <summary>
        /// Create a RecordCollectionSample from a IRecordCollection_SPtr
        /// </summary>
        /// <param name="recordCollection">IRecordCollection_SPtr to sample.</param>
        /// <param name="maxSampleSize">Maximum number of records to sample.</param>
        /// <returns>RecordCollectionSample for the given IRecordCollection_SPtr.</returns>
        public static RecordCollectionSample Create(IRecordCollection_SPtr recordCollection, int maxSampleSize = DefaultSampleSize)
        {
            var sample = new RecordCollectionSample()
            {
                SampleSize = 0,
                Fields = new Dictionary<string, FieldValuesSample>()
            };

            var definition = recordCollection.getRecordDefinition();
            var fields = definition.FieldDefinitions.Select(x => x.getName()).ToList();
            var index = 0;
            foreach (var field in fields)
            {
                sample.Fields[field] = new FieldValuesSample()
                {
                    FieldIndex = index,
                    FieldName = field,
                    IsNumeric = definition.getFieldDefinition(index).isNumeric(),
                    DistinctValues = new SortedSet<object>()
                };
                index++;
            }

            var iterator = recordCollection.getIterator();
            while (sample.SampleSize < maxSampleSize && !iterator.end())
            {
                var record = iterator.getRecord();

                fields.ForEach(field =>
                {
                    sample.Fields[field].DistinctValues.Add(
                        record.getFieldValueByName(field)
                              .ToDotNetObject());
                });

                iterator.next();
                sample.SampleSize++;
            }

            return sample;
        }
    }
}
