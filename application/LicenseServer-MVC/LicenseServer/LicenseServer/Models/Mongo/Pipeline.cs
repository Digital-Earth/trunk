using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using Pyxis.Contract.Publishing;

namespace LicenseServer.Models.Mongo
{
    public class Pipeline : Pyxis.Contract.Publishing.Pipeline, IMongoResource
    {
        // for deserializing from string
        public Pipeline()
        {
        }

        public Pipeline(List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn)
            : base(licenses, metadata, version, procRef, definition, basedOn)
        {
        }

        public Pipeline(Pipeline basedOnPipeline)
            : base(basedOnPipeline)
        {
        }

        public Pipeline(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn)
            : base(id, licenses, metadata, version, procRef, definition, basedOn)
        {
        }
    }
}