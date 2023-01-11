using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.Analysis;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class CalculateOperation : IOperationMode
    {
        class Options
        {
            public string Expression { get; set; }
            public bool Save { get; set; }
            public string Type { get; set; }
        }

        public string Command
        {
            get { return "calc"; }
        }

        public string Description
        {
            get { return "calculate an expression"; }
        }

        public void Run(string[] args)
        {
            var options = new Options()
            {
                Type = "double"
            };

            var types = new Dictionary<string, Type>
            {
                {"bool", typeof(bool)},
                {"byte", typeof(byte)},
                {"int", typeof(int)},
                {"double", typeof(double)}
            };

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("t|type", (value) => options.Type = value),
                new ArgsParser.Option("s|save", (value) => options.Save = true));

            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} [-s|save] [-t|type=bool|byte|int|double] expression",
                    Command);
                return;
            }

            options.Expression = String.Join(" ",args.Skip(1));

            var outputType = types[types.ContainsKey(options.Type) ? options.Type : "double"];          

            var calculatedGeoSource = Program.Engine.Calculate(options.Expression, outputType);

            if (options.Save)
            {
                var filename = String.Format("{0}.json", calculatedGeoSource.Id);
                System.IO.File.WriteAllText(filename, JsonConvert.SerializeObject(calculatedGeoSource));
                Console.WriteLine(filename);
            }
            else
            {
                Console.WriteLine(options.Expression + " - ok");
            }
        }
    }
}
