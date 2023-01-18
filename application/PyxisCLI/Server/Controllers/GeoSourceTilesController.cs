using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Web.Http;
using ApplicationUtility;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson.Specialized;
using PyxisCLI.Server.Services;
using PyxisCLI.Server.Utilities;
using PyxisCLI.Server.WebConfig;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// The GeoSourceController provides client access to data from the GeoSource.
    /// </summary>
    [RoutePrefix("api/v1/GeoSource")]
    public class GeoSourceTilesController : ApiController
    {

        [Route("{geoSourceId}/Tile")]
        [HttpGet]
        [TimeTrace("geoSourceId,root,depth,field")]
        public async Task<HttpResponseMessage> GetPyxisTile2(Guid geoSourceId, string root, int depth, string field = "")
        {
            var index = new PYXIcosIndex(root);
            var tile = PYXTile.create(index, index.getResolution() + depth);

            var state = GeoSourceInitializer.GetGeoSourceState(geoSourceId);
            var process = await state.GetProcess();
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            if (coverage.isNull())
            {
                throw new Exception("Get PYXValueTile only supported for coverage at the moment");   
            }

            var valueTile = coverage.getCoverageTile(tile.get());

            var base64 = valueTile.toBase64String();
            var bytes = Convert.FromBase64String(base64);

            return BytesToResponse(bytes, "application/pyxtile");
        }
        

        [Route("{geoSourceId}/Cell")]
        [HttpGet]
        [TimeTrace("geoSourceId,cell,field")]
        public async Task<object> GetPyxisCell2(Guid geoSourceId, string cell, string field = "")
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSourceId);
            var process = await state.GetProcess();
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            if (coverage.isNull())
            {
                throw new Exception("Get PYXValueTile only supported for coverage at the moment");
            }

            var index = 0;
            if (field.HasContent())
            {
                index = coverage.getCoverageDefinition().getFieldIndex(field);
            }

            var value = coverage.getCoverageValue(new PYXIcosIndex(cell),index);

            return value.ToDotNetObject();
        }

        /// <summary>
        /// forward a GET HTTP request to GeoSource server
        /// </summary>
        /// <param name="geoSourceId">GeoSource Id</param>
        /// <param name="pathInfo">Sub path that should be passed to the target proxy</param>
        /// <returns>HttpResponse</returns>

        [Route("{geoSourceId}/Tile/Where")]
        [HttpGet]
        [TimeTrace("geoSourceId,root,depth,field")]
        public async Task<HttpResponseMessage> GetPyxisTile(Guid geoSourceId, string root, int depth, string field = "",string min = "",string max = "",string format = "raw")
        {
            var index = new PYXIcosIndex(root);
            var tile = PYXTile.create(index, index.getResolution() + depth);

            var state = GeoSourceInitializer.GetGeoSourceState(geoSourceId);
            var process = await state.GetProcess();

            var condition = CreateWhereCondition(process, field, min, max);
            var tileCollection = condition(tile);

            if (format == "json")
            {
                return Request.CreateResponseWithCache(new TileCollectionGeometry(tileCollection));    
            }

            var base64 = PYXGeometrySerializer.serializeToBase64(pyxlib.DynamicPointerCast_PYXGeometry(tileCollection).get());
            var bytes = Convert.FromBase64String(base64);
            return BytesToResponse(bytes, "application/pyxtilecollection");
        }

        private Func<PYXTile_SPtr,PYXTileCollection_SPtr> CreateWhereCondition(IProcess_SPtr process, string field, string min, string max)
        {
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
            var features = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

            //if this is a coverage
            if (coverage.isNotNull())
            {
                var condition = PYXWhereCondition.coverageHasValues(coverage);
                if (!String.IsNullOrEmpty(min) && !String.IsNullOrEmpty(max))
                {
                    var minNumeric = Double.Parse(min);
                    var maxNumeric = Double.Parse(max);

                    condition = condition.range(new PYXValue(minNumeric), new PYXValue(maxNumeric));
                }

                return tile => condition.match(tile.get());
            }
            else if (features.isNotNull())
            {
                var condition = PYXWhereCondition.featuresHasValues(features);


                if (!String.IsNullOrEmpty(field))
                {
                    var index = features.getFeatureDefinition().getFieldIndex(field);
                    if (index == -1)
                    {
                        throw new Exception("Can't find field with name: " + field);
                    }
                    condition = condition.field(field);

                    var numeric = features.getFeatureDefinition().getFieldDefinition(index).isNumeric();

                    if (!String.IsNullOrEmpty(min) && !String.IsNullOrEmpty(max))
                    {
                        if (numeric)
                        {
                            //use numeric values
                            var minNumeric = Double.Parse(min);
                            var maxNumeric = Double.Parse(max);
                            condition = condition.range(new PYXValue(minNumeric), new PYXValue(maxNumeric));
                        }
                        else
                        {
                            //use string values
                            condition = condition.range(new PYXValue(min), new PYXValue(max));
                        }
                    }
                }

                return tile => condition.match(tile.get());
            }

            throw new Exception("unsupported GeoSource format");
        }

        [Route("{geoSourceId}/Rhombus/Where")]
        [HttpGet]
        [TimeTrace("geoSourceId,key,depth,field")]
        public async Task<HttpResponseMessage> RasterRhombus2(Guid geoSourceId, string key, int size, string field = "", string min = "", string max = "")
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSourceId);
            var process = await state.GetProcess();

            var condition = CreateWhereCondition(process, field, min, max);

            byte[] bytes = null;

            if (RhombusHelper.IsTopRhombus(key))
            {
                var subRhombusSize = RhombusHelper.GetLowerSize(size);
                var subRhombusResolution = RhombusHelper.SizeToResolution(size) - 2;

                var rhombusIndex = int.Parse(key);
                bytes = new byte[size * size];

                for (var childRhombus = 0; childRhombus < 9; childRhombus++)
                {
                    var parentOffset = RhombusHelper.ChildRhombusOffset(childRhombus, size);
                    var childBytes = GetRhombusBytes(key + childRhombus, subRhombusSize, condition);

                    for (var y = 0; y < subRhombusSize; y++)
                    {
                        Array.Copy(childBytes, y * subRhombusSize, bytes, RhombusHelper.RhombusUvToOffset(size, parentOffset, y), subRhombusSize);
                    }
                }
            }
            else
            {
                bytes = GetRhombusBytes(key, size, condition);    
            }
            
            var rgba = ConvertByteToColor(bytes);

            using (var bitmap = BitmapFromRawBytes(rgba, size, size))
            {
                return BitmapToResponse(bitmap, "png");
            }
        }

        private byte[] GetRhombusBytes(string key, int size, Func<PYXTile_SPtr, PYXTileCollection_SPtr> condition)
        {
            var rhombus = RhombusHelper.GetRhombusFromKey(key);
            using (var rasterizer = new PYXRhombusRasterizer(rhombus, RhombusHelper.SizeToResolution(size)))
            {

                while (!rasterizer.isReady())
                {
                    var tile = rasterizer.getNeededTile();
                    rasterizer.setTileGeometry(tile, pyxlib.DynamicPointerCast_PYXGeometry(condition(tile)));
                }

                var base64 = rasterizer.rasterToBase64();
                var bytes = Convert.FromBase64String(base64);
                return bytes;
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
            switch (format)
            {
                case "png":
                    result.Content.Headers.ContentType = new MediaTypeHeaderValue("image/png");
                    break;
                case "jpeg":
                    result.Content.Headers.ContentType = new MediaTypeHeaderValue("image/jpeg");
                    break;
                default:
                    result.Content.Headers.ContentType = new MediaTypeHeaderValue(format);
                    break;
            }
            
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
