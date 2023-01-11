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
using System.Web.Http;
using ApplicationUtility.Visualization;
using GeoWebCore.Models;
using GeoWebCore.Utilities;
using Pyxis.Contract.Publishing;
using Palette = Pyxis.Contract.Publishing.Palette;
using GeoWebCore.WebConfig;
using GeoWebCore.Services;
using GeoWebCore.Services.Cache;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// Return image generated from rhombus values. used for visualization of data
    /// </summary>
    [RoutePrefix("api/v1/Rhombus")]
    public class ImageController : ApiController
    {
        /// <summary>
        /// Generate jpeg image for rhombus and a given geosource 
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <returns>Jpeg image</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RGB")]
        [ApiCache]
        [TimeTrace("geoSource,key,size")]
        public HttpResponseMessage RGB(string key, int size, Guid geoSource)
        {
            return RGB(key, size, "jpeg", geoSource, null);
        }

        /// <summary>
        /// Generate jpeg image for rhombus and a given geosource 
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <param name="fromCache">if ture, request will only generate rhombus is all data in cache</param>
        /// <returns>Jpeg image</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RGB")]
        [ApiCache]
        [TimeTrace("geoSource,key,size,fromCache")]
        public HttpResponseMessage RGB(string key, int size, Guid geoSource, bool fromCache)
        {
            return RGB(key, size, "jpeg", geoSource, null, fromCache);
        }

        /// <summary>
        /// Generate an image wth the specificed format for rhombus and a geoSource
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="format">"jpeg" or "png"</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <returns>image</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RGB")]
        [ApiCache]
        [TimeTrace("geoSource,key,size,format")]
        public HttpResponseMessage RGB(string key, int size, string format, Guid geoSource)
        {
            return RGB(key, size, format, geoSource, null);
        }

        /// <summary>
        /// Generate an image wth the specificed format for rhombus and a geoSource
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="format">"jpeg" or "png"</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <param name="fromCache">if ture, request will only generate rhombus is all data in cache</param>
        /// <returns>image</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RGB")]
        [ApiCache]
        [TimeTrace("geoSource,key,size,format,fromCache")]
        public HttpResponseMessage RGB(string key, int size, string format, Guid geoSource, bool fromCache)
        {
            return RGB(key, size, format, geoSource, null, fromCache);
        }

        /// <summary>
        /// Generate an png image for rhombus and a geoSource styled by a given pallete
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <param name="palette">string specify the stlying of values. it encoded as number:css-color,number:css-color,.... example: "0:#000,1000:#fff"</param>
        /// <returns>image</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RGB")]
        [ApiCache]
        public HttpResponseMessage RGB(string key, int size, Guid geoSource, string palette)
        {
            return RGB(key, size, "png", geoSource, palette);
        }

        /// <summary>
        /// Generate an png image for rhombus and a geoSource styled by a given pallete
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <param name="palette">string specify the stlying of values. it encoded as number:css-color,number:css-color,.... example: "0:#000,1000:#fff"</param>
        /// <param name="fromCache">if ture, request will only generate rhombus is all data in cache</param>
        /// <returns>image</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RGB")]
        [ApiCache]
        public HttpResponseMessage RGB(string key, int size, Guid geoSource, string palette, bool fromCache)
        {
            return RGB(key, size, "png", geoSource, palette, fromCache);
        }

        /// <summary>
        /// Generate an png image for rhombus and a geoSource styled by a given pallete
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="format">"jpeg" or "png"</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <param name="palette">string specify the stlying of values. it encoded as number:css-color,number:css-color,.... example: "0:#000,1000:#fff"</param>
        /// <returns>image</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RGB")]
        [ApiCache]
        public HttpResponseMessage RGB(string key, int size, string format, Guid geoSource, string palette)
        {
            return RGB(key, size, format, geoSource, palette, false);
        }

        /// <summary>
        /// Generate an image for rhombus and a geoSource styled by a given pallete
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhombus size</param>
        /// <param name="format">"jpeg" or "png"</param>
        /// <param name="geoSource">GeoSource to use</param>
        /// <param name="palette">string specify the stlying of values. it encoded as number:css-color,number:css-color,.... example: "0:#000,1000:#fff"</param>
        /// <param name="fromCache">if ture, request will only generate rhombus is all data in cache</param>
        /// <returns>image</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RGB")]
        [ApiCache]
        public HttpResponseMessage RGB(string key, int size, string format, Guid geoSource, string palette,
            bool fromCache)
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

            if (fromCache && !RhombusHelper.WillLoadFast(key, size, coverage))
            {
                return new HttpResponseMessage()
                {
                    StatusCode = HttpStatusCode.PreconditionFailed
                };
            }

            var bytes = GetRhombusBGRA(key, size, coverage, palette);

            using (var bitmap = BitmapFromRawBytes(bytes, size, size))
            {
                return BitmapToResponse(bitmap, format);
            }
        }

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
        [AuthorizeGeoSource]
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

            var bytes = GetRhombusBGRA(key, size, coverage, null);

            using (var bitmap = BitmapFromRawBytes(bytes, size, size))
            {
                imageBytes = EncodeImage(bitmap, format);
                GeoSourceBlobCacheSingleton.WriteBlob(geoSource, key, size, safeFormat, imageBytes);

                return BytesToResponse(imageBytes, format);
            }
        }

        /// <summary>
        /// generate an image using a multi-channel request
        /// </summary>
        /// <param name="request">MultiChannelRhombusRequest request</param>
        /// <returns>image</returns>
        [HttpPost]
        [Route("Multi")]
        public HttpResponseMessage MultiChannel(MultiChannelRhombusRequest request)
        {
            Console.WriteLine(request.ToString());
            if (!RhombusHelper.IsValidSize(request.Size))
            {
                return null;
            }
            var size = request.Size;

            var pixels = new byte[size * size * 4];

            foreach (var channel in request.Channels)
            {
                var style = new Style
                {
                    Fill = new FieldStyle
                    {
                        Style = FieldStyleOptions.Palette,
                        PaletteExpression = channel.Field,
                        Palette = new Palette
                        {
                            Steps = (channel.StringValue == null
                                ? new List<Palette.Step>
                                {
                                    new Palette.Step {Color = Color.Black, Value = channel.Min},
                                    new Palette.Step {Color = Color.White, Value = channel.Max}
                                }
                                : new List<Palette.Step>
                                {
                                    new Palette.Step
                                    {
                                        Color = Color.Black,
                                        Value = channel.StringValue.Substring(0, channel.StringValue.Length - 2)
                                    },
                                    new Palette.Step {Color = Color.White, Value = channel.StringValue},
                                    new Palette.Step {Color = Color.Black, Value = channel.StringValue + " "}
                                })

                        }
                    }
                };

                var paletteString = String.Format("{0}:#000000,{1}:#ffffff", channel.Min, channel.Max);

                var process = GeoSourceInitializer.InitializeAsCoverage(channel.GeoSource, style);
                var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

                var bytes = channel.Field == null
                    ? GetRhombusBGRA(request.Key, size, coverage, paletteString)
                    : GetRhombusBGRA(request.Key, size, coverage, null);

                var offset = 0;

                switch (channel.Channel)
                {
                    case "R":
                        offset = 2;
                        break;
                    case "G":
                        offset = 1;
                        break;
                    case "B":
                        offset = 0;
                        break;
                    case "A":
                        offset = 3;
                        break;
                }

                for (var i = 0; i < bytes.Length; i += 4)
                {
                    pixels[i + offset] = bytes[i];
                }
            }

            using (var bitmap = BitmapFromRawBytes(pixels, size, size))
            {
                return BitmapToResponse(bitmap, "png");
            }
        }

        private byte[] GetRhombusBGRA(string key, int size, ICoverage_SPtr coverage, string palette)
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

                    var base64 = string.IsNullOrEmpty(palette)
                        ? PYXRhombusUtils.loadRhombusBGRA(rhombus, subRhombusResolution, coverage)
                        : PYXRhombusUtils.loadRhombusBGRAPalette(rhombus, subRhombusResolution, coverage,
                            ParsePalette(palette));

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

                var base64 = string.IsNullOrEmpty(palette)
                    ? PYXRhombusUtils.loadRhombusBGRA(rhombus, RhombusHelper.SizeToResolution(size), coverage)
                    : PYXRhombusUtils.loadRhombusBGRAPalette(rhombus, RhombusHelper.SizeToResolution(size), coverage,
                        ParsePalette(palette));

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

        private HttpResponseMessage BitmapToResponse(Bitmap bitmap, string format)
        {
            return BytesToResponse(EncodeImage(bitmap,format),format);
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
        

        internal class PaletteStep
        {
            public double Value { get; set; }

            public Color Color { get; set; }
        }

        private static Color Blend(Color color, Color backColor, double amount)
        {
            byte r = (byte) ((color.R*amount) + backColor.R*(1 - amount));
            byte g = (byte) ((color.G*amount) + backColor.G*(1 - amount));
            byte b = (byte) ((color.B*amount) + backColor.B*(1 - amount));
            return Color.FromArgb(r, g, b);
        }

        internal static Color GetColor(PaletteStep[] palette, double value)
        {
            for (int i = 0; i < palette.Length; i++)
            {
                if (palette[i].Value > value)
                {
                    if (i == 0) return palette.First().Color;

                    var distance = palette[i].Value - palette[i - 1].Value;
                    var ratio = (value - palette[i - 1].Value)/distance;

                    return Blend(palette[i].Color, palette[i - 1].Color, ratio);
                }
            }
            return palette.Last().Color;
        }

        private string ParsePalette(string paletteString)
        {
            var paletteParts =
                paletteString.Split(',')
                    .Select(x => x.Split(':'))
                    .Select(x => new PaletteStep() {Value = double.Parse(x[0]), Color = ColorTranslator.FromHtml(x[1])})
                    .ToArray();

            var palette = new NumericPalette();
            palette.Steps.AddRange(paletteParts.Select(x => new NumericPalette.Step() {Value = x.Value, Color = x.Color}));

            return palette.ToString();
        }
    }
}