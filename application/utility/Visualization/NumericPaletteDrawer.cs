using System.Drawing;

namespace ApplicationUtility.Visualization
{
    public class NumericPaletteDrawer
    {
        private readonly NumericPalette _palette;

        public NumericPaletteDrawer(NumericPalette palette)
        {
            _palette = palette;
        }

        public void Draw(Rectangle rectangle,Graphics graphics)
        {
            var range = _palette.MaxValue-_palette.MinValue;
            for (int i = 0; i < _palette.Steps.Count - 1; i++)
            {
                var step = _palette.Steps[i];
                var nextStep = _palette.Steps[i+1];
                var x0 = (int)((rectangle.Width - 10)*(step.Value - _palette.MinValue)/range + rectangle.X + 5);
                var x1 = (int)((rectangle.Width - 10)*(nextStep.Value - _palette.MinValue)/range + rectangle.X + 5);

                if (x1 == x0)
                    continue;

                var nonAlphaLinearBrash = new System.Drawing.Drawing2D.LinearGradientBrush(
                    new Point(x0 - 1, 0),
                    new Point(x1, 0),
                    Color.FromArgb(255, step.Color),
                    Color.FromArgb(255, nextStep.Color));

                var linearBrash = new System.Drawing.Drawing2D.LinearGradientBrush(
                    new Point(x0 - 1, 0),
                    new Point(x1, 0),
                    step.Color,
                    nextStep.Color);

                graphics.DrawLine(Pens.DarkGray, x0, rectangle.Top + 1, x0, rectangle.Bottom - 1);
                graphics.DrawLine(Pens.DarkGray, x1, rectangle.Top + 1, x1, rectangle.Bottom - 1);
                graphics.FillRectangle(nonAlphaLinearBrash, x0, rectangle.Top + 3, x1 - x0, rectangle.Height / 2 - 3);
                graphics.FillRectangle(linearBrash, x0, rectangle.Top + rectangle.Height / 2, x1 - x0, rectangle.Height / 2 - 3);
            }

            graphics.FillRectangle(new SolidBrush(Color.FromArgb(255, _palette.Steps[0].Color)), rectangle.Left, rectangle.Top + 3, 5, rectangle.Height / 2 - 3);
            graphics.FillRectangle(new SolidBrush(_palette.Steps[0].Color), rectangle.Left, rectangle.Top + rectangle.Height / 2, 5, rectangle.Height / 2 - 3);

            graphics.FillRectangle(new SolidBrush(Color.FromArgb(255, _palette.Steps[_palette.Steps.Count - 1].Color)), rectangle.Right - 5, rectangle.Top + 3, 5, rectangle.Height / 2 - 3);
            graphics.FillRectangle(new SolidBrush(_palette.Steps[_palette.Steps.Count - 1].Color), rectangle.Right - 5, rectangle.Top + rectangle.Height / 2, 5, rectangle.Height / 2 - 3);
        }
    }
}