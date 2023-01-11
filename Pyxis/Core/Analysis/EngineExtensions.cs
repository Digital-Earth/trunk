using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Core.Analysis.Expressions;

namespace Pyxis.Core.Analysis
{
    /// <summary>
    /// Extension methods for Pyxis.Core.Engine.
    /// </summary>
    public static class EngineExtensions
    {
        /// <summary>
        /// Get the Pyxis.Core.IO.PipelineDefinition of a Pyxis.Contract.Publishing.Pipeline.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="pipeline">The Pyxis.Contract.Publishing.Pipeline to get the Pyxis.Core.IO.PipelineDefinition of.</param>
        /// <returns>The Pyxis.Core.IO.PipelineDefinition of <paramref name="pipeline"/>.</returns>
        public static PipelineSpecification GetSpecification(this Engine engine, Pipeline pipeline)
        {
            return new PipelineSpecificationCreator(engine, pipeline).GetSpecification();
        }

        /// <summary>
        /// Initialize a Pyxis.Core.Analysis.StatisticsCreator for a Pyxis.Contract.Publishing.Pipeline.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="pipeline">The Pyxis.Contract.Publishing.Pipeline to create statistics for.</param>
        /// <returns>The Pyxis.Core.Analysis.StatisticsCreator for <paramref name="pipeline"/>.</returns>
        public static StatisticsCreator Statistics(this Engine engine, Pipeline pipeline)
        {
            return new StatisticsCreator(engine, pipeline);
        }

        /// <summary>
        /// Determine if a PYXIS process supports a specified Pyxis.Core.IO.PipelineDefinition.PipelineOutputType.
        /// </summary>
        /// <param name="process">The PYXIS process to determine if it supports <paramref name="outputType"/>.</param>
        /// <param name="outputType">The Pyxis.Core.IO.PipelineDefinition.PipelineOutputType to determine if it is supported.</param>
        /// <returns>true if <paramref name="process"/> supports <paramref name="outputType"/>.</returns>
        internal static bool SupportOutputType(this IProcess_SPtr process,PipelineSpecification.PipelineOutputType outputType)
        {
            var output = process.getOutput();
            switch (outputType)
            {
                case PipelineSpecification.PipelineOutputType.Coverage:
                    return pyxlib.QueryInterface_ICoverage(output).isNotNull();

                case PipelineSpecification.PipelineOutputType.Feature:
                    return pyxlib.QueryInterface_ICoverage(output).isNull() &&
                           pyxlib.QueryInterface_IFeatureCollection(output).isNotNull();
            }
            return false;
        }

        /// <summary>
        /// Create a Pyxis.Core.Analysis.FeatureGetter for a Pyxis.Contract.Publishing.Pipeline representing a featuree PYXIS process.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="pipeline">The Pyxis.Contract.Publishing.Pipeline to get Pyxis.Core.IO.GeoJson.Feature of.</param>
        /// <returns>The Pyxis.Core.Analysis.FeatureGetter for <paramref name="pipeline"/>.</returns>
        /// <exception cref="System.InvalidOperationException">Unable to open <paramref name="pipeline"/> as a feature pipeline.</exception>
        public static FeatureGetter GetAsFeature(this Engine engine, Pipeline pipeline)
        {
            var process = engine.GetProcess(pipeline);
            if (!process.SupportOutputType(PipelineSpecification.PipelineOutputType.Feature))
            {
                throw new InvalidOperationException("can't open pipeline as feature pipeline");
            }
            return new FeatureGetter(engine,process);
        }

        /// <summary>
        /// Create a Pyxis.Core.Analysis.CoverageGetter for a Pyxis.Contract.Publishing.Pipeline representing a coverage PYXIS process.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="pipeline">The Pyxis.Contract.Publishing.Pipeline to get Pyxis.Core.IO.GeoJson.Feature of.</param>
        /// <returns>The Pyxis.Core.Analysis.CoverageGetter for <paramref name="pipeline"/>.</returns>
        /// <exception cref="System.InvalidOperationException">Unable to open <paramref name="pipeline"/> as a coverage pipeline.</exception>
        public static CoverageGetter GetAsCoverage(this Engine engine, Pipeline pipeline)
        {
            var process = engine.GetProcess(pipeline);
            if (!process.SupportOutputType(PipelineSpecification.PipelineOutputType.Coverage))
            {
                throw new InvalidOperationException("can't open pipeline as coverage pipeline");
            }
            return new CoverageGetter(process);
        }

        /// <summary>
        /// Attempt to create a GeoSource that is a mosaic of <paramref name="geoSources"/>.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="geoSources">The GeoSources to attempt to mosaic.</param>
        /// <returns>A GeoSource that is a mosaic of <paramref name="geoSources"/>.</returns>
        /// <exception cref="System.InvalidOperationException"><paramref name="geoSources"/> is empty or no method exists for creating a mosaic with <paramref name="geoSources"/>.</exception>
        public static GeoSource Mosaic(this Engine engine, IEnumerable<GeoSource> geoSources)
        {
            return new GeoSourceMosaic(engine).Mosaic(geoSources);
        }

        /// <summary>
        /// Attempt to create a GeoSource that is a calculation of <paramref name="expression"/> based of <paramref name="geoSources"/> .
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="expression">The Expression to calculate</param>
        /// <returns>A GeoSource that is a calculation of <paramref name="geoSources"/>.</returns>
        /// <exception cref="System.InvalidOperationException"><paramref name="geoSources"/> is empty or we failed to parse <paramref name="expression"/>.</exception>
        public static GeoSource Calculate(this Engine engine, string expression, Type outputType)
        {
            Func<string,GeoSource> defaultResolver = (string reference) => engine.ResolveReference(reference) as GeoSource;
            return new GeoSourceCalculator(engine).Calculate(defaultResolver, expression, outputType);
        }

        /// <summary>
        /// Attempt to create a GeoSource that is a calculation of <paramref name="expression"/> based of <paramref name="geoSources"/> .
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="resolver">Resolver of references</param>
        /// <param name="expression">The Expression to calculate</param>
        /// <returns>A GeoSource that is a calculation of <paramref name="geoSources"/>.</returns>
        /// <exception cref="System.InvalidOperationException"><paramref name="geoSources"/> is empty or we failed to parse <paramref name="expression"/>.</exception>
        public static GeoSource Calculate(this Engine engine, Func<string,GeoSource> resolver, string expression, Type outputType)
        {
            return new GeoSourceCalculator(engine).Calculate(resolver, expression, outputType);
        }

        /// <summary>
        /// Parse an expression into AST (tree of ExpressionNode) that can be used to investigate the expression
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="expression">The Expression to parse</param>
        /// <returns>ExpressionNode as the AST root</returns>
        public static ExpressionNode ParseExpression(this Engine engine, string expression)
        {
            return new GeoSourceCalculator(engine).ParseExpression(expression);
        }


        /// <summary>
        /// Calculate the DGGS Perimieter at a given resolution for a geometry.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="geometry">Geometry</param>
        /// <param name="resolution">DGGS resolution to claculate perimeter on.</param>
        /// <returns>PerimeterCalculator</returns>
        public static PerimeterCalculator CalculatePerimeter(this Engine engine, IGeometry geometry, int resolution)
        {
            return new PerimeterCalculator(engine,geometry,resolution);
        }
    }
}
