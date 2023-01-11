using System;
using System.Collections.Generic;

namespace Pyxis.Contract.Publishing
{
    /// <summary>
    /// Provide a method to define a multi domain GeoSource
    /// </summary>
    public class MultiDomainGeoSource : GeoSource
    {
        /// <summary>
        /// List of Domain
        /// </summary>
        public List<Domain> Domains { get; set; }

        // for deserializing from string
        public MultiDomainGeoSource()
        {
        }

        public MultiDomainGeoSource(List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState? state, long? dataSize, List<ResourceReference> styles, List<Guid> usedBy, List<Guid> related)
            : base(licenses, metadata, version, procRef, definition, basedOn, state, dataSize, styles, usedBy, related)
        {
        }

        public MultiDomainGeoSource(GeoSource basedOnGeoSource)
            : base(basedOnGeoSource)
        {
            Domains = null;
        }


        public MultiDomainGeoSource(MultiDomainGeoSource basedOnGeoSource)
            : base(basedOnGeoSource)
        {
            Domains = basedOnGeoSource.Domains == null ? null : new List<Domain>(basedOnGeoSource.Domains);
        }

        // For unit testing
        public MultiDomainGeoSource(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState? state, long? dataSize, List<ResourceReference> styles, List<Guid> usedBy, List<Guid> related)
            : this(licenses, metadata, version, procRef, definition, basedOn, state, dataSize, styles, usedBy, related)
        {
            Id = id;
        }
    }
}