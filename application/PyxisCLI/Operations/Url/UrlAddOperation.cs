using System;
using System.Linq;
using ApplicationUtility;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Utilities;
using PyxisCLI.State;
using File = System.IO.File;

namespace PyxisCLI.Operations.Url
{
    class UrlAddOperation : IOperationMode
    {
        public string Command
        {
            get { return "url add"; }
        }


        public string Description
        {
            get { return "add urls"; }
        }

        public void Run(string[] args)
        {
            string source = null;
            bool skipValidation = false;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("s|source", (value) => source = value),
                new ArgsParser.Option("skip-validation", (value) => skipValidation = true));

            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0}", Command);
                return;
            }

            if (source.HasContent())
            {
                AddUrl(skipValidation, File.ReadAllLines(source).Where(line => line.HasContent()).Distinct().ToArray());
            }
            else
            {
                AddUrl(skipValidation, args.Skip(2).Distinct().ToArray());
            }
        }

        private void AddUrl(bool skipValidation, params string[] urls)
        {
            foreach (var url in urls)
            {
                UrlDiscoveryReport urlReport = null;
                var conflictedRoot = MongoPersistance.GetConflictedRoot(url);
                if (conflictedRoot == null)
                {
                    if (skipValidation)
                    {
                        urlReport = MongoPersistance.AddRoot(url);
                    }
                    else
                    {
                        var discoveryResult = LocalGazetteer.DiscoveryOnce(Program.Engine, url);

                        if (discoveryResult != null)
                        {
                            urlReport = MongoPersistance.AddRoot(url, discoveryResult.ServiceIdentifier);
                        }
                    }

                    if (urlReport != null)
                    {
                        AutomationLog.PushInfo("urls", urlReport);
                    }
                }
                else
                {
                    Console.WriteLine("Discovered new root : {0}", url);
                    Console.WriteLine("new root was not added because it conflicts with " + conflictedRoot.Uri);
                }
            }
        }
    }
}