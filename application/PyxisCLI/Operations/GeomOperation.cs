using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text.RegularExpressions;
using ApplicationUtility;
using Nest;
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
    class GeomOperation : IOperationMode
    {
        public string Command
        {
            get { return "geom"; }
        }

        //in the future this should also be geo-tiff
        public string Description
        {
            get { return "export a dggs geometry as geojson"; }
        }

        public void Run(string[] args)
        {
            var rootDir = Environment.CurrentDirectory;

            var stopwatch = new StopwatchProfiler("geom to jeoson");

            IGeometry geometry = null;
            int resolution = 14;
            var outputFileName = "";

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("r|resolution", (value) => resolution = int.Parse(value)),
                new ArgsParser.Option("o|out", (value) => outputFileName = value)
            );

            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} [-r|resolution=res] [geometry]", Command);
                return;
            }

            var geomArg = args[1];

            if (System.IO.File.Exists(geomArg))
            {
                geometry = JsonConvert.DeserializeObject<IGeometry>(System.IO.File.ReadAllText(geomArg));
            }
            else
            {
                geometry = JsonConvert.DeserializeObject<IGeometry>(geomArg);
            }

            var calculator = Program.Engine.CalculatePerimeter(geometry, resolution);
            //var cells = calculator.GetCandidateCells();
            //calculator = new PerimeterCalculator(cells);
            var perimeter = calculator.GetPerimeterPolygon();

            Console.WriteLine("Tiles: {0}", calculator.Performance.Tiles);
            Console.WriteLine("Cells: {1} total, {0} checked", calculator.Performance.CheckedCells, calculator.Performance.Cells);
            Console.WriteLine("Perimeter Cells: {0}", calculator.Performance.PerimeterCells);
            Console.WriteLine("Polygon: {0} Rings, {1} Vertices", calculator.Performance.Rings, calculator.Performance.VerticesCount);

            Console.WriteLine(calculator.Performance.Log.Output);

            if (outputFileName.HasContent())
            {
                Console.WriteLine("Write output to {0}", outputFileName);
                System.IO.File.WriteAllText(System.IO.Path.Combine(rootDir, outputFileName), JsonConvert.SerializeObject(perimeter));
            }           
        }
    }
}

