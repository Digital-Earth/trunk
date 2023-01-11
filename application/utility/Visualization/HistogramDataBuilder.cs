using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Linq;
using System;

namespace ApplicationUtility.Visualization
{
    public interface IHistogramBinsProvider
    {
        List<BinData> GetBins();
    }

#region BinData

    public class BinData
    {
        public string Name { get; set; }
        public double ValueMax { get; set; }
        public double ValueMin { get; set; }

        public bool Selected { get; set; }
        public Color? BinColor { get; set; }
        public string ToolTip { get; set; }
    }

    public class NumericBinData : BinData
    {
        public double RangeMin { get; set; }
        public double RangeMax { get; set; }
    }

    public class StringBinData : BinData
    {
        public string RangeMin { get; set; }
        public string RangeMax { get; set; }
    }

#endregion

    public enum HistogramScale
    {
        None,
        Linear,
        Normalized
    }

    public class HistogramBinExtractor : IHistogramBinsProvider
    {
        private PYXHistogram_SPtr m_histogram;
        private HistogramScale m_scale;
        private int m_binCount = 20;

        public HistogramBinExtractor(PYXHistogram_SPtr histogram)
        {
            m_histogram = histogram;
            m_scale = HistogramScale.None;
        }

        public HistogramBinExtractor(PYXHistogram_SPtr histogram,HistogramScale scale, int binCount)
        {
            m_histogram = histogram;
            m_scale = scale;
            m_binCount = binCount;
        }

        private PYXHistogramBinVector ExtractBins()
        {
            switch (m_scale)
            {
                case HistogramScale.None:
                    return m_histogram.getBins();
                case HistogramScale.Linear:
                    return m_histogram.getNormalizedBins(PYXHistogram.Normalization.knLinearBins, m_binCount);
                case HistogramScale.Normalized:
                    return m_histogram.getNormalizedBins(PYXHistogram.Normalization.knNormalizedBin, m_binCount);
                default:
                    throw new Exception("unknown scale: " + m_scale);
            }
        }

        public List<BinData> GetBins()
        {
            var result = new List<BinData>();
            var boundaries = m_histogram.getBoundaries();

            if (m_histogram.getBoundaries().min.getType() == PYXValue.eType.knString)
            {
                foreach (var bin in ExtractBins())
                {
                    result.Add(new StringBinData()
                                {
                                    RangeMin = bin.range.min.getString(),
                                    RangeMax = bin.range.max.getString(),
                                    ValueMin = bin.count.min,
                                    ValueMax = bin.count.max,
                                });
                }
            }
            else
            {
                foreach (var bin in ExtractBins())
                {
                    result.Add(new NumericBinData()
                                {
                                    RangeMin = bin.range.min.getDouble(),
                                    RangeMax = bin.range.max.getDouble(),
                                    ValueMin = bin.count.min,
                                    ValueMax = bin.count.max,
                                });
                }
            }

            return result;
        }
    }

    public class HistogramCellBinExtractor : IHistogramBinsProvider
    {
        private PYXCellHistogram_SPtr m_histogram;
        private HistogramScale m_scale;
        private int m_binCount = 20;

        public HistogramCellBinExtractor(PYXCellHistogram_SPtr histogram)
        {
            m_histogram = histogram;
            m_scale = HistogramScale.None;
        }

        public HistogramCellBinExtractor(PYXCellHistogram_SPtr histogram, HistogramScale scale, int binCount)
        {
            m_histogram = histogram;
            m_scale = scale;
            m_binCount = binCount;
        }

        private PYXCellHistogramBinVector ExtractBins()
        {
            switch (m_scale)
            {
                case HistogramScale.None:
                    return m_histogram.getCellBins();
                case HistogramScale.Linear:
                    return m_histogram.getCellNormalizedBins(PYXHistogram.Normalization.knLinearBins, m_binCount);
                case HistogramScale.Normalized:
                    return m_histogram.getCellNormalizedBins(PYXHistogram.Normalization.knNormalizedBin, m_binCount);
                default:
                    throw new Exception("unknown scale: " + m_scale);
            }
        }

        public List<BinData> GetBins()
        {
            var result = new List<BinData>();
            var boundaries = m_histogram.getBoundaries();
            if (boundaries.min.getType() == PYXValue.eType.knString)
            {                
                foreach (var bin in ExtractBins())
                {
                    result.Add(new StringBinData()
                                {
                                    RangeMin = bin.range.min.getString(),
                                    RangeMax = bin.range.max.getString(),
                                    ValueMin = bin.area.min,
                                    ValueMax = bin.area.max
                                });
                }               
            }
            else
            {
                foreach (var bin in ExtractBins())
                {
                    result.Add(new NumericBinData()
                                {
                                    RangeMin = bin.range.min.getDouble(),
                                    RangeMax = bin.range.max.getDouble(),
                                    ValueMin = bin.area.min,
                                    ValueMax = bin.area.max
                                });
                }               
            }

            return result;
        }
    }

    public class HistogramLegend : IHistogramBinsProvider
    {
        private IHistogramBinsProvider m_binExractor;
        private Dictionary<string, string> m_legend;

        public HistogramLegend(IHistogramBinsProvider binExractor, Dictionary<string, string> legend)            
        {
            m_binExractor = binExractor;
            m_legend = legend;
        }

        public List<BinData> GetBins()
        {
            if (m_binExractor == null)
            {
                return null;
            }
            if (m_legend == null)
            {
                return m_binExractor.GetBins();
            }
            return m_binExractor.GetBins().Select(bin => ConvertBin(bin)).ToList();
        }

        private BinData ConvertBin(BinData bin)
        {
            if (bin is NumericBinData)
            {
                //two steps: convert bin from numeric to string and then call ConvertBin to apply legened
                var numericBin = bin as NumericBinData;
                return ConvertBin(new StringBinData()
                    {
                        RangeMin = numericBin.RangeMin.ToString(),
                        RangeMax = numericBin.RangeMax.ToString(),
                        ValueMin = numericBin.ValueMin,
                        ValueMax = numericBin.ValueMax
                    });
                
            }
            else if (bin is StringBinData)
            {
                return ConvertBin(bin as StringBinData);
            }
            else
            {
                return bin;
            }
        }

        private BinData ConvertBin(StringBinData bin)
        {
            var newMin = bin.RangeMin;
            var newMax = bin.RangeMin;
            //apply legend if available
            m_legend.TryGetValue(newMin, out newMin);
            m_legend.TryGetValue(newMax, out newMax);

            return new StringBinData()
                    {
                        RangeMin = newMin,
                        RangeMax = newMax,
                        ValueMin = bin.ValueMin,
                        ValueMax = bin.ValueMax
                    };
        }
    }

    public class HistogramDataLabeler : IHistogramBinsProvider
    {
        private IHistogramBinsProvider m_binExractor;

        public HistogramDataLabeler(IHistogramBinsProvider binExractor)
        {
            m_binExractor = binExractor;
        }

        public List<BinData> GetBins()
        {
            if (m_binExractor == null)
            {
                return null;
            }
            var bins = m_binExractor.GetBins();
            var total = bins.Sum(bin => bin.ValueMax);
           
            foreach(var bin in bins)
            {
                CreateBinName(bin);
                CreateToolTip(bin, total);
            }

            return bins;
        }        

        virtual protected void CreateBinName(BinData bin)
        {
            if (bin is NumericBinData)
            {
                var numericBin = bin as NumericBinData;

                if (numericBin.RangeMin == numericBin.RangeMax)
                {
                    numericBin.Name = String.Format("{0:F3}", numericBin.RangeMin);
                }
                else
                {
                    numericBin.Name = String.Format("{0:F3}~{1:F3}", numericBin.RangeMin, numericBin.RangeMax);
                }
            }
            else if (bin is StringBinData)
            {
                var stringBin = bin as StringBinData;

                if (stringBin.RangeMin == stringBin.RangeMax)
                {
                    stringBin.Name = stringBin.RangeMin;
                }
                else
                {
                    stringBin.Name = stringBin.RangeMin + "~" + stringBin.RangeMax;
                }
            }
        }

        virtual protected string CreateToolTip(BinData bin, double total)
        {
            string label = bin.Name + " : ";

            if (bin.ValueMin == bin.ValueMax)
            {
                label += String.Format("{0} ({1:F3}%)",
                    bin.ValueMin,
                    bin.ValueMin * 100 / total);
            }
            else
            {
                label += String.Format("{0} ~ {1} ({2:F3}%~{3:F3}%)",
                    bin.ValueMin,bin.ValueMax,
                    bin.ValueMin * 100 / total, bin.ValueMax * 100 / total);
            }
            return label;
        }
    }

    public class CellHistogramDataLabeler : HistogramDataLabeler
    {
        public CellHistogramDataLabeler(IHistogramBinsProvider binExractor) : base(binExractor)
        {            
        } 

        override protected string CreateToolTip(BinData bin, double total)
        {
            string label = bin.Name + " : ";

            if (bin.ValueMin == bin.ValueMax)
            {
                label += String.Format("{0} ({1:F3}%)",
                    ToHectar(bin.ValueMin),
                    bin.ValueMin * 100 / total);
            }
            else
            {
                label += String.Format("{0} ~ {1} ({2:F3}%~{3:F3}%)",
                    ToHectar(bin.ValueMin), ToHectar(bin.ValueMax),
                    bin.ValueMin * 100 / total, bin.ValueMax * 100 / total);
            }
            return label;
        }

        private string ToHectar(double value)
        {
            return string.Format("{0:F3}", value / 10000.0 + " ha");
        }
    }
}
