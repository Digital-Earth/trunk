using System;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.Analysis.Expressions
{
    internal class FirstNotNullExpressionCompiler : IExpressionNodeCompiler
    {
        public CompiledGeoSource Compile(ExpressionNode expressionNode, ExpressionContext context, string outputType)
        {
            var helper = new ExpressionNodeCompilerHelper(context);

            var firstNotNullInfo = new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageFirstNotNull);

            var resolvedGeoSources =
                expressionNode.SubExpressions.Select(node => context.Compile(node, outputType)).ToList();

            helper.AddGeoSourcesToInput(firstNotNullInfo, 0, resolvedGeoSources);

            var firstNotNullProcess = PYXCOMFactory.CreateProcess(firstNotNullInfo);

            if (firstNotNullProcess.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                throw new InvalidOperationException("Failed to generate process for expression: " + expressionNode.Expression);
            }

            return helper.MakeGeoSourceWithCoverageCache(expressionNode, firstNotNullProcess, resolvedGeoSources.Select(x=>x.GeoSource).ToList());
        }
    }
}