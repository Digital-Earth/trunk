using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using ApplicationUtility.Visualization;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;

namespace Pyxis.Core.Analysis
{
    /// <summary>
    /// A utility for creating statistics about the fields of a Pyxis.Contract.Publishing.Pipeline.
    /// </summary>
    public class StatisticsCreator
    {
        private readonly Engine m_engine;
        private readonly Pipeline m_pipeline;        
        private IProcess_SPtr m_process;
        private readonly object m_lockObject = new object();

        private const int MinBinCount = 10;
        private const int MaxBinCount = 200;

        private const int SmallHistogram = 50;

        /// <summary>
        /// Create a StatisticsCreator for a given pipeline.
        /// </summary>
        /// <param name="engine">Pyxis.Engine to use for calculations</param>
        /// <param name="pipeline">Pipeline object to use</param>
        public StatisticsCreator(Engine engine, Pipeline pipeline)
        {
            m_engine = engine;
            m_pipeline = pipeline;
        }

        /// <summary>
        /// Create a StatisticsCreator for a given pipeline.
        /// </summary>
        /// <param name="engine">Pyxis.Engine to use for calculations</param>
        /// <param name="process">pyxlib IProcess_SPtr object to use</param>
        public StatisticsCreator(Engine engine, IProcess_SPtr process)
        {
            m_engine = engine;
            m_process = process;
        }

        private IProcess_SPtr Process
        {
            get
            {
                lock (m_lockObject)
                {
                    if (m_process == null || m_process.isNull()) 
                    {
                        m_process = m_engine.GetProcess(m_pipeline);
                    }
                }
                return m_process;
            }
        }

        private bool? m_isCoverage;

        /// <summary>
        /// Gets if the Pyxis.Contract.Publishing.Pipeline is a coverage.
        /// </summary>
        public bool IsCoverage
        {
            get
            {
                lock (m_lockObject)
                {
                    if (!m_isCoverage.HasValue)
                    {
                        m_isCoverage = pyxlib.QueryInterface_ICoverage(Process.getOutput()).isNotNull();
                    }
                }
                return m_isCoverage.Value;
            }
        }

        private bool? m_isFeatureCollection;

        /// <summary>
        /// Gets if the Pyxis.Contract.Publishing.Pipeline is a feature collection.
        /// </summary>
        public bool IsFeatureCollection
        {
            get
            {
                lock (m_lockObject)
                {
                    if (!m_isFeatureCollection.HasValue)
                    {
                        m_isFeatureCollection = !IsCoverage && pyxlib.QueryInterface_IFeatureCollection(Process.getOutput()).isNotNull();
                    }
                }
                return m_isFeatureCollection.Value;
            }
        }

        private bool? m_isFeatureGroup;

        /// <summary>
        /// Gets if the Pyxis.Contract.Publishing.Pipeline is a feature group.
        /// </summary>
        public bool IsFeatureGroup
        {
            get
            {
                lock (m_lockObject)
                {
                    if (!m_isFeatureGroup.HasValue)
                    {
                        m_isFeatureGroup = pyxlib.QueryInterface_IFeatureGroup(Process.getOutput()).isNotNull();
                    }
                }
                return m_isFeatureGroup.Value;
            }
        }

        /// <summary>
        /// Create Pyxis.Core.IO.FieldStatistics about a field in the Pyxis.Contract.Publishing.Pipeline.
        /// </summary>
        /// <param name="fieldName">The name of the field to create statistics about.</param>
        /// <param name="binCount">Amount of histogram bins to generate (valid range is 10..200)</param>
        /// <returns>The Pyxis.Core.IO.FieldStatistics about the specified field.</returns>
        public FieldStatistics GetFieldStatistics(string fieldName, int binCount = MinBinCount)
        {
            return GetFieldStatistics(null, fieldName, binCount);
        }

        /// <summary>
        /// Create Pyxis.Core.IO.FieldStatistics about a field in the Pyxis.Contract.Publishing.Pipeline inside of a PYXIS.Core.IO.IGeometry.
        /// </summary>
        /// <param name="geometry">The geometry to compute the Pyxis.Core.IO.FieldStatistics within.</param>
        /// <param name="fieldName">The name of the field to create statistics about.</param>
        /// <param name="binCount">Amount of histogram bins to generate (valid range is 10..200)</param>
        /// <returns>The Pyxis.Core.IO.FieldStatistics about the specified field.</returns>
        public FieldStatistics GetFieldStatistics(IGeometry geometry, string fieldName, int binCount = MinBinCount)
        {
            binCount = Math.Min(Math.Max(binCount, MinBinCount), MaxBinCount);

            if (IsCoverage)
            {
                //First, lets see if this coverage look like categores (coverage of type bool / byte)
                var coverageDefinition = pyxlib.QueryInterface_ICoverage(m_process.getOutput()).getCoverageDefinition();
                var fieldIndex = coverageDefinition.getFieldIndex(fieldName);
                if (fieldIndex != -1)
                {
                    var fieldDefinition = coverageDefinition.getFieldDefinition(fieldIndex);

                    switch (fieldDefinition.getType())
                    {
                        case PYXValue.eType.knBool:
                        case PYXValue.eType.knChar:
                        case PYXValue.eType.knUInt8:
                        case PYXValue.eType.knInt8:
                            return GetCoverageFieldStatistics(geometry, fieldName, binCount, HistogramScale.None);
                    }
                }

                //seems this is coverage has many values, keep to linear
                return GetCoverageFieldStatistics(geometry, fieldName, binCount, HistogramScale.Linear);
            }
            if (IsFeatureGroup)
            {
                return GetGroupFieldStatistics(geometry, fieldName, binCount);
            }
            if (IsFeatureCollection)
            {
                return GetFeatureFieldStatistics(geometry, fieldName, binCount);
            }
            return null;
        }

        /// <summary>
        /// Create Pyxis.Core.IO.FieldStatistics about a field in the Pyxis.Contract.Publishing.Pipeline inside of a PYXIS.Core.IO.IGeometry.
        /// </summary>
        /// <param name="geometry">The geometry to compute the Pyxis.Core.IO.FieldStatistics within.</param>
        /// <param name="fieldName">The name of the field to create statistics about.</param>
        /// <param name="binCount">Amount of histogram bins to generate (valid range is 10..200)</param>
        /// <returns>The Pyxis.Core.IO.FieldStatistics about the specified field.</returns>
        public FieldStatistics GetFieldNormalizedStatistics(IGeometry geometry, string fieldName, int binCount = MinBinCount)
        {
            binCount = Math.Min(Math.Max(binCount, MinBinCount), MaxBinCount);

            if (IsCoverage)
            {
                return GetCoverageFieldStatistics(geometry, fieldName, binCount, HistogramScale.Normalized);
            }
            if (IsFeatureGroup)
            {
                return GetGroupFieldStatistics(geometry, fieldName, binCount);
            }
            if (IsFeatureCollection)
            {
                return GetFeatureFieldStatistics(geometry, fieldName, binCount);
            }
            return null;
        }

        /// <summary>
        /// Create Pyxis.Core.IO.FieldStatistics about a field with specific value in the Pyxis.Contract.Publishing.Pipeline inside of a PYXIS.Core.IO.IGeometry.
        /// </summary>
        /// <param name="geometry">The geometry to compute the Pyxis.Core.IO.FieldStatistics within.</param>
        /// <param name="fieldName">The name of the field to create statistics about.</param>
        /// <param name="value">The value to count</param>
        /// <returns>The Pyxis.Core.IO.FieldStatistics about the specified field.</returns>
        public FieldStatistics GetFieldStatisticsWithValue(IGeometry geometry, string fieldName,object value)
        {
            if (IsCoverage)
            {
                return GetCoverageFieldStatisticsWithValue(geometry, fieldName, value);
            }
            if (IsFeatureGroup)
            {
                return GetGroupFieldStatisticsWithValue(geometry, fieldName, value);
            }
            if (IsFeatureCollection)
            {
                return GetFeatureFieldStatisticsWithValue(geometry, fieldName, value);                
            }
            return null;
        }


        private PYXCellHistogram_SPtr GetCoverageHistogram(IGeometry geometry, string fieldName)
        {
            if (!IsCoverage)
            {
                return null;
            }

            if (geometry == null)
            {
                return null;
            }

            var coverage = pyxlib.QueryInterface_ICoverage(Process.getOutput());

            var histogramCalculatorProcess = PYXCOMFactory.CreateProcess(
                   new PYXCOMProcessCreateInfo("5636F858-F2CA-44c9-8E15-F00936FDDE75")
                   .AddInput(0, Process));

            if (histogramCalculatorProcess.initProc() != IProcess.eInitStatus.knInitialized)
            {
                return null;
            }

            var featureProc = PYXCOMFactory.CreateSimpleFeature("1", m_engine.ToPyxGeometry(geometry), null);
            featureProc.initProc();
            var feature = pyxlib.QueryInterface_IFeature(featureProc.getOutput());

            var histogramCalculator = pyxlib.QueryInterface_ICoverageHistogramCalculator(pyxlib.QueryInterface_PYXCOM_IUnknown(histogramCalculatorProcess));

            var histogram = histogramCalculator.getHistogram(coverage.getCoverageDefinition().getFieldIndex(fieldName), feature);

            return histogram;
        }

        private FieldStatistics GetCoverageFieldStatistics(IGeometry geometry, string fieldName, int binCount, HistogramScale scale)
        {
            if (!IsCoverage)
            {
                return null;
            }

            if (geometry == null)
            {
                return null;
            }

            var histogram = GetCoverageHistogram(geometry, fieldName);

            if (histogram == null || histogram.isNull())
            {
                return null;
            }

            var boundaries = histogram.getBoundaries();

            var isNumeric = boundaries.min.getType().ToFieldType() == PipelineSpecification.FieldType.Number;

            var result = new FieldStatistics()
            {
                MinCount = histogram.getArea().min,
                MaxCount = histogram.getArea().max,
                Min = boundaries.min.ToDotNetObject(),
                Max = boundaries.max.ToDotNetObject(),
                Average = isNumeric ? histogram.getAverage().ToDotNetObject() : null,
                Sum = isNumeric ? histogram.getSum().ToDotNetObject() : null,
                Distribution = new Distribution() { Histogram = new List<Distribution.Bin>() }
            };

            var binExtractor = new HistogramCellBinExtractor(histogram, scale, binCount);
            var bins = binExtractor.GetBins();

            foreach (var numericBin in bins.Select(bin => bin as NumericBinData))
            {
                if (numericBin == null)
                {
                    throw new Exception("Histogram bin is not from type NumericBinData");
                }

                result.Distribution.Histogram.Add(new Distribution.Bin()
                {
                    Min = numericBin.RangeMin,
                    Max = numericBin.RangeMax,
                    MinCount = numericBin.ValueMin,
                    MaxCount = numericBin.ValueMax,
                });
            }

            var totalCount = result.Distribution.Histogram.Sum(bin => (bin.MinCount + bin.MaxCount) / 2);
            result.Distribution.Histogram.ForEach(bin => { bin.Frequency = (bin.MinCount + bin.MaxCount) / 2 / totalCount; });

            return result;
        }

        private FieldStatistics GetCoverageFieldStatisticsWithValue(IGeometry geometry, string fieldName, object value)
        {
            var pyxValue = new PYXValue(value.ToString());

            if (!IsCoverage)
            {
                return null;
            }

            if (geometry == null)
            {
                return null;
            }

            var histogram = GetCoverageHistogram(geometry, fieldName);

            if (histogram == null || histogram.isNull())
            {
                return null;
            }

            var result = histogram.getFeatureCount(new RangePYXValue(pyxValue));

            return new FieldStatistics()
            {
                Min = value,
                Max = value,
                MinCount = result.min,
                MaxCount = result.max
            };
        }

        private PYXHistogram_SPtr GetFeatureHistogram(IGeometry geometry, string fieldName)
        {
            if (!IsFeatureCollection)
            {
                return null;
            }

            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(Process.getOutput());
            var fieldIndex = featureCollection.getFeatureDefinition().getFieldIndex(fieldName);

            var featureIterator = geometry == null
                ? featureCollection.getIterator()
                : featureCollection.getIterator(m_engine.ToPyxGeometry(geometry).__deref__());

            var histogram = PYXHistogram.createFromFeatures(featureIterator, fieldIndex);                

            return histogram;
        }

        private PYXHistogram_SPtr GetGroupHistogram(IGeometry geometry, string fieldName)
        {
            if (!IsFeatureGroup)
            {
                return null;
            }

            var featureGroup = pyxlib.QueryInterface_IFeatureGroup(Process.getOutput());
            var fieldIndex = featureGroup.getFeatureDefinition().getFieldIndex(fieldName);

            var histogram = geometry == null ? 
                featureGroup.getFieldHistogram(fieldIndex) : 
                featureGroup.getFieldHistogram(m_engine.ToPyxGeometry(geometry).__deref__(), fieldIndex);

            return histogram;
        }

        private FieldStatistics GetGroupFieldStatistics(IGeometry geometry, string fieldName, int binCount)
        {
            if (!IsFeatureGroup)
            {
                return null;
            }
            
            PYXHistogram_SPtr histogram = GetGroupHistogram(geometry,fieldName);

            if (histogram == null || histogram.isNull())
            {
                return null;
            }

            return CreateFeatureFieldStatistics(binCount, histogram);
        }

        private FieldStatistics GetFeatureFieldStatistics(IGeometry geometry, string fieldName, int binCount)
        {
            if (!IsFeatureCollection)
            {
                return null;
            }

            PYXHistogram_SPtr histogram = GetFeatureHistogram(geometry, fieldName);

            if (histogram == null || histogram.isNull())
            {
                return null;
            }

            return CreateFeatureFieldStatistics(binCount, histogram);
        }

        private static FieldStatistics CreateFeatureFieldStatistics(int binCount, PYXHistogram_SPtr histogram)
        {
            var boundaries = histogram.getBoundaries();

            var isNumeric = boundaries.min.getType().ToFieldType() == PipelineSpecification.FieldType.Number;

            var result = new FieldStatistics()
            {
                MinCount = histogram.getFeatureCount().min,
                MaxCount = histogram.getFeatureCount().max,
                Min = boundaries.min.ToDotNetObject(),
                Max = boundaries.max.ToDotNetObject(),
                Average = isNumeric ? histogram.getAverage().ToDotNetObject() : null,
                Sum = isNumeric ? histogram.getSum().ToDotNetObject() : null,
                Distribution = new Distribution() {Histogram = new List<Distribution.Bin>()}
            };

            //auto adjust bin count for small set of features.
            if (histogram.getFeatureCount().max < SmallHistogram)
            {
                binCount = Math.Max(histogram.getFeatureCount().max, binCount);
            }

            var binExtractor = new HistogramBinExtractor(histogram, HistogramScale.Normalized, binCount);
            var bins = binExtractor.GetBins();

            foreach (var bin in bins)
            {
                if (isNumeric)
                {
                    var numericBin = bin as NumericBinData;

                    if (numericBin == null)
                    {
                        throw new Exception("Histogram bin is not from type NumericBinData");
                    }

                    result.Distribution.Histogram.Add(new Distribution.Bin()
                    {
                        Min = numericBin.RangeMin,
                        Max = numericBin.RangeMax,
                        MinCount = numericBin.ValueMin,
                        MaxCount = numericBin.ValueMax,
                    });
                }
                else
                {
                    var stringBin = bin as StringBinData;

                    if (stringBin == null)
                    {
                        throw new Exception("Histogram bin is not from type StringBinData");
                    }

                    result.Distribution.Histogram.Add(new Distribution.Bin()
                    {
                        Min = stringBin.RangeMin,
                        Max = stringBin.RangeMax,
                        MinCount = stringBin.ValueMin,
                        MaxCount = stringBin.ValueMax,
                    });
                }
            }

            var totalCount = result.Distribution.Histogram.Sum(bin => (bin.MinCount + bin.MaxCount)/2);
            result.Distribution.Histogram.ForEach(bin => { bin.Frequency = (bin.MinCount + bin.MaxCount)/2/totalCount; });

            return result;
        }

        private FieldStatistics GetGroupFieldStatisticsWithValue(IGeometry geometry, string fieldName, object value)
        {
            var pyxValue = new PYXValue(value.ToString());

            if (!IsFeatureGroup)
            {
                return null;
            }

            var histogram = GetGroupHistogram(geometry, fieldName);

            if (histogram == null || histogram.isNull())
            {
                return null;
            }

            var result = histogram.getFeatureCount(new RangePYXValue(pyxValue));

            return new FieldStatistics()
            {
                Min = value,
                Max = value,
                MinCount = result.min,
                MaxCount = result.max
            };
        }

        private FieldStatistics GetFeatureFieldStatisticsWithValue(IGeometry geometry, string fieldName, object value)
        {
            var pyxValue = new PYXValue(value.ToString());

            if (!IsFeatureCollection)
            {
                return null;
            }

            var histogram = GetFeatureHistogram(geometry, fieldName);

            if (histogram == null || histogram.isNull())
            {
                return null;
            }

            var result = histogram.getFeatureCount(new RangePYXValue(pyxValue));

            return new FieldStatistics()
            {
                Min = value,
                Max = value,
                MinCount = result.min,
                MaxCount = result.max
            };
        }
    }
}
