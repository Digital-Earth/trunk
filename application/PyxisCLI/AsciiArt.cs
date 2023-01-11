using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PyxisCLI
{
    internal class AsciiArt
    {
        private static readonly char[] s_codes = new char[] { ' ', '·', '~', '¢', 'c', '»', '¤', 'X', 'M', '¶' };

        private static readonly ConsoleColor[] s_colors = new ConsoleColor[]
        {
            ConsoleColor.Black,
            ConsoleColor.DarkBlue,
            ConsoleColor.DarkCyan,
            ConsoleColor.DarkGreen,
            ConsoleColor.DarkGray,
            ConsoleColor.Gray,
            ConsoleColor.Cyan,
            ConsoleColor.Green,
            ConsoleColor.Yellow,
            ConsoleColor.Magenta,
            ConsoleColor.Red
        };

        public static char FromNumber(double number)
        {
            if (Math.Abs(number) < Double.Epsilon)
            {
                return s_codes[0];
            }
            var index = (int)Math.Round((s_codes.Length - 2)*number + 1);
            return s_codes[Math.Min(s_codes.Length-1,index)];
        }

        public static void WriteColorStrip(IEnumerable<double> numbers)
        {
            foreach (var number in numbers)
            {
                if (Math.Abs(number) < Double.Epsilon)
                {
                    Console.BackgroundColor = ConsoleColor.Black;
                    Console.Write("-");
                }
                else
                {

                    var index = (int)Math.Round((s_colors.Length - 2) * number + 1);
                    var color = s_colors[Math.Min(s_colors.Length - 1, index)];
                    Console.BackgroundColor = color;
                    Console.Write("X");
                }
            }
            Console.BackgroundColor = ConsoleColor.Black;
        }
    }
}
