using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ApplicationUtility.Visualization.Controls
{
    public partial class HistogramControl : UserControl
    {
        private int m_lastBinToolTop = -1;

        public IHistogramDrawer HistogramDrawer { get; set; }
        public List<BinData> Bins { get; set; }

        private string m_histogramMessage;
        public string HistogramMessage 
        { 
            get
            {
                return m_histogramMessage;
            }
            set
            {
                if (m_histogramMessage != value)
                {
                    m_histogramMessage = value;
                    Invalidate();
                }
            }
        }

        public event EventHandler OnSelectedBinChange;

        private int m_selectedBinIndex = -1;
        public int SelectedBinIndex
        {
            get
            {
                return m_selectedBinIndex;
            }
            set
            {
                if (m_selectedBinIndex != value)
                {
                    if (m_selectedBinIndex < -1 )
                    {
                        throw new ArgumentOutOfRangeException("SelectedBinIndex");
                    }
                    if (Bins != null && m_selectedBinIndex >= Bins.Count)
                    {
                        throw new ArgumentOutOfRangeException("SelectedBinIndex");
                    }
                    if (Bins == null && value != -1)
                    {
                        throw new ArgumentOutOfRangeException("SelectedBinIndex");
                    }
                    m_selectedBinIndex = value;

                    if (Bins != null)
                    {
                        int i = 0;
                        foreach(var bin in Bins)
                        {
                            bin.Selected = i == m_selectedBinIndex;
                            ++i;
                        }
                    }
                    Pyxis.Utilities.InvokeExtensions.SafeInvoke(OnSelectedBinChange,this);
                    Invalidate();
                }
            }
        }

        public HistogramControl()
        {
            InitializeComponent();
            HistogramDrawer = new HistogramBarsDrawer(new RectangleF(PointF.Empty, Size), 5, Font);
        }

        private void HistogramControl_Paint(object sender, PaintEventArgs e)
        {
            HistogramDrawer.Rectangle = new RectangleF(PointF.Empty,Size);

            if (Bins != null && HistogramDrawer != null)
            {
                HistogramDrawer.Draw(Bins, e.Graphics);
            }
            else if (m_histogramMessage != null)
            {
                var size = e.Graphics.MeasureString(m_histogramMessage, Font);

                e.Graphics.DrawString(m_histogramMessage, Font, Brushes.Black,
                                      Width / 2 - size.Width / 2,
                                      Height / 2 - size.Height / 2);
            }
        }

        private void HistogramControl_MouseMove(object sender, MouseEventArgs e)
        {
            if (Bins == null || HistogramDrawer == null)
            {
                m_toolTip.SetToolTip(this, null);
                return;
            }

            var i = HistogramDrawer.GetBinIndex(Bins, e.Location);

            if (i >= 0 && i != m_lastBinToolTop)
            {
                m_lastBinToolTop = i;

                var bin = Bins[i];

                if (String.IsNullOrEmpty(bin.ToolTip))
                {
                    if (bin.ValueMin == bin.ValueMax)
                    {
                        m_toolTip.SetToolTip(this,
                                           bin.Name + " has " + bin.ValueMin + " features");
                    }
                    else
                    {
                        m_toolTip.SetToolTip(this,
                                           bin.Name + " has " + bin.ValueMin + " ~ " + bin.ValueMax + " features");
                    }
                }
                else
                {
                    m_toolTip.SetToolTip(this, bin.ToolTip);
                }
            }
        }

        private void HistogramControl_MouseClick(object sender, MouseEventArgs e)
        {
            if (Bins == null || HistogramDrawer == null)
            {
                return;
            }

            SelectedBinIndex = HistogramDrawer.GetBinIndex(Bins, e.Location);            
        }
        
        public void ApplyPalette(Palette palette,bool useBinsRanges = true)
        {
            if (Bins == null) return;

            if (palette.IsStringPalette)
            {
                if (IsNumericBins)
                {
                    //what can we do here?
                    return;
                }

                foreach (StringBinData bin in Bins)
                {
                    bin.BinColor = palette.GetColor(bin.RangeMin);                    
                }
            }
            else
            {
                var fixedPalette = palette.GetNumericPalette();

                if (IsNumericBins && useBinsRanges)
                {
                    var min = ((Bins.First() as NumericBinData).RangeMin + (Bins.First() as NumericBinData).RangeMax) / 2;
                    var max = ((Bins.Last() as NumericBinData).RangeMin + (Bins.Last() as NumericBinData).RangeMax) / 2;
                    
                    if (fixedPalette.MinValue != min || fixedPalette.MaxValue != max)
                    {
                        fixedPalette = new NumericPalette(fixedPalette.ToString());
                        fixedPalette.ScaleToRange(min, max);
                    }
                    foreach (NumericBinData bin in Bins)
                    {
                        bin.BinColor = fixedPalette.GetColor((bin.RangeMin + bin.RangeMax) / 2);
                    }
                }
                else
                {
                    //rescale palette to mach number of bins
                    fixedPalette = new NumericPalette(fixedPalette.ToString());
                    fixedPalette.ScaleToRange(0, Bins.Count-1);

                    int i = 0;
                    foreach (BinData bin in Bins)
                    {
                        bin.BinColor = fixedPalette.GetColor(i);
                        ++i;
                    }
                }
            }
        }

        public Palette ExtractPaletteFromBins()
        {
            if (IsNumericBins)
            {
                var palette = new NumericPalette();

                foreach (NumericBinData bin in Bins)
                {
                    palette.Steps.Add(new NumericPalette.Step() { Color = bin.BinColor.Value, Value = (bin.RangeMin+bin.RangeMax)/2 });
                }

                return new Palette(palette);
            }
            else
            {
                var palette = new StringPalette();

                foreach (StringBinData bin in Bins)
                {
                    palette.Steps[bin.RangeMin] = bin.BinColor.Value;
                }

                return new Palette(palette);
            }
        }


        public bool IsNumericBins
        {
            get
            {
                return Bins[0] is NumericBinData;
            }
        }
    }
}
