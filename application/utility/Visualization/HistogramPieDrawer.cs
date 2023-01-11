using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace ApplicationUtility.Visualization
{
    public class HistogramPieDrawer : IHistogramDrawer
    {
        private class PieHelper
        {
            public float X { get; private set; }
            public float Y { get; private set; }
            public float R { get; private set; }

            public RectangleF Rectangle
            {
                get { return new RectangleF(X - R, Y - R, R * 2, R * 2); }
            }

            public PieHelper(float x, float y, float r)
            {
                X = x;
                Y = y;
                R = r;
            }

            public PointF GetPoint(float degree)
            {
                return GetPoint(degree, R);
            }

            public PointF GetPoint(float degree, float r)
            {
                return new PointF((float)(X + r * Math.Cos(degree / 180 * Math.PI)), (float)(Y + r * Math.Sin(degree / 180 * Math.PI)));
            }

            public float GetDegree(float x, float y)
            {
                var d = (float)(Math.Atan2(y - Y, x - X) * 180 / Math.PI);
                if (d < 0) d += 360;
                return d;
            }

            public float GetRadius(float x, float y)
            {
                return (float)Math.Sqrt((y - Y) * (y - Y) + (x - X) * (x - X));
            }

            private bool IsBetween(float min, float max, float v)
            {
                return min <= v && v <= max;
            }

            public bool Inside(RectangleF rect, float degreeMin, float degreeMax)
            {
                return IsBetween(degreeMin, degreeMax, GetDegree(rect.Left, rect.Top)) && GetRadius(rect.Left, rect.Top) < R &&
                       IsBetween(degreeMin, degreeMax, GetDegree(rect.Right, rect.Top)) && GetRadius(rect.Right, rect.Top) < R &&
                       IsBetween(degreeMin, degreeMax, GetDegree(rect.Left, rect.Bottom)) && GetRadius(rect.Left, rect.Bottom) < R &&
                       IsBetween(degreeMin, degreeMax, GetDegree(rect.Right, rect.Bottom)) && GetRadius(rect.Right, rect.Bottom) < R;
            }
        }

        private PieHelper _helper;
        private RectangleF _rectangle;

        public Font Font { get; set; }
        public Palette Palette { get; set; }
        public PaletteUsageMethod PaletteUsage { get; set; }

        public Pen SelectedPen { get; set; }

        private float _edgeSpacing = 15;
        public float EdgeSpacing
        {
            get { return _edgeSpacing; }
            set
            {
                _edgeSpacing = value;
                InitHelper();
            }
        }

        public RectangleF Rectangle
        {
            get { return _rectangle; }
            set
            {
                _rectangle = value;
                InitHelper();
            }
        }

        public HistogramPieDrawer(RectangleF rectangle, float edgeSpacing, Font font)
        {
            _edgeSpacing = edgeSpacing;
            _rectangle = rectangle;

            InitHelper();

            Font = font;
            Palette = new Palette("2  0 120 150 255 255 1 120 255 150 255");
            SelectedPen = new Pen(Color.DarkOrange, 2);
        }

        private void InitHelper()
        {
            var x = _rectangle.Left + _rectangle.Width / 2;
            var y = _rectangle.Top + _rectangle.Height / 2;
            var r = Math.Max(5, Math.Min(_rectangle.Width / 2, _rectangle.Height / 2) - _edgeSpacing);
            _helper = new PieHelper(x, y, r);
        }

        public int GetBinIndex(List<BinData> data, Point point)
        {
            var averageSum = 0.0;
            foreach (var bin in data)
            {
                averageSum += (bin.ValueMax + bin.ValueMin) / 2;
            }

            if (_helper.GetRadius(point.X, point.Y) > _helper.R)
            {
                return -1;
            }

            var degree = _helper.GetDegree(point.X, point.Y);

            var angle = 0.0;
            var i = 0;
            foreach (var bin in data)
            {
                var average = (bin.ValueMax + bin.ValueMin) / 2;
                var sweep = 360 * average / averageSum;
                if (angle < degree && degree < angle + sweep)
                {
                    return i;
                }
                angle += sweep;
                ++i;
            }
            return -1;
        }

        public void Draw(List<BinData> data, Graphics graphics)
        {
            var averageSum = 0.0f;
            foreach (var bin in data)
            {
                averageSum += (float)(bin.ValueMax + bin.ValueMin) / 2;
            }

            var angle = 0.0f;

            graphics.SmoothingMode = SmoothingMode.HighQuality;

            var pieRect = _helper.Rectangle;
            var w = pieRect.Left + pieRect.Width / 2;
            var h = pieRect.Top + pieRect.Height / 2;

            graphics.FillEllipse(Brushes.LightGray, pieRect);
            //e.Graphics.DrawEllipse(Pens.Black,w-r,h-r,r*2,r*2);

            var i = 0;
            bool prevSelected = false;
            foreach (var bin in data)
            {
                var color = GetBinColor(data, i);
                Brush brush = new SolidBrush(Color.FromArgb(100, color));

                var average = (float)(bin.ValueMax + bin.ValueMin) / 2;
                var sweep = (float)(360 * average / averageSum);
                var error = (float)(360 * (average - bin.ValueMin) / averageSum);
                for (var er = 0; er < 5; ++er)
                {
                    var curError = er * error / 5;
                    graphics.FillPie(brush, pieRect.X, pieRect.Y, pieRect.Width, pieRect.Height, angle + curError / 2, sweep - curError);
                }

                if (bin.Selected)
                {
                    graphics.DrawPie(SelectedPen, pieRect, angle, sweep);
                    prevSelected = true;
                }
                else
                {
                    graphics.DrawArc(Pens.Black, pieRect, angle, sweep);
                    if (!prevSelected)
                    {
                        graphics.DrawLine(Pens.Black, new PointF(_helper.X, _helper.Y), _helper.GetPoint(angle));
                    }
                    prevSelected = false;
                }

                var size = graphics.MeasureString(bin.Name, Font);
                var point = _helper.GetPoint(angle + sweep / 2, Math.Max(_helper.R * 0.75f, _helper.R - 20));
                var rect = new RectangleF(point.X - size.Width / 2, point.Y - size.Height / 2, size.Width, size.Height);

                if (!_helper.Inside(rect, angle, angle + sweep))
                {
                    var point2 = _helper.GetPoint(angle + sweep / 2, Math.Max(_helper.R * 0.5f, _helper.R - size.Width / 2));
                    var rect2 = new RectangleF(point2.X - size.Width / 2, point2.Y - size.Height / 2, size.Width, size.Height);

                    if (!_helper.Inside(rect2, angle, angle + sweep))
                    {
                        point = point2;
                        rect = rect2;
                    }
                }


                if (_helper.Inside(rect, angle, angle + sweep))
                {
                    graphics.DrawString(bin.Name, Font, Brushes.Black, point.X - size.Width / 2, point.Y - size.Height / 2);
                }
                else
                {
                    var r1Point = _helper.GetPoint(angle + sweep / 2, _helper.R - 10);
                    var r2Point = _helper.GetPoint(angle + sweep / 2, _helper.R + 5);

                    point = _helper.GetPoint(angle + sweep / 2, _helper.R + 10);

                    if (point.Y + size.Height / 2 > _rectangle.Bottom || point.Y - size.Height / 2 < _rectangle.Top)
                    {
                        graphics.DrawLine(Pens.Gray, r1Point, r2Point);

                        if (point.X > w)
                        {
                            graphics.DrawString(bin.Name, Font, Brushes.Black, point.X + 10, r2Point.Y - size.Height / 2);
                            graphics.DrawLine(Pens.Gray, r2Point, new PointF(point.X + 10, r2Point.Y));
                        }
                        else
                        {
                            graphics.DrawString(bin.Name, Font, Brushes.Black, point.X - 10 - size.Width, r2Point.Y - size.Height / 2);
                            graphics.DrawLine(Pens.Gray, r2Point, new PointF(point.X - 10, r2Point.Y));
                        }

                    }
                    else
                    {
                        if (point.X > w)
                        {
                            graphics.DrawString(bin.Name, Font, Brushes.Black, point.X, point.Y - size.Height / 2);
                        }
                        else
                        {
                            graphics.DrawString(bin.Name, Font, Brushes.Black, point.X - size.Width, point.Y - size.Height / 2);
                        }
                        graphics.DrawLine(Pens.Gray, r1Point, point);
                    }
                }

                angle += sweep;
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