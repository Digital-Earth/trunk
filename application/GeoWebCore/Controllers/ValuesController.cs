using GeoWebCore.Models;
using GeoWebCore.Services;
using GeoWebCore.Utilities;
using GeoWebCore.WebConfig;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Runtime.InteropServices;
using System.Web.Http;
using GeoWebCore.Services.Cache;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// Return values associated with a Rhombus or cell for a given GeoSource &amp; field
    /// </summary>
    [RoutePrefix("api/v1/Rhombus")]
    public class ValuesController : ApiController
    {        
        private static string IncrementKey(string key)
        {
            var endindex = key.Length - 1;
            if (endindex > 0 && key[endindex] == '8')
            {
                return IncrementKey(key.Substring(0, endindex)) + "0";
            }
            return key.Substring(0, endindex) + new String((char) (key[endindex] + 1), 1);
        }

        /// <summary>
        /// Return list of rhombus keys that the given geoSource intersects / or has value
        /// </summary>
        /// <param name="samples">Number of samples to perform (same as rhombus size)</param>
        /// <param name="geoSource">GeoSource to load</param>
        /// <param name="field">Field to use to retrieve values on</param>
        /// <param name="depth">Depth of the search to perform given as rhombus resolution</param>
        /// <returns>Json string of list of valid rhombus keys</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RhombusesWithValue")]
        [Obsolete]
        [TimeTrace("geoSource,samples,field,depth")]
        public HttpResponseMessage RhombusesWithValue(int samples, Guid geoSource, string field, int depth)
        {
            return RhombusesWithValue(samples, geoSource, field, depth, 0, "", "none");
        }

        /// <summary>
        /// Return list of rhombus keys that the given geoSource intersects / or has value
        /// </summary>
        /// <param name="samples">Number of samples to perform (same as rhombus size)</param>
        /// <param name="geoSource">GeoSource to load</param>
        /// <param name="field">Field to use to compare the given value against</param>
        /// <param name="depth">Depth of the search to perform given as rhombus resolution</param>
        /// <param name="value">Numeric value to filter by</param>
        /// <returns>Json string of list of valid rhombus keys</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RhombusesWithValue")]
        [Obsolete]
        [TimeTrace("geoSource,samples,field,depth,value")]
        public HttpResponseMessage RhombusesWithValue(int samples, Guid geoSource, string field, int depth, float value)
        {
            return RhombusesWithValue(samples, geoSource, field, depth, value, "", "float");
        }

        /// <summary>
        /// Return list of rhombus keys that the given geoSource intersects / or has value
        /// </summary>
        /// <param name="samples">Number of samples to perform (same as rhombus size)</param>
        /// <param name="geoSource">GeoSource to load</param>
        /// <param name="field">Field to use to compare the given value against</param>
        /// <param name="depth">Depth of the search to perform given as rhombus resolution</param>
        /// <param name="svalue">String value to filter by</param>
        /// <returns>Json string of list of valid rhombus keys</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RhombusesWithValue")]
        [Obsolete]
        [TimeTrace("geoSource,samples,field,depth,value,svalue")]
        public HttpResponseMessage RhombusesWithValue(int samples, Guid geoSource, string field, int depth,
            string svalue)
        {
            return RhombusesWithValue(samples, geoSource, field, depth, 0, svalue, "string");
        }

        /// <summary>
        /// Return list of rhombus keys that the given geoSource intersects / or has value
        /// </summary>
        /// <param name="samples">Number of samples to perform (same as rhombus size)</param>
        /// <param name="geoSource">GeoSource to load</param>
        /// <param name="field">Field to use to compare the given value against</param>
        /// <param name="depth">Depth of the search to perform given as rhombus resolution</param>
        /// <param name="value">Numeric value to filber by</param>
        /// <param name="svalue">String value to filter by</param>
        /// <param name="valueType">Which field type to use: "string"/"float"</param>
        /// <returns>Json string of list of valid rhombus keys</returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [Route("RhombusesWithValue")]
        [Obsolete]
        [TimeTrace("geoSource,samples,field,depth,value,svalue,valueType")]
        public HttpResponseMessage RhombusesWithValue(int samples, Guid geoSource, string field, int depth, float value,
            string svalue, string valueType)
        {
            if (!RhombusHelper.IsValidSize(samples) || field == null)
            {
                return null;
            }
            List<string> rhombusList = new List<string>();
            
            var style = new Style
            {
                Fill = new FieldStyle
                {
                    Style = FieldStyleOptions.Palette,
                    PaletteExpression = field,
                    Palette = new Palette
                    {
                        Steps = (valueType == "float"
                            ? new List<Palette.Step>
                            {
                                new Palette.Step {Color = Color.Black, Value = value*0.999f},
                                new Palette.Step {Color = Color.White, Value = value},
                                new Palette.Step {Color = Color.Black, Value = value*1.001f}
                            }
                            : (valueType == "string"
                                ? new List<Palette.Step>
                                {
                                    new Palette.Step
                                    {
                                        Color = Color.Black,
                                        Value = svalue.Substring(0, svalue.Length - 2)
                                    },
                                    new Palette.Step {Color = Color.White, Value = svalue},
                                    new Palette.Step {Color = Color.Black, Value = svalue + " "}
                                }
                                : new List<Palette.Step>
                                {
                                    new Palette.Step {Color = Color.Black, Value = 0},
                                    new Palette.Step {Color = Color.White, Value = 1}
                                }))
                    }
                }
            };

            var process = GeoSourceInitializer.InitializeAsCoverage(geoSource, style);
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            string ri = new String('0', depth + 2);
            for (;; ri = IncrementKey(ri))
            {
                if (ri[0] > '9')
                {
                    break;
                }

                var rhombus = RhombusHelper.GetRhombusFromKey(ri);
                var base64 = PYXRhombusUtils.loadRhombusBGRA(rhombus, RhombusHelper.SizeToResolution(samples), coverage);

                var bytes = Convert.FromBase64String(base64);
                for (var i = 0; i < bytes.Length; i += 4)
                {
                    if (bytes[i] > 0)
                    {
                        rhombusList.Add(ri);
                        break;
                    }

                }
            }
            var result = new HttpResponseMessage(HttpStatusCode.OK)
            {
                Content = new StringContent(JsonConvert.SerializeObject(rhombusList.ToArray()))
            };
            return result;
        }

        /// <summary>
        /// Get values from a GeoSource on a given rhombus.
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Rhoumbs size</param>
        /// <param name="geoSource">GeoSource to load values from</param>
        /// <returns>RhombusValues object</returns>
        [HttpGet()]
        [AuthorizeGeoSource]
        [Route("Value")]
        [TimeTrace("key,size,geoSource")]
        public RhombusValues Value(string key, int size, Guid geoSource)
        {
            if (!RhombusHelper.IsValidSize(size))
            {
                return null;
            }

            var process = GeoSourceInitializer.Initialize(geoSource);
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            var result = new RhombusValues
            {
                Values = new object[size * size]
            };

            //TODO: We handle root rombuses [0..9] in special case bacause lack of support in PYXRhombus
            if (RhombusHelper.IsTopRhombus(key))
            {
                var subRhombusSize = RhombusHelper.GetLowerSize(size);
                var subRhombusResolution = RhombusHelper.SizeToResolution(size) - 2;

                var rhombusIndex = int.Parse(key);

                for (var childRhombus = 0; childRhombus < 9; childRhombus++)
                {
                    var rhombus = new PYXRhombus(rhombusIndex * 9 + childRhombus);
                    var cursor = new PYXRhombusCursor(rhombus, subRhombusResolution);

                    var parentOffset = RhombusHelper.ChildRhombusOffset(childRhombus, size);

                    var pi = 0;
                    while (!cursor.end())
                    {
                        int u, v;

                        RhombusHelper.RhombusOffsetToUv(pi, subRhombusSize, out u, out v);
                        var index = RhombusHelper.RhombusUvToOffset(size, u, v) + parentOffset;

                        var value = coverage.getCoverageValue(cursor.getIndex());

                        result.Values[index] = value.ToDotNetObject();

                        pi++;
                        cursor.next();
                    }
                }
            }
            else
            {
                var rhombus = RhombusHelper.GetRhombusFromKey(key);
                var cursor = new PYXRhombusCursor(rhombus, RhombusHelper.SizeToResolution(size));
                var idx = 0;

                while (!cursor.end())
                {
                    var value = coverage.getCoverageValue(cursor.getIndex());

                    result.Values[idx] = value.ToDotNetObject();

                    idx++;
                    cursor.next();
                }
            }

            return result;
        }

        /// <summary>
        /// Return values of GeoSource on a given rhombus as a RGBA texture.
        /// 
        /// The values of the GeoSource have to be numeric. The function convert the numeric values to 0..1 range.
        /// The conversion to 0..1 range is done using Min..Max values:
        /// if value &lt; min -> 0
        /// if value &gt; max -> 1
        /// else return (value - min)/(max-min)
        /// 
        /// The alpha channel of the image is used to indicate if there is value for each given cell/pixel
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">size (in cell pixels) of the rhombus</param>
        /// <param name="geoSource">GeoSource to load</param>
        /// <param name="min">The minimum value to consider</param>
        /// <param name="max">The maximum value to consider</param>
        /// <returns>png image</returns>
        [HttpGet()]
        [AuthorizeGeoSource]
        [Route("TextureValue")]
        [ApiCache()]
        [TimeTrace("key,size,geoSource,min,max")]
        public HttpResponseMessage RGB(string key, int size, Guid geoSource, double min, double max)
        {
            return RGB(key, size, /*16-bits by default*/16, geoSource, min, max);
        }

        /// <summary>
        /// Return values of GeoSource on a given rhombus as a RGBA texture.
        /// 
        /// The values of the GeoSource have to be numeric. The function convert the numeric values to 0..1 range.
        /// The conversion to 0..1 range is done using Min..Max values:
        /// if value &lt; min -> 0
        /// if value &gt; max -> 1
        /// else return (value - min)/(max-min)
        /// 
        /// The alpha channel of the image is used to indicate if there is value for each given cell/pixel
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">size (in cell pixels) of the rhombus</param>
        /// <param name="bits">Number of bits to use for compressing the floats value: 16/24</param>
        /// <param name="geoSource">GeoSource to load</param>
        /// <param name="min">The minimum value to consider</param>
        /// <param name="max">The maximum value to consider</param>
        /// <returns>png image</returns>
        [HttpGet()]
        [AuthorizeGeoSource]
        [Route("TextureValue")]
        [ApiCache()]
        [TimeTrace("key,size,geoSource,bits,min,max")]
        public HttpResponseMessage RGB(string key, int size, int bits, Guid geoSource, double min, double max)
        {
            if (!RhombusHelper.IsValidSize(size))
            {
                return null;
            }

            if (max <= min)
            {
                max = min + 1;
            }

            byte[] bytes;
            int hasValuesBlockSize;

            LoadRhombusValues(key, size, geoSource, out bytes, out hasValuesBlockSize);

            var result = new HttpResponseMessage(HttpStatusCode.OK);

            INumberToColorConverter converter;

            //For migration to the new version of WebGlobe - use 16 bits by default, because old version supports only 16 bits
            switch (bits)
            {
                case 24:
                    converter = new Number24BitToColorConverter(min, max);
                    break;
                // ReSharper disable once RedundantCaseLabel
                case 16:
                default:
                    converter = new Number16BitToColorConverter(min, max);
                    break;
            }


            const int bytesPerPixel = 4;

            var bitmapBytes = new byte[size * size * bytesPerPixel];
            var rgba = new byte[bytesPerPixel];

            for (var y = 0; y < size; y++)
            {
                for (var x = 0; x < size; x++)
                {
                    var index = y * size + x;

                    if (bytes[index] == 0)
                    {
                        continue;
                    }

                    var value = BitConverter.ToSingle(bytes, hasValuesBlockSize + index * sizeof(float));
                    converter.ToRgba(value, rgba);

                    //store rgba in Microsoft order
                    bitmapBytes[bytesPerPixel * index + 0] = rgba[2]; //B
                    bitmapBytes[bytesPerPixel * index + 1] = rgba[1]; //G
                    bitmapBytes[bytesPerPixel * index + 2] = rgba[0]; //R
                    bitmapBytes[bytesPerPixel * index + 3] = rgba[3]; //A
                }
            }

            using (var bitmap = new Bitmap(size, size, PixelFormat.Format32bppArgb))
            {
                var bitmapData = bitmap.LockBits(new Rectangle(0, 0, size, size), ImageLockMode.WriteOnly,
                    PixelFormat.Format32bppPArgb);
                Marshal.Copy(bitmapBytes, 0, bitmapData.Scan0, bitmapBytes.Length);
                bitmap.UnlockBits(bitmapData);

                using (var memoryStream = new MemoryStream())
                {
                    bitmap.Save(memoryStream, ImageFormat.Png);
                    result.Content = new ByteArrayContent(memoryStream.ToArray());
                    result.Content.Headers.ContentType = new MediaTypeHeaderValue("image/png");
                    result.AddCache(Request);
                }
            }

            return result;
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
        [AuthorizeGeoSource]
        [Route("TextureValue")]
        [ApiCache()]
        [TimeTrace("key,size,geoSource,format")]
        public HttpResponseMessage RGB(string key, int size, Guid geoSource, string format)
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
    }
}