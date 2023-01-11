using System;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class SearchOperation : IOperationMode
    {
        public string Command
        {
            get { return "search"; }
        }

        public string Description
        {
            get { return "search for datasets to import"; }
        }

        public void Run(string[] args)
        {
            var save = false;
            var numberOfResults = 20;
            
            args = ArgsParser.Parse(args,
                new ArgsParser.Option("save", (value) => save = true),
                new ArgsParser.Option("n", (value) => numberOfResults = Int32.Parse(value)));
            
            if (args.Length < 2)
            {
                Console.WriteLine("usage: pyx {0} [-n=10] [-save] query", Command);
                return;
            }

            foreach (
                var geoSource in
                Program.Engine.GetChannel()
                    .GeoSources.Search(string.Join(" ", args))
                    .Where(x => x.State == PipelineDefinitionState.Active)
                    .Take(numberOfResults))
            {
                Console.WriteLine("{0} {1}", geoSource.Id, geoSource.Metadata.Name);
                if (save)
                {
                    var filename = String.Format("{0}.json", geoSource.Id);
                    System.IO.File.WriteAllText(filename, JsonConvert.SerializeObject(geoSource));
                }
            }            
        }
    }
}