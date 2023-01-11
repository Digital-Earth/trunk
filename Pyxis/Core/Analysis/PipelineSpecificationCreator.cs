using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.Analysis
{
    internal class PipelineSpecificationCreator
    {
        private Engine m_engine;
        private Contract.Publishing.Pipeline m_pipeline;
        
        internal PipelineSpecificationCreator(Engine engine, Contract.Publishing.Pipeline pipeline)
        {
            m_engine = engine;
            m_pipeline = pipeline;
        }

        public PipelineSpecification GetSpecification()
        {
            if (m_pipeline.Specification != null && m_pipeline.Specification.Fields != null && m_pipeline.Specification.OutputType.HasValue)
            {
                return m_pipeline.Specification;
            }

            var process = m_engine.GetProcess(m_pipeline);

            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

            if (coverage.isNotNull() || featureCollection.isNotNull())
            {
                return process.CreatePipelineSpecification();
            }

            return null;
        }
    }
}
