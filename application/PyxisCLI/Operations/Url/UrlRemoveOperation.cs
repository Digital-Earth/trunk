using System;
using System.IO;
using System.Linq;
using ApplicationUtility;
using Pyxis.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Operations.Url
{
    class UrlRemoveOperation : IOperationMode
    {
        public string Command
        {
            get { return "url remove"; }
        }

        public string Description
        {
            get { return "remove url"; }
        }

        public void Run(string[] args)
        {
            string source = null;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("s|source", (value) => source = value));

            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0} {{action}}", Command);
                return;
            }

            if (source.HasContent())
            {
                RemoveUrl(File.ReadAllLines(source).Where(line => line.HasContent()).Distinct().ToArray());
            }
            else
            {
                RemoveUrl(args.Skip(2).ToArray());
            }
        }

        private void RemoveUrl(string[] urls)
        {
            foreach (var url in urls)
            {
                Console.WriteLine(url);
                MongoPersistance.RemoveRoot(url);
            }
        }
    }
}