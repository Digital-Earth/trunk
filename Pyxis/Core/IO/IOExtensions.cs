using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.Publishing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.IO
{
    /// <summary>
    /// Provides helper methods to for Pyxis.Core.IO namespace
    /// </summary>
    public static class IOExtensions
    {
        /// <summary>
        /// Create a pipeline specification from an pipeline.
        /// </summary>
        /// <param name="process">The head of the pipeline.</param>
        /// <returns>The pipeline specification or throws an exception.</returns>
        public static PipelineSpecification CreatePipelineSpecification(this IProcess_SPtr process)
        {
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
            
            if (coverage.isNotNull())
            {
                return FromPyxDefiniton(PipelineSpecification.PipelineOutputType.Coverage, coverage.getCoverageDefinition());
            }

            var features = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());
            if (features.isNotNull()) 
            {
                return FromPyxDefiniton(PipelineSpecification.PipelineOutputType.Feature, features.getFeatureDefinition());
            }

            throw new InvalidOperationException("Unsupported pipeline output type");
        }

        internal static PipelineSpecification FromPyxDefiniton(PipelineSpecification.PipelineOutputType outputType,PYXTableDefinition_CSPtr tableDefinition)
        {
            var definition = new PipelineSpecification() { Fields = new List<PipelineSpecification.FieldSpecification>(), OutputType = outputType };

            foreach (var field in tableDefinition.FieldDefinitions)
            {
                definition.Fields.Add(new PipelineSpecification.FieldSpecification()
                {
                    Name = field.getName(),
                    FieldType = field.ToFieldType(),
                    Metadata = new Contract.Publishing.SimpleMetadata()
                    {
                        Name = field.getName(),
                        Description = ""
                    }
                });
            }            

            return definition;
        }
    }
}
