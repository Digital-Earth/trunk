using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Utilities;

namespace Pyxis.Core
{
    /// <summary>
    /// Calcualte Perimeter based on PyxTileCollection
    /// 
    /// This PerimeterCalculator can calculate 2 types of Perimeter:
    /// 1) Find all Cells that are on the Edge of PYXTileCollection. 
    ///    Edge cell is defined as a cell that is conatined by tile collection and have a neighboor cell that is not contained.
    /// 2) Generate a GeoJson polygon that describe the Perimieter of the PyxTileCollection. the perimeter polygon edges
    ///    the edge between the cells that are conainted in the tile collection and the cells that are not.
    /// 
    /// The PerimeterCalculator also generate Performance log to see how many checks happen while the operation
    /// was exectued. Use GetCandidateCells() function to visualize all the cells that been checked.
    /// 
    /// TODO: move this into C++ pyxlib library.
    /// </summary>
    public class PerimeterCalculator
    {
        public class Segment
        {
            public List<string> Indices { get; set; }

            public string Start
            {
                get { return Indices[0]; }
            }

            public string End
            {
                get { return Indices[Indices.Count - 1]; }
            }

            public int Length
            {
                get { return Indices.Count; }
            }

            public bool Closed
            {
                get { return Start == End; }
            }

            public Segment(string a, string b)
            {
                Indices = new List<string> { a, b };
            }

            public static Segment FromNeighborsCells(string cellA, string cellB)
            {
                var match = new List<string>();
                var vertices = new HashSet<string>();
                using (var aVertices = new PYXVertexIterator(new PYXIcosIndex(cellA)))
                {
                    while (!aVertices.end())
                    {
                        vertices.Add(aVertices.getIndex().toString());
                        aVertices.next();
                    }
                }

                using (var bVertices = new PYXVertexIterator(new PYXIcosIndex(cellB)))
                {
                    while (!bVertices.end())
                    {
                        var v = bVertices.getIndex().toString();
                        if (vertices.Contains(v))
                        {
                            match.Add(v);
                        }
                        bVertices.next();
                    }
                }

                return match.Count >= 2 ? new Segment(match[0], match[1]) : null;
            }

            public void Merge(Segment other)
            {
                if (End == other.Start)
                {
                    Indices.AddRange(other.Indices.Skip(1));
                }
                else if (End == other.End)
                {
                    Indices.AddRange(other.Indices.AsEnumerable().Reverse().Skip(1));
                }
                else if (Start == other.End)
                {
                    Indices = other.Indices.Concat(Indices.Skip(1)).ToList();
                }
                else if (Start == other.Start)
                {
                    Indices = other.Indices.AsEnumerable().Reverse().Concat(Indices.Skip(1)).ToList();
                }
            }
        }

        public class SegmentsSet
        {
            public List<Segment> Rings { get; set; }
            public List<Segment> ActiveSegments { get; set; }
            public Dictionary<string, Segment> Edges { get; set; }

            public SegmentsSet()
            {
                Rings = new List<Segment>();
                ActiveSegments = new List<Segment>();
                Edges = new Dictionary<string, Segment>();
            }

            public int Size
            {
                get { return ActiveSegments.Count + Rings.Count; }
            }

            public int VerticesCount
            {
                get { return Rings.Sum(ring => ring.Length - 1) + ActiveSegments.Sum(segment => segment.Length); }
            }

            public void Add(Segment segment)
            {
                while (segment != null)
                {
                    if (Edges.ContainsKey(segment.Start))
                    {
                        var match = Edges[segment.Start];

                        Edges.Remove(match.Start);
                        Edges.Remove(match.End);
                        ActiveSegments.Remove(match);

                        match.Merge(segment);

                        segment = match;
                    }
                    else if (Edges.ContainsKey(segment.End))
                    {
                        var match = Edges[segment.End];

                        Edges.Remove(match.Start);
                        Edges.Remove(match.End);
                        ActiveSegments.Remove(match);

                        match.Merge(segment);

                        segment = match;
                    }
                    else
                    {
                        if (segment.Closed)
                        {
                            Rings.Add(segment);
                        }
                        else
                        {
                            Edges[segment.Start] = segment;
                            Edges[segment.End] = segment;
                            ActiveSegments.Add(segment);
                        }
                        segment = null;
                    }
                }
            }
        }

        public class CalculatorPerformance
        {
            public int Cells { get; set; }
            public int Tiles { get; set; }
            public int CheckedCells { get; set; }
            public int PerimeterCells { get; set; }
            public int Rings { get; set; }
            public int VerticesCount { get; set; }
            public StopwatchProfiler Log { get; set; }
        }

        private readonly PYXTileCollection_SPtr m_tileCollection;
        public CalculatorPerformance Performance { get; private set; }

        public PerimeterCalculator(Engine engine, IGeometry geometry, int resolution)
        {
            var pyxGeometry = engine.ToPyxGeometry(geometry, resolution);
            m_tileCollection = PYXTileCollection.create();
            pyxGeometry.copyTo(m_tileCollection.get(), resolution);            
        }

        public PerimeterCalculator(PYXTileCollection_SPtr tileCollection)
        {
            m_tileCollection = tileCollection;
        }

        public PYXTileCollection_SPtr GetPerimeterCells()
        {
            ResetPerformance();

            var perimeterTileCollection = PYXTileCollection.create();
            perimeterTileCollection.setCellResolution(m_tileCollection.getCellResolution());

            ForeachCandidateCell((index, parentTile) =>
            {
                var negiborIterator = PYXNeighbourIterator.create(index);
                while (!negiborIterator.end())
                {
                    if (!parentTile.hasIndex(negiborIterator.getIndex()))
                    {
                        using (var cell = PYXCell.create(negiborIterator.getIndex()))
                        {
                            if (!m_tileCollection.intersects(cell.get()))
                            {
                                Performance.PerimeterCells++;
                                perimeterTileCollection.addTile(index, perimeterTileCollection.getCellResolution());
                                return;
                            }
                        }
                    }
                    negiborIterator.next();
                }                
            });

            Performance.Log.LogCompletion("completed");

            return perimeterTileCollection;            
        }

        public PYXTileCollection_SPtr GetCandidateCells()
        {
            ResetPerformance();

            var perimeterTileCollection = PYXTileCollection.create();
            perimeterTileCollection.setCellResolution(m_tileCollection.getCellResolution());

            ForeachCandidateCell((index, parentTile) =>
            {
                perimeterTileCollection.addTile(index, perimeterTileCollection.getCellResolution());   
            });

            Performance.Log.LogCompletion("completed");

            return perimeterTileCollection;
        }

        
        public IGeometry GetPerimeterPolygon()
        {
            ResetPerformance();

            var segments = new SegmentsSet();

            ForeachCandidateCell((index, parentTile) =>
            {
                var perimeter = false;
                var negiborIterator = PYXNeighbourIterator.create(index);
                while (!negiborIterator.end())
                {
                    if (!parentTile.hasIndex(negiborIterator.getIndex()))
                    {
                        using (var cell = PYXCell.create(negiborIterator.getIndex()))
                        {
                            if (!m_tileCollection.intersects(cell.get()))
                            {
                                perimeter = true;
                                segments.Add(Segment.FromNeighborsCells(
                                    index.toString(),
                                    negiborIterator.getIndex().toString()));
                            }
                        }
                    }
                    negiborIterator.next();
                }
                if (perimeter)
                {
                    Performance.PerimeterCells++;
                }
            });

            Performance.Log.LogCompletion("cells iteration");

            var polygon = new PolygonGeometry()
            {
                Coordinates = new List<List<GeographicPosition>>()
            };

            foreach (var ring in segments.Rings)
            {
                var ringPoints = ring.Indices.Select(GeographicPosition.FromIndex).ToList();
                polygon.Coordinates.Add(ringPoints);
            }

            Performance.Rings = segments.Rings.Count;
            Performance.VerticesCount = segments.VerticesCount;

            if (segments.ActiveSegments.Count > 0)
            {
                throw new Exception("Failed to find perimeter as not all segments been closed");
            }

            Performance.Log.LogCompletion("completed");

            return polygon;            
        }

        private void ForeachCandidateCell(Action<PYXIcosIndex,PYXTile_SPtr> callback)
        {
            var tileIterator = m_tileCollection.getTileIterator();

            while (!tileIterator.end())
            {
                Performance.Tiles++;
                var tile = tileIterator.getTile();

                ForeachCandidateCellInTile(tile, tile, callback);

                tileIterator.next();
            }
        }

        private void ForeachCandidateCellInTile(PYXTile_SPtr tile, PYXTile_SPtr parentTile,
            Action<PYXIcosIndex, PYXTile_SPtr> callback)
        {            
            if (tile.getDepth() >= 3)
            {
                var root = tile.getRootIndex();

                if (!root.hasVertexChildren())
                {
                    root.incrementResolution();
                }

                var neighboors = new PYXNeighbourIterator(root);
                while (!neighboors.end())
                {
                    if (
                        !m_tileCollection.contains(
                            PYXTile.create(neighboors.getIndex(), tile.getCellResolution()).get()))
                    {
                        break;
                    }
                    neighboors.next();
                }

                //this tile is completey inside
                if (neighboors.end())
                {
                    Performance.Cells += tile.getCellCount();
                    return;
                }

                var childrenIterator = new PYXChildIterator(root);

                var childRoot = childrenIterator.getIndex();

                var prefix = childRoot.ToString();
                var depth = tile.getCellResolution() - childRoot.getResolution();
                var gap = 0;

                PYXIcosMath.getCellGap(childRoot, ref gap);

                Performance.Cells += PYXIcosMath.getCellCount(childRoot, tile.getCellResolution());

                for (var d = 1; d <= 6; d++)
                {
                    if (gap == d)
                    {
                        continue;
                    }

                    var index = prefix;
                    for (var r = 0; r < depth; r++)
                    {
                        if (r % 2 == 0)
                        {
                            index += d;
                        }
                        else
                        {
                            index += '0';
                        }
                    }

                    var newIndex = new PYXIcosIndex(index);

                    Performance.CheckedCells++;

                    callback(newIndex,parentTile);
                }

                childrenIterator.next();

                while (!childrenIterator.end())
                {
                    var newRoot = childrenIterator.getIndex();
                    ForeachCandidateCellInTile(PYXTile.create(newRoot, tile.getCellResolution()),parentTile,callback);                    
                    childrenIterator.next();
                }
            }
            else
            {
                var iterator = tile.getIterator();

                Performance.Cells += tile.getCellCount();

                while (!iterator.end())
                {
                    var index = iterator.getIndex();
                    Performance.CheckedCells++;
                    callback(index, parentTile);
                    iterator.next();
                }
            }
        }

        private void ResetPerformance()
        {
            Performance = new CalculatorPerformance()
            {
                Log = new StopwatchProfiler("perimeter")
            };
        }

    }
}
