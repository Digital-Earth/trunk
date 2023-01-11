using ApplicationUtility;
using GeoWebCore.Models;
using GeoWebCore.Services;
using GeoWebCore.WebConfig;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using Pyxis.Core.Measurements;
using Pyxis.UI.Layers.Globe;
using System;
using System.Net;
using System.Reflection;
using System.Web.Http;
using GeoWebCore.Services.Cache;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// Local controller is a collection of functions that were implemneted by Downloadable studio and needed to be modified to GeoWebCore logic.
    /// </summary>
    [Obsolete("This controller is a collection of functions that needed to be refactored")]
    [RoutePrefix("api/v1/Local")]
    public class LocalController : ApiController
    {
        private static string s_watershedCalculationProcess = "{4D367363-2421-4DF2-AD97-529A4EDDE0A8}";

        /// <summary>
        /// Get GeoWebCore version
        /// </summary>
        /// <returns>string represent the version of GeoWebCore: eg: 1.2.0.0</returns>
        [HttpGet]
        [Route("Version")]      
        [ApiCache(Profile=ApiCacheAttribute.Profiles.Short)]
        [TimeTrace()]
        public string Version()
        {
            return Assembly.GetExecutingAssembly().GetName().Version.ToString();
        }

        /// <summary>
        /// Get GeoWebCore health state (can help to verify that services are performing correctly)
        /// </summary>
        /// <returns>string represent the version of GeoWebCore: eg: 1.2.0.0</returns>
        [HttpGet]
        [Route("Health")]
        [TimeTrace()]
        public string Health()
        {
            HttpStatusCode status;
            // TODO: here actual service state verifications should be performed.
            // In the current implementation (as of changeset version 1415) GWC is stateless
            // and can be considered healthy as long as it responds.
            // See https://msdn.microsoft.com/en-us/library/system.net.httpstatuscode(v=vs.110).aspx for HTTP error codes.
            status = HttpStatusCode.OK;
            return status.ToString();
        }

        /// <summary>
        /// Get a specification of a GeoSource using a GeoSource model.
        /// </summary>
        /// <param name="geoSource">GoeSource model to generate a specification on</param>
        /// <returns>PipelineSpecification object</returns>
        [HttpPost]
        [AuthorizeGeoSource(GeoSourceKey = "geoSource.Id")]
        [Route("Specification")]
        [TimeTrace()]
        [Obsolete("this request moved to GeoSourceController")]
        public PipelineSpecification Specification(GeoSource geoSource)
        {
            return GeoSourceInitializer.Engine.GetSpecification(geoSource);
        }

        /// <summary>
        /// Geo a Style for a given GeoSource.
        /// a base style can be provided to guide the style generation for the given GeoSource.
        /// </summary>
        /// <param name="styleRequest">StyleRequest paramers</param>
        /// <returns>generate Style object</returns>
        [HttpPost]
        [AuthorizeGeoSource(GeoSourceKey = "styleRequest.GeoSource.Id")]
        [Route("Style")]
        [TimeTrace()]
        [Obsolete("this request moved to GeoSourceController")]
        public Style GetStyle(StyleRequest styleRequest)
        {
            var styledGeoSource = StyledGeoSource.Create(GeoSourceInitializer.Engine, styleRequest.GeoSource);

            if (styledGeoSource.Style == null || (styledGeoSource.Style.Fill == null && styledGeoSource.Style.Icon == null && styledGeoSource.Style.Line == null)) {
                return styledGeoSource.CreateDefaultStyle(styleRequest.Style ?? RandomStyleGenerator.Create(styleRequest.GeoSource.Id));
            } else {
                return styledGeoSource.Style;
            }
        }

        /// <summary>
        /// Generate a style based on the data disterbution of a given GeoSource field.
        /// This method can also be supplied with a Geometry to sample only data insde it.
        /// </summary>
        /// <param name="styleRequest">AutoStyleRequest paramertes.</param>
        /// <returns>Newly created Style object.</returns>
        [HttpPost]
        [AuthorizeGeoSource(GeoSourceKey = "styleRequest.GeoSource.Id")]
        [Route("AutoStyle")]
        [TimeTrace()]
        [Obsolete("this request moved to GeoSourceController")]
        public Style AutoStyle(AutoStyleRequestLegacy styleRequest)
        {
            var styledGeoSource = StyledGeoSource.Create(GeoSourceInitializer.Engine, styleRequest.GeoSource,
                styleRequest.Style);

            if (styleRequest.Geometry == null)
            {
                return styledGeoSource.CreateStyleByField(
                    styleRequest.Field,
                    styleRequest.Palette);
            }
            else
            {
                return styledGeoSource.CreateStyleByField(
                    styleRequest.Field,
                    styleRequest.Palette,
                    styleRequest.Geometry);
            }
        }

        /// <summary>
        /// Convert a circle geometry into PYXIcosIndex
        /// </summary>
        /// <param name="circle">A GeoJson Circle geometry</param>
        /// <returns>string representing a PYXIcosIndex</returns>
        [HttpPost]
        [Route("Index")]
        [TimeTrace()]
        public string PyxisIndex(CircleGeometry circle)
        {
            var snyder = SnyderProjection.getInstance();
            var resolution = snyder.precisionToResolution(circle.Radius.InRadians);
            return circle.Coordinates.ToPointLocation().asPYXIcosIndex(resolution).ToString();
        }
        
        
        /// <summary>
        /// post a geometry and get back a key (hash) representing that geometry so it can be used later on for other requests
        /// </summary>
        /// <param name="geometry">A GeoJson geometry</param>
        /// <returns>key - hash string value</returns>
        [HttpPost]
        [Route("Geometry")] //this route should be deprecated
        [Route("GeometryHash")]
        [TimeTrace()]
        public string GetGeometryHash(IGeometry geometry)
        {
            return GeometryCacheSingleton.Add(geometry);
        }


        /// <summary>
        /// post a style and get back a key (hash) representing that geometry so it can be used later on for other requests
        /// </summary>
        /// <param name="style">Style to generate has for</param>
        /// <returns>key - hash string value</returns>
        [HttpPost]
        [Route("StyleHash")]
        [TimeTrace()]
        public string GetStyleHash(Style style)
        {
            return StyleCacheSingleton.Add(style);
        }

        /// <summary>
        /// Return the area of a geometry in square meters on earth
        /// </summary>
        /// <param name="geometry">A GeoJson geometry</param>
        /// <returns>Area in square meters</returns>
        [HttpPost]
        [Route("Area")]
        [TimeTrace()]
        public double Area(IGeometry geometry)
        {
            return geometry.GetArea(GeoSourceInitializer.Engine).InSquareMeters;
        }
        
        /// <summary>
        /// Create a watershed geometry for a given location using a specified geoSource as elevation model
        /// </summary>
        /// <param name="geoSource">GeoSource to be used as elevation</param>
        /// <param name="location">circle geometry to specify the location</param>
        /// <returns></returns>
        [HttpGet]
        [AuthorizeGeoSource]
        [ApiCache]
        [Route("Watershed")]
        [TimeTrace("geoSource,location")]
        public IGeometry Watershed(Guid geoSource,string location)
        {
            var geoJsonGeometry = JsonConvert.DeserializeObject<IGeometry>(location);
            var circleGeometry = geoJsonGeometry as CircleGeometry;

            if (circleGeometry == null)
            {
                throw new Exception("Watershed calculation only supports circle geometry as starting location.");
            }

            var index = new PYXIcosIndex(PyxisIndex(circleGeometry));
            var cell = PYXCell.create(index);
            var geometry = pyxlib.DynamicPointerCast_PYXGeometry(cell);

            var elevationProcess = GeoSourceInitializer.Initialize(geoSource);

            var coverage = pyxlib.QueryInterface_ICoverage(elevationProcess.getOutput());

            if (coverage.getCoverageValue(index).isNull())
            {
                throw new Exception("Can't calculate watershed starting from a cell with no data");
            }

            var locationProcess = PYXCOMFactory.CreateSimpleFeature(index.toString(), geometry);

            var watershedProcess = PYXCOMFactory.CreateProcess(
                new PYXCOMProcessCreateInfo(s_watershedCalculationProcess)
                    .AddInput(0, elevationProcess)
                    .AddInput(1, locationProcess));

            if (watershedProcess.initProc() != IProcess.eInitStatus.knInitialized)
            {
                return null;
            }

            var resultGeometry = pyxlib.QueryInterface_IFeature(watershedProcess.getOutput()).getGeometry();

            return Geometry.FromPYXGeometry(resultGeometry);
        }

        /// <summary>
        /// Return a bounding circle of a geometry
        /// </summary>
        /// <param name="geometry">A GeoJson geometry</param>
        /// <returns>A GeoJson CircleGeometry that covers the given GeoJson geometry</returns>
        [HttpPost]
        [Route("BoundingCircle")]
        [TimeTrace()]
        public CircleGeometry BoundingCircle(IGeometry geometry)
        {
            var pyxGeometry = geometry.ToPyxGeometry(GeoSourceInitializer.Engine);

            var boundingCircle = pyxGeometry.getBoundingCircle();

            return new CircleGeometry()
            {
                Coordinates = new GeographicPosition(PointLocation.fromXYZ(boundingCircle.getCenter())),
                Radius = boundingCircle.getRadius() * SphericalDistance.Radian
            };
        }

        /// <summary>
        /// Check if a given url is not a pointer an "unsafe" path: windows directory, program files and application data.
        /// </summary>
        /// <param name="url">url for a local file</param>
        /// <returns>true if path is unsafe</returns>
        private bool UnsafePath(string url)
        {
            var safePath = System.IO.Path.GetFullPath(url);

            var unsafeDirectories = new [] { 
                Environment.SpecialFolder.Windows, 
                Environment.SpecialFolder.ProgramFiles, 
                Environment.SpecialFolder.ProgramFilesX86, 
                Environment.SpecialFolder.System, 
                Environment.SpecialFolder.SystemX86,
                Environment.SpecialFolder.ApplicationData,
                Environment.SpecialFolder.CommonApplicationData,
                Environment.SpecialFolder.LocalApplicationData
            };

            foreach (var unsafeDir in unsafeDirectories)
            {
                if (safePath.IndexOf(Environment.GetFolderPath(unsafeDir), StringComparison.OrdinalIgnoreCase) >= 0)
                {
                    return true;
                }
            }
            return false;
        }
    }    
}
