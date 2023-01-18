using System;
using System.Collections.Generic;
using System.Linq;
using System.Web.Http;
using ApplicationUtility;
using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.Analysis;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using PyxisCLI.Server.Cache;
using PyxisCLI.Server.Models;
using PyxisCLI.Server.Services;
using PyxisCLI.Server.Utilities;
using PyxisCLI.Server.WebConfig;
using Style = Pyxis.Contract.Publishing.Style;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// The GeoSourceController provides client access to data from the GeoSource.
    /// </summary>
    [RoutePrefix("api/v1/GeoSource")]
    public class GeoSourceController : ApiController
    {
        /// <summary>
        /// Returned the GeoSource details for a given id
        /// </summary>
        /// <param name="geoSource">GeoSource Id</param>
        /// <returns>GeoSource object</returns>

        [Route("{geoSource}")]
        [HttpGet]
        [TimeTrace("geoSource")]
        public GeoSource GetGeo(Guid geoSource)
        {
            return GeoSourceInitializer.GetGeoSource(geoSource);
        }

        /// <summary>
        /// Update GeoSource by Id
        /// </summary>
        /// <param name="geoSource">GeoSource object</param>
        /// <returns>GeoSource object</returns>

        [Route("{geoSource}")]
        [HttpPost]
        [TimeTrace("geoSource")]
        public GeoSource PostGeo([FromBody] GeoSource geoSource)
        {
            GeoSourceInitializer.Initialize(geoSource);
            return geoSource;
        }

        /// <summary>
        /// Notify GeoWebCore that a specific geosoruce need to be refreshed
        /// </summary>
        /// <param name="geoSource">GeoSource Id</param>
        /// <returns>GeoSource object</returns>

        [Route("{geoSource}/Refresh")]
        [HttpGet]
        [TimeTrace("geoSource")]
        public string Refresh(Guid geoSource)
        {
            GeoSourceInitializer.InvalidateGeoSource(geoSource, TimeSpan.FromSeconds(30));
            return "Ok";
        }

        /// <summary>
        /// Data request allow users to access data from coverages and vectors.
        /// The request allow the user to perform simple OData queries on top of the data.
        /// 
        /// For Coverage: the client can access data using $intersects=pyx-index
        /// For Vector: the client can access data as a flat list or a tree using $aggregate options
        ///             moreover, $intersects can be any kind of geo-json geometry
        /// 
        /// See more information at PyxisOData class for OData options
        /// 
        /// Examples:
        ///   api/v1/GeoSource/geo-source-guid/Data?$select=Age,Country&amp;$top=100
        ///   api/v1/GeoSource/geo-source-guid/Data?$intersects=1-12030131
        ///   api/v1/GeoSource/geo-source-guid/Data?$intersects=geo-json-geometry&amp;$select=Age,Country      
        /// </summary>
        /// <param name="geoSource">Guid represent the geosource</param>
        /// <param name="group">optional group id</param>
        /// <returns>FeatureGroup represent the return data</returns>

        [Route("{geoSource}/Data")]
        [Route("{geoSource}/Data/{group}")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource,group,$select")]
        public FeatureGroup Data2(
            Guid geoSource,
            string group = null)
        {
            //fetch odata parameters from Request uri query
            var odataModel = PyxisOData.FromRequest(Request);

            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            var process = state.GetProcess().Result;

            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            //if this is a coverage
            if (coverage.isNotNull())
            {
                //get coverage data
                return CoverageData(coverage, odataModel.Intersects);
            }

            //if this a feature group
            var featureGroup = pyxlib.QueryInterface_IFeatureGroup(process.getOutput());

            if (featureGroup.isNotNull() && !string.IsNullOrEmpty(group))
            {
                //access sub-group if group-id specified
                featureGroup = featureGroup.getFeatureGroup(group);
            }

            if (odataModel.Aggregate)
            {
                //aggregation mode
                if (featureGroup.isNotNull())
                {
                    //return data from a specific feature group
                    return FeatureGroupData(featureGroup, odataModel);
                }
            }
            else
            {
                var featureCollection = featureGroup.isNotNull()
                    ? pyxlib.QueryInterface_IFeatureCollection(featureGroup)
                    : pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                if (featureCollection.isNotNull())
                {
                    if (odataModel.CompleteEnumeration)
                    {
                        return GetCachedFeatureData(state.GetGeoSource().Result, featureCollection, odataModel);
                    }
                    //this geosource is a feature collection 
                    return FeatureData(featureCollection, odataModel);
                }
            }

            //if we got here we failed to get data
            return null;
        }

        private static FeatureGroup FeatureGroupData(IFeatureGroup_SPtr featureGroup, PyxisOData odataModel)
        {
            //please note - we ignore more odata parameters such as $skip,$top,$intersects
            var result = new FeatureGroup()
            {
                Features = new List<Feature>(),
                Groups = new List<Feature>()
            };

            foreach (var pyxFeature in featureGroup.GetGroupEnumerator())
            {
                var group = pyxlib.QueryInterface_IFeatureGroup(pyxFeature);

                if (group.isNotNull())
                {
                    result.Groups.Add(odataModel.Apply(group));
                }
                else
                {
                    result.Features.Add(odataModel.Apply(pyxFeature));
                }
            }

            return result;
        }

        private static FeatureGroup GetCachedFeatureData(GeoSource geoSource, IFeatureCollection_SPtr featureCollection, PyxisOData odataModel)
        {
            var result = new FeatureGroup
            {
                Features = new List<Feature>()
            };

            var startBlock = odataModel.Skip / PyxisOData.MaximumTop;
            var endBlock = (odataModel.Skip + odataModel.Top - 1) / PyxisOData.MaximumTop;
            var requiredBlocks = Enumerable.Range(startBlock, endBlock - startBlock + 1).ToList();

            foreach (var blockIndex in requiredBlocks)
            {
                var startIndex = blockIndex*PyxisOData.MaximumTop;

                var blockName = startIndex + "." + PyxisOData.MaximumTop + ".json";
                var chunk = GeoSourceBlobCacheSingleton.GetBlob<List<Feature>>(geoSource.Id, "json", blockName);

                if (chunk == null)
                {
                    chunk =
                        featureCollection.GetFeaturesEnumerator()
                            .Skip(startIndex)
                            .Take(PyxisOData.MaximumTop)
                            .Select(odataModel.Apply)
                            .ToList();

                    GeoSourceBlobCacheSingleton.WriteBlob<List<Feature>>(geoSource.Id, "json", blockName, chunk);
                }

                //trim end
                if (startIndex + chunk.Count > odataModel.Skip + odataModel.Top)
                {
                    var endIndex = odataModel.Skip + odataModel.Top - startIndex;
                    chunk.RemoveRange(endIndex, chunk.Count - endIndex);
                }

                //trim start
                if (startIndex < odataModel.Skip)
                {
                    chunk.RemoveRange(0, odataModel.Skip - startIndex);
                }
                
                result.Features.AddRange(chunk);
            }

            return result;
        }

        private static FeatureGroup FeatureData(IFeatureCollection_SPtr featureCollection, PyxisOData odataModel)
        {
            var result = new FeatureGroup
            {
                Features = new List<Feature>()
            };

            if (odataModel.Intersects != null)
            {
                var pyxGeometry = Program.Engine.ToPyxGeometry(odataModel.Intersects).__deref__();

                foreach (var pyxFeature in featureCollection.GetFeaturesEnumerator(pyxGeometry)
                    .Where(f => f.getGeometry().intersects(pyxGeometry))
                    .Skip(odataModel.Skip)
                    .Take(odataModel.Top)
                    )
                {
                    result.Features.Add(odataModel.Apply(pyxFeature));
                }
            }
            else
            {
                foreach (var pyxFeature in featureCollection.GetFeaturesEnumerator()
                    .Skip(odataModel.Skip)
                    .Take(odataModel.Top)
                    )
                {
                    result.Features.Add(odataModel.Apply(pyxFeature));
                }
            }

            return result;
        }

        /// <summary>
        /// Convert a GeoJson geometry into an index if possible.
        /// 
        /// Possibly meaning that the geometry looks like a cell (PYXCell or Circle)
        /// </summary>
        /// <param name="geometry">Geometry to convert</param>
        /// <returns>PYXIcosIndex or null if conversion is not possible</returns>
        private static PYXIcosIndex ConvertGeometryToPyxisIndexIfPossible(IGeometry geometry)
        {
            var cell = geometry as CellGeometry;

            if (cell != null)
            {
                return new PYXIcosIndex(cell.Index);
            }

            var circle = geometry as CircleGeometry;

            if (circle != null)
            {
                var snyder = SnyderProjection.getInstance();
                var resolution = snyder.precisionToResolution(circle.Radius.InRadians);
                return circle.Coordinates.ToPointLocation().asPYXIcosIndex(resolution);
            }

            return null;
        }

        /// <summary>
        /// Old studio only reads odd resolutions for visualization.
        /// 
        /// In order to make the web-globe return the same results we are fixing the index resolution to match
        /// </summary>
        /// <param name="index">index to fix</param>
        /// <returns>fixed index</returns>
        private static PYXIcosIndex MakeSurePickedIndexIsBackwardCompatible(PYXIcosIndex index)
        {
            var resolution = index.getResolution();

            //In order to mimic the c++ code. we make sure we return odd resolutions only
            if (resolution % 2 == 0)
                resolution--;

            //And the minimum resolution requested by the c++ code is 5
            if (resolution < 5)
                resolution = 5;

            if (resolution == index.getResolution())
            {
                return index;
            }

            var modifiedIndex = new PYXIcosIndex(index);
            modifiedIndex.setResolution(resolution);
            return modifiedIndex;
        }

        private static FeatureGroup CoverageData(ICoverage_SPtr coverage, IGeometry geometry)
        {
            var result = new FeatureGroup { Features = new List<Feature>() };

            var index = ConvertGeometryToPyxisIndexIfPossible(geometry);

            if (index == null)
            {
                return result;
            }

            index = MakeSurePickedIndexIsBackwardCompatible(index);

            var pyxcell = PYXCell.create(index);

            if (!coverage.getGeometry().intersects(pyxcell.__deref__()))
            {
                return result;
            }

            var properties = new Dictionary<string, object>();
            var i = 0;
            foreach (var field in coverage.getCoverageDefinition().FieldDefinitions)
            {
                var name = field.getName();
                var value = coverage.getCoverageValue(index, i);
                properties[name] = value.ToDotNetObject();
                i++;
            }

            result.Features.Add(new Feature(null, new CellGeometry(index), properties));
            return result;
        }

        /// <summary>
        /// Return a single feature from a GeoSource based on the feature Id
        /// </summary>
        /// <param name="geoSource">GeoSource Id to use.</param>
        /// <param name="id">feature Id to extract.</param>
        /// <returns>GeoJson description of the feature</returns>

        [Route("{geoSource}/Feature/{id}")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource,id")]
        public Feature Feature3(
            Guid geoSource,
            string id)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            var process = state.GetProcess().Result;
            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

            if (featureCollection == null || featureCollection.isNull())
            {
                throw new Exception("the GeoSource has to be of vector type");
            }

            return Pyxis.Core.IO.GeoJson.Feature.FromIFeature(featureCollection.getFeature(id));
        }

        /// <summary>
        /// Stats request allows users to access statistics of the data from coverages and vectors.
        ///
        /// Limitations:
        /// 1) $select must be specified and it can include only 1 field
        /// 2) $intersects must be specified for coverage (no general stats are available)
        /// 3) $top,$skip are not relevant and are ignored.
        /// 
        /// Examples:
        ///   api/v1/GeoSource/geo-source-guid/Stats?$select=Age
        ///   api/v1/GeoSource/geo-source-guid/Stats?$select=Elevation&amp;$intersects=geo-json-geometry        
        /// </summary>
        /// <param name="geoSource">GeoSource to request statistics on</param>
        /// <param name="bins">Number of bins to return in the value histogram (default:20)</param>
        /// <returns>FieldStatistics for the given GeoSource</returns>
 
        [Route("{geoSource}/Stats")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource,$select")]
        public FieldStatistics Stats(Guid geoSource, int bins = 20)
        {
            //fetch odata parameters from Request uri query
            var odataModel = PyxisOData.FromRequest(Request);

            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            var process = state.GetProcess().Result;

            if (odataModel.Select == null || odataModel.Select.Count != 1)
            {
                throw new Exception("Stats have to have a single selected field");
            }

            var calculator = new StatisticsCreator(Program.Engine, process);

            if (odataModel.Intersects != null)
            {
                return calculator.GetFieldStatistics(odataModel.Intersects, odataModel.Select[0], bins);
            }
            else
            {
                return calculator.GetFieldStatistics(odataModel.Select[0], bins);
            }
        }

        /// <summary>
        /// Get the PipelineSpecification of a GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to request specification on</param>
        /// <returns>PipelineSpecification for the given GeoSource</returns>

        [Route("{geoSource}/Specification")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource")]
        public PipelineSpecification Specification(Guid geoSource)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            return state.GetSpecification().Result;
        }


        /// <summary>
        /// Return the total size of supporting files for the given GeoSource, not including supporting files of GeoSource that been used by the given GeoSource.        
        /// </summary>
        /// <param name="geoSource">Id of the Given GeoSource</param>
        /// <returns>size in bytes</returns>

        [Route("{geoSource}/DataSize")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource")]
        public long DataSize(Guid geoSource, bool includeGeoTag = false)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            var process = state.GetProcess().Result;
            var geoPacketSource = process.GeoPacketSources().First();

            var manifests = includeGeoTag
                ? geoPacketSource.WalkPipelines().ExtractManifests().ToList()
                : geoPacketSource.WalkPipelinesExcludeGeoPacketSourcesAfterParent().ExtractManifests().ToList();
            return manifests.SelectMany(m => m.Entries).Sum(e => e.FileSize);
        }

        /// <summary>
        /// Get a bounding circle of a GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to request bounding circle on</param>
        /// <returns>CircleGeometry for the given GeoSource</returns>

        [Route("{geoSource}/BoundingCircle")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource")]
        public CircleGeometry BoundingCircle2(Guid geoSource)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            return state.GetCharacterization().Result.BoundingCircle;
        }

        /// <summary>
        /// Get a bounding bbox of a GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to request bounding box on</param>
        /// <returns>BBox for the given GeoSource</returns>

        [Route("{geoSource}/BoundingBox")]
        [HttpGet]
        [ApiCache()]
        [TimeTrace("geoSource")]
        public BoundingBox BoundingBox2(Guid geoSource)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);

            var feature = pyxlib.QueryInterface_IFeature(state.GetProcess().Result.getOutput());
            if (feature == null || feature.isNull())
            {
                throw new Exception("Failed to get bounding box from GeoSource.");
            }

            var rect1 = new PYXRect2DDouble();
            var rect2 = new PYXRect2DDouble();

            feature.getGeometry().getBoundingRects(new WGS84CoordConverter(), rect1, rect2);

            if (!rect1.empty())
            {
                return new BoundingBox()
                {
                    Srs = "4326",
                    LowerLeft = new BoundingBoxCorner() {X = rect1.xMin(), Y = rect1.yMin()},
                    UpperRight = new BoundingBoxCorner() {X = rect1.xMax(), Y = rect1.yMax()}
                };
            }

            if (!rect2.empty())
            {
                return new BoundingBox()
                {
                    Srs = "4326",
                    LowerLeft = new BoundingBoxCorner() {X = rect2.xMin(), Y = rect2.yMin()},
                    UpperRight = new BoundingBoxCorner() {X = rect2.xMax(), Y = rect2.yMax()}
                };
            }

            return null;
        }

        /// <summary>
        /// Get a characterization of the data of the GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to request data characterization on</param>
        /// <returns>GeoSourceDataCharacterization for the given GeoSource</returns>

        [Route("{geoSource}/Characterize")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource")]
        public GeoSourceDataCharacterization Characterize2(Guid geoSource)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            return state.GetCharacterization().Result;
        }

        private static StyleAndHash CreateStyleResponse(Style style)
        {
            var hash = StyleCacheSingleton.Add(style);
            if (!hash.StartsWith("{"))
            {
                return new StyleAndHash { Style = style, Hash = hash };
            }
            else
            {
                //no hash is needed
                return new StyleAndHash { Style = style };
            }
        }
        
        /// <summary>
        /// Get a characterization of the data of the GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to request data characterization on</param>
        /// <returns>GeoSourceDataCharacterization for the given GeoSource</returns>

        [Route("{geoSource}/Style")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource")]
        public StyleAndHash Style(Guid geoSource)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            return CreateStyleResponse(state.GetStyle().Result);
        }

        /// <summary>
        /// Get a characterization of the data of the GeoSource
        /// </summary>
        /// <param name="geoSource">GeoSource to request data characterization on</param>
        /// <param name="styleRequest">Style request on how to generate new style</param>
        /// <returns>GeoSourceDataCharacterization for the given GeoSource</returns>
        [Route("{geoSource}/Style")]
        [HttpPost]
        [ApiCache]
        [TimeTrace("geoSource")]
        public StyleAndHash Style2(Guid geoSource, [FromBody] AutoStyleRequest styleRequest)
        {
            var state = GeoSourceInitializer.GetGeoSourceState(geoSource);
            return CreateStyleResponse(state.GetStyle(styleRequest).Result);
        }

        private static string s_watershedCalculationProcess = "{4D367363-2421-4DF2-AD97-529A4EDDE0A8}";

        private static string PyxisIndex(CircleGeometry circle)
        {
            var snyder = SnyderProjection.getInstance();
            var resolution = snyder.precisionToResolution(circle.Radius.InRadians);
            return circle.Coordinates.ToPointLocation().asPYXIcosIndex(resolution).ToString();
        }

        /// <summary>
        /// Create a watershed geometry for a given location using a specified geoSource as elevation model
        /// </summary>
        /// <param name="geoSource">GeoSource to be used as elevation</param>
        /// <param name="location">circle geometry to specify the location</param>
        /// <returns></returns>
        [Route("{geoSource}/Watershed")]
        [HttpGet]
        [ApiCache]
        [TimeTrace("geoSource,location")]
        public IGeometry Watershed(Guid geoSource, string location)
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
    }
}
