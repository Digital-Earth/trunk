using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Web.Http;
using GeoWebCore.Models;
using GeoWebCore.WebConfig;
using GeoWebCore.Services;
using GeoWebCore.Services.Cache;
using GeoWebCore.Services.Cluster;
using GeoWebCore.Utilities;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using Pyxis.Core.Measurements;
using Pyxis.Utilities;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// The GeoSourceController provides client access to data from the GeoSource.
    /// </summary>
    [RoutePrefix("api/v1/Geometry")]
    public class GeometryController : ApiController
    {
        private enum GeometryResolverStatus
        {
            Resolving,
            Resolved,
            Error
        };

        private class GeometryResolver
        {
            public GeometryResolverStatus Status { get; set; }
            public int OriginalResolution { get; set; }
            public IGeometry Geometry { get; set; }
            public PYXGeometry_SPtr PyxGeometry { get; set; }
            public GeoSourceDataCharacterization Characterization { get; set; }
            public Func<PYXTile_SPtr, PYXGeometry_SPtr> TileGenertor { get; set; }

            public byte[] GetRhombusBytes(string key, int size)
            {
                var rhombus = RhombusHelper.GetRhombusFromKey(key);

                var rasterizer = new PYXRhombusRasterizer(rhombus, RhombusHelper.SizeToResolution(size));

                if (rasterizer.getCellResolution() <= OriginalResolution || TileGenertor == null)
                {
                    rasterizer.setGeometry(PyxGeometry);
                }
                else 
                {
                    while (!rasterizer.isReady())
                    {
                        var tile = rasterizer.getNeededTile();
                        rasterizer.setTileGeometry(tile, TileGenertor(tile));
                    }    
                }
                
                var base64 = rasterizer.rasterToBase64();
                var bytes = Convert.FromBase64String(base64);
                return bytes;
            }

            public void GenerateTileGenertorIfNeeded()
            {
                if (!(Geometry is BooleanGeometry))
                {
                    return;
                }

                var booleanGeometry = Geometry as BooleanGeometry;

                var generators = new List<Func<PYXTile_SPtr, PYXTileCollection_SPtr>>();
                foreach (var operation in booleanGeometry.Operations)
                {
                    generators.Add(CreateGeneratorForOperation(operation.Geometry));
                }

                TileGenertor = (tile) =>
                {
                    var result = pyxlib.DynamicPointerCast_PYXGeometry(generators[0](tile));
                    foreach (var generator in generators.Skip(1))
                    {
                        if (result.isEmpty())
                        {
                            break;
                        }
                        result = result.intersection(generator(tile).get());
                    }
                    return result;
                };
            }

            private Func<PYXTile_SPtr, PYXTileCollection_SPtr> CreateGeneratorForOperation(IGeometry geometry)
            {
                if (geometry is ConditionGeometry)
                {
                    return CreateGeneratorForCondition(geometry as ConditionGeometry);
                }
                else
                {
                    var pyxGeometry = GeoSourceInitializer.Engine.ToPyxGeometry(geometry);
                    var condition = PYXWhereCondition.geometry(pyxGeometry);
                    return (tile) => condition.match(tile.get());
                }
            }

            private Func<PYXTile_SPtr, PYXTileCollection_SPtr> CreateGeneratorForCondition(
                ConditionGeometry conditionGeometry)
            {
                GeoSourceClusterResolver geoSourceResolver = null;

                if (conditionGeometry.Resource != null)
                {
                    geoSourceResolver = Program.RunInformation.Cluster.ResolveGeoSource(conditionGeometry.Resource.Id);    
                } 
                else if (conditionGeometry.Reference != null)
                {
                    var geoSource = GeoSourceInitializer.Engine.ResolveReference(conditionGeometry.Reference);
                    geoSourceResolver = Program.RunInformation.Cluster.ResolveGeoSource(geoSource.Id);    
                }

                return (tile) => geoSourceResolver.GetWhereTile(
                    tile.get(),
                    conditionGeometry.Property,
                    conditionGeometry.Range.Min.ToString(),
                    conditionGeometry.Range.Max.ToString());
            }
        }

        private static readonly LimitedSizeDictionary<string, GeometryResolver> s_resolvers = new LimitedSizeDictionary<string, GeometryResolver>(256);
        private static readonly object s_resolversLock = new object();

        private GeometryResolver GetResolver(string key)
        {
            lock (s_resolversLock)
            {
                if (s_resolvers.ContainsKey(key))
                {
                    return s_resolvers[key];
                }

                var resolver = new GeometryResolver()
                {
                    Status = GeometryResolverStatus.Resolved,
                    Geometry = GeometryCacheSingleton.Get(key)
                };

                resolver.PyxGeometry = Program.RunInformation.Cluster.ResolveGeometry(key).GetPYXGeometry();

                var boundingCircle = resolver.PyxGeometry.getBoundingCircle();

                var circleGeometry = new CircleGeometry()
                {
                    Coordinates = new GeographicPosition(PointLocation.fromXYZ(boundingCircle.getCenter())),
                    Radius = boundingCircle.getRadius() * SphericalDistance.Radian
                };

                resolver.Characterization = new GeoSourceDataCharacterization()
                {
                    NativeResolution = resolver.PyxGeometry.getCellResolution(),
                    BoundingCircle = circleGeometry
                };

                resolver.OriginalResolution = resolver.Characterization.NativeResolution;

                resolver.GenerateTileGenertorIfNeeded();

                return s_resolvers[key] = resolver;
            }
        }

        /// <summary>
        /// post a geometry and get back a key (hash) representing that geometry so it can be used later on for other requests
        /// </summary>
        /// <param name="geometry">A GeoJson geometry</param>
        /// <returns>key - hash string value</returns>
        [HttpPost]
        [Route("")]
        [TimeTrace()]
        public string PostGeometry(IGeometry geometry)
        {
            return GeometryCacheSingleton.Add(geometry);
        }

        [Route("{geometryId}")]
        [TimeTrace()]
        public IGeometry GetGeometry(string geometryId)
        {
            var geometry = GeometryCacheSingleton.Get(geometryId);
            return geometry;
        }

        /// <summary>
        /// Return the area of a geometry in square meters on earth
        /// </summary>
        /// <param name="geometryId">geometry hash id</param>
        /// <returns>Area in square meters</returns>
        [HttpGet]
        [Route("{geometryId}/Area")]
        [TimeTrace()]
        public double Area(string geometryId)
        {
            var geometry = GetGeometry(geometryId);
            return geometry.GetArea(GeoSourceInitializer.Engine).InSquareMeters;
        }

        /// Get a characterization of the data of the GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to request data characterization on</param>
        /// <returns>GeoSourceDataCharacterization for the given GeoSource</returns>
        [HttpGet]
        [Route("{geometryId}/Characterize")]
        [ApiCache()]
        [TimeTrace("geometryId")]
        public GeoSourceDataCharacterization Characterize(string geometryId)
        {
            var resolver = GetResolver(geometryId);
            var pyxGeometry = resolver.PyxGeometry;

            var boundingCircle = pyxGeometry.getBoundingCircle();

            var circleGeometry = new CircleGeometry()
            {
                Coordinates = new GeographicPosition(PointLocation.fromXYZ(boundingCircle.getCenter())),
                Radius = boundingCircle.getRadius() * SphericalDistance.Radian
            };

            return new GeoSourceDataCharacterization()
            {
                NativeResolution = pyxGeometry.getCellResolution(),
                BoundingCircle = circleGeometry
            };
        }

        /// <summary>
        /// Return a bounding circle of a geometry
        /// </summary>
        /// <param name="geometryId">geometry hash id</param>
        /// <returns>A GeoJson CircleGeometry that covers the given GeoJson geometry</returns>
        [HttpGet]
        [Route("{geometryId}/BoundingCircle")]
        [TimeTrace()]
        public CircleGeometry BoundingCircle(string geometryId)
        {
            var resolver = GetResolver(geometryId);
            var pyxGeometry = resolver.PyxGeometry;

            var boundingCircle = pyxGeometry.getBoundingCircle();

            return new CircleGeometry()
            {
                Coordinates = new GeographicPosition(PointLocation.fromXYZ(boundingCircle.getCenter())),
                Radius = boundingCircle.getRadius() * SphericalDistance.Radian
            };
        }
        

        /// <summary>
        /// forward a GET HTTP request to GeoSource server
        /// </summary>
        /// <param name="geometryId">geometry hash id</param>
        /// <returns>HttpResponse that includes TileCollection result for the intersection of the given geometry with the requested tile</returns>
        [HttpGet]
        [Route("{geometryId}/Tile")]
        [TimeTrace("geometryId,root,depth")]
        public async Task<HttpResponseMessage> Get(string geometryId, string root, int depth)
        {
            var tile = PYXTile.create(new PYXIcosIndex(root), depth);

            var resolver = GetResolver(geometryId);
            var pyxGeometry = resolver.PyxGeometry;

            var tileCollection = pyxlib.DynamicPointerCast_PYXTileCollection(pyxGeometry.intersection(tile.get()));
            
            return Request.CreateResponseWithCache(new TileCollectionGeometry(tileCollection));
        }

        [HttpGet]
        [Route("{geometryId}/Rhombus")]
        [TimeTrace("geometryId,key,size")]
        public async Task<HttpResponseMessage> RasterRhombus(string geometryId, string key, int size, string format = "png")
        {
            var resolver = GetResolver(geometryId);
            
            byte[] bytes = null;

            if (RhombusHelper.IsTopRhombus(key))
            {
                var subRhombusSize = RhombusHelper.GetLowerSize(size);

                bytes = new byte[size * size];

                for (var childRhombus = 0; childRhombus < 9; childRhombus++)
                {
                    var parentOffset = RhombusHelper.ChildRhombusOffset(childRhombus, size);
                    var childBytes = resolver.GetRhombusBytes(key + childRhombus, subRhombusSize);

                    for (var y = 0; y < subRhombusSize; y++)
                    {
                        Array.Copy(childBytes, y * subRhombusSize, bytes, RhombusHelper.RhombusUvToOffset(size, parentOffset, y), subRhombusSize);
                    }
                }
            }
            else
            {
                bytes = resolver.GetRhombusBytes(key, size);    
            }
            
            var rgba = ConvertByteToColor(bytes);

            using (var bitmap = BitmapFromRawBytes(rgba, size, size))
            {
                return BitmapToResponse(bitmap, "png");
            }
        }

        private byte[] ConvertByteToColor(byte[] bytes)
        {
            var colors = new byte[bytes.Length*4];
            for (var i = 0; i < bytes.Length; i++)
            {
                if (bytes[i] > 0)
                {
                    colors[i * 4] = 255;
                    colors[i * 4 + 1] = 255;
                    colors[i * 4 + 2] = 255;
                    colors[i * 4 + 3] = 255;
                }
            }
            return colors;
        }

        private HttpResponseMessage BitmapToResponse(Bitmap bitmap, string format)
        {
            return BytesToResponse(EncodeImage(bitmap, format), format);
        }

        private byte[] EncodeImage(Bitmap bitmap, string format)
        {
            using (var memoryStream = new MemoryStream())
            {
                bitmap.Save(memoryStream, format == "png" ? ImageFormat.Png : ImageFormat.Jpeg);
                return memoryStream.ToArray();
            }
        }

        private HttpResponseMessage BytesToResponse(byte[] bytes, string format)
        {
            var result = new HttpResponseMessage(HttpStatusCode.OK);

            result.Content = new ByteArrayContent(bytes);
            result.Content.Headers.ContentType = format == "png" ? new MediaTypeHeaderValue("image/png") : new MediaTypeHeaderValue("image/jpeg");

            result.AddCache(Request);

            return result;
        }

        private static Bitmap BitmapFromRawBytes(byte[] bytes, int width, int height)
        {
            var bitmap = new Bitmap(width, height, PixelFormat.Format32bppArgb);
            var bitmapData = bitmap.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.WriteOnly,
                PixelFormat.Format32bppPArgb);
            Marshal.Copy(bytes, 0, bitmapData.Scan0, bytes.Length);
            bitmap.UnlockBits(bitmapData);
            
            return bitmap;
        }
    }
}
