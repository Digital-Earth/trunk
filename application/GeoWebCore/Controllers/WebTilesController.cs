using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;
using System.Web;
using System.Web.Http;
using ApplicationUtility;
using GeoWebCore.Services;
using GeoWebCore.WebConfig;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// Generate web-tiles for a given geosource
    /// </summary>
    [RoutePrefix("api/v1/Tiles")]
    public class WebTilesController : ApiController
    {
        private static readonly object s_coordConverterLock = new object();
        private static ICoordConverter_SPtr s_coordConverter;

        private static readonly double MAX_X = 20037508.3428;
        //private static readonly double MAX_Y = 19971868.8804;
        private static readonly double MAX_Y = 20037508.3428;

        private ICoordConverter_SPtr CoordConverter
        {
            get
            {
                lock (s_coordConverterLock)
                {
                    if (s_coordConverter != null)
                    {
                        return s_coordConverter;
                    }
                    return s_coordConverter = PYXCOMFactory.CreateCoordConvertorFromWKT("epsg:3785");
                }
            }
        }


        [HttpGet]
        [Route("{geoSource}/{z}/{x}/{y}.png")]
        [TimeTrace("geoSource,z,x,y")]
        public HttpResponseMessage Tile(string geoSource, int z, int x, int y, int cellWidth = 20)
        {
            var bursh = new SolidBrush(Color.FromArgb(128,Color.Green));
            var guid = Guid.Parse(geoSource);

            var process = GeoSourceInitializer.InitializeAsCoverage(guid);
            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            var bitmap = new Bitmap(255, 255, PixelFormat.Format32bppPArgb);

            var topLeft = GetNative(z, x, y, 0.1f, 0.1f);
            var bottomRight = GetNative(z, x, y, 255.9f, 255.9f);

            for (var r = 2; r < 8; r++)
            {
                Console.WriteLine("{0} --> {1}", r, SnyderProjection.getInstance().resolutionToPrecision(r));
            }

            var resolutionsWeight = new Dictionary<int,double[]>();
            const int samples = 16;

            for (var dy = 0; dy < samples; dy++)
            {
                var py = 255.0f * dy / samples + 0.5f;
                var p1 = GetLatLon(z, x, y, 128, py);
                var p2 = GetLatLon(z, x, y, 128 + 1, py);

                var d = SphereMath.distanceBetween(p1, p2) * cellWidth;

                var res = PYXBoundingCircle.estimateResolutionFromRadius(d);

                var minRadius = SnyderProjection.getInstance().resolutionToPrecision(res);
                var maxRadius = SnyderProjection.getInstance().resolutionToPrecision(res-1);

                if (res > 2)
                {
                    var resWeight = 1 - GetBlend(d, minRadius, maxRadius);

                    if (resWeight > 0)
                    {
                        if (!resolutionsWeight.ContainsKey(res))
                        {
                            resolutionsWeight[res] = new double[samples];
                        }
                        resolutionsWeight[res][dy] = resWeight;
                    }

                    if (resWeight < 1)
                    {
                        if (!resolutionsWeight.ContainsKey(res - 1))
                        {
                            resolutionsWeight[res - 1] = new double[samples];
                        }
                        resolutionsWeight[res - 1][dy] = 1 - resWeight;
                    }
                }
                else
                {
                    if (!resolutionsWeight.ContainsKey(res))
                    {
                        resolutionsWeight[res] = new double[samples];
                    }
                    resolutionsWeight[res][dy] = 1;
                }
            }


            var center = GetLatLon(z, x, y, 128, 128);
            var leftToCenter = GetLatLon(z, x, y, 128 + cellWidth, 128);

            var distance = SphereMath.distanceBetween(center, leftToCenter);
            var resolution = PYXBoundingCircle.estimateResolutionFromRadius(distance);
            resolution = Math.Max(2, resolution);

            var radius = SnyderProjection.getInstance().resolutionToPrecision(resolution);
            var radius2 = SnyderProjection.getInstance().resolutionToPrecision(resolution - 1);

            Console.WriteLine("{0} < {1} < {2}",radius,distance,radius2);

            var nativeBBox = new PYXRect2DDouble(topLeft.x(), bottomRight.y(), bottomRight.x(), topLeft.y());

            Console.WriteLine("{0} {1} {2}",nativeBBox.width(),nativeBBox.height(), resolution);

            foreach(var keyValue in resolutionsWeight)
            {
                var res = keyValue.Key;
                var weights = keyValue.Value;
                RenderResolution(z, x, y, nativeBBox, res, bitmap, coverage, bursh, weights);
            }

            

            return BitmapToResponse(bitmap,"png");
        }

        private void RenderResolution(int z, int x, int y, PYXRect2DDouble nativeBBox, int resolution, Bitmap bitmap,
            ICoverage_SPtr coverage, SolidBrush brush, double[] weights)
        {
            var bbox =
                new PYXXYBoundsGeometry(nativeBBox, CoordConverter.get(), resolution);

            var iterator = bbox.getIterator();

            using (var graphics = Graphics.FromImage(bitmap))
            {
                graphics.SmoothingMode = SmoothingMode.AntiAlias;
                //graphics.DrawRectangle(Pens.Gray,new Rectangle(0,0,255,255));
                //graphics.DrawString(String.Format("{0}/{1}/{2}",z,y,x),new Font(SystemFonts.DefaultFont.FontFamily, 12.0f),Brushes.Black,128,128);

                var cellCenter = new PYXCoord2DDouble();
                var native = new PYXCoord2DDouble();

                var vertices = new PYXVertexIterator(iterator.getIndex());

                while (!iterator.end())
                {
                    var index = new PYXIcosIndex(iterator.getIndex());

                    CoordConverter.pyxisToNative(index, cellCenter);

                    var cellCenterPoint = GetPointFromNative(cellCenter, z, x, y, cellCenter);

                    var samples = weights.Length;
                    var sampleIndex = (int)Math.Floor(samples*cellCenterPoint.Y/256.0);
                    sampleIndex = Math.Max(0,Math.Min(sampleIndex, samples - 1));

                    if (weights[sampleIndex] == 0)
                    {
                        iterator.next();
                        continue;
                    }

                    var cellPen = new Pen(Color.FromArgb((int)(255 * weights[sampleIndex]), Color.Black));

                    vertices.reset(index);
                    var points = new List<PointF>();
                    while (!vertices.end())
                    {
                        CoordConverter.pyxisToNative(vertices.getIndex(), native);
                        points.Add(GetPointFromNative(native, z, x, y, cellCenter));
                        vertices.next();
                    }
                    points.Add(points[0]);

                    var path = new GraphicsPath();
                    path.AddLines(points.ToArray());


                    var value = coverage.getCoverageValue(index);

                    if (!value.isNull())
                    {
                        graphics.FillPath(brush, path);
                    }

                    graphics.DrawPath(cellPen, path);


                    iterator.next();
                }
            }
        }

        private double GetBlend(double size, double min, double max)
        {
            var frac = (size - min)/(max - min);
            if (frac < 0.25)
            {
                return 0;
            }
            if (frac > 0.75)
            {
                return 1;
            }

            var x = 2*frac - 0.5;
            return x*x*(3 - 2*x);
        }

        private PYXCoord2DDouble GetNative(int z, int x, int y, float px, float py)
        {
            var scale = Math.Pow(2, z);

            var dx = ((x + px / 256.0) / scale * 2 - 1) * MAX_X;
            var dy = (1 - (y + py / 256.0) / scale * 2) * MAX_Y;

            return new PYXCoord2DDouble(dx, dy);
        }

        private CoordLatLon GetLatLon(int z, int x, int y, float px, float py)
        {
          
            var latlon = new CoordLatLon();
            CoordConverter.nativeToLatLon(GetNative(z,x,y,px,py), latlon);
            return latlon;
        }

        private PointF GetPointFromLatLon(CoordLatLon latlon, int z, int x, int y, PYXCoord2DDouble reference)
        {
            var native = new PYXCoord2DDouble();

            CoordConverter.latLonToNative(latlon, native);

            return GetPointFromNative(native, z, x, y, reference);
        }

        private PointF GetPointFromNative(PYXCoord2DDouble native, int z, int x, int y, PYXCoord2DDouble reference)
        {
            var dx = native.x() / MAX_X;
            var dy = native.y() / MAX_Y;

            var referenceX = reference.x()/MAX_X;

            var scale = Math.Pow(2, z);

            var xratio = x/scale;

            if (Math.Abs(referenceX - dx) > 1)
            {
                if (referenceX > 0)
                {
                    dx += 2;
                }
                if (referenceX < 0)
                {
                    dx -= 2;
                }
            }


            dx = (dx + 1)/2 * scale;
            dy = (1 - dy)/2 * scale;

            return new PointF((float)(dx - x) * 256, (float)(dy - y) * 256);
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
        
    }
}
