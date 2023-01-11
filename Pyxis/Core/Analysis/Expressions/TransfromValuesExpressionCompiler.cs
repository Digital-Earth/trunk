using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Converters;
using Pyxis.Contract.Publishing;

namespace Pyxis.Core.Analysis.Expressions
{
    internal class TransfromValuesExpressionCompiler : IExpressionNodeCompiler
    {
        private enum TransformMode
        {
            Coverage,
            RgbCoverage,
            Features
        }

        public CompiledGeoSource Compile(ExpressionNode expressionNode, ExpressionContext context, string outputType)
        {
            //syntax:
            //transform([geo-source],{json-transform},false|true,min|max|average)
            //              +-------------|-----------------|---------|---------------- reference to a geo-source
            //                            +-----------------|---------|---------------- json object: { from:to, from:to }
            //                                              +---------|---------------- extactMatch. default is true
            //                                                        +---------------- aggregation mode. default is average
            //
            // {json-transform} is a json object that act as a dictionary. object keys are transform to values.
            //
            // Depend on the input. the following keys are expected:
            //
            // 1) RGB values: keys should be Css style colors: #aaa, #ff0000 or rgba(12,255,12,0.1)
            // 2) Numeric values: keys should be integers or doubles.
            // 3) String values: keys should be the string values.
            //
            // All values are should be numbers. 
            // If the numbers are between 0 and 255 and all integers the output coverage will be bytes.
            

            var helper = new ExpressionNodeCompilerHelper(context);

            if (expressionNode.SubExpressions.Count < 2)
            {
                throw new Exception("Transform function doesn't have enough arguments. Usage: transform([ref],{mapping},(optional)extactMatch,(optional)min|max|average)");
            }

            var resolvedGeoSource = context.Compile(expressionNode.SubExpressions[0], outputType);

            var transformMode = GetTransformMode(helper, resolvedGeoSource);

            var coverageTransform = "{6C918668-70FD-4AC6-A5B5-5491BD0FCC4D}";
            var featureFieldRasterizer2 = "{446B0C8B-2ED0-4967-A479-7138B3022D4C}";
            var rgbToValueTransform = "{E23B6008-7F32-4BA7-BA37-D4038B30FB6D}";

            string processId;

            switch (transformMode)
            {
                case TransformMode.Features:
                    processId = featureFieldRasterizer2;
                    break;
                case TransformMode.Coverage:
                    processId = coverageTransform;
                    break;
                case TransformMode.RgbCoverage:
                    processId = rgbToValueTransform;
                    break;
                default:

                    throw new Exception("Unsupported transform mode");
            }

            var transformValuesInfo = new PYXCOMProcessCreateInfo(processId);
            
            var transformJson = expressionNode.SubExpressions[1].Expression;
            var extactMatch = expressionNode.SubExpressions.Count < 3 || expressionNode.SubExpressions[2].Expression.Trim().ToLower() == "true";
            var aggregate = expressionNode.SubExpressions.Count < 4 ? "average" : expressionNode.SubExpressions[3].Expression.Trim().ToLower();

            if (transformMode == TransformMode.Features)
            {
                if (!resolvedGeoSource.FieldName.HasContent())
                {
                    throw new Exception("Can't raster features without a field name");
                }
                transformValuesInfo.AddAttribute("FieldName", resolvedGeoSource.FieldName);
                transformValuesInfo.AddAttribute("Aggregate", aggregate);
                helper.AddGeoSourcesToInput(transformValuesInfo, 0, new List<CompiledGeoSource> {resolvedGeoSource},
                    ExpressionNodeCompilerHelper.GeoSourceOutputOptions.VectorOnly);
            }
            else
            {
                helper.AddGeoSourcesToInput(transformValuesInfo, 0, new List<CompiledGeoSource> {resolvedGeoSource});
            }

            transformValuesInfo.AddAttribute("Output", DetectOutputType(transformJson));
            transformValuesInfo.AddAttribute("Transform", BuildTransformString(transformMode, transformJson));

            if (transformMode != TransformMode.RgbCoverage)
            {
                transformValuesInfo.AddAttribute("ExactMatch", extactMatch.ToString());
            }

            var transformValueProcess = PYXCOMFactory.CreateProcess(transformValuesInfo);

            if (transformValueProcess.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                throw new InvalidOperationException("Failed to generate process for expression: " + expressionNode.Expression);
            }

            var result = helper.MakeGeoSourceWithCoverageCache(expressionNode, transformValueProcess, new List<GeoSource> { resolvedGeoSource });

            var fieldName = context.Engine.GetSpecification(result).Fields[0].Name;

            result.Style = transformMode == TransformMode.RgbCoverage
                ? CreateColorPreseveringStyle(fieldName, transformJson)
                : CreateStyle(fieldName, transformJson);

            return result;
        }

        private TransformMode GetTransformMode(ExpressionNodeCompilerHelper helper, CompiledGeoSource resolvedGeoSource)
        {
            if (helper.IsCoverage(resolvedGeoSource))
            {
                var fieldType = resolvedGeoSource.GeoSource.Specification.Fields[0].FieldType;
                return fieldType == PipelineSpecification.FieldType.Color ? TransformMode.RgbCoverage : TransformMode.Coverage;
            }
            return TransformMode.Features;
        }

        /// <summary>
        /// Creates a style based on the output values at the tansformation json.
        /// </summary>
        /// <param name="fieldName">Output field name</param>
        /// <param name="transformJson">Transformation json object</param>
        /// <returns>Style object</returns>
        private Style CreateStyle(string fieldName, string transformJson)
        {
            var transform = JsonConvert.DeserializeObject<Dictionary<string, object>>(transformJson);

            var values = transform.Values.Select(x => Convert.ChangeType(x, typeof(double))).Distinct().OrderBy(x => x).ToList();

            var random = new Random();

            var result = new Style
            {
                Fill = FieldStyle.FromPalette(fieldName, new Palette()
                {
                    Steps = values.Select(x => new Palette.Step()
                    {
                        Color = Color.FromArgb(random.Next(0, 255), random.Next(0, 255), random.Next(0, 255)),
                        Value = x
                    }).ToList()
                })
            };

            return result;
        }


        /// <summary>
        /// Creates a style based on the output values at the tansformation json.
        /// </summary>
        /// <param name="fieldName">Output field name</param>
        /// <param name="transformJson">Transformation json object</param>
        /// <returns>Style object</returns>
        private Style CreateColorPreseveringStyle(string fieldName, string transformJson)
        {
            var transform = JsonConvert.DeserializeObject<Dictionary<string, object>>(transformJson);

            var valuesToColors = new Dictionary<double, Color>();

            foreach (var keyValue in transform)
            {
                if (keyValue.Value == null)
                {
                    continue;
                }
                var valueAsDouble = Convert.ChangeType(keyValue.Value, typeof(double));
                if (valueAsDouble == null)
                {
                    continue;
                }
                var value = (double) valueAsDouble;
                valuesToColors[value] = CssColorConverter.FromCss(keyValue.Key);
            }

            var result = new Style
            {
                Fill = FieldStyle.FromPalette(fieldName, new Palette()
                {
                    Steps = valuesToColors.OrderBy(x=>x.Key).Select(x=>new Palette.Step()
                    {
                        Color = x.Value,
                        Value = x.Key
                    }).ToList()
                })
            };

            return result;
        }

        private string DetectOutputType(string transformJson)
        {
            var transform = JsonConvert.DeserializeObject<Dictionary<string, object>>(transformJson);

            if (transform.All(x => x.Value is int || x.Value is long))
            {
                var values = transform.Select(x => (int) Convert.ChangeType(x.Value, typeof(int))).ToList();

                if (values.Max() < 256 && values.Min() >= 0)
                {
                    return "byte";
                }

                return "int";
            }
            else
            {
                return "double";
            }
        }

        private string BuildTransformString(TransformMode transformMode, string transformJson)
        {
            var transform = JsonConvert.DeserializeObject<Dictionary<string,object>>(transformJson);

            var outputType = DetectOutputType(transformJson);

            var numericKeys = transform.Keys.All(x =>
                                    {
                                        var t = 0.0;
                                        return double.TryParse(x, out t);
                                    });

            var result = new StringWriter();
            result.Write("{0}", transform.Count);

            foreach (var keyValue in transform.OrderBy(x => x.Key))
            {
                if (transformMode == TransformMode.RgbCoverage)
                {
                    var color = CssColorConverter.FromCss(keyValue.Key);

                    result.Write(" uint8_t[3] {0} {1} {2}", color.R, color.G, color.B);
                }
                else if (numericKeys)
                {
                    result.Write(" double {0}", double.Parse(keyValue.Key));
                }
                else
                {
                    result.Write(" string {0} {1}", keyValue.Key.Length, keyValue.Key);
                }

                switch (outputType)
                {
                    case "byte":
                        result.Write(" uint8_t {0}", Convert.ChangeType(keyValue.Value, typeof(int)));
                        break;
                    case "int":
                        result.Write(" int32_t {0}", Convert.ChangeType(keyValue.Value, typeof(int)));
                        break;
                    default:
                        result.Write(" double {0}", Convert.ChangeType(keyValue.Value, typeof(double)));
                        break;
                }
            }

            return result.ToString();
        }
    }
}