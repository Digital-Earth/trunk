using System;
using System.Linq;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Core.DERM;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class DggsOperation : IOperationMode
    {
        public string Command
        {
            get { return "dggs"; }
        }

        public string Description
        {
            get { return "do cell conversions from lat/lon to dggs"; }
        }

        public void Run(string[] args)
        {
            double? lat = null;
            double? lng = null;
            double? radius = null;
            int? resolution = null;

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("lat", (value) => lat = double.Parse(value)),
                new ArgsParser.Option("long|lng", (value) => lng = double.Parse(value)),
                new ArgsParser.Option("radius", (value) => radius = double.Parse(value)),
                new ArgsParser.Option("r|resolution", (value) => resolution = int.Parse(value)));

            var dggs = Program.Engine.DERM();

            if (lat.HasValue && lng.HasValue)
            {
                if (radius.HasValue)
                {
                    resolution = dggs.ResolutionFromDistance(radius.Value);
                }

                if (resolution.HasValue)
                {
                    var cell = dggs.Cell(new GeographicPosition()
                    {
                        Latitude = lat.Value,
                        Longitude = lng.Value
                    }, resolution.Value);

                    Console.WriteLine(cell.Index);
                }

                if (!resolution.HasValue && !radius.HasValue)
                {
                    Console.WriteLine("please provide resolutions (integer) or radius (meters) to convert lat/long into cell");
                }
            }
            else
            {
                foreach (var index in args.Skip(1))
                {
                    var cell = dggs.Cell(index);

                    Console.WriteLine("Latitude={0}, Longitude={1}, Radius={2}[m]", cell.Center.Latitude,cell.Center.Longitude,cell.Radius.InMeters);
                }
            }
        }
    }
}