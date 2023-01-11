using System;
using System.Linq;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class WhereOperation : IOperationMode
    {
        public string Command
        {
            get { return "where"; }
        }

        public string Description
        {
            get { return "return where a geoSource or a geometry is"; }
        }

        public void Run(string[] args)
        {
            var map = false;

            args = ArgsParser.Parse(args, new ArgsParser.Option("map", (value) => map = true));

            if (args.Length == 1)
            {
                Console.WriteLine("usage: pyx {0} geosource", Command);
                return;
            }

            foreach (var id in args.Skip(1))
            {
                Guid guid;
                
                if (Guid.TryParse(id.Replace(".json",""), out guid))
                {
                    var geoSource =
                        JsonConvert.DeserializeObject<GeoSource>(
                            System.IO.File.ReadAllText(String.Format("{0}.json", guid)));

                    if (map)
                    {
                        DrawWhereMap(Program.Engine, geoSource);
                    }
                    else
                    {
                        var geometry = Program.Engine.GetGeometry(geoSource);
                        Console.WriteLine(JsonConvert.SerializeObject(geometry));    
                    }
                }
            }
        }

        private static void DrawWhereMap(Engine engine, GeoSource geoSource)
        {
            var process = engine.GetProcess(geoSource);
            var feature = pyxlib.QueryInterface_IFeature(process.getOutput());
            var geometry = feature.getGeometry();

            Console.WriteLine("   -180    -90       0       90     180|");
            Console.WriteLine("   |--------+--------+--------+--------|");

            for (var lat = 90; lat > -90; lat -= 10)
            {
                var line = String.Format("{0,3}:",lat-5);
                for (var lon = -180; lon < 180; lon += 10)
                {
                    var count = CountIntersection(lat - 10, lat, lon, lon + 10, geometry);
                    line += AsciiArt.FromNumber(count/100.0);
                }   
                Console.WriteLine(line);
            }
        }

        private static int CountIntersection(int latMin, int latMax, int lonMin, int lonMax, PYXGeometry_SPtr geometry)
        {
            var interestsCount = 0;
            for (var lat = latMin; lat < latMax; lat ++)
            {
                for (var lon = lonMin; lon <= lonMax; lon ++)
                {
                    var point = PointLocation.fromWGS84(lat, lon);
                    var cell = pyxlib.DynamicPointerCast_PYXGeometry(PYXCell.create(point.asPYXIcosIndex(14))).get();
                    if (geometry.intersects(cell))
                    {
                        interestsCount++;
                    }
                }
            }
            return interestsCount;
        }
    }
}
