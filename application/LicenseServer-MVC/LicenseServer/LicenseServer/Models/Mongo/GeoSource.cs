using System;
using System.Collections.Generic;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class GeoSource : Pyxis.Contract.Publishing.GeoSource, IMongoResource
    {
        // for deserializing from string
        public GeoSource()
        {
        }

        public GeoSource(List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState? state, long? dataSize, List<ResourceReference> styles, List<Guid> usedBy, List<Guid> related)
            : base(licenses, metadata, version, procRef, definition, basedOn, state, dataSize, styles, usedBy, related)
        {
        }

        public GeoSource(GeoSource basedOnGeoSource)
            : base(basedOnGeoSource)
        {
        }

        // For unit testing
        public GeoSource(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState? state, long? dataSize, List<ResourceReference> styles, List<Guid> usedBy, List<Guid> related)
            : base(id, licenses, metadata, version, procRef, definition, basedOn, state, dataSize, styles, usedBy, related)
        {
        }
    }
}