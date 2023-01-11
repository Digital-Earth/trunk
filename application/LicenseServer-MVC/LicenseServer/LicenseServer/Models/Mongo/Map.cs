using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class Map : Pyxis.Contract.Publishing.Map, IMongoResource
    {
        // for deserializing from string
        public Map()
        {
        }

        public Map(List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState state, Camera camera, DateTime? time, List<Guid> related, ResourceReference theme)
            : base(licenses, metadata, version, procRef, definition, basedOn, state, camera, time, related, theme)
        {
        }

        public Map(Map basedOnMap)
            : base(basedOnMap)
        {
        }

        // For unit testing
        public Map(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn,
            PipelineDefinitionState state, Camera camera, DateTime time, List<Guid> related, ResourceReference theme)
            : base(id, licenses, metadata, version, procRef, definition, basedOn, state, camera, time, related, theme)
        {
        }
    }
}