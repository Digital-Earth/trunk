using GeoWebCore.Utilities;
using Newtonsoft.Json;
using Pyxis.Core.IO.GeoJson;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore
{
    class GeoPackets
    {
        List<string> IndexLut = new List<string> { "", "" };

        int OriginalVertexCount = 0;
        SortedSet<string> Vertices = new SortedSet<string>();

        List<string> VerticesIndex;
        public int CellResolution { get; set; }

        public GeoPackets(Guid geoSource,int resolution)
        {
            CellResolution = resolution;
            var process = GeoSourceInitializer.Initialize(geoSource);
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

            var snyder = SnyderProjection.getInstance();

            var features = new List<Feature>();
        
            foreach (var feature in featureCollection.GetFeaturesEnumerator())
            {
                var geoJsonFeatrure = Feature.FromIFeature(feature);

                OrderFeature(feature, geoJsonFeatrure);
                //CompressGeometry(feature, geoJsonFeatrure);
                IndexGeometryVertices(geoJsonFeatrure);

                CreateMultiResolutionGeometry(geoJsonFeatrure);

                features.Add(geoJsonFeatrure);

                if (features.Count % 100 == 0)
                {
                    Console.WriteLine("processed " + features.Count + " features");
                }
            }

            VerticesIndex = Vertices.ToList();

            Console.WriteLine("Vertex reduction: " + OriginalVertexCount + " >> " + VerticesIndex.Count);

            foreach (var feature in features)
            {
                CompressGeometryUsingVerticesIndex(feature);
            }
            
            for(var i=0; i<VerticesIndex.Count;i++)
            {
                var oldIndex = CompactIndex(VerticesIndex[i]);
                VerticesIndex[i] = CompactIndex(oldIndex, IndexLut);
                IndexLut[0] = oldIndex;
            }

            features = features.OrderBy(x => x.Properties["$order"].ToString()).ToList();

            var fields = features[0].Properties.Keys;

            foreach (var field in fields)
            {
                if (field == "$order") continue;

                var values = features.Select(x => x.Properties[field]).ToList();

                var json = new Dictionary<string, object>();
                if (field == "$geometry")
                {
                    json["type"] = "geometry";
                    json["values"] = values;
                    json["lut"] = VerticesIndex;
                }
                else
                {
                    var uniqueValues = values.Distinct().ToList();
                    Console.WriteLine(field + " has " + uniqueValues.Count + " values");

                    if (uniqueValues.Count == 1)
                    {
                        json["type"] = "const";
                        json["value"] = uniqueValues[0];
                    }                    
                    else if (uniqueValues.Count < features.Count / 10)
                    {
                        uniqueValues.Sort();                        
                        var valueIndexes = values.Select(x => uniqueValues.BinarySearch(x)).ToList();

                        json["type"] = "lut";
                        json["values"] = valueIndexes;
                        json["lut"] = uniqueValues;

                    }
                    else
                    {
                        json["type"] = "normal";
                        json["values"] = values;
                    }
                }

                System.IO.File.WriteAllText("geopackets\\" + field + ".json", Newtonsoft.Json.JsonConvert.SerializeObject(json));
            }
        }

        private void CreateMultiResolutionGeometry(Feature geoJsonFeatrure)
        {
            if (geoJsonFeatrure.Geometry is MultiPolygonGeometry)
            {
                var multiPolygon = geoJsonFeatrure.Geometry as MultiPolygonGeometry;

                foreach (var poly in multiPolygon.Coordinates)
                {
                    foreach (var ring in poly)
                    {
                        var lod = CreateMultiResolutionRing(ring, CellResolution);

                        foreach (var level in lod)
                        {
                            Console.WriteLine(JsonConvert.SerializeObject(level));
                        }
                    }
                }
            }
            else if (geoJsonFeatrure.Geometry is PolygonGeometry)
            {
                var polygon = geoJsonFeatrure.Geometry as PolygonGeometry;

                foreach (var ring in polygon.Coordinates)
                {
                    var lod = CreateMultiResolutionRing(ring, CellResolution);

                    foreach (var level in lod)
                    {
                        Console.WriteLine(JsonConvert.SerializeObject(level));
                    }
                }
            }
        }

        private List<string> MakeRingUnique(List<string> indices)
        {
            var unique = new List<string>();

            var lastIndex = "";

            //same index twice removal
            foreach (var index in indices)
            {
                if (index != lastIndex)
                {
                    unique.Add(index);
                }
                lastIndex = index;
            }

            //reduce [a,b,a,b] into [a,b]
            for(int i=unique.Count-1;i>3;i--)
            {
                if (unique[i-2] == unique[i] && unique[i-3] == unique[i-1])
                {
                    unique.RemoveRange(i-1,2);
                    i--;
                }         
            }

            return unique;
        }

        private List<string> CreateIndexRing(List<GeographicPosition> ring, int resolution)
        {
            var indices = ring.Select(x => x.ToPointLocation().asPYXIcosIndex(resolution).toString()).ToList();

            return MakeRingUnique(indices);
        }

        private List<List<string>> CreateMultiResolutionRing(List<GeographicPosition> ring,int resolution)
        {
            var result = new List<List<string>>();

            var indices = CreateIndexRing(ring, resolution);

            result.Insert(0, indices);

            while (resolution > 7)
            {
                resolution -= 4;
                indices = MakeRingUnique(indices.Select(x => x.Substring(0, x.Length - 4)).ToList());
                result.Insert(0, indices);                
            }

            result = result.Select(x => x.Select(y => CompactIndex(y)).ToList()).ToList();

            
            for (int i = result.Count-1; i > 0; i--)
            {
                result[i] = result[i].Select(x => CompactIndex(x, result[i - 1])).ToList();
            }

            return result;
        }

        private void IndexGeometryVertices(Feature geoJsonFeatrure)
        {
            if (geoJsonFeatrure.Geometry is LineStringGeometry)
            {
                var line = geoJsonFeatrure.Geometry as LineStringGeometry;
                foreach (var point in line.Coordinates)
                {
                    Vertices.Add(point.ToPointLocation().asPYXIcosIndex(CellResolution).toString());
                    OriginalVertexCount++;
                }
            }
            else if (geoJsonFeatrure.Geometry is PolygonGeometry)
            {
                var polygon = geoJsonFeatrure.Geometry as PolygonGeometry;
                foreach (var ring in polygon.Coordinates)
                {
                    foreach (var point in ring)
                    {
                        Vertices.Add(point.ToPointLocation().asPYXIcosIndex(CellResolution).toString());
                        OriginalVertexCount++;
                    }
                }
            }
            else if (geoJsonFeatrure.Geometry is MultiPolygonGeometry)
            {
                var multipolygon = geoJsonFeatrure.Geometry as MultiPolygonGeometry;
                foreach (var polygon in multipolygon.Coordinates)
                {
                    foreach (var ring in polygon)
                    {
                        foreach (var point in ring)
                        {
                            Vertices.Add(point.ToPointLocation().asPYXIcosIndex(CellResolution).toString());
                            OriginalVertexCount++;
                        }
                    }
                }
            }
            else
            {
            }
        }

        private void CompressGeometryUsingVerticesIndex(Feature geoJsonFeatrure)
        {
            var lastIndex = 0;

            if (geoJsonFeatrure.Geometry is LineStringGeometry)
            {
                var line = geoJsonFeatrure.Geometry as LineStringGeometry;

                var lut = new List<int>();
                
                foreach (var point in line.Coordinates)
                {
                    var newIndex = VerticesIndex.BinarySearch(point.ToPointLocation().asPYXIcosIndex(CellResolution).toString());
                    if (newIndex != lastIndex)
                    {
                        lut.Add(newIndex - lastIndex);
                    }
                    lastIndex = newIndex;
                }

                geoJsonFeatrure.Properties["$geometry"] = new Dictionary<string, object> { { "t", "l" }, { "i", lut } };
            }
            else if (geoJsonFeatrure.Geometry is PolygonGeometry)
            {
                var polygon = geoJsonFeatrure.Geometry as PolygonGeometry;

                var lut = new List<List<int>>();
                foreach (var ring in polygon.Coordinates)
                {
                    var lutRing = new List<int>();
                    foreach (var point in ring)
                    {
                        var newIndex = VerticesIndex.BinarySearch(point.ToPointLocation().asPYXIcosIndex(CellResolution).toString());
                        if (newIndex != lastIndex)
                        {
                            lutRing.Add(newIndex - lastIndex);
                        }
                        lastIndex = newIndex;
                    }
                    lut.Add(lutRing);
                }

                geoJsonFeatrure.Properties["$geometry"] = new Dictionary<string, object> { { "t", "p" }, { "i", lut } };
            }
            else if (geoJsonFeatrure.Geometry is MultiPolygonGeometry)
            {
                var multipolygon = geoJsonFeatrure.Geometry as MultiPolygonGeometry;
                var lut = new List<List<List<int>>>();
                foreach (var polygon in multipolygon.Coordinates)
                {
                    var lutPolygon = new List<List<int>>();
                    foreach (var ring in polygon)
                    {
                        var lutRing = new List<int>();
                        foreach (var point in ring)
                        {
                            var newIndex = VerticesIndex.BinarySearch(point.ToPointLocation().asPYXIcosIndex(CellResolution).toString());
                            if (newIndex != lastIndex)
                            {
                                lutRing.Add(newIndex - lastIndex);
                            }
                            lastIndex = newIndex;
                        }
                        lutPolygon.Add(lutRing);
                    }
                    lut.Add(lutPolygon);
                }                
                geoJsonFeatrure.Properties["$geometry"] = new Dictionary<string, object> { { "t", "mp" }, { "i", lut } };
            }
            else
            {
            }
        }

        private void OrderFeature(IFeature_SPtr feature, Feature geoJsonFeatrure)
        {
            var circle = feature.getGeometry().getBoundingCircle();
            var resolution = Math.Max(2, PYXBoundingCircle.estimateResolutionFromRadius(circle.getRadius()));
            var index = PointLocation.fromXYZ(circle.getCenter()).asPYXIcosIndex(resolution).toString();
            index = CompactIndex(index);

            geoJsonFeatrure.Properties["$order"] = index;
        }

        private void CompressGeometry(IFeature_SPtr feature, Feature geoJsonFeatrure)
        {
            var circle = feature.getGeometry().getBoundingCircle();
            var resolution = Math.Max(2, PYXBoundingCircle.estimateResolutionFromRadius(circle.getRadius()));
            var index = PointLocation.fromXYZ(circle.getCenter()).asPYXIcosIndex(resolution).toString();
            index = CompactIndex(index);

            var compactIndex = CompactIndex(index, IndexLut);
            IndexLut[0] = index;
            //Console.WriteLine(feature.getID() + " : " + index.toString());

            if (geoJsonFeatrure.Geometry is LineStringGeometry)
            {
                var line = geoJsonFeatrure.Geometry as LineStringGeometry;

                var indices = new List<string>();


                foreach (var point in line.Coordinates)
                {
                    //TODO: this is not good enough - we need to find the min distance between 2 points and used that the resolution.
                    var pointIndex = point.ToPointLocation().asPYXIcosIndex(Math.Min(40, resolution + 11)).toString();
                    pointIndex = CompactIndex(pointIndex);
                    indices.Add(CompactIndex(pointIndex, IndexLut));
                    IndexLut[1] = pointIndex;
                }
                geoJsonFeatrure.Properties["$geometry"] = new Dictionary<string, object> { { "t", "l" }, { "r", compactIndex }, { "i", indices } };
            }
            else if (geoJsonFeatrure.Geometry is PolygonGeometry)
            {
                var polygon = geoJsonFeatrure.Geometry as PolygonGeometry;

                var indices = new List<List<string>>();

                foreach (var ring in polygon.Coordinates)
                {
                    var indexRing = new List<string>();
                    foreach (var point in ring)
                    {
                        //TODO: this is not good enough - we need to find the min distance between 2 points and used that the resolution.
                        var pointIndex = point.ToPointLocation().asPYXIcosIndex(Math.Min(40, resolution + 11)).toString();
                        pointIndex = CompactIndex(pointIndex);
                        indexRing.Add(CompactIndex(pointIndex, IndexLut));
                        IndexLut[1] = pointIndex;
                    }
                    indices.Add(indexRing);
                }
                geoJsonFeatrure.Properties["$geometry"] = new Dictionary<string, object> { { "t", "p" }, { "r", compactIndex }, { "i", indices } };
            }
            else if (geoJsonFeatrure.Geometry is MultiPolygonGeometry)
            {
                var multipolygon = geoJsonFeatrure.Geometry as MultiPolygonGeometry;

                var indices = new List<List<List<string>>>();

                foreach (var polygon in multipolygon.Coordinates)
                {
                    var polyIndex = new List<List<string>>();
                    foreach (var ring in polygon)
                    {
                        var indexRing = new List<string>();
                        foreach (var point in ring)
                        {
                            //TODO: this is not good enough - we need to find the min distance between 2 points and used that the resolution.
                            var pointIndex = point.ToPointLocation().asPYXIcosIndex(Math.Min(40, resolution + 11)).toString();
                            pointIndex = CompactIndex(pointIndex);
                            indexRing.Add(CompactIndex(pointIndex, IndexLut));
                            IndexLut[1] = pointIndex;
                        }
                        polyIndex.Add(indexRing);
                    }
                    indices.Add(polyIndex);
                }
                geoJsonFeatrure.Properties["$geometry"] = new Dictionary<string, object> { { "t", "mp" }, { "r", compactIndex }, { "i", indices } };
            }
            else
            {
                geoJsonFeatrure.Properties["$geometry"] = null;
            }            
        }

        Dictionary<string, string> ReplaceMap = new Dictionary<string, string>
        {
            {"00","z"},
            {"01","a"},
            {"02","b"},
            {"03","c"},
            {"04","d"},
            {"05","e"},
            {"06","f"},
            {"10","A"},
            {"20","B"},
            {"30","C"},
            {"40","D"},
            {"50","F"},
            {"60","E"}
        };

        string CompactIndex(string index)
        {
            var parts = index.Split('-');

            var compact = "";
            while (parts[1].Length > 1)
            {
                var end = parts[1].Substring(parts[1].Length - 2);
                parts[1] = parts[1].Substring(0,parts[1].Length - 2);
                compact = ReplaceMap[end] + compact;
            }

            //return parts[0] + "-" + parts[1].Replace("10", "a").Replace("20", "b").Replace("30", "c").Replace("40", "d").Replace("50", "e").Replace("60", "f").Replace("00", "z");
            return parts[0] + "-" + parts[1] + compact;
        }

        string CompactIndex(string index, List<string> lookup)
        {            
            var result = index;

            for (int i = 0; i < lookup.Count; i++)
            {
                var prefix = lookup[i];
                int l = 0;
                for (; l < prefix.Length && l < index.Length; l++)
                {
                    if (prefix[l] != index[l])
                    {
                        break;
                    }
                }
                if (l > 2)
                {
                    string compression;
                    if (prefix.Length == l)
                    {
                        compression = String.Format("{0}+{1}", i, index.Substring(l));
                    }
                    else
                    {
                        compression = String.Format("{0}-{1}+{2}", i, prefix.Length - l, index.Substring(l));
                    }
                    
                    if (compression.Length < result.Length)
                    {
                        result = compression;
                    }
                }
            }

            return result;
        }
    }    
}
