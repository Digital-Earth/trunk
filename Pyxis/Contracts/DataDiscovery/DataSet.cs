using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;

namespace Pyxis.Contract.DataDiscovery
{

    /// <summary>
    /// Data wrapper around a resource (local or external) that represents a GeoSource that can be imported
    /// </summary>
    //[Serializable] - json serialization is broken when "Serializable" is present without DataMembers attributes
    public class DataSet
    {
        /// <summary>
        /// Type of the resource
        /// </summary>
        public string Type { get { return ResourceType.DataSet.ToString(); } }

        /// <summary>
        /// The path to the resource
        /// </summary>
        public string Uri { get; set; }

        /// <summary>
        /// Resource Metadata
        /// </summary>
        public SimpleMetadata Metadata { get; set; }

        /// <summary>
        /// DiscoveryReport can be added to a DataSet to give more information about discovery report
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public DataSetDiscoveryReport DiscoveryReport { get; set; }

        /// <summary>
        /// Internal path is used when the uri is a compressed file like (zip/tar) and the data set is located inside that file.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore,DefaultValueHandling = DefaultValueHandling.Ignore)]
        public string InternalPath { get; set; }

        /// <summary>
        /// The layer name
        /// </summary>
        public string Layer { get; set; }

        // TODO: This should be converted to a list of FieldSpecifications
        /// <summary>
        /// Fields within the layer.
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> Fields { get; set; }

        /// <summary>
        /// The Dataset Specification
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public PipelineSpecification Specification { get; set; }

        /// <summary>
        /// Any missing files required to load the data set (all files required)
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> MissingRequiredFilesAllOf { get; set; }

        /// <summary>
        /// Any missing files required to load the data set (one file required)
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> MissingRequiredFilesOneOf { get; set; }

        /// <summary>
        /// Any missing optional files associated with the data set
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<string> MissingOptionalFiles { get; set; }

        //TODO: add domains suppport on pyxlib side
        /// <summary>
        /// List of possible domains to be used for this GeoSource
        /// </summary>
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public List<Domain> Domains { get; set; }

        public List<BoundingBox> BBox { get; set; }

        /// <summary>
        /// Constructor ensures there are no null fields.
        /// </summary>
        public DataSet()
        {
            Uri = "";
            Metadata = new SimpleMetadata();
            Layer = "";
            Fields = new List<string>();
            MissingRequiredFilesAllOf = new List<string>();
            MissingRequiredFilesOneOf = new List<string>();
            MissingOptionalFiles = new List<string>();
        }

        /// <summary>
        /// Is a data set loadable (i.e. all required files a present)
        /// </summary>
        /// <returns>true if the data set is loadable, otherwise false</returns>
        public bool IsLoadable()
        {
            return !MissingRequiredFilesAllOf.Any() && !MissingRequiredFilesOneOf.Any();
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        /// <param name="other">Dataset to copy</param>
        public DataSet(DataSet other)
        {
            Uri = other.Uri;
            if (other.Metadata != null)
            {
                Metadata = new SimpleMetadata(other.Metadata);
            }
            if (other.Domains != null)
            {
                Domains = new List<Domain>(other.Domains.Select(d => d != null ? new Domain(d) : null));
            }
            InternalPath = other.InternalPath;
            Layer = other.Layer;

            if (other.Fields != null)
            {
                Fields = new List<string>(other.Fields);    
            }
            
            Specification = other.Specification;
            if (other.BBox != null)
            {
                BBox = new List<BoundingBox>(other.BBox);
            }
            DiscoveryReport = other.DiscoveryReport;

            MissingRequiredFilesAllOf = new List<string>(other.MissingRequiredFilesAllOf);
            MissingRequiredFilesOneOf = new List<string>(other.MissingRequiredFilesOneOf);
            MissingOptionalFiles = new List<string>(other.MissingOptionalFiles);
        }

        /// <summary>
        /// Generate a string representation of the DataSet.
        /// </summary>
        /// <returns>A string representation of the data set</returns>
        public override string ToString()
        {
            // Note: Fields.Aggregate() throws InvalidOperationException if Fields is empty
            var fields = Fields != null ? Fields.Any() ? Fields.Aggregate((x, y) => x + "," + y) : null : null;
            return string.Join(":", Uri, Layer, fields);
        }

        public bool AddTag(string tag)
        {
            if (String.IsNullOrWhiteSpace(tag))
            {
                return false;
            }
            tag = tag.Trim();

            if (Metadata == null)
            {
                throw new Exception("Metadata is not set, please create Metadata object before adding Tags");
            }
            if (Metadata.Tags == null)
            {
                Metadata.Tags = new List<string>() { tag };
                return true;
            }
            if (Metadata.Tags.Contains(tag))
            {
                return false;
            }
            Metadata.Tags.Add(tag);
            return true;
        }

        public bool AddSystemTag(string tag)
        {
            if (String.IsNullOrWhiteSpace(tag))
            {
                return false;
            }
            tag = tag.Trim();

            if (Metadata == null)
            {
                throw new Exception("Metadata is not set, please create Metadata object before adding Tags");
            }
            if (Metadata.SystemTags == null)
            {
                Metadata.SystemTags = new List<string>() { tag };
                return true;
            }
            if (Metadata.SystemTags.Contains(tag))
            {
                return false;
            }
            Metadata.SystemTags.Add(tag);
            return true;
        }
    }
}
