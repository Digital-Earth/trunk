using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.Analysis
{
    static class PipelineSpecificationExtensions
    {
        static public bool IsSameAs(this PipelineSpecification firstSpec, IReadOnlyList<PipelineSpecification> specs)
        {
            // all specs provide the same OutputType
            return specs.All(spec => spec.OutputType == firstSpec.OutputType)
                   && (
                       // all Coverages have the same FieldType
                       (firstSpec.OutputType == PipelineSpecification.PipelineOutputType.Coverage
                        && specs.All(x =>
                            x.Fields.Select(field1 => field1.FieldType).SequenceEqual(
                                firstSpec.Fields.Select(field2 => field2.FieldType))))
                           // all Features have the same fields
                       || (firstSpec.OutputType == PipelineSpecification.PipelineOutputType.Feature
                           && specs.All(spec =>
                               spec.Fields.Select(field1 => field1.FieldType).SequenceEqual(firstSpec.Fields.Select(field2 => field2.FieldType))
                               &&
                               spec.Fields.Select(field1 => field1.Name).SequenceEqual(firstSpec.Fields.Select(field2 => field2.Name))
                               ))
                       );
        }
    }
}
