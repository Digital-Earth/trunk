using System;

namespace PyxisCLI.Operations.Url
{
    class UrlOperation : IOperationMode
    {
        public string Command
        {
            get { return "url"; }
        }

        public string Description
        {
            get { return "control urls for remote data sync"; }
        }

        public void Run(string[] args)
        {
            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0} {{action}}", Command);
                return;
            }            
        }
    }
}