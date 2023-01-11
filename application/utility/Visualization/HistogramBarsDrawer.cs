using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace ApplicationUtility.Visualization
{
    public class HistogramBarsDrawer : IHistogramDrawer
    {
        private class TextAnnotation
        {
            public string Text { get; set; }
            public SizeF Size { get; set; }
        }

        private RectangleF _rectangle;
        private float _edgeSpacing = 0;
        public bool Vertical { get; set; }
        public Font Font { get; set; }
        public Palette Palette { get; set; }
        public PaletteUsageMethod PaletteUsage { get; set; }
        public Pen SelectedPen { get; set; }

        public float EdgeSpacing
        {
            get { return _edgeSpacing; }
            set
            {
                _edgeSpacing = value;
            }
        }

        public RectangleF Rectangle
        {
            get { return _rectangle; }
            set
            {
                _rectangle = value;
            }
        }

        public HistogramBarsDrawer(RectangleF rectangle, float edgeSpacing, Font font)
        {
            _rectangle = rectangle;
            _edgeSpacing = edgeSpacing;

            Font = font;
            Palette = new Palette("2  0 120 150 255 255 1 120 255 150 255");
            SelectedPen = new Pen(Color.DarkOrange, 2);
        }


        public int GetBinIndex(List<BinData> data, Point point)
        {
            var barWidth = (_rectangle.Width - _edgeSpacing * 2) / data.Count;
            var barSpacing = Math.Min(5, barWidth / 8);

            var i = 0;
            foreach (var bin in data)
            {
                if (_rectangle.X + _edgeSpacing + barWidth * i + barSpacing < point.X &&
                    _rectangle.X + _edgeSpacing + barWidth * i + barWidth - barSpacing > point.X)
                {
                    return i;
                }
                ++i;
            }
            return -1;
        }

        public void Draw(List<BinData> data, Graphics graphics)
        {
            var barWidth = (_rectangle.Width - _edgeSpacing * 2) / data.Count;
            var barSpacing = Math.Min(5, barWidth / 8);

            var barsMaxHeight = _rectangle.Height - _edgeSpacing * 2;

            var annotations = new List<TextAnnotation>();

            var max = 0.0;
            bool doubleSpacing = false;

            foreach (var bin in data)
            {
                if (max < bin.ValueMax)
                {
                    max = bin.ValueMax;
                }
                SizeF tempSize = graphics.MeasureString(bin.Name, Font);
                var annotation = new TextAnnotation()
                                     {
                                         Text = bin.Name,
                                         Size = (Vertical) ? new SizeF(tempSize.Height, tempSize.Width) : tempSize
                                     };

                if (annotation.Size.Width > barWidth * 2 - barSpacing * 2 )
                {
                    while (annotation.Text.Length > 0 && annotation.Size.Width > barWidth * 2 - barSpacing * 2)
                    {
                        annotation.Text = annotation.Text.Remove(annotation.Text.Length - 1);
                        annotation.Size = graphics.MeasureString(annotation.Text + "...", Font);
                        if (Vertical)
                        {
                            annotation.Size = new SizeF(annotation.Size.Height, annotation.Size.Width);
                        }
                    }
                    if (annotation.Text.Length > 0)
                    {
                        annotation.Text += "...";
                    }
                    else
                    {
                        annotation.Size = (Vertical) ? 
                            new SizeF(0, annotation.Size.Height):
                            new SizeF(annotation.Size.Height, 0);
                    }
                }

                if (annotation.Size.Width > barWidth - barSpacing * 2 && !Vertical)
                    doubleSpacing = true;

                annotations.Add(annotation);
            }

            barsMaxHeight -= annotations[0].Size.Height;
            if (doubleSpacing)
            {
                barsMaxHeight -= annotations[0].Size.Height;
            }

            graphics.SmoothingMode = SmoothingMode.HighQuality;

            var i = 0;

            var secondMax = 0.0;
            foreach (var bin in data)
            {
                if (secondMax < bin.ValueMax && bin.ValueMax < max)
                {
                    secondMax = bin.ValueMax;
                }
            }

            if (secondMax > 0 && secondMax < max / 5)
            {
                max = secondMax * 2;
            }

            graphics.DrawLine(Pens.Black,
                              _rectangle.Left + _edgeSpacing, _rectangle.Top + _edgeSpacing + barsMaxHeight,
                              _rectangle.Right - _edgeSpacing, _rectangle.Top + _edgeSpacing + barsMaxHeight);

            var linesPen = barWidth < 20 ? new Pen(Color.FromArgb((int)(Math.Max(0, (barWidth - 10) / 10 * 255)), Color.Black)) : Pens.Black;
            var lighterPen = new Pen(Color.FromArgb(linesPen.Color.A / 2, linesPen.Color));

            var line = 0;
            var needTwoLines = false;

            foreach (var bin in data)
            {
                var color = GetBinColor(data, i);
                Brush lightBrush = new SolidBrush(Color.FromArgb(100, color));
                Brush brush = new SolidBrush(color);

                var average = (bin.ValueMax + bin.ValueMin) / 2;

                var left = (float)(_rectangle.X + _edgeSpacing + i * barWidth + barSpacing);
                var width = (float)(barWidth - barSpacing * 2);
                var topMax = (float)(_rectangle.Y + _edgeSpacing + (max - bin.ValueMax) / max * (barsMaxHeight));
                var heightMax = (float)(bin.ValueMax / max * (barsMaxHeight));

                var topMin = (float)(_rectangle.Y + _edgeSpacing + (max - bin.ValueMin) / max * (barsMaxHeight));
                var heightMin = (float)(bin.ValueMin / max * (barsMaxHeight));

                if (bin.ValueMax > max)
                {
                    topMax = _rectangle.Y + _edgeSpacing;
                    heightMax = barsMaxHeight;

                    topMin = (float)(_rectangle.Y + _edgeSpacing + (bin.ValueMax - bin.ValueMin) / bin.ValueMax * (barsMaxHeight));
                    heightMin = (float)(bin.ValueMin / bin.ValueMax * (barsMaxHeight));
                }

                graphics.FillRectangle(lightBrush, left, topMax, width, heightMax);
                graphics.FillRectangle(brush, left, topMin, width, heightMin);

                if (bin.Selected)
                {
                    graphics.DrawRectangle(SelectedPen, left, topMax, width, heightMax);
                    graphics.DrawLine(new Pen(Color.FromArgb(100, SelectedPen.Color)),
                                      left + 1, topMin, left + width - 1, topMin);
                }
                else
                {
                    graphics.DrawRectangle(linesPen, left, topMax, width, heightMax);
                    graphics.DrawLine(lighterPen, left + 1, topMin, left + width - 1, topMin);
                }

                var annotation = annotations[i];
                if (annotation.Text.Length > 0)
                {
                    if (annotation.Size.Width > barWidth - barSpacing * 2 && !Vertical || needTwoLines)
                    {
                        line = 1 - line;
                        needTwoLines = annotation.Size.Width > barWidth - barSpacing * 2;
                    }
                    else
                    {
                        line = 0;
                    }

                    System.Drawing.StringFormat drawFormat = new System.Drawing.StringFormat();
                    if (Vertical)
                    {
                    	drawFormat.FormatFlags = StringFormatFlags.DirectionVertical;
                    }

                    graphics.DrawString(annotation.Text, Font, Brushes.Black,
                                        _rectangle.X + _edgeSpacing + i * barWidth + (barWidth - annotation.Size.Width) / 2,
                                        _rectangle.Y + _edgeSpacing + barsMaxHeight + 5 + annotation.Size.Height * line, drawFormat);

                    if (line > 0)
                    {
                        graphics.DrawLine(Pens.Gray,
                                          _rectangle.X + _edgeSpacing + i * barWidth + barWidth / 2,
                                          _rectangle.Y + _edgeSpacing + barsMaxHeight + 5,
                                          _rectangle.X + _edgeSpacing + i * barWidth + barWidth / 2,
                                          _rectangle.Y + _edgeSpacing + barsMaxHeight + 5 + annotation.Size.Height * line);
                    }
                }

                if (bin.ValueMax > max)
                {
                    graphics.FillRectangle(Brushes.White, left, _rectangle.Top + barsMaxHeight / 3, width, 5);
                    graphics.DrawLine(Pens.Black, left - 2, _rectangle.Top + barsMaxHeight / 3, left + width + 2, _rectangle.Top + barsMaxHeight / 3);
                    graphics.DrawLine(Pens.Black, left - 2, _rectangle.Top + barsMaxHeight / 3 + 5, left + width + 2, _rectangle.Top + barsMaxHeight / 3 + 5);
                }
                ++i;
            }
        }

        public Color GetBinColor(List<BinData> data, int binIndex)
        {
            var bin = data[binIndex];

            if (bin.BinColor.HasValue)
            {
                return bin.BinColor.Value;
            }

            if (PaletteUsage != null)
            {
                return PaletteUsage(bin, binIndex, Palette);
            }

            return Palette.IsStringPalette ? 
                Palette.GetColor(bin.Name) : 
                Palette.GetColor((0.0f + binIndex) / data.Count);
        }
    }
}