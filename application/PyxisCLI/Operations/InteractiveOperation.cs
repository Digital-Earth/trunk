using System;
using System.Collections.Generic;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Core.DERM;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class InteractiveOperation : IOperationMode
    {
        public string Command
        {
            get { return "interactive"; }
        }

        public string Description
        {
            get { return "enable interactive commands line"; }
        }

        public void Run(string[] args)
        {
            var lineArgs = new string[0];

            while(true)
            {
                Console.Write("PYX> ");
                var line = Console.ReadLine();
                lineArgs = SplitLineToArgs(line);
                if (lineArgs.Length > 0 && (lineArgs[0] == "exit" || lineArgs[0] == "quit"))
                {
                    return;
                }
                
                var start = DateTime.Now;

                Program.RunOperation(lineArgs);

                if (Program.MeasureTime)
                {
                    var duration = DateTime.Now - start;
                    Console.WriteLine("Time: {0}[sec]", duration.TotalSeconds);
                }
            }            
        }

        private string[] SplitLineToArgs(string line)
        {
            bool inQuotes = false;

            return SplitLine(line, c =>
                {
                    if (c == '\"')
                        inQuotes = !inQuotes;

                    return !inQuotes && c == ' ';
                })
                .Select(arg => TrimMatchingQuotes(arg.Trim(), '\"'))
                .Where(arg => !string.IsNullOrEmpty(arg))
                .ToArray();
        }

        private static IEnumerable<string> SplitLine(string str,
                                            Func<char, bool> controller)
        {
            int nextPiece = 0;

            for (int c = 0; c < str.Length; c++)
            {
                if (controller(str[c]))
                {
                    yield return str.Substring(nextPiece, c - nextPiece);
                    nextPiece = c + 1;
                }
            }

            yield return str.Substring(nextPiece);
        }

        private static string TrimMatchingQuotes(string input, char quote)
        {
            if ((input.Length >= 2) &&
                (input[0] == quote) && (input[input.Length - 1] == quote))
                return input.Substring(1, input.Length - 2);

            return input;
        }
    }
}