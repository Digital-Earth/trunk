using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    /// <summary>
    /// Describes the data structure of a Pyxis.Contract.Publishing.Pipeline resource.
    /// </summary>
    public class PipelineSpecification
    {
        /// <summary>
        /// Gets or sets Pyxis.Core.IO.PipelineDefinition.PipelineOutputType of the PYXIS pipeline.
        /// </summary>
        [JsonConverter(typeof(StringEnumConverter))]
        public PipelineOutputType? OutputType { get; set; }

        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.PipelineDefinition.FieldType of the PYXIS pipeline's fields.
        /// </summary>
        public List<FieldSpecification> Fields { get; set; }

        /// <summary>
        /// PYXIS pipeline output types.
        /// </summary>
        public enum PipelineOutputType
        {
            /// <summary>
            /// A coverage output type.
            /// </summary>
            Coverage,
            /// <summary>
            /// A feature output type.
            /// </summary>
            Feature
        }

        /// <summary>
        /// PYXIS pipeline field types.
        /// </summary>
        public enum FieldType
        {
            /// <summary>
            /// A true/false field type.
            /// </summary>
            Boolean,
            /// <summary>
            /// A numeric field type.
            /// </summary>
            Number,
            /// <summary>
            /// A string field type.
            /// </summary>
            String,
            /// <summary>
            /// A color field type.
            /// </summary>
            Color,
            /// <summary>
            /// A date field type.
            /// </summary>
            Date
        }

        /// <summary>
        /// Defines a Pyxis.Core.IO.FieldDefinition unit.
        /// </summary>
        public class FieldUnit
        {
            /// <summary>
            /// Gets or sets the name of the unit
            /// </summary>
            public string Name { get; set; }
        }

        //TODO: consider rename to Type (merge with Domain)
        /// <summary>
        /// Defines a Pyxis.Core.IO.PipelineDefinition field.
        /// </summary>
        public class FieldSpecification
        {
            /// <summary>
            /// Gets or sets the name of the field.
            /// </summary>
            public string Name { get; set; }

            //TODO: consider rename to Type (merge with Domain)
            /// <summary>
            /// Gets or sets the type of the field.
            /// </summary>
            [JsonConverter(typeof(StringEnumConverter))]
            public FieldType FieldType { get; set; }

            //TODO: consider rename to Type (merge with Domain)
            /// <summary>
            /// Gets or sets the measurement unit for the field.
            /// </summary>
            public FieldUnit FieldUnit { get; set; }

            /// <summary>
            /// Gets or sets the Pyxis.Contract.Publishing.SimpleMetadata about the field.
            /// </summary>
            public SimpleMetadata Metadata { get; set; }

            /// <summary>
            /// Gets or sets the value translation rules to use when interpreting field values.
            /// </summary>
            [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
            public Dictionary<string, string> ValueTranslation { get; set; }

            public void Merge(FieldSpecification referenceField)
            {
                if (referenceField.FieldUnit != null)
                {
                    FieldUnit = new FieldUnit() {Name = referenceField.FieldUnit.Name};
                }
                if (referenceField.Metadata != null)
                {
                    if (Metadata == null)
                    {
                        Metadata = new SimpleMetadata();
                    }
                    if (referenceField.Metadata.Name != null)
                    {
                        Metadata.Name = referenceField.Metadata.Name;
                    }
                    if (referenceField.Metadata.Description != null)
                    {
                        Metadata.Description = referenceField.Metadata.Description;
                    }
                }
                if (referenceField.ValueTranslation != null)
                {
                    ValueTranslation = new Dictionary<string, string>(referenceField.ValueTranslation);
                }
            }
        }

        public bool HasField(string field)
        {
            return Fields.Any(f => f.Name == field);
        }

        public IEnumerable<string> FieldNames()
        {
            return Fields.Select(f => f.Name);
        }

        public int FieldIndex(string field)
        {
            for (var i = 0; i < Fields.Count; i++)
            {
                if (Fields[i].Name == field)
                {
                    return i;
                }
            }
            return -1;
        }

        public void Merge(PipelineSpecification referenceSpecification)
        {
            foreach (var field in Fields)
            {
                var fieldIndex = referenceSpecification.FieldIndex(field.Name);
                if (fieldIndex != -1)
                {
                    var referenceField = referenceSpecification.Fields[fieldIndex];
                    field.Merge(referenceField);
                }
            }
        }
    }
}
