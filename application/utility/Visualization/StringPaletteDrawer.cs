using System.Drawing;

namespace ApplicationUtility.Visualization
{
    public class StringPaletteDrawer
    {
        private readonly StringPalette _palette;

        public StringPaletteDrawer(StringPalette palette)
        {
            _palette = palette;
        }

        public void Draw(Rectangle rectangle, Graphics graphics)
        {
            var i = 0;
            foreach(var step in _palette.Steps)
            {
                var x0 = (int)((rectangle.Width - 10) * i / _palette.Steps.Count + rectangle.X + 5);
                var x1 = (int)((rectangle.Width - 10) * (i+1) / _palette.Steps.Count + rectangle.X + 5);
                ++i;

                if (x1 == x0)
                    continue;

                var nonAlphaBrush = new SolidBrush(Color.FromArgb(255,step.Value));
                var alphaBrush = new SolidBrush(step.Value);

                graphics.DrawLine(Pens.DarkGray, x0, rectangle.Top + 1, x0, rectangle.Bottom - 1);
                graphics.DrawLine(Pens.DarkGray, x1, rectangle.Top + 1, x1, rectangle.Bottom - 1);
                graphics.FillRectangle(nonAlphaBrush, x0, rectangle.Top + 3, x1 - x0, rectangle.Height / 2 - 3);
                graphics.FillRectangle(alphaBrush, x0, rectangle.Top + rectangle.Height / 2, x1 - x0, rectangle.Height / 2 - 3);
            }
        }
    }
}