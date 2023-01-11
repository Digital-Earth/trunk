using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Core.Analysis.Expressions;

namespace Pyxis.Core.Analysis
{
    /// <summary>
    /// Responsible for creating GeoSources that are cell based calculation of existing GeoSources.
    /// </summary>
    public class GeoSourceCalculator
    {
        private Engine Engine { get; set; }

        /// <summary>
        /// Initializes a new instance of the Pyxis.Core.Analysis.GeoSourceCalculator class.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        public GeoSourceCalculator(Engine engine)
        {
            Engine = engine;
        }

        public ExpressionNode ParseExpression(string expression)
        {
            var context = CreateContext(null);
            var parser = CreateExpressionParser(expression, context);
            return parser.ExpressionTree;
        }

        /// <summary>
        /// Calculate the expression on the given list of GeoSources
        /// </summary>
        /// <param name="geoSources">Input GeoSource to based the calculation on.</param>
        /// <param name="expression">Expression to evaluate on every cell.</param>
        /// <returns>A new Coverage GeoSource represent the calculation of the expression.</returns>
        public GeoSource Calculate(Func<string, GeoSource> resolver, string expression)
        {
            return Calculate(resolver, expression, typeof(double));
        }
        
        /// <summary>
        /// Calculate the expression on the given list of GeoSources
        /// </summary>
        /// <param name="resolver">GeoSource resolver to use for references.</param>
        /// <param name="expression">Expression to evaluate on every cell.</param>
        /// <param name="outputType">Output type of the result coverage: bool,int,double</param>
        /// <returns>A new Coverage GeoSource represent the calculation of the expression.</returns>
        public GeoSource Calculate(Func<string, GeoSource> resolver, string expression, Type outputType)
        {
            
            var context = CreateContext(resolver);
            var parser = CreateExpressionParser(expression, context);
            var normalizedOutputType = NormalizedOutputType(outputType);
            var finalNode = context.Compile(parser.ExpressionTree, normalizedOutputType);
            var helper = new ExpressionNodeCompilerHelper(context);
            if (!helper.IsCoverage(finalNode.GeoSource) && finalNode.FieldName.HasContent())
            {
                return helper.MakeGeoSourceWithCoverageCache(parser.ExpressionTree, helper.ConvertToCoverage(finalNode), new List<GeoSource>());
               
            }
            else
            {
                return finalNode.GeoSource;
            }
        }

        private static ExpressionParser CreateExpressionParser(string expression, ExpressionContext context)
        {
            var parser = new ExpressionParser(expression, context);

            try
            {
                parser.Parse();
            }
            catch (Exception ex)
            {
                throw new Exception("failed to parse expression : " + ex.Message);
            }
            return parser;
        }

        private ExpressionContext CreateContext(Func<string, GeoSource> resolver)
        {
            var context = new ExpressionContext()
            {
                Engine = Engine,
                Resolver = resolver,
                ComplexFunctions = RegisterComplexFunctions()
            };
            return context;
        }

        private static Dictionary<string, IExpressionNodeCompiler> RegisterComplexFunctions()
        {
            return new Dictionary<string, IExpressionNodeCompiler>()
            {
                {"slope", new SlopeExpressionCompiler("Slope")},
                {"aspect", new SlopeExpressionCompiler("Aspect")},
                {"first", new FirstNotNullExpressionCompiler()},
                {"transform", new TransfromValuesExpressionCompiler()}
            };
        }

        private static string NormalizedOutputType(Type outputType)
        {
            var normalizedOutputType = "";

            if (outputType == typeof(bool))
            {
                normalizedOutputType = "bool";
            }
            else if (outputType == typeof(byte))
            {
                normalizedOutputType = "byte";
            }
            else if (outputType == typeof(int) || outputType == typeof(uint))
            {
                normalizedOutputType = "int";
            }
            else if (outputType == typeof(double) || outputType == typeof(float))
            {
                normalizedOutputType = "double";
            }
            return normalizedOutputType;
        }
    }
}