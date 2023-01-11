using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Runtime.InteropServices;
using System.Web.Http;
using PyxisCLI.Server.Cache;
using PyxisCLI.Server.Models;
using PyxisCLI.Server.Services;
using PyxisCLI.Server.Utilities;
using PyxisCLI.Server.WebConfig;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// Return image generated from rhombus values. used for visualization of data
    /// </summary>
    [RoutePrefix("api/v1/Rhombus")]
    public class ImageController : ApiController
    {
        /// <summary>
        /// Generate an image for rhombus and a geoSource styled by a given style
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="style">style object or hashed code for stlye to use</param>
        /// <param name="format">"jpeg" or "png"</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <param name="fromCache">if ture, request will only generate rhombus is all data in cache</param>
        /// <returns>image</returns>
        [HttpGet]
        [Route("Style")]
        [ApiCache]
        [TimeTrace("geoSource,key,size,format,style,fromCache")]
        public HttpResponseMessage RGBWithStyle(string key, int size, Guid geoSource, string style, string format = "png", bool fromCache = false)
        {
            if (!RhombusHelper.IsValidSize(size))
            {
                return null;
            }

            if (format == "png" || format == "image/png")
            {
                format = "png";
            }
            else
            {
                format = "jpeg";
            }

            var parsedStyle = StyleCacheSingleton.Get(style);

            var safeFormat = UniqueHashGenerator.FromObject(parsedStyle) + "-" + format;

            var imageBytes = GeoSourceBlobCacheSingleton.GetBlob(geoSource, key, size, safeFormat);

            if (imageBytes != null)
            {
                return BytesToResponse(imageBytes, format);
            }

            var process = GeoSourceInitializer.InitializeAsCoverage(geoSource, parsedStyle);
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            if (fromCache && !RhombusHelper.WillLoadFast(key, size, coverage))
            {
                return new HttpResponseMessage()
                {
                    StatusCode = HttpStatusCode.PreconditionFailed
                };
            }

            var bytes = GetRhombusBGRA(key, size, coverage);

            using (var bitmap = BitmapFromRawBytes(bytes, size, size))
            {
                imageBytes = EncodeImage(bitmap, format);
                GeoSourceBlobCacheSingleton.WriteBlob(geoSource, key, size, safeFormat, imageBytes);

                return BytesToResponse(imageBytes, format);
            }
        }

       
        /// <summary>
        /// Return values of GeoSource on a given rhombus as texture using a given file format
        /// 
        /// The values of the GeoSource have to be numeric.
        /// 
        /// Currently supported formats are: "PYX0"       
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">size (in cell pixels) of the rhombus</param>
        /// <param name="geoSource">GeoSource to load</param>
        /// <param name="format">"PYX0"</param>
        /// <returns>PYX0 file format</returns>
        [HttpGet()]
        [Route("TextureValue")]
        [ApiCache()]
        [TimeTrace("key,size,geoSource,format")]
        public HttpResponseMessage TextureValue(string key, int size, Guid geoSource, string format)
        {
            if (!RhombusHelper.IsValidSize(size))
            {
                return null;
            }

            if (format.ToLower() != "pyx0")
            {
                throw new Exception("unsupported format");
            }

            var resultBytes = GeoSourceBlobCacheSingleton.GetBlob(geoSource, key, size, format);

            if (resultBytes == null)
            {
                byte[] bytes;
                int hasValuesBlockSize;

                LoadRhombusValues(key, size, geoSource, out bytes, out hasValuesBlockSize);
                var compressedRhombus = CompressedFloat32Rhombus.Create(size, hasValuesBlockSize, bytes);

                resultBytes = compressedRhombus.GzippedData;

                GeoSourceBlobCacheSingleton.WriteBlob(geoSource, key, size, format, resultBytes);
            }

            var result = new HttpResponseMessage(HttpStatusCode.OK);
            result.Content = new ByteArrayContent(resultBytes);
            result.Content.Headers.ContentType = new MediaTypeHeaderValue("application/pyx0");
            result.Content.Headers.ContentEncoding.Add("gzip");
            result.AddCache(Request);

            return result;
        }

        /// <summary>
        /// Generate an image for rhombus and a geoSource styled by a given pallete
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="format">"jpeg" or "png"</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <returns>image</returns>
        [HttpGet]
        [Route("RGB")]
        [ApiCache]
        public HttpResponseMessage RGB(string key, int size, string format, Guid geoSource)
        {

            if (!RhombusHelper.IsValidSize(size))
            {
                return null;
            }

            if (format == "png" || format == "image/png")
            {
                format = "png";
            }
            else
            {
                format = "jpeg";
            }

            var process = GeoSourceInitializer.InitializeAsCoverage(geoSource);
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            var bytes = GetRhombusBGRA(key, size, coverage);

            using (var bitmap = BitmapFromRawBytes(bytes, size, size))
            {
                var imageBytes = EncodeImage(bitmap, format);
                //GeoSourceBlobCacheSingleton.WriteBlob(geoSource, key, size, "rgb", imageBytes);
                return BytesToResponse(imageBytes, format);
            }
        }

        private static void LoadRhombusValues(string key, int size, Guid geoSource, out byte[] bytes, out int hasValuesBlockSize)
        {
            var process = GeoSourceInitializer.Initialize(geoSource);
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());
            if (coverage.isNull())
            {
                throw new Exception("failed to get coverage");
            }

            hasValuesBlockSize = size * size * sizeof(byte);

            //TODO: We handle root rombuses [0..9] in special case because lack of support in PYXRhombus
            if (RhombusHelper.IsTopRhombus(key))
            {
                bytes = new byte[size * size * sizeof(float) + hasValuesBlockSize];

                var subRhombusSize = RhombusHelper.GetLowerSize(size);
                var subRhombusResolution = RhombusHelper.SizeToResolution(size) - 2;

                var rhombusIndex = int.Parse(key);

                for (var childRhombus = 0; childRhombus < 9; childRhombus++)
                {
                    var rhombus = new PYXRhombus(rhombusIndex * 9 + childRhombus);
                    var parentOffset = RhombusHelper.ChildRhombusOffset(childRhombus, size);

                    var subRhombusData =
                        Convert.FromBase64String(PYXRhombusUtils.loadRhombusFloat(rhombus, subRhombusResolution,
                            coverage));

                    for (var y = 0; y < subRhombusSize; y++)
                    {
                        Array.Copy(
                            //source
                            subRhombusData, //array
                            y * subRhombusSize, //offset
                            //destination
                            bytes, //array
                            RhombusHelper.RhombusUvToOffset(size, parentOffset, y), //offset
                            subRhombusSize); //size

                        Array.Copy(
                            //source
                            subRhombusData, //array
                            subRhombusSize * subRhombusSize + y * subRhombusSize * sizeof(float), //offset
                            //destination
                            bytes, //array
                            hasValuesBlockSize + RhombusHelper.RhombusUvToOffset(size, parentOffset, y) * sizeof(float), //offset
                            subRhombusSize * sizeof(float)); //size
                    }
                }
            }
            else
            {
                var rhombus = RhombusHelper.GetRhombusFromKey(key);

                bytes = Convert.FromBase64String(
                    PYXRhombusUtils.loadRhombusFloat(rhombus, RhombusHelper.SizeToResolution(size), coverage));
            }
        }

        private byte[] GetRhombusBGRA(string key, int size, ICoverage_SPtr coverage)
        {
            byte[] bytes;
            //TODO: We handle root rombuses [0..9] in special case bacause lack of support in PYXRhombus
            if (RhombusHelper.IsTopRhombus(key))
            {
                var subRhombusSize = RhombusHelper.GetLowerSize(size);
                var subRhombusResolution = RhombusHelper.SizeToResolution(size) - 2;

                var rhombusIndex = int.Parse(key);

                bytes = new byte[size * size * 4];

                for (var childRhombus = 0; childRhombus < 9; childRhombus++)
                {
                    var rhombus = new PYXRhombus(rhombusIndex * 9 + childRhombus);

                    var parentOffset = RhombusHelper.ChildRhombusOffset(childRhombus, size);

                    var base64 = PYXRhombusUtils.loadRhombusBGRA(rhombus, subRhombusResolution, coverage);

                    var subRhombusData = Convert.FromBase64String(base64);

                    for (var y = 0; y < subRhombusSize; y++)
                    {
                        Array.Copy(subRhombusData, y * subRhombusSize * 4, bytes,
                            RhombusHelper.RhombusUvToOffset(size, parentOffset, y) * 4, subRhombusSize * 4);
                    }
                }
            }
            else
            {
                var rhombus = RhombusHelper.GetRhombusFromKey(key);

                var base64 = PYXRhombusUtils.loadRhombusBGRA(rhombus, RhombusHelper.SizeToResolution(size), coverage);

                bytes = Convert.FromBase64String(base64);
            }
            return bytes;
        }


        private static Bitmap BitmapFromRawBytes(byte[] bytes, int width, int height)
        {
            var bitmap = new Bitmap(width, height, PixelFormat.Format32bppPArgb);
            var bitmapData = bitmap.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.WriteOnly,
                PixelFormat.Format32bppPArgb);
            Marshal.Copy(bytes, 0, bitmapData.Scan0, bytes.Length);
            bitmap.UnlockBits(bitmapData);

            return bitmap;
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
    }
}