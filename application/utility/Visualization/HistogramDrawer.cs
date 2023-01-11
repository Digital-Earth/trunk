using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System;

namespace ApplicationUtility.Visualization
{

    public delegate Color PaletteUsageMethod(BinData bin, int index, Palette palette);

    public interface IHistogramDrawer
    {      
        Font Font { get; set; }
        Palette Palette { get; set; }
        PaletteUsageMethod PaletteUsage { get; set; }

        Pen SelectedPen { get; set; }
        float EdgeSpacing { get; set; }
        RectangleF Rectangle { get; set; }

        void Draw(List<BinData> data, Graphics graphics);
        int GetBinIndex(List<BinData> data, Point p);
        Color GetBinColor(List<BinData> data, int binIndex);
    }
}
