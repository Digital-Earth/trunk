using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;

namespace Pyxis.Contract.Publishing
{
    public class Pipeline : Resource
    {
        public string ProcRef { get; set; }
        public string Definition { get; set; }
        public PipelineSpecification Specification { get; set; }
        public List<ResourceReference> BasedOn { get; set; }

        // for deserializing from string
        public Pipeline()
        {
        }

        public Pipeline(List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn)
            : base(ResourceType.Pipeline, licenses, metadata, version)
        {
            ProcRef = procRef;
            Definition = definition;
            BasedOn = new List<ResourceReference>(basedOn);
        }

        public Pipeline(Pipeline basedOnPipeline)
            : base(basedOnPipeline)
        {
            ProcRef = basedOnPipeline.ProcRef;
            Definition = basedOnPipeline.Definition;
            Specification = JsonConvert.DeserializeObject<PipelineSpecification>(JsonConvert.SerializeObject(basedOnPipeline.Specification));
            BasedOn = new List<ResourceReference>(basedOnPipeline.BasedOn);
        }

        public Pipeline(Guid id, List<LicenseReference> licenses, Metadata metadata, Guid version, string procRef, string definition, List<ResourceReference> basedOn)
            : this(licenses, metadata, version, procRef, definition, basedOn)
        {
            Id = id;
        }
    }
}
