using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class MultiDomainGeoSource : Pyxis.Contract.Publishing.MultiDomainGeoSource, IMongoResource
    {
        // for deserializing from string
        public MultiDomainGeoSource()
        {
        }

        public MultiDomainGeoSource(List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState? state, long? dataSize, List<ResourceReference> styles, List<Guid> usedBy, List<Guid> related)
            : base(licenses, metadata, version, procRef, definition, basedOn, state, dataSize, styles, usedBy, related)
        {
        }

        public MultiDomainGeoSource(MultiDomainGeoSource basedOnGeoSource)
            : base(basedOnGeoSource)
        {
        }

        // For unit testing
        public MultiDomainGeoSource(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState? state, long? dataSize, List<ResourceReference> styles, List<Guid> usedBy, List<Guid> related)
            : base(id, licenses, metadata, version, procRef, definition, basedOn, state, dataSize, styles, usedBy, related)
        {
        }
    }
}