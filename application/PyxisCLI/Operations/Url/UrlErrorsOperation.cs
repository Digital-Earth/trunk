using System;
using System.Linq;
using Pyxis.Utilities;
using PyxisCLI.State;

namespace PyxisCLI.Operations.Url
{
    class UrlErrorsOperation : IOperationMode
    {
        public string Command
        {
            get { return "url errors"; }
        }


        public string Description
        {
            get { return "display histogram of all issues found in datasets"; }
        }

        public void Run(string[] args)
        {
            bool jsonOutput = false;
            
            args = ArgsParser.Parse(args,
                new ArgsParser.Option("json", (value) => jsonOutput = true));

            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0}", Command);
                return;
            }

            foreach (var issue in MongoPersistance.GetDataSetsIssues().OrderByDescending(x=>x.Count).Take(100))
            {
                Console.WriteLine("{0}, {1}", issue.Issue, issue.Count);
            }
        }
    }
}