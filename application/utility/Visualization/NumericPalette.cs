using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Linq;

namespace ApplicationUtility.Visualization
{
    public class NumericPalette : INumericPalette
    {
        public static readonly Dictionary<string, string> DefaultPalettes = new Dictionary<string, string>()
                                                                                {
                                                                                    { "Default",   "2  0 0 0 0 255  1 255 255 255 255"},
                                                                                    { "Red To Green", "2  0 255 0 0 255  100 0 255 0 255"},
                                                                                    { "HSV", "7  0 255 0 0 255  10 255 255 0 255  20 0 255 0 255  30 0 255 255 255  40 0 0 255 255  50 255 0 255 255  60 255 0 0 255"},
                                                                                    { "Grayscale (8 steps)", "17  0 0 0 0 255  10 0 0 0 255  10 31 31 31 255  20 31 31 31 255  20 63 63 63 255  30 63 63 63 255  30 95 95 95 255  40 95 95 95 255  40 127 127 127 255  50 127 127 127 255  50 159 159 159 255  60 159 159 159 255  60 191 191 191 255  70 191 191 191 255  70 223 223 223 255  80 223 223 223 255  80 255 255 255 255"},
                                                                                    { "Standard Deviation", "13  -2 151 0 0 255  -2 151 0 0 255  -2 161 48 48 255  -1.5 161 48 48 255  -1.5 171 96 96 255  -1 171 96 96 255  -1 192 192 192 255  1 192 192 192 255  1 96 160 96 255  1.5 96 160 96 255  1.5 48 144 48 255  2 48 144 48 255  2 0 128 0 255"},
                                                                                    { "Standard Deviation (middle transparent)", "13  -2 151 0 0 255  -2 151 0 0 255  -2 161 48 48 255  -1.5 161 48 48 255  -1.5 171 96 96 255  -1 171 96 96 255  -1 192 192 192 0  1 192 192 192 0  1 96 160 96 255  1.5 96 160 96 255  1.5 48 144 48 255  2 48 144 48 255  2 0 128 0 255"},
                                                                                    { "NASA MODIS 'greenness'", "15  0 0 0 0 0  1 0 0 0 0  1 152 133 92 255  1000 152 133 92 255  1000 129 96 56 255  2000 129 96 56 255  2000 96 75 49 255  3500 96 75 49 255  3500 59 56 26 255  4000 59 56 26 255  4000 47 58 24 255  5000 47 58 24 255  5000 26 42 12 255  6000 26 42 12 255  6000 0 36 0 255"}
                                                                                };

        public static NumericPalette FromColors(params Color[] colors)
        {
            if (colors.Length < 2)
            {
                throw new Exception("Palette require 2 or more colors");
            }

            return new NumericPalette() { Steps = colors.Select((c, i) => new Step { Color = c, Value = ((double)i) / (colors.Length - 1) }).ToList() };
        }

        public static NumericPalette FromColors(double rangeMin,double rangeMax, params Color[] colors)
        {
            var palette = NumericPalette.FromColors(colors);
            palette.ScaleToRange(rangeMin, rangeMax);
            return palette;
        }

        public NumericPalette()
        {
            Steps = new List<Step>();
        }

        public NumericPalette(string value)
        {
            ParseSteps(value);
        }

        public class Step
        {
            public double Value { get; set; }
            public Color Color { get; set; }
        }

        public List<Step> Steps { get; set; }

        public Color GetColor(double value)
        {
            if (Steps.Count == 0)
            {
                return Color.Transparent;
            }

            if (value <= Steps[0].Value)
            {
                return Steps[0].Color;
            }

            if (value >= Steps[Steps.Count-1].Value)
            {
                return Steps[Steps.Count - 1].Color;
            }

            var i = 1;
            for (; i < Steps.Count && value >= Steps[i].Value; ++i) ;
            var total = Steps[i].Value - Steps[i - 1].Value;
            var p = (value - Steps[i - 1].Value)/total;
            var a = Steps[i-1].Color;
            var b = Steps[i].Color;
            return Color.FromArgb(
                (int) (a.A*(1 - p) + b.A*p), 
                (int) (a.R*(1 - p) + b.R*p),
                (int) (a.G*(1 - p) + b.G*p),
                (int) (a.B*(1 - p) + b.B*p));
        }

        public void ParseSteps(string value)
        {
            if (value == "")
            {
                ParseSteps(DefaultPalettes["Default"]);
            }
            if (DefaultPalettes.ContainsKey(value))
            {
                ParseSteps(DefaultPalettes[value]);
            }

            var numbers = value.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

            var count = int.Parse(numbers[0]);

            Steps = new List<Step>();
            var n = 1;
            for(var i=0;i<count;++i)
            {
                var stepValue = double.Parse(numbers[n]);
                ++n;
                var color = Color.FromArgb(int.Parse(numbers[n + 3]),
                                           int.Parse(numbers[n]), 
                                           int.Parse(numbers[n + 1]),
                                           int.Parse(numbers[n + 2]));
                n += 4;

                Steps.Add(new Step {Value = stepValue, Color = color});
            }
        }

        public override string ToString()
        {
            var s = new StringBuilder();
            s.Append(Steps.Count).Append(" ");

            foreach (var step in Steps)
            {
                s.AppendFormat(" {0} {1} {2} {3} {4}", step.Value, step.Color.R, step.Color.G, step.Color.B, step.Color.A);
            }

            return s.ToString();
        }

        public void ScaleToRange(double min,double max)
        {
            if (Steps.Count == 0)
                return;

            if (Steps.Count == 1)
            {
                Steps[0].Value = (min + max)/2;
                return;
            }

            var oldMin = MinValue;
            var oldMax = MaxValue;

            foreach(var step in Steps)
            {
                step.Value = (step.Value - oldMin)/(oldMax - oldMin)*(max - min) + min;
            }
        }

        public double MinValue
        {
            get { return Steps[0].Value; }
            set
            {
                ScaleToRange(value,MaxValue);
            }
        }

        public double MaxValue
        {
            get { return Steps[Steps.Count-1].Value; }
            set
            {
                ScaleToRange(MinValue,value);
            }
        }
    }
}