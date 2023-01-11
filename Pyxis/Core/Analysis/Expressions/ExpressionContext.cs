using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Remoting.Messaging;
using System.Text.RegularExpressions;
using System.Threading;
using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;

namespace Pyxis.Core.Analysis.Expressions
{
    public class ExpressionContext
    {
        public Engine Engine { get; set; }
        public List<GeoSource> GeoSources { get; set; }
        public Func<string, GeoSource> Resolver { get; set; }
        public Dictionary<string, IExpressionNodeCompiler> ComplexFunctions { get; set; }


        public CompiledGeoSource Compile(ExpressionNode node, string outputType = "double")
        {
            switch (node.Type)
            {
                case ExpressionNodeType.Caclulation:
                    return MakeCoverageCalculatorGeoSource(node, outputType);

                case ExpressionNodeType.ComplexFunction:
                    return MakeComplexFunctionGeoSource(node, outputType);

                case ExpressionNodeType.Reference:
                    return ResolveGeoSourceReference(node.Expression);

                default:
                    throw new Exception("unsupported node type " + node.Type);
            }
        }

        private CompiledGeoSource ResolveGeoSourceReference(string fieldReference)
        {
            var reg = new Regex(@"^\[(?<field>.*\@|)(?<fieldReference>[^\:\]]*)\]$");

            var match = reg.Match(fieldReference);

            if (!match.Success)
            {
                throw new Exception("failed to resolve: " + fieldReference);
            }

            var field = match.Groups["field"].Value.TrimEnd('@');
            var reference = match.Groups["fieldReference"].Value;
            
            //custom resolver
            if (Resolver != null)
            {
                var result = Resolver(reference);

                if (result != null)
                {
                    return new CompiledGeoSource(result, field, true);
                }
            }

            throw new Exception("failed to resolve: " + fieldReference);
        }

        private CompiledGeoSource MakeComplexFunctionGeoSource(ExpressionNode expressionNode, string normalizedOutputType)
        {
            var functionName = expressionNode.Expression.Substring(0, expressionNode.Expression.IndexOf('(')).Trim().ToLower();

            if (ComplexFunctions.ContainsKey(functionName))
            {
                return ComplexFunctions[functionName].Compile(expressionNode, this, normalizedOutputType);
            }
            else
            {
                throw new Exception("unknown built-in function " + functionName);
            }
        }

        private CompiledGeoSource MakeCoverageCalculatorGeoSource(ExpressionNode expressionNode, string outputType)
        {
            var helper = new ExpressionNodeCompilerHelper(this);
            var processInfo = new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.Calculator);

            processInfo
                .AddAttribute("expression", expressionNode.Expression)
                .AddAttribute("mode", "DifferentExpressions")
                .AddAttribute("outputType", outputType);

            var resolvedGeoSources =
                expressionNode.SubExpressions.Select(node => Compile(node, outputType)).ToList();

            helper.AddGeoSourcesToInput(processInfo, 0, resolvedGeoSources);

            var calculatorProcess = PYXCOMFactory.CreateProcess(processInfo);

            if (calculatorProcess.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                throw new InvalidOperationException("Failed to parse expression: " + expressionNode.Expression);
            }

            return helper.MakeGeoSourceWithCoverageCache(expressionNode, calculatorProcess, resolvedGeoSources.Select(x => x.GeoSource).ToList());
        }
    }
}