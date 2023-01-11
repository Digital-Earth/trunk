using ApplicationUtility;
using Newtonsoft.Json;
using Newtonsoft.Json.Converters;
using Pyxis.Contract.Publishing;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using Pyxis.Core.Analysis;

namespace Pyxis.Core.IO
{
    /// <summary>
    /// Manages the style information about a PYXIS processes viewed on the globe.
    /// </summary>
    public static class StyleExtensions 
    {
        /// <summary>
        /// Extract the style information from a PYXIS process.
        /// </summary>
        /// <param name="process">Pointer to a PYXIS process.</param>
        /// <returns>The Pyxis.Contract.Publishing.Style of <paramref name="process"/>.</returns>
        public static Style ExtractStyle(this IProcess_SPtr process)
        {
            var finalStyle = new Style();

            var feature = pyxlib.QueryInterface_IFeature(process.getOutput());
            var styleStr = feature.getStyle();

            if (String.IsNullOrEmpty(styleStr))
            {
                styleStr = "<style></style>";
            }
            
            finalStyle.ParseXmlStyleDocument(feature, XDocument.Parse(styleStr));
            
            return finalStyle;
        }

        private static void ParseXmlStyleDocument(this Style finalStyle, IFeature_SPtr feature,XDocument doc)
        {
            finalStyle.Fill = null;
            finalStyle.Line = null;
            finalStyle.Icon = null;

            var style = doc.Element("style");
            var icon = style.Element("Icon");
            var line = style.Element("Line");
            var area = style.Element("Area");
            var coverageStyle = style.Element("Coverage");

            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(feature);

            if (area != null)
            {
                finalStyle.Fill = FieldStyleFromElement(area);
                if (!VerifyFeatureStyleElement(featureCollection, finalStyle.Fill))
                {
                    finalStyle.Fill = null;
                }
            }

            if (line != null)
            {
                finalStyle.Line = FieldStyleFromElement(line);
                if (!VerifyFeatureStyleElement(featureCollection, finalStyle.Line))
                {
                    finalStyle.Line = null;
                }
            }

            if (icon != null)
            {
                Bitmap bitmap = null;
                double scale = 1.0;

                if (icon.Element("Bitmap").SafeHasValue())
                {
                    var srcElement = icon.Element("Bitmap").Element("src");
                    // if <Bitmap> specifies a data uri in a <src> element, include the <src> markup not only the value (the data uri)
                    var definition = srcElement.SafeHasValue() ? srcElement.ToString() : icon.Element("Bitmap").Value; 
                    bitmap = ManagedBitmapServer.Instance.LoadBitmapFromDefinition(definition);
                }

                if (icon.Element("Scale") != null)
                {
                    if (icon.Element("Scale").Attribute("Width") != null &&
                        double.TryParse(icon.Element("Scale").Attribute("Width").Value, out scale))
                    {
                        scale = Math.Max(0.0,Math.Min(1.0,scale));
                    }
                }

                if (icon.Element("Colour").SafeHasValue())
                {
                    finalStyle.Icon = IconFieldStyle.FromImage(bitmap, ColorFromPYXValueStr(icon.Element("Colour").Value), scale);
                }

                if (icon.Element("ColourPaletteField").SafeHasValue() &&
                    icon.Element("ColourPalette").SafeHasValue())
                {
                    finalStyle.Icon = IconFieldStyle.FromImageAndPalette(bitmap, 
                                icon.Element("ColourPaletteField").Value,
                                new ApplicationUtility.Visualization.Palette(icon.Element("ColourPalette").Value).ToContractPalette(),
                                scale);
                }
                
                if (!VerifyFeatureStyleElement(featureCollection, finalStyle.Icon))
                {
                    finalStyle.Icon = null;
                }
            }

            if (coverageStyle != null)
            {
                var coverage = pyxlib.QueryInterface_ICoverage(feature);
                
                if (coverageStyle.Element("Palette").SafeHasValue())
                {
                    finalStyle.Fill = FieldStyle.FromPalette(
                            coverage.getCoverageDefinition().getFieldDefinition(0).getName(),
                            new ApplicationUtility.Visualization.Palette(coverageStyle.Element("Palette").Value).ToContractPalette());
                }

                if (coverageStyle.Element("ShowAsElevation").SafeHasValue())
                {
                    finalStyle.ShowAsElevation = coverageStyle.Element("ShowAsElevation").Value == "1";
                }                
            }
            else
            {
                var coverage = pyxlib.QueryInterface_ICoverage(feature);

                if (coverage.isNotNull())
                {
                    var fieldDefinition = coverage.getCoverageDefinition().getFieldDefinition(0);

                    finalStyle.ShowAsElevation =
                        fieldDefinition.getContext() == PYXFieldDefinition.eContextType.knContextElevation ||
                        fieldDefinition.getContext() == PYXFieldDefinition.eContextType.knContextNone ||
                        fieldDefinition.getContext() == PYXFieldDefinition.eContextType.knContextGreyScale;

                    if (finalStyle.ShowAsElevation == true)
                    {
                        //apply default elevation style
                        finalStyle.Fill = FieldStyle.FromPalette(
                            coverage.getCoverageDefinition().getFieldDefinition(0).getName(),
                            new ApplicationUtility.Visualization.Palette("2  -11000 0 0 0 255  9000 255 255 255 255").ToContractPalette());
                    }
                }
            }
        }

        private static bool VerifyFeatureStyleElement(IFeatureCollection_SPtr featureCollection, FieldStyle fieldStyle)
        {
            if (fieldStyle == null)
            {
                return true;
            }
            if (fieldStyle.Style == FieldStyleOptions.Palette && !String.IsNullOrEmpty(fieldStyle.PaletteExpression))
            {               
                var paletteExpressionFieldFound =
                    featureCollection.getFeatureDefinition()
                        .FieldDefinitions.Any(x => x.getName() == fieldStyle.PaletteExpression);

                if (!paletteExpressionFieldFound)
                {
                    return false;
                }
            }
            if (fieldStyle.Style == FieldStyleOptions.SolidColor && !fieldStyle.Color.HasValue)
            {
                return false;
            }
            return true;
        }

        private static bool SafeHasValue(this XElement element)
        {
            return element != null && !String.IsNullOrEmpty(element.Value);            
        }

        private static FieldStyle FieldStyleFromElement(XElement element)
        {
            FieldStyle style = null;

            if (element.Element("Colour").SafeHasValue())
            {
                style = FieldStyle.FromColor(ColorFromPYXValueStr(element.Element("Colour").Value));

                if (element.Element("Opacity").SafeHasValue())
                {
                    int opacity = 100;

                    if (int.TryParse(element.Element("Opacity").Value, out opacity))
                    {
                        opacity = Math.Min(100, Math.Max(0, opacity));
                        style.Color = Color.FromArgb(opacity * 255 / 100, style.Color.Value);
                    }
                }
            }
            if (element.Element("PaletteField").SafeHasValue() &&
                element.Element("Palette").SafeHasValue())
            {
                var field = element.Element("PaletteField").Value;
                var palette =
                    new ApplicationUtility.Visualization.Palette(element.Element("Palette").Value).ToContractPalette();

                style = FieldStyle.FromPalette(field, palette);                    
            }

            return style;
        }

        private static Color ColorFromPYXValueStr(string str)
        {
            var value = StringUtils.strToPYXValue(str);
            if (value.getArraySize() == 4)
            {
                return Color.FromArgb(value.getUInt8(3), value.getUInt8(0), value.getUInt8(1), value.getUInt8(2));
            }
            else if (value.getArraySize() == 3)
            {
                return Color.FromArgb(value.getUInt8(0), value.getUInt8(1), value.getUInt8(2));
            }
            throw new Exception("PYXValue must have 3 or 4 values in order to extract RGB(A) color");
        }

        private static string ColorToPYXValueStr(Color color)
        {
            return String.Format("uint8_t[3] {0} {1} {2}", color.R, color.G, color.B);
        }

        /// <summary>
        /// Use the style information to style a PYXIS process.
        /// </summary>
        /// <param name="style">The style information.</param>
        /// <param name="process">Pointer to the PYXIS process.</param>
        /// <returns>A PYXIS pointer to the newly styled process.</returns>
        public static IProcess_SPtr ApplyStyle(this Style style, IProcess_SPtr process)
        {
            var clearProcess = StripStyleProcess(process);

            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
            var isCoverage = coverage.isNotNull();

            var styleProcessInfo = isCoverage ?
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.StyledCoverage) :
                new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.StyledFeaturesSummary);

            styleProcessInfo.BorrowNameAndDescription(process);

            if (!isCoverage)
            {
                //make sure we have feature summary output
                var isFeatureGroup = pyxlib.QueryInterface_IFeatureGroup(process.getOutput()).isNotNull();
                if (!isFeatureGroup)
                {
                    clearProcess = PYXCOMFactory.CreateProcess(
                        new PYXCOMProcessCreateInfo(PYXCOMFactory.WellKnownProcesses.FeaturesSummary)
                            .AddInput(0, clearProcess)
                            .BorrowNameAndDescription(clearProcess)
                        );
                }
            }

            styleProcessInfo.AddInput(0, clearProcess);

            if (isCoverage)
            {
                switch ((style.Fill ?? FieldStyle.NoStyle).Style)
                {
                    case FieldStyleOptions.Palette:
                        var fieldDefinition =
                            coverage.getCoverageDefinition()
                                .FieldDefinitions.First(field => field.getName() == style.Fill.PaletteExpression);

                        if (fieldDefinition.isNumeric())
                        {
                            styleProcessInfo
                                .AddAttribute("palette", style.Fill.Palette.ToPyxisStringAsNumericPalette());
                        }
                        else
                        {
                            //we don't support non-numeric palettes
                            throw new NotSupportedException("Can't style coverage with non-numeric palette.");
                        }
                        
                        break;
                }

                if (style.ShowAsElevation.HasValue)
                {
                    styleProcessInfo
                        .AddAttribute("show_as_elevation", style.ShowAsElevation == true ? "1" : "0");
                }
            }
            else
            {
                var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                switch ((style.Icon ?? IconFieldStyle.NoIcon).Style)
                {
                    case FieldStyleOptions.None:
                        styleProcessInfo
                            .AddAttribute("IconStyle", "NoIcon");
                        break;

                    case FieldStyleOptions.SolidColor:
                        styleProcessInfo
                            .AddAttribute("BitmapPipeline", PipeManager.writePipelineToNewString(PYXCOMFactory.CreateBitmapPipelineFromUrl(style.Icon.IconDataUrl)))
                            .AddAttribute("IconIndex", "0")
                            .AddAttribute("IconScale", ((int)(style.Icon.Scale * 100)).ToString())
                            .AddAttribute("IconStyle", "SolidColor")
                            .AddAttribute("IconColour", ColorToPYXValueStr(style.Icon.Color ?? Color.White));
                        break;

                    case FieldStyleOptions.Palette:
                        var fieldDefinition = 
                            featureCollection.getFeatureDefinition()
                                .FieldDefinitions.First(field => field.getName() == style.Icon.PaletteExpression);

                        styleProcessInfo
                            .AddAttribute("BitmapPipeline",
                                PipeManager.writePipelineToNewString(
                                    PYXCOMFactory.CreateBitmapPipelineFromUrl(style.Icon.IconDataUrl)))
                            .AddAttribute("IconIndex", "0")
                            .AddAttribute("IconScale", ((int) (style.Icon.Scale*100)).ToString())
                            .AddAttribute("IconStyle", "Palette")
                            .AddAttribute("IconColourPalette",
                                fieldDefinition.isNumeric()
                                    ? style.Icon.Palette.ToPyxisStringAsNumericPalette()
                                    : style.Icon.Palette.ToPyxisStringAsStringPalette())
                            .AddAttribute("IconColourPaletteField", style.Icon.PaletteExpression);


                        break;
                }

                switch ((style.Fill ?? FieldStyle.NoStyle).Style)
                {
                    case FieldStyleOptions.None:
                        styleProcessInfo
                            .AddAttribute("AreaStyle", "NoFill");
                        break;

                    case FieldStyleOptions.SolidColor:
                        styleProcessInfo
                            .AddAttribute("AreaStyle", "SolidColor")
                            .AddAttribute("AreaColour", ColorToPYXValueStr(style.Fill.Color ?? Color.Red))
                            .AddAttribute("AreaOpacity", (100 * (style.Fill.Color ?? Color.Red).A / 255).ToString());
                        break;

                    case FieldStyleOptions.Palette:
                        var fieldDefinition =
                            featureCollection.getFeatureDefinition()
                                .FieldDefinitions.First(field => field.getName() == style.Fill.PaletteExpression);

                        styleProcessInfo
                            .AddAttribute("AreaStyle", "Palette")
                            .AddAttribute("AreaPalette",
                                fieldDefinition.isNumeric()
                                    ? style.Fill.Palette.ToPyxisStringAsNumericPalette()
                                    : style.Fill.Palette.ToPyxisStringAsStringPalette())
                            .AddAttribute("AreaPaletteField", style.Fill.PaletteExpression);
                        break;
                }

                switch ((style.Line ?? FieldStyle.NoStyle).Style)
                {
                    case FieldStyleOptions.None:
                        styleProcessInfo
                            .AddAttribute("BorderStyle", "NoLine");
                        break;

                    case FieldStyleOptions.SolidColor:
                        styleProcessInfo
                            .AddAttribute("BorderStyle", "SolidColor")
                            .AddAttribute("LineColour", ColorToPYXValueStr(style.Line.Color ?? Color.Black))
                            .AddAttribute("LineOpacity", (100 * (style.Line.Color ?? Color.Black).A / 255).ToString());
                        break;
                }
            }

            var styleProc = PYXCOMFactory.CreateProcess(styleProcessInfo);
            styleProc.initProc(true);
            return styleProc;
        }

        private static IProcess_SPtr StripStyleProcess(IProcess_SPtr process)
        {
            var processClass = process.getSpec().getClass();

            if (processClass == PYXCOMFactory.WellKnownProcesses.StyledFeaturesSummary ||
                processClass == PYXCOMFactory.WellKnownProcesses.StyledCoverage ||
                processClass == PYXCOMFactory.WellKnownProcesses.LineStyledFeatureCollection ||
                processClass == PYXCOMFactory.WellKnownProcesses.IconStyledFeatureCollection ||
                processClass == PYXCOMFactory.WellKnownProcesses.AreaStyledFeatureCollection)
            {
                return StripStyleProcess(process.getParameter(0).getValue(0));
            }
            return process;
        }

        /// <summary>
        /// Create Pyxis.UI.Layers.Globe.StyleInformation using a field of the underlying data.
        /// </summary>
        /// <param name="fieldName">Name of the field.</param>
        /// <param name="palette">Pyxis.UI.Layers.Globe.Palette to style the field values.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.StyleInformation.</returns>
        /// <exception cref="System.NotSupportedException">Throws when styling a coverage without statistics calculated.</exception>
        public static Style CreateStyleByField(this Engine engine, GeoSource geoSource, string fieldName, Palette palette)
        {
            var style = geoSource.Style;
            var spec = engine.GetSpecification(geoSource);

            if (spec.OutputType == PipelineSpecification.PipelineOutputType.Coverage)
            {
                if (style.Fill == null || style.Fill.Palette == null)
                {
                    throw new NotSupportedException("can't generate auto style for coverages");
                }

                var oldPalette = style.Fill.Palette.AsNumericPalette();
                var newPalette = palette.AsNumericPalette();

                newPalette.ScaleToRange(oldPalette.MinValue, oldPalette.MaxValue);

                return new Style
                {
                    Fill = FieldStyle.FromPalette(
                        style.Fill.PaletteExpression,
                        new ApplicationUtility.Visualization.Palette(newPalette).ToContractPalette()),
                    ShowAsElevation = style.ShowAsElevation
                };
            }

            var stats = engine.Statistics(geoSource).GetFieldStatistics(fieldName, 10);
            var finalPalette = ScalePaletteFromStatistics(palette, stats);

            if (ShouldApplyIconStyling(engine,geoSource,spec,style))
            {
                return new Style
                {
                    Icon = IconFieldStyle.ChangePalette(
                        style.Icon ?? RandomStyleGenerator.Create(geoSource.Id).Icon,
                        fieldName, finalPalette)
                };
            }
            else
            {
                return new Style
                {
                    Fill = FieldStyle.FromPalette(fieldName, finalPalette)
                };
            }
        }


        /// <summary>
        /// Create Pyxis.UI.Layers.Globe.StyleInformation using a field of the underlying data.
        /// </summary>
        /// <param name="fieldName">Name of the field.</param>
        /// <param name="palette">Pyxis.UI.Layers.Globe.Palette to style the field values.</param>
        /// <param name="geometry">geometry to use to for styling</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.StyleInformation.</returns>
        public static Style CreateStyleByField(this Engine engine, GeoSource geoSource, string fieldName, Palette palette, IGeometry geometry)
        {
            var style = geoSource.Style;
            var spec = engine.GetSpecification(geoSource);

            var stats = engine.Statistics(geoSource).GetFieldNormalizedStatistics(geometry, fieldName, 10);
            var finalPalette = ScalePaletteFromStatistics(palette, stats);

            if (spec.OutputType == PipelineSpecification.PipelineOutputType.Coverage)
            {
                return new Style
                {
                    Fill = FieldStyle.FromPalette(style.Fill.PaletteExpression, finalPalette),
                    ShowAsElevation = style.ShowAsElevation
                };
            }
            else if (ShouldApplyIconStyling(engine,geoSource,spec,style))
            {
                return new Style
                {
                    Icon = IconFieldStyle.ChangePalette(
                        style.Icon ?? RandomStyleGenerator.Create(geoSource.Id).Icon ,
                        fieldName, finalPalette)
                };
            }
            else
            {
                return new Style
                {
                    Fill = FieldStyle.FromPalette(fieldName, finalPalette)
                };
            }
        }

        /// <summary>
        /// Scale a palette using the given field statistics.
        /// This function will use the distribution of the values inside the FieldStatistics
        /// to adjust the ranges on the input Palette.
        /// </summary>
        /// <param name="palette">Input palette to based the style upon</param>
        /// <param name="stats">FieldStatistics to be used to define the palette range and steps</param>
        /// <returns>A normalized palette based on the FieldStatistics</returns>
        private static Palette ScalePaletteFromStatistics(Palette palette, FieldStatistics stats)
        {
            //normalize color range
            var basePalette = palette.AsNumericPalette();
            basePalette.ScaleToRange(0, 1);

            var finalPalette = new Palette() { Steps = new List<Palette.Step>() };

            double i = 0;
            var stepsCount = stats.Distribution.Histogram.Count;

            foreach (var bin in stats.Distribution.Histogram)
            {
                finalPalette.Steps.Add(new Palette.Step
                {
                    Value = bin.Min,
                    Color = basePalette.GetColor(SafeRatio(i, stepsCount))
                });
                i++;
            }

            if (stepsCount > 0)
            {
                var lastValue = stats.Distribution.Histogram.Last().Max;
                var lastStep = finalPalette.Steps.Last();

                if (lastStep.Value.Equals(lastValue))
                {
                    lastStep.Color = basePalette.GetColor(1);
                }
                else
                {
                    finalPalette.Steps.Add(new Palette.Step
                    {
                        Value = lastValue,
                        Color = basePalette.GetColor(1)
                    });
                }
            }

            return finalPalette;
        }

        private static double SafeRatio(double number, int total)
        {
            if (total > 0) return number / total;
            return 0;
        }


        public static Style CreateDefaultStyle(this Engine engine, GeoSource geoSource)
        {
            return engine.CreateDefaultStyle(geoSource, RandomStyleGenerator.Create(geoSource.Id));
        }
        
        /// <summary>
        /// Create a default style based on an existing style.
        /// </summary>
        /// <param name="engine">Pyxis.Engine to use</param>
        /// <param name="geoSource">GeoSource to create default stlye to</param>
        /// <param name="basedOn">The style on which to base the default style</param>
        /// <returns>The default style</returns>
        public static Style CreateDefaultStyle(this Engine engine, GeoSource geoSource, Style basedOn)
        {
            var style = geoSource.Style;
            var spec = engine.GetSpecification(geoSource);

            if (ShouldApplyIconStyling(engine,geoSource,spec,style))
            {
                return new Style
                {
                    Icon = basedOn.Icon
                };

            }
            else
            {
                
                if (spec.OutputType == PipelineSpecification.PipelineOutputType.Coverage)
                {
                    //detect simple rgb/a coverages
                    
                    if (spec.Fields[0].FieldType == PipelineSpecification.FieldType.Color)
                    {
                        return new Style() { ShowAsElevation = false };
                    }

                    var result = new Style
                    {
                        Fill = style != null ? style.Fill : null,
                        ShowAsElevation = style != null ? style.ShowAsElevation : null
                    };

                    if (basedOn != null)
                    {
                        if (basedOn.ShowAsElevation.HasValue)
                        {
                            result.ShowAsElevation = basedOn.ShowAsElevation;
                        }

                        if (basedOn.Fill != null && basedOn.Fill.Palette != null)
                        {
                            result.Fill =
                                CreateStyleByField(engine, geoSource, result.Fill.PaletteExpression, basedOn.Fill.Palette,
                                    engine.GetGeometry(geoSource)).Fill;
                        }
                        else if (basedOn.Fill != null && basedOn.Fill.Color.HasValue)
                        {
                            var fieldName = spec.Fields[0].Name;
                            
                            //TODO: make really need to have CreateStyleByField from data for coverages.
                            //make a default style from 0 to 100 for now
                            result.Fill = FieldStyle.FromPalette(fieldName, new Palette()
                            {
                                Steps = new List<Palette.Step>()
                                {
                                    new Palette.Step() { Color = Color.FromArgb(0,0,0), Value = 0 },
                                    new Palette.Step() { Color = basedOn.Fill.Color.Value, Value = 100 }
                                }
                            });
                        }
                    }

                    //return the default style we have
                    return result;
                }
                else
                {
                    var result = new Style
                    {
                        Fill = basedOn.Fill,
                        Line = basedOn.Line
                    };

                    //validate fill palette range are applyed correctly
                    if (basedOn.Fill != null && basedOn.Fill.Palette != null)
                    {
                        result.Fill = CreateStyleByField(engine,geoSource, result.Fill.PaletteExpression, basedOn.Fill.Palette).Fill;
                    }

                    return result;
                }
            }
        }

        private static bool ShouldApplyIconStyling(Engine engine, GeoSource geoSource, PipelineSpecification specification, Style style)
        {
            if (style.Icon != null && (style.Icon.Style == FieldStyleOptions.Palette || style.Fill == null))
            {
                return true;
            }

            if (specification.OutputType == PipelineSpecification.PipelineOutputType.Coverage)
            {
                //always use filling
                return false;
            }


            var process = engine.GetProcess(geoSource);

            //check if the geometry of the first 10 features are points...
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

            if (featureCollection.isNotNull())
            {
                var featureIterator = featureCollection.getIterator();
                var pointCount = 0;

                while (!featureIterator.end() && pointCount < 10)
                {
                    if (!IsPointGeometry(featureIterator.getFeature().getGeometry()))
                    {
                        return false;
                    }
                    featureIterator.next();
                    pointCount++;
                }
                return pointCount > 0;
            }

            return false;
        }

        /// <summary>
        /// return true if the geometry looks like a point
        /// </summary>
        /// <param name="geometry">geometry to check</param>
        /// <returns>true if geometry is cell or point</returns>
        private static bool IsPointGeometry(PYXGeometry_SPtr geometry)
        {
            if (geometry.isNull())
            {
                return false;
            }
            var cell = pyxlib.DynamicPointerCast_PYXCell(geometry);
            if (cell.isNotNull())
            {
                return true;
            }

            var vectorGeometry2 = pyxlib.DynamicPointerCast_PYXVectorGeometry2(geometry);

            if (vectorGeometry2.isNotNull())
            {
                var point = pyxlib.DynamicPointerCast_PYXVectorPointRegion(vectorGeometry2.getRegion());

                if (point.isNotNull())
                {
                    return true;
                }
            }

            return false;
        }
    }
}
