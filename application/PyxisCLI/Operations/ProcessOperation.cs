using System;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;
using Pyxis.Core.Measurements;
using Pyxis.Utilities;

namespace PyxisCLI.Operations
{
    class ProcessOperation : IOperationMode
    {
        public string Command
        {
            get { return "process"; }
        }

        public string Description
        {
            get { return "show geosource specification"; }
        }

        public void Run(string[] args)
        {
            int resolution = 0;
            
            args = ArgsParser.Parse(args,
                new ArgsParser.Option("r|resolution", (value) => resolution = int.Parse(value)));

            if (args.Length < 2)
            {
                Console.WriteLine("usage: pyx {0} reference", Command);
                return;
            }

            var reference = new Reference(args[1]);

            if (Program.Workspaces.WorkspaceExists(reference.Workspace))
            {
                var geoSource = Program.Workspaces.ResolveGeoSource(reference);

                ProcessTiles(geoSource, resolution);
            }
        }

        public void ProcessTiles(GeoSource geoSource, int resolution)
        {
            var process = Program.Engine.GetProcess(geoSource);

            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            var geometry = Program.Engine.GetGeometry(geoSource);
            var tileCollection = GetTilesCoverage(geometry, resolution);

            var start = DateTime.Now;
            var totalTiles = tileCollection.getCellCount();
            var tileIndex = 0;
            PYXIterator_SPtr it = tileCollection.getIterator();
            while (!it.end())
            {
                var percent = 100*(tileIndex+1)/totalTiles;
                Console.WriteLine("Tile root: {0} depth: {1} ({2}%)", 
                    it.getIndex().toString(),
                    resolution - it.getIndex().getResolution(),
                    percent);
                using (var tile = coverage.getCoverageTile(PYXTile.create(it.getIndex(), resolution).get()))
                {
                    //do nothing - just dispose it
                }
                tileIndex++;
                it.next();
            }

            var totalSeconds = (DateTime.Now - start).TotalSeconds;
            Console.WriteLine("Processed {0} in {1} sec, {2} tile/sec", totalTiles, totalSeconds, totalTiles / totalSeconds);
        }

        public PYXTileCollection_SPtr GetTilesCoverage(IGeometry geometry, int resolution)
        {
            var defaultTileDepth = PYXTile.knDefaultTileDepth;

            PYXGeometry_SPtr pyxGeometry = geometry.ToPyxGeometry(Program.Engine);

            if (pyxGeometry == null || pyxGeometry.isNull())
            {
                throw new Exception("The geometry of the geoSource is null");
            }

            // determine the tile depth for tile fetches
            if ((resolution - defaultTileDepth) < PYXIcosIndex.knMinSubRes)
            {
                defaultTileDepth = resolution - PYXIcosIndex.knMinSubRes;
            }

            PYXTileCollection_SPtr tiles = PYXTileCollection.create();
            pyxGeometry.copyTo(tiles.get(), resolution - defaultTileDepth);
            
            return tiles;
        }
    }
}
