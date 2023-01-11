using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;

namespace ApplicationUtility.Visualization
{
    /// <summary>
    /// StringPalette is a Dictionary&lt;string, Color&gt;. each Tuple&lt;string, Color&gt; is called a step.
    /// 
    /// StringPalette is been serialized to a single string in the following format:
    /// string = number of steps ' ' steps
    /// steps = step ' ' step ' ' ....
    /// step = stepKey ' ' color.A ' ' color.R ' ' color.G ' ' color.B
    /// stepKey = @ or BASE64
    /// 
    /// we encode the string value into BASE64. However, if the step key is empty string, then it get encoded to '@'.
    /// </summary>
    public class StringPalette : IStringPalette
    {
        public Dictionary<string, Color> Steps { get; set; }

        public StringPalette()
        {
            Steps = new Dictionary<string, Color>();
        }

        public StringPalette(string value)
        {
            ParseSteps(value);
        }

        public Color GetColor(string value)
        {
            if (Steps.ContainsKey(value))
            {
                return Steps[value];
            }
            return Color.Transparent;
        }

        public void ParseSteps(string value)
        {
            Steps = new Dictionary<string, Color>();
            var numbers = value.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

            var count = int.Parse(numbers[0]);

            var n = 1;
            for (var i = 0; i < count; ++i)
            {
                var stepKey = String.Empty;
                if (numbers[n] != "@")
                {
                    stepKey = System.Text.Encoding.UTF8.GetString(Convert.FromBase64String(numbers[n]));
                }
                ++n;
                var color = Color.FromArgb(int.Parse(numbers[n + 3]),
                                           int.Parse(numbers[n]),
                                           int.Parse(numbers[n + 1]),
                                           int.Parse(numbers[n + 2]));

                n += 4;

                Steps[stepKey] = color;
            }
        }

        public override string ToString()
        {
            var s = new StringBuilder();
            s.Append(Steps.Count).Append(" ");

            var keys = new List<string>(Steps.Keys);
            keys.Sort();

            foreach (var stepKey in keys)
            {
                var color = Steps[stepKey];
                var encodedKey = "@"; //represnt empty string;
                if (!String.IsNullOrEmpty(stepKey))
                {
                    encodedKey = Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(stepKey));
                }

                s.AppendFormat(" {0} {1} {2} {3} {4}", encodedKey, color.R, color.G, color.B, color.A);
            }

            return s.ToString();
        }
    }
}