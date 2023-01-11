using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class StatsOperation : IOperationMode
    {
        public string Command
        {
            get { return "stats"; }
        }

        public string Description
        {
            get { return "calculate stats on a geosource"; }
        }

        public void Run(string[] args)
        {
            string inputField = null;
            string geom = null;
            var json = false;

            args = ArgsParser.Parse(args,
               new ArgsParser.Option("j|json", (value) => json = true),
               new ArgsParser.Option("f|field", (value) => inputField = value),
               new ArgsParser.Option("g|geometry", (value) => geom = value));

            if (args.Length <= 1)
            {
                Console.WriteLine("usage: pyx {0} [-j|json] [-f|field=field] [-g|geometry=file] reference", Command);
                return;
            }

            IGeometry geometry = null;
            if (geom.HasContent())
            {
                if (System.IO.File.Exists(geom))
                {
                    geometry = JsonConvert.DeserializeObject<IGeometry>(System.IO.File.ReadAllText(geom));
                }
                else
                {
                    geometry = JsonConvert.DeserializeObject<IGeometry>(geom);    
                }
            }

            var reference = new Reference(args[1]);
            var geoSource = Program.Workspaces.ResolveGeoSource(reference);

            if (geoSource.Specification.OutputType == PipelineSpecification.PipelineOutputType.Coverage && geometry == null )
            {
                throw new Exception("Coverage pipelines doesn't support global stats at the moment, please provide a geometry.");
            }

            var fields = new List<string>();
            if (inputField.HasContent())
            {
                if (geoSource.Specification.HasField(inputField))
                {
                    fields.Add(inputField);
                }
                else
                {
                    throw new Exception(
                        string.Format("Reference {0} has no field named {1}. Availabled fields: {2}",
                            args[1], inputField, string.Join(", ", geoSource.Specification.FieldNames())));
                }
            }
            else
            {
                fields.AddRange(geoSource.Specification.FieldNames());
            }

            foreach (var field in fields)
            {
                var stats = geometry != null
                    ? Program.Engine.Statistics(geoSource).GetFieldStatistics(geometry, field)
                    : Program.Engine.Statistics(geoSource).GetFieldStatistics(field);

                if (json)
                {
                    Console.WriteLine(JsonConvert.SerializeObject(stats, Formatting.Indented));
                }
                else
                {
                    Console.WriteLine("\n{0}\n{1}",field,new string('-',field.Length));
                    Console.WriteLine("Max: {0}", stats.Max);
                    if (stats.Average != null)
                    {
                        Console.WriteLine("Average: {0}", stats.Average);
                    }
                    Console.WriteLine("Min: {0}",stats.Min);
                    Console.WriteLine("Count: {0}", stats.MinCount);
                    
                    foreach (var bin in stats.Distribution.Histogram)
                    {
                        Console.WriteLine("[{0} .. {1}] -> {2:P}", bin.Min, bin.Max, bin.Frequency);
                    }
                }
            }
        }
    }
}
