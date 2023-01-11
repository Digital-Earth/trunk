using System;
using System.CodeDom;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading;
using ApplicationUtility;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.Analysis.Expressions
{
    public class ExpressionNodeCompilerHelper
    {
        public ExpressionContext Context { get; set; }

        public ExpressionNodeCompilerHelper(ExpressionContext context)
        {
            Context = context;
        }

        public enum GeoSourceOutputOptions
        {
            CoveragesOnly,
            VectorOnly,
            Both
        }

        public void AddGeoSourcesToInput(PYXCOMProcessCreateInfo processInfo, int inputIndex, List<CompiledGeoSource> resolvedGeoSources, GeoSourceOutputOptions options = GeoSourceOutputOptions.CoveragesOnly)
        {
            foreach (var geoSource in resolvedGeoSources)
            {
                var input = Context.Engine.GetProcess(geoSource).StripCoverageStyleIfPossible();

                if (!geoSource.Published)
                {
                    input = input.StripCacheIfPossible();
                }

                switch (options)
                {
                    case GeoSourceOutputOptions.CoveragesOnly:
                        if (!IsCoverage(input))
                        {
                            //try to auto convert input into coverage
                            input = ConvertToCoverage(input, geoSource.FieldName);
                        }
                        break;
                    case GeoSourceOutputOptions.VectorOnly:
                        if (IsCoverage(input))
                        {
                            throw new Exception("only vector inputs are supported");
                        }
                        break;
                    case GeoSourceOutputOptions.Both:
                        //nothing to worry about
                        break;
                    default:
                        throw new ArgumentOutOfRangeException("options", options, null);
                }

                processInfo.AddInput(inputIndex, input);
            }
        }
        
        //TODO: change this function ExtractFieldAsCoverage, I want this to also be used to extract R/B/G channels from Color coverages
        //Howver, this is only been working for vectors at the moment.
        public IProcess_SPtr ConvertToCoverage(CompiledGeoSource source)
        {
            var input = Context.Engine.GetProcess(source.GeoSource).StripCoverageStyleIfPossible();

            if (!source.Published)
            {
                input = input.StripCacheIfPossible();
            }

            return ConvertToCoverage(input, source.FieldName);
        }

        private IProcess_SPtr ConvertToCoverage(IProcess_SPtr input, string geoSourceFieldName)
        {
            if (!geoSourceFieldName.HasContent())
            {
                throw new Exception("Can't raster features without a field name");
            }

            var featuresDefinition = pyxlib.QueryInterface_IFeatureCollection(input.getOutput()).getFeatureDefinition();
            var fieldIndex = featuresDefinition.getFieldIndex(geoSourceFieldName);

            if (fieldIndex == -1)
            {
                throw new Exception("features collection doesn't have field '" + geoSourceFieldName + "'");
            }

            var fieldDefiniton = featuresDefinition.getFieldDefinition(fieldIndex);

            if (!fieldDefiniton.isNumeric())
            {
                throw new Exception("can only raster numric fields");
            }

            var outputType = "";
            switch (fieldDefiniton.getType())
            {
                case PYXValue.eType.knDouble:
                    outputType = "double";
                    break;

                case PYXValue.eType.knFloat:
                    outputType = "float";
                    break;

                case PYXValue.eType.knInt8:
                case PYXValue.eType.knInt16:
                case PYXValue.eType.knUInt16:
                case PYXValue.eType.knInt32:
                case PYXValue.eType.knUInt32:
                    outputType = "int";
                    break;

                case PYXValue.eType.knUInt8:
                case PYXValue.eType.knBool:
                    outputType = "byte";
                    break;

                default:
                    throw new Exception("unsupported field type " + fieldDefiniton.getType());
            }

            var rasterizerInfo = new PYXCOMProcessCreateInfo("{446B0C8B-2ED0-4967-A479-7138B3022D4C}")
                .AddInput(0, input)
                .AddAttribute("FieldName",geoSourceFieldName)
                .AddAttribute("OutputType",outputType)
                .BorrowNameAndDescription(input);

            var rasterizer = PYXCOMFactory.CreateProcess(rasterizerInfo);

            if (rasterizer.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                throw new Exception("Failed to initialize field rasterizer");
            }

            return rasterizer;
        }

        public bool IsCoverage(GeoSource geoSource)
        {
            return IsCoverage(Context.Engine.GetProcess(geoSource));
        }

        public bool IsCoverage(IProcess_SPtr process)
        {
            return process.isNotNull() && pyxlib.QueryInterface_ICoverage(process.getOutput()).isNotNull();
        }


        public GeoSource MakeGeoSourceWithCoverageCache(ExpressionNode expressionNode, IProcess_SPtr process, List<GeoSource> resolvedGeoSources)
        {
            var cache =
                PYXCOMFactory.CreateProcess(new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.CoverageCache)
                    .AddInput(0, process)
                    .SetName(CleanGeoSourceName(expressionNode.OriginalExpression))
                    .SetDescription(string.Format("References:\n{0}",
                        string.Join("\n", resolvedGeoSources.Select(x => x.Metadata.Name)))));

            if (cache.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                throw new Exception("Failed to create GeoSource cache.");
            }

            var result = cache.AsGeoSource(Context.Engine);

            //add supporting geoSources
            result.BasedOn = resolvedGeoSources.Where(geoSource => HasProviders(geoSource)).Select(ResourceReference.FromResource).ToList();

            return result;
        }

        private string CleanGeoSourceName(string name)
        {
            //remove [] chars from references. and @ for field specificier
            name = name.Replace("[", "").Replace("]", "").Replace("@", " ");

            //remove {json} objects
            var reg = new Regex(@"\{.*\}");

            name = reg.Replace(name, "<transform>");

            return name;
        }

        private bool HasProviders(GeoSource geoSource)
        {
            return geoSource.Metadata != null &&
                   geoSource.Metadata.Providers != null &&
                   geoSource.Metadata.Providers.Count > 0;
        }
    }
}