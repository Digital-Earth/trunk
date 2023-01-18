using ApplicationUtility;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Workspaces;
using Pyxis.Core.Analysis;
using Pyxis.Core.DERM;
using Pyxis.Core.IO;
using Pyxis.Core.IO.Core;
using Pyxis.Core.IO.EDR;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using PyxisCLI.Server.Services;
using PyxisCLI.Server.WebConfig;
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
using System.Web.Http;
using static Pyxis.Contract.Publishing.PipelineSpecification;

namespace PyxisCLI.Server.Controllers
{
    /// <summary>
    /// Output DGGS data
    /// </summary>
    [RoutePrefix("api/v1/edr")]
    public class EDRAPIController : AuthorizedApiController
    {
        private static Pyxis.Core.DERM.Derm s_derm;
        private static object s_dermLock = new object();
        private static EngineCleanup s_cleanup = new EngineCleanup(Cleanup);

        public Derm Derm
        {
            get
            {
                lock (s_dermLock)
                {
                    if (s_derm == null)
                    {
                        s_derm = new Derm(Program.Engine);
                    }
                }

                return s_derm;
            }
        }

        private static void Cleanup()
        {
            lock (s_dermLock)
            {
                s_derm = null;
            }
        }

        private static readonly object s_coordConverterLock = new object();
        private static ICoordConverter_SPtr s_coordConverter;

        private static readonly double MAX_X = 20037508.3428;
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

/*        [Route("collections")]
        [HttpGet]
        public List<Collection> Collections()
        {
            const string workspace = "test";

            var properties = new Dictionary<string, object>();

            Link link1 = new Link()
            {
                Href = "http:host/api/v1/edr/collections",
                Rel = "self"
            };

            Link[] links = new Link[] { link1 };

            List<Collection> result = new List<Collection>();

            if (Program.Workspaces.WorkspaceExists(workspace))
            {
                foreach (var import in Workspaces.GetWorkspace(workspace).Imports)
                {

                    var reference = new Reference(workspace + "/" + import.Key);
                    var geoSource = Program.Workspaces.ResolveGeoSource(reference);

                    var process = GeoSourceInitializer.Initialize(geoSource);

                    var bbox = BoundingBox(process);
                    
                    var extentProperties = new Dictionary<string, SpatialExtent>();
                    double[] bboxSpatial = new double[] { bbox.LowerLeft.X, bbox.LowerLeft.Y, bbox.UpperRight.X, bbox.UpperRight.Y };

                    SpatialExtent spatialExtent = new SpatialExtent()
                    {
                        Bbox = bboxSpatial,
                        Crs = bbox.Srs,
                    };

                    extentProperties.Add("spatial", spatialExtent);

                    Extent extent = new Extent()
                    {
                        Properties = extentProperties
                    };


                    var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

                    var fields = new Dictionary<string, object>();

                    //if this is a coverage
                    if (coverage.isNotNull())
                    {
                        foreach (var field in coverage.getCoverageDefinition().FieldDefinitions)
                        {
                            var fieldName = field.getName();
                            fields.Add(fieldName, field.getType().ToString());
                        }
                        
                        Collection col = new Collection()
                        {
                            Id = import.Key,
                            Extent = extent,
                            Links = links,
                            Parameter_names = fields
                        };
                        result.Add(col);
                    }

                    else
                    {
                        var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                        if (featureCollection.isNotNull())
                        {
                            foreach (var field in featureCollection.getFeatureDefinition().FieldDefinitions)
                            {
                                var fieldName = field.getName();
                                fields.Add(fieldName, field.getType().ToString());
                            }                              
                            
                            

                            Collection col = new Collection()
                            {
                                Id = import.Key,
                                Extent = extent,
                                Links = links,
                                Parameter_names = fields
                                *//*featureCollection.getFeatureDefinition().getFieldNames()*//*
                            };
                            result.Add(col);
                        }

                    }
                }
            }
            return result;
         }*/

        [Route("collections")]
        [HttpGet]
        public Collections Collections(string bbox = null)
        {

            const string workspace = "test";

            var properties = new Dictionary<string, object>();

            Link link1 = new Link()
            {
                Href = "https:digitalearthsolutions/api/v1/edr/collections",
                Rel = "self",
                Type = "application/json"
            };

            Link[] links = new Link[] { link1 };

            List<Collection> collectionsList = new List<Collection>();

            if (Program.Workspaces.WorkspaceExists(workspace))
            {
                foreach (var import in Workspaces.GetWorkspace(workspace).Imports)
                {

                    var reference = new Reference(workspace + "/" + import.Key);
                    var geoSource = Program.Workspaces.ResolveGeoSource(reference);
                    var description = geoSource.Metadata.Description;
                    var title = geoSource.Metadata.Name;
                    DataQueries areaQuery = generateAreaQuery(import.Key);
                    var process = GeoSourceInitializer.Initialize(geoSource);

                    var bbox_process = BoundingBox(process);

                    if (bbox.HasContent())
                    {
                        var numbers = bbox.Split(',').Select(x => double.Parse(x)).ToArray();

                        {
                            BoundingBox query_bbox = new BoundingBox()
                            {
                                Srs = "4326",
                                LowerLeft = new BoundingBoxCorner() { X = numbers[0], Y = numbers[1] },
                                UpperRight = new BoundingBoxCorner() { X = numbers[2], Y = numbers[3] }
                            };
                            if (!bbox_process.Intersects(query_bbox))
                            {
                                continue;
                            }
                        }
                    }
                    double[] bboxSpatial = new double[] { bbox_process.LowerLeft.X, bbox_process.LowerLeft.Y, bbox_process.UpperRight.X, bbox_process.UpperRight.Y };

                        SpatialExtent spatialExtent = new SpatialExtent()
                        {
                            Bbox = bboxSpatial,
                            Crs = bbox_process.Srs,
                        };

                        Extent extent = new Extent()
                        {
                            spatial = spatialExtent
                        };


                        var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

                        var fields = new Dictionary<string, object>();

                        List<FieldSpecification> fieldSpecifications = geoSource.Specification.Fields;
                    var unit = "";
                    var symbol = "";
                    var fieldDescription = "";

                    

                    //if this is a coverage
                    if (coverage.isNotNull())
                        {
                        var counter = 0;
                            foreach (var field in coverage.getCoverageDefinition().FieldDefinitions)
                            {
                                var fieldName = field.getName();
                            if (fieldSpecifications[counter].Name == fieldName)
                            {
                                if (fieldSpecifications[counter].Metadata != null)
                                {
                                    if (fieldSpecifications[counter].Metadata.Name != null)
                                    {
                                        fieldName = fieldSpecifications[counter].Metadata.Name;
                                    }
                                    if (fieldSpecifications[counter].Metadata.Description != null)
                                    {
                                        fieldDescription = fieldSpecifications[counter].Metadata.Description;
                                    }
                                }
                                if (fieldSpecifications[counter].FieldUnit != null)
                                {
                                    if (fieldSpecifications[counter].FieldUnit.Name != null)
                                    {
                                        unit = fieldSpecifications[counter].FieldUnit.Name;
                                    }

                                    if (fieldSpecifications[counter].FieldUnit.Symbol != null)
                                    {
                                        symbol = fieldSpecifications[counter].FieldUnit.Symbol;
                                    }


                                }
                            }
                            fields.Add(fieldName, new ParameterNames()
                                {
                                    Id = fieldName,
                                    Type = "Parameter",
                                    Description= fieldDescription,
                                    Unit = new Unit()
                                    {
                                        Label = unit,
                                        Symbol = new Symbol()
                                        {
                                            Value = symbol
                                        }
                                    },
                                    ObservedProperty = new ObservedProperty()
                                    {
                                        Label = fieldName
                                    },
                                    Data_type = field.getType().ToString(),
                                });
                            counter = counter + 1;
                            }
                    

                            Collection col = new Collection()
                            {
                                Id = import.Key,
                                Title = title,
                                Description = description,
                                Extent = extent,
                                Links = links,
                                Parameter_names = fields,
                                Data_queries = areaQuery
                            };
                            collectionsList.Add(col);
                        }

                        else
                        {
                            var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                            if (featureCollection.isNotNull())
                            {
                            var counter = 0;
                            foreach (var field in featureCollection.getFeatureDefinition().FieldDefinitions)
                            {
                                var fieldName = field.getName();
                                if (fieldSpecifications[counter].Name == fieldName)
                                {
                                    if (fieldSpecifications[counter].Metadata != null)
                                    {
                                        if (fieldSpecifications[counter].Metadata.Name != null)
                                        {
                                            fieldName = fieldSpecifications[counter].Metadata.Name;
                                        }
                                        if (fieldSpecifications[counter].Metadata.Description != null)
                                        {
                                            fieldDescription = fieldSpecifications[counter].Metadata.Description;
                                        }
                                    }
                                    if (fieldSpecifications[counter].FieldUnit != null)
                                    {
                                        if (fieldSpecifications[counter].FieldUnit.Name != null)
                                        {
                                            unit = fieldSpecifications[counter].FieldUnit.Name;
                                        }

                                        if (fieldSpecifications[counter].FieldUnit.Symbol != null)
                                        {
                                            symbol = fieldSpecifications[counter].FieldUnit.Symbol;
                                        }


                                    }
                                }
                                fields.Add(fieldName, new ParameterNames(){
                                    Id = fieldName,
                                    Type = "Parameter",
                                    Description = fieldDescription,
                                    Unit = new Unit()
                                    {
                                        Label = unit,
                                        Symbol = new Symbol()
                                        {
                                            Value = symbol
                                        }
                                    },
                                    ObservedProperty = new ObservedProperty()
                                    {
                                        Label = fieldName
                                    },
                                    Data_type = field.getType().ToString(),
                                });
                                counter = counter + 1;
                            }
                                Collection col = new Collection()
                                {
                                    Id = import.Key,
                                    Extent = extent,
                                    Title = title,
                                    Description = description,
                                    Links = links,
                                    Parameter_names = fields,
                                    Data_queries = areaQuery
                                };
                                collectionsList.Add(col);
                        }

                        }

                }
            }
            Collections result = new Collections()
            {
                Collection = collectionsList.ToArray(),
                Links = links
            };

            return result;
        }

        private DataQueries generateAreaQuery(string key)
        {
            return new DataQueries()
            {
                Area = new Query()
                {
                    Link = new Link()
                    {
                        Href = "http:34.95.36.182/api/v1/edr/collections/" + key + "/area",
                        Rel = "data",
                        Templated = false,
                        Hreflang = "en",
                        Variables = new Variables()
                        {
                            Title = "Area query",
                            Query_type = "area",
                            Output_formats = new string[] { "GeoJSON" },
                            Default_output_formats = "GeoJSON",
                            Crs_details = new CrsDetails()
                            {
                                Crs = "4326",
                                Wkt = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]"
                            }
                        }
                    }
                }
            };
        }

        private BoundingBox BoundingBox(IProcess_SPtr process)
        {
            var feature = pyxlib.QueryInterface_IFeature(process.getOutput());
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
                    LowerLeft = new BoundingBoxCorner() { X = rect1.xMin(), Y = rect1.yMin() },
                    UpperRight = new BoundingBoxCorner() { X = rect1.xMax(), Y = rect1.yMax() }
                };
            }

            if (!rect2.empty())
            {
                return new BoundingBox()
                {
                    Srs = "4326",
                    LowerLeft = new BoundingBoxCorner() { X = rect2.xMin(), Y = rect2.yMin() },
                    UpperRight = new BoundingBoxCorner() { X = rect2.xMax(), Y = rect2.yMax() }
                };
            }

            return null;
        }

        [Route("collections/{collectionId}")]
        [HttpGet]
        public Collection Collection(string collectionId)
        {
            const string workspace = "test";

            var properties = new Dictionary<string, object>();

            Link link1 = new Link()
            {
                Href = "http:host/api/v1/edr/collections",
                Rel = "self"
            };

            Link[] links = new Link[] { link1 };

            Collection result = new Collection();

            var reference = new Reference(workspace + "/" + collectionId);
            var geoSource = Program.Workspaces.ResolveGeoSource(reference);
            DataQueries areaQuery = generateAreaQuery(collectionId);
            var process = GeoSourceInitializer.Initialize(geoSource);

            var bbox_process = BoundingBox(process);

            double[] bboxSpatial = new double[] { bbox_process.LowerLeft.X, bbox_process.LowerLeft.Y, bbox_process.UpperRight.X, bbox_process.UpperRight.Y };

            SpatialExtent spatialExtent = new SpatialExtent()
            {
                Bbox = bboxSpatial,
                Crs = bbox_process.Srs,
            };

            Extent extent = new Extent()
            {
                spatial = spatialExtent
            };

            var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

            var fields = new Dictionary<string, object>();

            //if this is a coverage
            if (coverage.isNotNull())
            {
                foreach (var field in coverage.getCoverageDefinition().FieldDefinitions)
                {
                    var fieldName = field.getName();
                    fields.Add(fieldName, new ParameterNames()
                    {
                        Id = fieldName,
                        Type = "Parameter",
                        ObservedProperty = new ObservedProperty()
                        {
                            Label = fieldName
                        },
                        Data_type = field.getType().ToString(),
                    });
                }


                Collection col = new Collection()
                {
                    Id = collectionId,
                    Extent = extent,
                    Links = links,
                    Parameter_names = fields,
                    Data_queries = areaQuery
                };
                return col;
            }

            else
            {
                var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                if (featureCollection.isNotNull())
                {
                    foreach (var field in featureCollection.getFeatureDefinition().FieldDefinitions)
                    {
                        var fieldName = field.getName();
                        fields.Add(fieldName, new ParameterNames()
                        {
                            Id = fieldName,
                            Type = "Parameter",
                            ObservedProperty = new ObservedProperty()
                            {
                                Label = fieldName
                            },
                            Data_type = field.getType().ToString(),
                        });
                    }
                    Collection col = new Collection()
                    {
                        Id = collectionId,
                        Extent = extent,
                        Links = links,
                        Parameter_names = fields,
                        Data_queries = areaQuery
                    };
                    return col;
                }
            }
            return null;
     }

        [Route("collections/{collectionId}/area")]
        [HttpGet]
        public EDRFeatureCollection Area(string collectionId, string coords, string parameter_name = null, string datetime=null, int bins = 20) {

            const string workspace = "test";

            //Properties
            var properties = new Dictionary<string, object>();

            var result = new Feature()
            {
                Id = Guid.NewGuid().ToString(),
                Geometry = null,
                Properties = properties
            };

            var reference = new Reference(workspace + "/" + collectionId);
/*            if (datetime != null)
            {
                reference.Domains = {

                };
            }
            */
            var geoSource = Program.Workspaces.ResolveGeoSource(reference);

            var process = GeoSourceInitializer.Initialize(geoSource);

            if (coords.StartsWith("POLYGON"))
            {
                // Strip out everything except the coordinates
                var coordRawText = coords.Replace("POLYGON((", "");
                coordRawText = coordRawText.Replace("))", "");
                coordRawText = coordRawText.Replace(", ", ",");

                // Seperate coordinates to iterate through
                var coordsArray = coordRawText.Split(',');
                var coordsEnumerable = coordsArray.Select(coord => coord.Replace(" ", ","));

                // Build list of coordinates
                var coordsList = new List<GeographicPosition>();
                var listOfList = new List<List<GeographicPosition>>();
                foreach (var coord in coordsEnumerable)
                {
                    var splt = coord.Split(',');
                    var pos = GeographicPosition.FromWgs84LatLon(double.Parse(splt[1]), double.Parse(splt[0]));
                    coordsList.Add(pos);
                }
                listOfList.Add(coordsList);

                PolygonGeometry geom = new PolygonGeometry()
                {
                    Coordinates = listOfList
                };
                result.Geometry = geom;
            }
            else if (coords.StartsWith("MULTIPOLYGON"))
            {
                // Strip out everything except the coordinates
                var coordRawText = coords.Replace("MULTIPOLYGON(", "");
                coordRawText = coordRawText.Replace("), (", "|");
                coordRawText = coordRawText.Replace("),(", "|");
                coordRawText = coordRawText.Replace("((", "(");
                coordRawText = coordRawText.Replace(")))", ")");

                // Separate polygons to iterate through
                var polysArray = coordRawText.Split('|');

                var listOfListOfList = new List<List<List<GeographicPosition>>>();
                var listOfList = new List<List<GeographicPosition>>();

                foreach (var pol in polysArray)
                {
                    var polRawText = pol.Replace("(", "");
                    polRawText = polRawText.Replace(")", "");
                    // Seperate coordinates to iterate through
                    var coordsArray = polRawText.Split(',');
                    var coordsEnumerable = coordsArray.Select(coord => coord.Replace(" ", ","));

                    // Build list of coordinates
                    var coordsList = new List<GeographicPosition>();

                    foreach (var coord in coordsEnumerable)
                    {
                        var splt = coord.Split(',');
                        var pos = GeographicPosition.FromWgs84LatLon(double.Parse(splt[1]), double.Parse(splt[0]));
                        coordsList.Add(pos);
                    }
                    listOfList.Add(coordsList);
                }
                listOfListOfList.Add(listOfList);

                MultiPolygonGeometry geom = new MultiPolygonGeometry()
                {
                    Coordinates = listOfListOfList
                };
                result.Geometry = geom;
            }

            else
            {
                throw new Exception("Geometry not yet supported");
            }

            var calculator = new StatisticsCreator(Program.Engine, process);

            //Parameters name
            if (parameter_name == null)
            {
                var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

                //if this is a coverage
                if (coverage.isNotNull())
                {
                    foreach (var field in coverage.getCoverageDefinition().FieldDefinitions)
                    {
                        var fieldName = field.getName();
                        FieldStatistics calculations = calculator.GetFieldStatistics(result.Geometry, fieldName, bins);
                        properties.Add(collectionId + "/" + fieldName + "/min", calculations.Min);
                        properties.Add(collectionId + "/" + fieldName + "/max", calculations.Max);
                        properties.Add(collectionId + "/" + fieldName + "/avg", calculations.Average);
                    }
                }
                else
                {
                    var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                    if (featureCollection.isNotNull())
                    {
                        foreach (var field in featureCollection.getFeatureDefinition().FieldDefinitions)
                        {
                            var fieldName = field.getName();
                            FieldStatistics calculations = calculator.GetFieldStatistics(result.Geometry, fieldName, bins);
                            properties.Add(collectionId + "/" + fieldName + "/min", calculations.Min);
                            properties.Add(collectionId + "/" + fieldName + "/max", calculations.Max);
                            properties.Add(collectionId + "/" + fieldName + "/avg", calculations.Average);
                        }
                    }
                }
            }

            else
            {

                var parameter_name_split = parameter_name.Split(',');
                foreach (var param in parameter_name_split)
                {

                    FieldStatistics calculations = calculator.GetFieldStatistics(result.Geometry, param, bins);

                    properties.Add(collectionId + "/" + param + "/min", calculations.Min);
                    properties.Add(collectionId + "/" + param + "/max", calculations.Max);
                    properties.Add(collectionId + "/" + param + "/avg", calculations.Average);
                }
            }
            var edrFeatures = new List<Feature>();
            edrFeatures.Add(result);

            var EDRresult = new EDRFeatureCollection()
            {
                Features = edrFeatures,
                Timestamp = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ssZ"),
                NumberMatched = 1,
                NumberReturned = 1
            };
            return EDRresult;
        }

        static T[,] CreateRectangularArray<T>(IList<T[,]> arrays)
        {
            // TODO: Validation and special-casing for arrays.Count == 0
            int minorLength = arrays[0].Length;
            T[,] ret = new T[arrays.Count, minorLength];
            for (int i = 0; i < arrays.Count; i++)
            {
                var array = arrays[i];
                if (array.Length != minorLength)
                {
                    throw new ArgumentException
                        ("All arrays must be the same length");
                }
                for (int j = 0; j < minorLength; j++)
                {
                    ret[i, j] = array[0, j];
                }
            }
            return ret;
        }

        /*[Route("{workspace}")]
        [HttpGet]
        [TimeTrace("workspace")]
        [AllowAnonymous]
        public Feature GeoJson(string workspace, double latitude, double longitude, int resolution)
        {

            var geoPosition = new GeographicPosition()
            {
                Latitude = latitude,
                Longitude = longitude
            };

            //Get cell from lat, long and resolution
            var cell = Derm.Cell(geoPosition, resolution);

            var point = new PointGeometry()
            {
                Coordinates = geoPosition
            };

            //Create polygon geometry for cell
            var polygon = new PolygonGeometry()
            {
                Coordinates = new List<List<GeographicPosition>>()
            };

            var ring = cell.Vertices().Select(c => c.Center).ToList();
            ring.Add(ring[0]); //close ring

            polygon.Coordinates.Add(ring);
            *//*
                        var result = new FeatureGroup
                        {
                            Features = new List<Feature>()
                        };*//*




            //Properties
            var properties = new Dictionary<string, object>();
            var result = new Feature()
            {
                Id = Guid.NewGuid().ToString(),
                Geometry = polygon,
                Properties = properties
            };


            if (Program.Workspaces.WorkspaceExists(workspace))
            {
                foreach (var import in Workspaces.GetWorkspace(workspace).Imports)
                {

                    var reference = new Reference(workspace+"/"+import.Key);
                    var geoSource = Program.Workspaces.ResolveGeoSource(reference);

                    var process = GeoSourceInitializer.Initialize(geoSource);

                    *//*            var state = GeoSourceInitializer.GetGeoSourceState(geoSource.Id);
                                    var process = state.GetProcess().Result;*//*

                    var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());



                    *//*       var value = CoverageData(coverage, icosIndex);
                           var value2 = coverage.getCoverageValue(icosIndex, 0);



                           properties.Add(process.getProcName(), value2.getInt(0).ToString() + "," + value2.getInt(1).ToString() + "," + value2.getInt(2).ToString());
               *//*
                    //Create feature
                    *//*            var feature = new Feature()
                                {
                                    Id = Guid.NewGuid().ToString(),
                                    Geometry = polygon,
                                    Properties = properties
                                };
                    *//*

                    //if this is a coverage
                    if (coverage.isNotNull())
                    {
                        var name = Guid.NewGuid().ToString();
                        try
                        {
                            name = import.Value.Layer;
                        }
                        catch
                        {
                            name = import.Key;
                        }
                        if (name == "")
                        {
                            name = import.Key;
                        }
                        //get coverage data
                        var icosIndex = new PYXIcosIndex(cell.Index);
                        var fetGroup = CoverageData(coverage, icosIndex);
                        if (fetGroup.Features.Count() > 0)
                        {
                            result.Properties.Add(name, fetGroup);
                        }

                    }

                    else
                    {
                        var featureCollection = pyxlib.QueryInterface_IFeatureCollection(process.getOutput());

                        if (featureCollection.isNotNull())
                        {
                            //this geosource is a feature collection 
                            var fetGroup = FeatureData(featureCollection, point);
                            if(fetGroup.Features.Count() > 0)
                            {
                                result.Properties.Add(import.Value.Layer, fetGroup);
                            }

                        }

                    }

                    *//*            return value;*/

        /*                    //Create feature
                            var vectorFeature = new Feature()
                            {
                                Id = Guid.NewGuid().ToString(),
                                Geometry = polygon,
                                Properties = properties
                            };

                            result.Properties.Add(Guid.NewGuid().ToString(), vectorFeature);*//*


                            //if we got here we failed to get data
                       *//*     return feature;*//*
                        }
                    }


                    return result;


                }

                private static FeatureGroup FeatureData(IFeatureCollection_SPtr featureCollection, IGeometry point)
                {
                    var result = new FeatureGroup
                    {
                        Features = new List<Feature>()
                    };

                    var pyxGeometry = Program.Engine.ToPyxGeometry(point).__deref__();

                    foreach (var pyxFeature in featureCollection.GetFeaturesEnumerator(pyxGeometry)
                           .Where(f => f.getGeometry().intersects(pyxGeometry)))
                    {
                        result.Features.Add(Apply(pyxFeature));
                    }

                    return result;
                }

                private static Feature Apply(IFeature_SPtr pyxFeature)
                {
                    var fields = new Dictionary<string, object>();

                    var fieldNames = pyxFeature.getDefinition().FieldDefinitions.Select(x => x.getName()).ToList();


                    foreach (var name in fieldNames)
                    {
                        var value = pyxFeature.getFieldValueByName(name);
                        fields[name] = value.ToDotNetObject();
                    }

                    Geometry geometry = null;

        *//*            geometry = Geometry.FromPYXGeometry(pyxFeature.getGeometry());*//*

                    return new Feature(pyxFeature.getID(), geometry, fields);
                }

                private static FeatureGroup CoverageData(ICoverage_SPtr coverage, PYXIcosIndex index)
                {
                    var result = new FeatureGroup { Features = new List<Feature>() };

                    *//*   var index = ConvertGeometryToPyxisIndexIfPossible(geometry);*//*

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

                [Route("{geoSource}/{z}/{x}/{y}.png")]
                [HttpGet]
                [TimeTrace("geoSource,z,x,y")]
                [AllowAnonymous]
                public HttpResponseMessage Tile(string geoSource, int z, int x, int y, int cellWidth = 20)
                {

                    //Create green brush
                    var brush = new SolidBrush(Color.FromArgb(128, Color.Green));

                    //Get geosource from uuid
                    var guid = Guid.Parse(geoSource);

                    //Initialize geosource
                    var process = GeoSourceInitializer.InitializeAsCoverage(guid);

                    //Get coverage
                    var coverage = pyxlib.QueryInterface_ICoverage(process.getOutput());

                    //Create empty image
                    var bitmap = new Bitmap(255, 255, PixelFormat.Format32bppPArgb);

                    //Coordinates of image
                    var topLeft = GetNative(z, x, y, 0.1f, 0.1f);
                    var bottomRight = GetNative(z, x, y, 255.9f, 255.9f);

                    //Get lat long of tile center
                    var center = GetLatLon(z, x, y, 128, 128);
                    var leftToCenter = GetLatLon(z, x, y, 128 + cellWidth, 128);

                    //Calculate radius
                    var distance = SphereMath.distanceBetween(center, leftToCenter);

                    //Estimate resolution
                    var resolution = PYXBoundingCircle.estimateResolutionFromRadius(distance);
                    resolution = Math.Max(2, resolution);

                    var radius = SnyderProjection.getInstance().resolutionToPrecision(resolution);
                    var radius2 = SnyderProjection.getInstance().resolutionToPrecision(resolution - 1);

                    Console.WriteLine("{0} < {1} < {2}", radius, distance, radius2);

                    var nativeBBox = new PYXRect2DDouble(topLeft.x(), bottomRight.y(), bottomRight.x(), topLeft.y());

                    Console.WriteLine("{0} {1} {2}", nativeBBox.width(), nativeBBox.height(), resolution);

                    var resolutionsWeight = GenerateWeights(z, x, y, cellWidth);

                    foreach (var keyValue in resolutionsWeight)
                    {
                        var res = keyValue.Key;
                        var weights = keyValue.Value;
                        RenderResolution(z, x, y, nativeBBox, res, bitmap, coverage, brush, weights);
                    }

                    return BitmapToResponse(bitmap, "png");
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
                            var sampleIndex = (int)Math.Floor(samples * cellCenterPoint.Y / 256.0);
                            sampleIndex = Math.Max(0, Math.Min(sampleIndex, samples - 1));

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

        *//*                    if (!value.isNull())
                            {
                                graphics.FillPath(brush, path);
                            }*//*

                            graphics.DrawPath(cellPen, path);


                            iterator.next();
                        }
                    }
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
                    CoordConverter.nativeToLatLon(GetNative(z, x, y, px, py), latlon);
                    return latlon;
                }

                private PointF GetPointFromNative(PYXCoord2DDouble native, int z, int x, int y, PYXCoord2DDouble reference)
                {
                    var dx = native.x() / MAX_X;
                    var dy = native.y() / MAX_Y;

                    var referenceX = reference.x() / MAX_X;

                    var scale = Math.Pow(2, z);

                    var xratio = x / scale;

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


                    dx = (dx + 1) / 2 * scale;
                    dy = (1 - dy) / 2 * scale;

                    return new PointF((float)(dx - x) * 256, (float)(dy - y) * 256);
                }

                private Dictionary<int, double[]> GenerateWeights(int z, int x, int y, int cellWidth)
                {
                    var resolutionsWeight = new Dictionary<int, double[]>();
                    const int samples = 16;

                    for (var dy = 0; dy < samples; dy++)
                    {
                        var py = 255.0f * dy / samples + 0.5f;
                        var p1 = GetLatLon(z, x, y, 128, py);
                        var p2 = GetLatLon(z, x, y, 128 + 1, py);

                        var d = SphereMath.distanceBetween(p1, p2) * cellWidth;

                        var res = PYXBoundingCircle.estimateResolutionFromRadius(d);

                        var minRadius = SnyderProjection.getInstance().resolutionToPrecision(res);
                        var maxRadius = SnyderProjection.getInstance().resolutionToPrecision(res - 1);

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

                    return resolutionsWeight;

                }

                private double GetBlend(double size, double min, double max)
                {
                    var frac = (size - min) / (max - min);
                    if (frac < 0.25)
                    {
                        return 0;
                    }
                    if (frac > 0.75)
                    {
                        return 1;
                    }

                    var x = 2 * frac - 0.5;
                    return x * x * (3 - 2 * x);
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
                }*/




    }
}
