using System.Drawing;
using System.Windows.Forms;

namespace ApplicationUtility.Visualization
{
    public interface IStringPalette
    {
        Color GetColor(string value);
    }

    public interface INumericPalette
    {
        Color GetColor(double value);
    }

    public class Palette : IStringPalette, INumericPalette
    {
        public static readonly string StringPaletteID = "[StringPalette]";
        public static readonly string NumericPaletteID = "[NumericPalette]";

        private IStringPalette _stringPalette = null;
        private INumericPalette _numericPalette = null;

        public Palette()
        {
            _numericPalette = new NumericPalette("");
        }

        public Palette(string value)
        {
            Parse(value);
        }

        public Palette (IStringPalette palette)
        {
            _stringPalette = palette;
        }

        public Palette(INumericPalette palette)
        {
            _numericPalette = palette;
        }

        public Color GetColor(string value)
        {
            if (_stringPalette != null)
                return _stringPalette.GetColor(value);

            double d = 0;
            if (_numericPalette != null && double.TryParse(value, out d))
                return _numericPalette.GetColor(d);

            return Color.Transparent;
        }

        public Color GetColor(double value)
        {
            if (_numericPalette != null)
                return _numericPalette.GetColor(value);

            if (_stringPalette != null)
                return _stringPalette.GetColor(value.ToString());

            return Color.Transparent;
        }

        public void Parse(string value)
        {
            _stringPalette = null;
            _numericPalette = null;

            if (value.StartsWith(StringPaletteID))
            {
                _stringPalette = new StringPalette(value.Substring(StringPaletteID.Length).Trim());
            }
            else if (value.StartsWith(NumericPaletteID))
            {
                _numericPalette = new NumericPalette(value.Substring(NumericPaletteID.Length).Trim());
            }
            else
            {
                _numericPalette = new NumericPalette(value);
            }
        }

        public bool IsStringPalette
        {
            get { return _stringPalette != null; }
        }

        public override string ToString()
        {
            if (_stringPalette != null)
                return StringPaletteID + " " + _stringPalette.ToString();

            if (_numericPalette != null)
                return NumericPaletteID + " " + _numericPalette.ToString();

            return "";
        }

        public StringPalette GetStringPalette()
        {
            return _stringPalette as StringPalette;
        }

        public NumericPalette GetNumericPalette()
        {
            return _numericPalette as NumericPalette;
        }
    }
}
