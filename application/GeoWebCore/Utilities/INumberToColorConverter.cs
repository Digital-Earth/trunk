using System.Drawing;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// Convert a number into a color
    /// </summary>
    internal interface INumberToColorConverter
    {
        double Min { get; }
        double Max { get; }

        void ToRgba(double value, byte[] rgba);
        Color ToColor(double value);
    }
}