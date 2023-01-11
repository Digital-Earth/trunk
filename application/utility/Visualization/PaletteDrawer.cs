using System.Drawing;

namespace ApplicationUtility.Visualization
{
    public class PaletteDrawer
    {
        private readonly Palette _palette;

        public PaletteDrawer(Palette palette)
        {
            _palette = palette;
        }

        public void Draw(Rectangle rectangle, Graphics graphics)
        {
            if (_palette.IsStringPalette)
            {
                new StringPaletteDrawer(_palette.GetStringPalette()).Draw(rectangle,graphics);
            }
            else
            {
                new NumericPaletteDrawer(_palette.GetNumericPalette()).Draw(rectangle, graphics);
            }
        }
    }
}