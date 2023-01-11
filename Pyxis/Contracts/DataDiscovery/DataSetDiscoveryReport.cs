using System;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.DataDiscovery
{
    /// <summary>
    /// Discovery status for the DataSet
    /// </summary>
    public enum DataSetDiscoveryStatus
    {
        Unknown,
        Failed,
        Successful
    }

    /// <summary>
    /// DataSet Discovery Report is generated to give more insight on how big and how long it take to import the DataSet
    /// </summary>
    public class DataSetDiscoveryReport
    {
        /// <summary>
        /// The DataSet Size (0 for unknown size). This can be used to change detection.
        /// </summary>
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Ignore)]
        public long DataSize { get; set; }

        /// <summary>
        /// The Feature count (0 for unknown size). This can be used to change detection.
        /// </summary>
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Ignore)]
        public long FeaturesCount { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public string GeometryType { get; set; }

        [JsonConverter(typeof(StringEnumConverter))]
        public DataSetDiscoveryStatus Status { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> Issues { get; set; }

        public DateTime DiscoveredTime { get; set; }

        public TimeSpan ImportTime { get; set; }

        public void AddError(string message)
        {
            if (Issues == null)
            {
                Issues = new List<string>();
            }
            Issues.Add(message);
        }

        public void AddError(Exception ex)
        {
            var aggEx = ex as AggregateException;

            if (aggEx != null)
            {
                foreach (var innerEx in aggEx.InnerExceptions)
                {
                    AddError(innerEx);
                }
            }
            else
            {
                AddError(ex.Message);
            }
        }
    }
}