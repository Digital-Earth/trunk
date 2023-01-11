using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    //TODO: consider merge with FieldSpecification
    /// <summary>
    /// Define a GeoSource.Domains specifications
    /// </summary>
    public class Domain
    {
        /// <summary>
        /// Gets or sets the name of the domain.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the Pyxis.Contract.Publishing.SimpleMetadata about the domain.
        /// </summary>
        public SimpleMetadata Metadata { get; set; }

        /// <summary>
        /// Gets or sets the type of the domain.
        /// </summary>
        [JsonConverter(typeof(StringEnumConverter))]
        public PipelineSpecification.FieldType Type { get; set; }

        /// <summary>
        /// Gets or sets the measurement unit for the domain.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public PipelineSpecification.FieldUnit Unit { get; set; }

        /// <summary>
        /// The value to be used for when loading data from domain.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public object Value;

        /// <summary>
        /// List of all possible values for the domain.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<object> Values;


        public Domain()
        {
        }

        public Domain(Domain other)
        {
            Name = other.Name;
            Value = other.Value;
            Type = other.Type;

            if (other.Metadata != null)
            {
                Metadata = new SimpleMetadata(other.Metadata);
            }

            if (other.Values != null)
            {
                Values = new List<object>(other.Values);
            }
            if (other.Unit != null)
            {
                Unit = new PipelineSpecification.FieldUnit() {Name = other.Unit.Name};
            }
        }
    }
}