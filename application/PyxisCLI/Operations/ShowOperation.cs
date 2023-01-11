using System;
using System.Collections.Generic;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class ShowOperation : IOperationMode
    {
        public string Command
        {
            get { return "show"; }
        }

        public string Description
        {
            get { return "show a ascii map of a given area"; }
        }

        public void Run(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("usage: pyx {0} geosource [field] [bbox]", Command);
                return;
            }

            string field = args.Length>2 ? args[2] : "";
            double[] bbox = null;

            if (args.Length >= 4)
            {
                bbox = args[3].Split(',').Select(x => double.Parse(x)).ToArray();
            }

            GeoSource geoSource;

            var reference = new Reference(args[1]);

            if (Program.Workspaces.WorkspaceExists(reference.Workspace))
            {
                geoSource = Program.Workspaces.ResolveGeoSource(reference);
            }
            else
            {
                Console.WriteLine("Can't find: {0}", args[1]);
                return;
            }

            var values = SampleValues(Program.Engine, geoSource, field, bbox);

            DrawWhereMap(values, bbox);
        }

        private List<List<double>> SampleValues(Engine engine, GeoSource geoSource, string field, double[] bbox)
        {
            var process = engine.GetProcess(geoSource);
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());


            var fieldIndex = !String.IsNullOrEmpty(field) ? coverage.getCoverageDefinition().getFieldIndex(field) : 0;

            if (bbox == null)
            {
                var geometry = coverage.getGeometry();
                var rect1 = new PYXRect2DDouble();
                var rect2 = new PYXRect2DDouble();
                geometry.getBoundingRects(new WGS84CoordConverter(), rect1, rect2);
                bbox = new[] {rect1.yMin(), rect1.xMin(), rect1.yMax(), rect1.xMax()};

                Console.WriteLine("bbox: latitude: {0}..{1}, longitude: {2}..{3}",bbox[0],bbox[2],bbox[1],bbox[3]);
            }

            var width = Math.Abs(bbox[2] - bbox[0]);
            var height = Math.Abs(bbox[3] - bbox[1]);
            var step = Math.Max(width, height) / 50;

            PointLocation p = PointLocation.fromWGS84(bbox[0],bbox[1]);
            PointLocation p2 = PointLocation.fromWGS84(bbox[0] + step, bbox[1] + step);

            
            var distanceOnEarthInRadians = p.distance(p2)/SphereMath.knEarthRadius;

            var resultion = SnyderProjection.getInstance().precisionToResolution(distanceOnEarthInRadians);

            var values = new List<List<double>>();

            for (var lat = bbox[2]; lat > bbox[0]; lat -= step)
            {
                var line = new List<double>();
                for (var lon = bbox[1]; lon < bbox[3]; lon += step)
                {
                    p = PointLocation.fromWGS84(lat,lon);
                    var value = coverage.getCoverageValue(p.asPYXIcosIndex(resultion), fieldIndex);
                    line.Add(value.getDouble());
                }
                values.Add(line);
            }
            return values;
        }

        private static void DrawWhereMap(List<List<double>> values, double[] bbox)
        {
            //orginaze all the values
            var orderedValues = values.SelectMany(line => line).Distinct().OrderBy(x => x).ToList();

            if (orderedValues.Count == 1)
            {
                //duplicate single value
                orderedValues.Add(orderedValues[0]);
            }

            Func<double,double> noramlizeValue = (value) => Math.Abs(0.0+orderedValues.BinarySearch(value))/(orderedValues.Count-1);

            var numberOfLines = Math.Min(values.Count,orderedValues.Count);
            var lineNumber = 0;

            foreach(var line in values)
            {
                Console.Write("|");
                AsciiArt.WriteColorStrip(line.Select(v => noramlizeValue(v)));
                Console.Write("| ");
                if (lineNumber < numberOfLines)
                {
                    AsciiArt.WriteColorStrip(new double[] {1.0*(numberOfLines - lineNumber - 1) / (numberOfLines-1) });
                    Console.WriteLine(" {0,10} ", orderedValues[(int)Math.Floor((orderedValues.Count - 1.0) * (numberOfLines - lineNumber) / numberOfLines)]);
                    lineNumber++;
                }
                else
                {
                    Console.WriteLine();
                }
                
            }
        }
    }
}
