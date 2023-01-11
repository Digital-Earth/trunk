using System;
using System.Drawing;
using System.Linq;

namespace GeoWebCore.Utilities
{
    /// <summary>
    /// Helper class that convert a number into a color.
    /// 
    /// we can use RGB to represent a number, Due to image compression and texture artifacts we want to create
    /// an image that is easy to compress. therefore, this class is trying to make the RGB values more or less
    /// the same value so it will be easy to compress the resulting image.
    /// 
    /// The idea is as follows:
    /// 1) convert the give floating number into integer.
    ///    1.1) convert the number into 0...1 range using the allowed Max and Min
    ///    1.2) convert the number into an integer between 0...256 * 256.
    /// 2) find RGB that 256*R+16*G+B == number while try to minimize: |R-G| + |R-B| + |G-B|
    ///    2.1) R = G = B = number / (256+16+1) -> good estimation
    ///    2.2) fix the estimation by fixing R then G then B.
    /// </summary>
    internal class Number16BitToColorConverter : INumberToColorConverter
    {
        public double Min { get; set; }
        public double Max { get; set; }

        private static readonly int[] s_factors = {256, 16, 1};
        private static readonly int s_sumFactors;

        static Number16BitToColorConverter()
        {
            s_sumFactors = s_factors.Sum();
        }

        public Number16BitToColorConverter(double min, double max)
        {
            Min = min;
            Max = max;
        }

        private static int ToValue(byte[] rgb)
        {
            return rgb[0]*s_factors[0] + rgb[1]*s_factors[1] + rgb[2]*s_factors[2];
        }

        public void ToRgba(double value, byte[] rgba)
        {
            rgba[3] = 255;

            //convert the value into number between 0 and 1
            var normalizedNumber = (Math.Min(Max, Math.Max(Min, value)) - Min)/(Max - Min);


            //convert to positive integer
            var intRange = 256*s_factors[0] - 1;
            var translatedValue = (int) (normalizedNumber*intRange);

            //initial gray guess
            rgba[0] = rgba[1] = rgba[2] = (byte)(translatedValue/s_sumFactors);

            //improve guess
            for (var factor = 0; factor < 3; factor++)
            {
                var currentValue = ToValue(rgba);
                rgba[factor] = (byte)(rgba[factor] + (translatedValue - currentValue) / s_factors[factor]);    
            }
        }

        public Color ToColor(double value)
        {
            var rgba = new byte[4];

            ToRgba(value,rgba);

            return Color.FromArgb(rgba[3], rgba[0], rgba[1], rgba[2]);
        }
    }
}