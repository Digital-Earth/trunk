using System;
using System.Collections.Generic;

namespace Pyxis.Contract.Workspaces.Domains
{
    public class NumericDomainRange : IDomain
    {
        public double From { get; set; }
        public double To { get; set; }
        public double Step { get; set; }

        public NumericDomainRange(double from, double to, double step)
        {
            From = from;
            To = to;
            Step = step;
        }

        public IEnumerable<string> Values
        {
            get
            {
                var value = From;
                var index = 0;

                while (value <= To)
                {
                    yield return value.ToString();
                    value = From + index*Step;
                }
            }
        }

        public string Type
        {
            get { return "number"; }
        }

        public bool Contains(string value)
        {
            var number = double.Parse(value);
            var epsilon = Step / 1000;

            if (number <= From - epsilon ||
                number >= To + epsilon)
            {
                return false;
            }

            var d = Math.Round((number - From) / Step);

            var match = From+ d * Step;

            return Math.Abs(number - match) < epsilon;
        }

        public string FormatValue(string value, string format)
        {
            double v = double.Parse(value);
            return v.ToString(format);
        }
    }
}