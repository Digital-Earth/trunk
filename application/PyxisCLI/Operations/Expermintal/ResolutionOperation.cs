using System;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Utilities;

namespace PyxisCLI.Operations.Expermintal
{
    class ResolutionOperation : IOperationMode
    {
        public string Command
        {
            get { return "resolution"; }
        }

        public string Description
        {
            get { return "display resolution information"; }
        }

        public void Run(string[] args)
        {
            var featureId = "0";
            var tilesResolution = 0;
            var geoSourceId = "";

            args = ArgsParser.Parse(args,
                new ArgsParser.Option("geo", (value) => geoSourceId = value),
                new ArgsParser.Option("fid", (value) => featureId = value),
                new ArgsParser.Option("tiles", (value) => tilesResolution  = int.Parse(value)));

            GeoSource geoSource = null;
            GeoSource geoSourceValues = null;
            ICoverage_SPtr valuesCoverages = null;

            if (geoSourceId != "")
            {
                geoSource = JsonConvert.DeserializeObject<GeoSource>(System.IO.File.ReadAllText(geoSourceId));

                var process = Program.Engine.GetProcess(geoSource);
            }

            if (args.Length > 1)
            {
                Console.WriteLine(args[1]);
                geoSourceValues = JsonConvert.DeserializeObject<GeoSource>(System.IO.File.ReadAllText(args[1]));

                valuesCoverages = pyxlib.QueryInterface_ICoverage(Program.Engine.GetProcess(geoSourceValues).getOutput());
            }

            //pipelines - e2785a9a-5649-4b47-98d2-6339bce53d39
            //wells - 653a4f0c-c35a-4098-8101-a656ca49c2fb
            var pipelines = JsonConvert.DeserializeObject<GeoSource>(System.IO.File.ReadAllText("e2785a9a-5649-4b47-98d2-6339bce53d39.json"));
            var pipelineFeatures = pyxlib.QueryInterface_IFeatureCollection(Program.Engine.GetProcess(pipelines).getOutput());

            var wells = JsonConvert.DeserializeObject<GeoSource>(System.IO.File.ReadAllText("653a4f0c-c35a-4098-8101-a656ca49c2fb.json"));
            var wellsFeatures = pyxlib.QueryInterface_IFeatureCollection(Program.Engine.GetProcess(wells).getOutput());

            var geometry = new Pyxis.Core.IO.GeoJson.Specialized.FeatureRefGeometry()
            {
                FeatureId = featureId,
                Resource = ResourceReference.FromResource(geoSource)
            };

            var geom = geometry.ToPyxGeometry(Program.Engine);
            var tileCollection = PYXTileCollection.create();

            //TODO: inverstiage with raster to tileResolution is missing several tiles.
            geom.copyTo(tileCollection.get(),tilesResolution+5);
            tileCollection.setCellResolution(tilesResolution);

            var start = DateTime.Now;
            long totalTiles = 0;
            long totalCells = 0;
            for (var iter = tileCollection.getIterator(); !iter.end(); iter.next())
            {
                totalTiles++;
                var tile = PYXTile.create(iter.getIndex(), tilesResolution + 11);

                using (var tileAggregtor = new TileGeometryAggregator(tile.get()))
                {

                    tileAggregtor.add(geom);
                    //if (tileAggregtor.getFoundCellCount() > 0) tileAggregtor.intersect(wellsFeatures);
                    //if (tileAggregtor.getFoundCellCount() > 0) tileAggregtor.intersect(pipelineFeatures);

                    //var resultTileCollection = tileAggregtor.asTileCollection();
                    //var count = CountTilesCells(resultTileCollection);

                    var count = tileAggregtor.getFoundCellCount();

                    if (valuesCoverages != null)
                    {
                        var valueTile = valuesCoverages.getCoverageTile(tile.get());
                    }

                    totalCells += count;
                    Console.WriteLine("Tile {0} has {1} intersecting cells", iter.getIndex().toString(), count);
                }
            }
            Console.WriteLine("Total tiles : " + totalTiles);
            Console.WriteLine("Total cells : " + totalCells);
            Console.WriteLine("Time : " + (DateTime.Now - start).TotalSeconds + "[s]");

            //var index = new PYXIcosIndex("A-00");
            //while (index.getResolution() < 40)
            //{
            //    Console.WriteLine("Cell res {0} distance between cells is {1}",index.getResolution(),SnyderProjection.getInstance().resolutionToPrecision(index.getResolution()));
            //    index.incrementResolution();
            //}

            /*
            var index = new PYXIcosIndex("A-00");
            while (index.getResolution() < 40)
            {
                geom.copyTo(tileCollection, index.getResolution());

                var cellsCount = CountTilesCells(tileCollection);

                Console.WriteLine("Alberta at res {0} get rastered into {1} cells that have {2} area", 
                    index.getResolution(),
                    cellsCount,
                    tileCollection.getAreaOnReferenceShpere());

                //Console.WriteLine("Cell res {0} has area of {1}[m2]", 
                //    index.getResolution(),
                //    SnyderProjection.getInstance().calcCellAreaOnReferenceSphere(index));
                index.incrementResolution();
            }
            */
        }

        private long CountTilesCells(PYXTileCollection_SPtr geom)
        {
            long count = 0;
            var iter = geom.getTileIterator();
            while (!iter.end())
            {
                count += iter.getTile().getCellCount();
                iter.next();
            }
            return count;
        }
    }
}