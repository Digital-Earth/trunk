using System;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.Analysis.Expressions
{
    internal class SlopeExpressionCompiler : IExpressionNodeCompiler
    {
        public string SlopeOutputType { get; private set; }

        public SlopeExpressionCompiler(string slopeOutputType)
        {
            SlopeOutputType = slopeOutputType;
        }

        public CompiledGeoSource Compile(ExpressionNode expressionNode, ExpressionContext context, string outputType)
        {
            var helper = new ExpressionNodeCompilerHelper(context);

            //elevation to normal
            var elevationToNormalInfo = new PYXCOMProcessCreateInfo("{DB041009-1DA3-4f8e-AA28-0E8EFBA5F6F8}");

            var resolvedGeoSources =
                expressionNode.SubExpressions.Select(node => context.Compile(node)).ToList();

            helper.AddGeoSourcesToInput(elevationToNormalInfo, 0, resolvedGeoSources);

            var elevationToNormalProcess = PYXCOMFactory.CreateProcess(elevationToNormalInfo);

            if (elevationToNormalProcess.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                throw new InvalidOperationException("Failed to generate process for expression: " + expressionNode.Expression);
            }

            //normal to slope
            var slopeInfo = new PYXCOMProcessCreateInfo("{A916F98B-E4A2-4563-BB14-5CA04699CAA3}");

            slopeInfo.AddAttribute("Output", SlopeOutputType);
            slopeInfo.AddInput(0, elevationToNormalProcess);

            var slopeProcess = PYXCOMFactory.CreateProcess(slopeInfo);

            if (slopeProcess.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                throw new InvalidOperationException("Failed to generate process for expression: " + expressionNode.Expression);
            }

            var geoSource = helper.MakeGeoSourceWithCoverageCache(expressionNode, slopeProcess, resolvedGeoSources.Select(x => x.GeoSource).ToList());

            if (SlopeOutputType == "Slope")
            {
                geoSource.Style = new Style()
                {
                    Fill =
                        FieldStyle.FromPalette("Slope",
                            new Palette().Add("#ffffff", 0).Add("#f2ff01", 10).Add("#9126ff", 20).Add("#000000", 30)),
                    ShowAsElevation = false
                };
            }
            else
            {
                geoSource.Style = new Style()
                {
                    Fill =
                        FieldStyle.FromPalette("Aspect",
                            new Palette().Add("#e87171", 0).Add("#ebd635", 90).Add("#79eb76", 180).Add("#81c5f3", 180).Add("#e87171", 360)),
                    ShowAsElevation = false
                };
            }

            return geoSource;
        }
    }
}