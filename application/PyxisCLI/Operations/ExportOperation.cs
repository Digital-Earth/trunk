using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Utilities;
using PyxisCLI.Utilities;

namespace PyxisCLI.Operations
{
    class ExportOperation : IOperationMode
    {
        class Options
        {
            public bool Json { get; set; }
            public int Skip { get; set; }
            public int Top { get; set; }
            public bool IncludeGeometry { get; set; }
            public string Intersects { get; set; }
            public List<string> Select { get; set; }
            public string Where { get; set; }
        }

        public string Command
        {
            get { return "export"; }
        }

        //in the future this should also be geo-tiff
        public string Description
        {
            get { return "export geo-source into geojson"; }
        }

        public void Run(string[] args)
        {
            var options = new Options()
            {
                Json = true,
                IncludeGeometry = true,
            };

            args = ArgsParser.Parse(args, 
                new ArgsParser.Option("s|skip", (value) => options.Skip = int.Parse(value)),
                new ArgsParser.Option("t|top", (value) => options.Top = int.Parse(value)),
                new ArgsParser.Option("w|where", (value) => options.Where = value),
                new ArgsParser.Option("ng|no-geometry", (value) => options.IncludeGeometry = false),
                new ArgsParser.Option("f|fields", (value) => options.Select = value.Split(',').Select(f=>f.Trim()).ToList()),
                new ArgsParser.Option("i|intersects", (value) => options.Intersects = value)          
                );

            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} [-s|skip=number] [-t|top=number] [-ng|no-geometry] [-f|fields=fields] [-i|intersects=geom] geosource", Command);
                return;
            }

            IGeometry geometry = null;
            if (options.Intersects.HasContent())
            {
                if (System.IO.File.Exists(options.Intersects))
                {
                    geometry = JsonConvert.DeserializeObject<IGeometry>(System.IO.File.ReadAllText(options.Intersects));
                }
                else
                {
                    geometry = JsonConvert.DeserializeObject<IGeometry>(options.Intersects);
                }
            }

            foreach (var arg in args.Skip(1))
            {
                var reference = new Reference(arg);

                var geoSource = Program.Workspaces.ResolveGeoSource(reference);

                if (geoSource != null)
                {
                    OutputFeatures(Program.Engine, geoSource, geometry, options);
                }
            }
        }

        private void OutputFeatures(Engine engine, GeoSource geoSource, IGeometry geometry, Options options)
        {
            var flag = options.IncludeGeometry ? FeatureExtractionFlags.All : FeatureExtractionFlags.Fields;
            var features = geometry == null ?
                engine.GetAsFeature(geoSource).EnumerateFeatures(flag) :
                engine.GetAsFeature(geoSource).EnumerateFeatures(geometry, flag);

            if (options.Where.HasContent())
            {
                var condition = FeatureFilterParser.Parse(options.Where);
                features = features.Where(condition);
            }

            if (options.Skip > 0)
            {
                features = features.Skip(options.Skip);
            }
            if (options.Top > 0)
            {
                features = features.Take(options.Top);
            }

            var firstFeature = true;

            Console.WriteLine("{\n  \"type\": \"FeatureCollection\",\n  \"features\": [");

            foreach (var feature in features)
            {
                if (firstFeature)
                {
                    firstFeature = false;
                }
                else
                {
                    Console.WriteLine(",");
                }
                Console.Write(JsonConvert.SerializeObject(feature, Formatting.Indented));
            }
            
            Console.WriteLine("]\n}");
        }
    }
}
