using System;
using System.Drawing;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// Helper class that convert a number into a color.
    /// 
    /// it uses RGB channel to represent the number.   
    /// number is get converted into 24 bit integer: number = 256*256*R + 256*G + B
    /// </summary>
    internal class Number24BitToColorConverter : INumberToColorConverter
    {
        public double Min { get; set; }
        public double Max { get; set; }

        public Number24BitToColorConverter(double min, double max)
        {
            Min = min;
            Max = max;
        }

        public void ToRgba(double value, byte[] rgba)
        {
            rgba[3] = 255;

            //convert the value into number between 0 and 1
            var normalizedNumber = (Math.Min(Max, Math.Max(Min, value)) - Min) / (Max - Min);


            //convert to positive integer
            const int intRange = 256*256*256 - 1;
            var translatedValue = (int)(normalizedNumber * intRange);

            rgba[0] = (byte) ((translatedValue >> 16)%256);
            rgba[1] = (byte) ((translatedValue >> 8)%256);
            rgba[2] = (byte) ((translatedValue)%256);
        }

        public Color ToColor(double value)
        {
            var rgba = new byte[4];

            ToRgba(value, rgba);

            return Color.FromArgb(rgba[3], rgba[0], rgba[1], rgba[2]);
        }
    }
}