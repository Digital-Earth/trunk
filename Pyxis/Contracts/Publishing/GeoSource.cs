using System;
using System.Collections.Generic;
using System.Security.Cryptography;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    public enum PipelineDefinitionState
    {
        Active,
        Removed,
        Broken
    }

    public class GeoSource : Pipeline
    {
        [JsonConverter(typeof(StringEnumConverter))]
        public PipelineDefinitionState? State { get; set; }
        public long? DataSize { get; set; }
        
        [JsonProperty(DefaultValueHandling = DefaultValueHandling.Ignore)]
        public Style Style { get; set; }

        //TODO: remove this when we can, will be replaced using Style
        public List<ResourceReference> Styles { get; set; }

        public List<Guid> UsedBy { get; set; }
        public List<Guid> Related { get; set; }

        // for deserializing from string
        public GeoSource()
        {
        }

        public GeoSource(List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState? state, long? dataSize, List<ResourceReference> styles, List<Guid> usedBy, List<Guid> related, PipelineSpecification specifiction = null)
            : base(licenses, metadata, version, procRef, definition, basedOn)
        {
            Type = ResourceType.GeoSource;
            State = state;
            DataSize = dataSize;
            Styles = new List<ResourceReference>(styles);
            Specification = specifiction;
            UsedBy = new List<Guid>(usedBy);
            Related = new List<Guid>(related);
        }

        public GeoSource(GeoSource BasedOnGeoSource)
            : base(BasedOnGeoSource)
        {
            State = BasedOnGeoSource.State;
            DataSize = BasedOnGeoSource.DataSize;
            Style = BasedOnGeoSource.Style;
            Styles = new List<ResourceReference>(BasedOnGeoSource.Styles);
            Specification = BasedOnGeoSource.Specification;
            UsedBy = new List<Guid>(BasedOnGeoSource.UsedBy);
            Related = new List<Guid>(BasedOnGeoSource.Related);
        }

        // For unit testing
        public GeoSource(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState? state, long? dataSize, List<ResourceReference> styles, List<Guid> usedBy, List<Guid> related, PipelineSpecification specifiction = null)
            : this(licenses, metadata, version, procRef, definition, basedOn, state, dataSize, styles, usedBy, related, specifiction)
        {
            Id = id;
        }
    }
}