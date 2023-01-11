using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;

namespace Pyxis.IO.Styles
{
    /// <summary>
    /// A styled GeoSource viewable on the globe.
    /// </summary>
    public class StyledGeoSource : IDisposable
    {
        /// <summary>
        /// Place holder to the engine that need to be used for processing this StyledGeoSource.
        /// </summary>
        private readonly Engine m_engine;

        /// <summary>
        /// Gets a unique identifier.
        /// </summary>
        public Guid Id { get; private set; }

        /// <summary>
        /// Gets if the underlying data is a coverage.
        /// </summary>
        public bool IsCoverage { get; private set; }

        /// <summary>
        /// Gets the Pyxis.Contract.Publishing.GeoSource being styled.
        /// </summary>
        public GeoSource GeoSource { get; private set; }
        /// <summary>
        /// Gets the styled Pyxis.Contract.Publishing.Pipeline for visualizing the styled GeoSource.
        /// </summary>
        public Pipeline StyledPipeline { get; private set; }
        /// <summary>
        /// Gets the Pyxis.UI.Layers.Globe.StyleInformation.
        /// </summary>
        public Style Style { get; private set; }

        /// <summary>
        /// Gets a ProcRef to the underlying PYXIS process.
        /// </summary>
        public ProcRef ProcRef
        {
            get
            {
                return new ProcRef(m_process);
            }
        }


        private IProcess_SPtr m_process;


        private bool m_disposed = false;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="engine">The engine.</param>
        /// <param name="geoSource">The geo source.</param>
        protected StyledGeoSource(Engine engine, GeoSource geoSource)
        {
            m_engine = engine;
            Id = Guid.NewGuid();
            GeoSource = geoSource;
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.StyledGeoSource from a Pyxis.Contract.Publishing.GeoSource.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="geoSource">The Pyxis.Contract.Publishing.GeoSource to style.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.StyledGeoSource.</returns>
        public static StyledGeoSource Create(Engine engine, GeoSource geoSource)
        {
            if (engine == null)
            {
                throw new ArgumentNullException("engine");
            }

            if (geoSource == null)
            {
                throw new ArgumentNullException("geoSource");
            }

            var styledGeoSource = new StyledGeoSource(engine, geoSource);

            if (geoSource.Styles != null && geoSource.Styles.Count > 0)
            {
                styledGeoSource.StyledPipeline = engine.GetChannel().GetResource(geoSource.Styles[0]);
            }

            styledGeoSource.m_process = engine.GetProcess(styledGeoSource.StyledPipeline ?? styledGeoSource.GeoSource);

            if (styledGeoSource.m_process == null || styledGeoSource.m_process.isNull())
            {
                throw new Exception("failed to create style pipeline for GeoSource");
            }

            styledGeoSource.IsCoverage = pyxlib.QueryInterface_ICoverage(styledGeoSource.m_process.getOutput()).isNotNull();
            
            // Try to extract the style information from the process
            styledGeoSource.Style = styledGeoSource.m_process.ExtractStyle();

            return styledGeoSource;
        }

        /// <summary>
        /// Create a Pyxis.UI.Layers.Globe.StyledGeoSource from a Pyxis.Contract.Publishing.GeoSource.
        /// </summary>
        /// <param name="engine">The Pyxis.Core.Engine to use.</param>
        /// <param name="geoSource">The Pyxis.Contract.Publishing.GeoSource to style.</param>
        /// <param name="style">Style to apply to the GeoSource.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.StyledGeoSource.</returns>
        public static StyledGeoSource Create(Engine engine, GeoSource geoSource, Style style)
        {
            var styledGeoSource = new StyledGeoSource(engine, geoSource);

            styledGeoSource.m_process = engine.GetProcess(styledGeoSource.GeoSource);
            styledGeoSource.IsCoverage = pyxlib.QueryInterface_ICoverage(styledGeoSource.m_process.getOutput()).isNotNull();
            styledGeoSource.ApplyStyle(style);

            return styledGeoSource;
        }

        /// <summary>
        /// Create Pyxis.UI.Layers.Globe.StyleInformation using a field of the underlying data.
        /// </summary>
        /// <param name="fieldName">Name of the field.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.StyleInformation.</returns>
        public Style CreateStyleByField(string fieldName)
        {
            return CreateStyleByField(fieldName, new[] { 
                Color.Black,
                Color.Purple,
                Color.Pink,
                Color.Yellow});
        }

        /// <summary>
        /// Create Pyxis.UI.Layers.Globe.StyleInformation using a field of the underlying data.
        /// </summary>
        /// <param name="fieldName">Name of the field.</param>
        /// <param name="paletteColors">System.Drawing.Color array to generate a Pyxis.UI.Layers.Globe.Palette to style the field values.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.StyleInformation.</returns>
        public Style CreateStyleByField(string fieldName, Color[] paletteColors)
        {
            return CreateStyleByField(fieldName, Palette.Create(paletteColors));
        }

        /// <summary>
        /// Create Pyxis.UI.Layers.Globe.StyleInformation using a field of the underlying data.
        /// </summary>
        /// <param name="fieldName">Name of the field.</param>
        /// <param name="palette">Pyxis.UI.Layers.Globe.Palette to style the field values.</param>
        /// <returns>The created Pyxis.UI.Layers.Globe.StyleInformation.</returns>
        /// <exception cref="System.NotSupportedException">Throws when styling a coverage without statistics calculated.</exception>
        public Style CreateStyleByField(string fieldName, Palette palette)
        {
            if (IsCoverage)
            {
                if (Style.Fill == null || Style.Fill.Palette == null)
                {
                    throw new NotSupportedException("can't generate auto style for coverages");
                }

                var oldPalette = Style.Fill.Palette.AsNumericPalette();
                var newPalette = palette.AsNumericPalette();

                newPalette.ScaleToRange(oldPalette.MinValue, oldPalette.MaxValue);

                return new Style
                {
                    Fill = FieldStyle.FromPalette(
                        Style.Fill.PaletteExpression,
                        new ApplicationUtility.Visualization.Palette(newPalette).ToContractPalette()),
                    ShowAsElevation = Style.ShowAsElevation
                };
            }

            var stats = m_engine.Statistics(GeoSource).GetFieldStatistics(fieldName, 10);

            var finalPalette = ScalePaletteFromStatistics(palette, stats);
            if (ShouldApplyIconStyling(Style))
            {
                return new Style
                {
                    Icon = IconFieldStyle.ChangePalette(
                        Style.Icon ?? IconFieldStyle.FromImage(Icons.Circle.AsBitmap(), 0.2),
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
        public Style CreateStyleByField(string fieldName, Palette palette, IGeometry geometry)
        {
            var stats = m_engine.Statistics(GeoSource).GetFieldNormalizedStatistics(geometry, fieldName, 10);

            var finalPalette = ScalePaletteFromStatistics(palette, stats);

            if (IsCoverage)
            {
                return new Style
                {
                    Fill = FieldStyle.FromPalette(Style.Fill.PaletteExpression, finalPalette),
                    ShowAsElevation = Style.ShowAsElevation
                };
            }
            else if (ShouldApplyIconStyling(Style))
            {
                return new Style
                {
                    Icon = IconFieldStyle.ChangePalette(
                        Style.Icon ?? IconFieldStyle.FromImage(Icons.Circle.AsBitmap(), 0.2),
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
        private Palette ScalePaletteFromStatistics(Palette palette, FieldStatistics stats)
        {
            //normalize color range
            var basePalette = palette.AsNumericPalette();
            basePalette.ScaleToRange(0, 1);

            var finalPalette = new Palette() {Steps = new List<Palette.Step>()};

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

        /// <summary>
        /// Create a default style based on an existing style.
        /// </summary>
        /// <param name="basedOn">The style on which to base the default style</param>
        /// <returns>The default style</returns>
        public Style CreateDefaultStyle(Style basedOn)
        {
            if (ShouldApplyIconStyling(Style))
            {
                return new Style
                {
                    Icon = basedOn.Icon
                };

            }
            else
            {
                if (IsCoverage)
                {
                    //detect simple rgb/a coverages
                    var spec = m_engine.GetSpecification(GeoSource);
                    if (spec.Fields[0].FieldType == PipelineSpecification.FieldType.Color)
                    {
                        return new Style() {ShowAsElevation = false};
                    }

                    var result = new Style
                    {
                        Fill = Style != null ? Style.Fill : null,
                        ShowAsElevation = Style != null ? Style.ShowAsElevation : null
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
                                CreateStyleByField(result.Fill.PaletteExpression, basedOn.Fill.Palette,
                                    m_engine.GetGeometry(GeoSource)).Fill;
                        }
                        else if (basedOn.Fill != null && basedOn.Fill.Color.HasValue)
                        {
                            var fieldName =
                                pyxlib.QueryInterface_ICoverage(m_process.getOutput())
                                    .getCoverageDefinition()
                                    .getFieldDefinition(0)
                                    .getName();

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
                        result.Fill = CreateStyleByField(result.Fill.PaletteExpression, basedOn.Fill.Palette).Fill;
                    }

                    return result;
                }
            }
        }

        private bool ShouldApplyIconStyling(Style style)
        {
            if (IsCoverage)
            {
                //always use filling
                return false;
            }
            if (style.Icon != null && (style.Icon.Style == FieldStyleOptions.Palette || style.Fill == null))
            {
                return true;
            }

            //check if the geometry of the first 10 features are points...
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(m_process.getOutput());

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
        private bool IsPointGeometry(PYXGeometry_SPtr geometry)
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

        private static double SafeRatio(double number, int total)
        {
            if (total > 0) return number / total;
            return 0;
        }

        /// <summary>
        /// Apply a new style to the GeoSource.
        /// </summary>
        /// <param name="newStyle">Pyxis.UI.Layers.Globe.StyleInformation describing the new style.</param>
        public void ApplyStyle(Style newStyle)
        {
            Style = newStyle;
            var newProcess = Style.ApplyStyle(m_process);

            Trace.info(GeoSource.Metadata.Name + " have new style (new ProcRef: " +
                       pyxlib.procRefToStr(new ProcRef(newProcess)) + ", was: " +
                       pyxlib.procRefToStr(new ProcRef(m_process)));

            StyledPipeline = new Pipeline()
            {
                Type = ResourceType.Pipeline,
                BasedOn = new List<ResourceReference>
                {
                    new ResourceReference
                    {
                        Type = ResourceType.GeoSource,
                        Id = GeoSource.Id,
                        Version = GeoSource.Version
                    }
                },
                Id = Guid.NewGuid(),
                Version = Guid.NewGuid(),
                ProcRef = pyxlib.procRefToStr(new ProcRef(newProcess)),
                Definition = PipeManager.writePipelineToNewString(newProcess)
            };

            //clear old pointer
            m_process.Dispose();

            //use new pointer. C# IProcess_SPtr is a class, so Process = newProcess is just change pointer.
            m_process = newProcess;
        }

        /// <summary>
        /// Add the Pyxis.UI.Layers.Globe.StyledGeoSource to a PYXIS viewpoint process.
        /// </summary>
        /// <param name="newViewPointProcess">PYXIS viewpoint process to add the Pyxis.UI.Layers.Globe.StyledGeoSource to.</param>
        public void AddToViewPoint(IProcess_SPtr newViewPointProcess)
        {
            if (m_process.initProc(true) != IProcess.eInitStatus.knInitialized)
            {
                Trace.error("failed to initialized a styled pipeline for GeoSource: " + GeoSource.Metadata.Name);
                return;
            }
            if (IsCoverage)
            {
                newViewPointProcess.getParameter(0).addValue(m_process);
            }
            else
            {
                newViewPointProcess.getParameter(1).addValue(m_process);
            }
        }

        /// <summary>
        /// Called by the garbage collector to release any unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            if (!m_disposed)
            {
                m_process.Dispose();
                m_process = null;
            }
            m_disposed = true;
        }
    }
}
