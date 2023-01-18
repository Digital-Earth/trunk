using System;
using System.Collections.Generic;
using System.Linq;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Query;
using Pyxis.Core.DERM;
using Pyxis.Core.IO;
using Pyxis.Core.IO.Core;
using Pyxis.Core.IO.DGGS;
using Pyxis.Core.IO.GeoJson;
using PyxisCLI.Server.Utilities;
using PyxisCLI.Server.WebConfig;

namespace PyxisCLI.Server.Controllers
{

    [RoutePrefix("api/v1/Dggs")]
    public class DggsController : ApiController
    {

        DGGS isea3hPYXIS = new DGGS()
        {
            Id = "isea3h-pyxis",
            Title = "PyxisISEA3H DGGS",
            Description = "The PYXIS DGGS uses the Snyder Equal Area Aperture 3 Hexagon (ISEA3H) DGGS to partition the Earth into a tessellation of nested hexagonal cells.",
            Links = new Link[] { new Link()
        {
            Href = "https:digitalearthsolutions/api/v1/dggs/isea3h-pyxis",
            Rel = "self",
            Type = "application/json"
        }
},
            Uri = "http://www.opengis.net/def/dggs/OGC/1.0/isea3h-pyxis",
            Crs = "http://www.opengis.net/def/crs/Snyder"
        };

        DGGS isea3hRhombus = new DGGS()
        {
            Id = "isea3h-rhombus",
            Title = "Rhombus ISEA3H DGGS",
            Description = "The Rhombus DGGS uses the Snyder Equal Area Aperture 3 Hexagon (ISEA3H) DGGS to partition the icosahedron faces into rhombus tiles.",
            Links = new Link[] { new Link()
        {
            Href = "https:digitalearthsolutions/api/v1/dggs/isea3h-rhombus",
            Rel = "self",
            Type = "application/json"
        } },
            Uri = "http://www.opengis.net/def/dggs/OGC/1.0/isea3h-rhombus",
            Crs = "http://www.opengis.net/def/crs/Snyder"
        };

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

        [Route("")]
        [HttpGet]
        public DGGSs Dggss()
        {

            Link link1 = new Link()
            {
                Href = "https:digitalearthsolutions/api/v1/dggs",
                Rel = "self",
                Type = "application/json"
            };

            Link[] links = new Link[] { link1 };

            List<DGGS> dggssList = new List<DGGS>();
            dggssList.Add(isea3hPYXIS);
            dggssList.Add(isea3hRhombus);   

            DGGSs result = new DGGSs()
            {
                Dggs = dggssList.ToArray(),
                Links = links
            };

            return result;
        }

        [Route("{dggsId}")]
        [HttpGet]
        public DGGS Dggs(string dggsId)
        {
            if (dggsId == "isea3h-pyxis")
            {
                return isea3hPYXIS;
            }

            if (dggsId == "isea3h-rhombus")
            {
                return isea3hRhombus;
            }

            return null;

        }

        [Route("{dggsId}/Zones")]
        [HttpGet]
        public List<string> Roots(string dggsId)
        {
            if (dggsId == "isea3h-pyxis")
            {
                return Derm.PrimeCells().Select(cell => cell.Index).ToList();
            }

            if (dggsId == "isea3h-rhombus")
            {
                return Enumerable.Range(0, 10).Select(i => i.ToString()).ToList();
            }


            return null;
            
        }

        [Route("{dggsId}/Zones/{index}")]
        [HttpGet]
        public object Cell(string dggsId,string index)
        {

            if (dggsId == "isea3h-pyxis")
            {
                var cell = Derm.Cell(index);

                return new
                {
                    Center = cell.Center,
                    Radius = cell.Radius.InMeters,
                    Index = cell.Index,
                };
            }

            if (dggsId == "isea3h-rhombus")
            {
                if (RhombusHelper.IsTopRhombus(index))
                {
                    return new
                    {
                        Resolution = "0",
                        Index = index,
                    };
                }
                var rhombus = RhombusHelper.GetRhombusFromKey(index);
                return new
                {
                    Center = rhombus.getIndex(0).ToString(),
                    Resolution = rhombus.getIndex(0).getResolution(),
                    Index = index,
                };
            }


            return null;
            
        }

        [Route("Cell")]
        [HttpGet]
        public List<string> Roots()
        {
            return Derm.PrimeCells().Select(cell => cell.Index).ToList();
        }


        [Route("Cell")]
        [HttpGet]
        public string Cell2(double latitude, double longitude, int resolution)
        {
            var cell = Derm.Cell(new GeographicPosition()
            {
                Latitude = latitude,
                Longitude = longitude
            }, resolution);
            return cell.Index;
        }

        [Route("Cell")]
        [HttpGet]
        public string Cell3(double latitude,double longitude, double radius)
        {
            var resolution = Derm.ResolutionFromDistance(radius);
            return Cell2(latitude, longitude, resolution);
        }

        [Route("Cell/{index}")]
        [HttpGet]
        public object Cell(string index)
        {
            var cell = Derm.Cell(index);

            return new
            {
                Center = cell.Center,
                Radius = cell.Radius.InMeters,
                Index = cell.Index,
            };
        }

        [Route("Cell/{index}/Children")]
        [HttpGet]
        public List<string> Children(string index)
        {
            var cell = Derm.Cell(index);

            return cell.Children().Select(c => c.Index).ToList();
        }

        [Route("Cell/{index}/Neighbors")]
        [HttpGet]
        public List<string> Neighbors(string index)
        {
            var cell = Derm.Cell(index);

            return cell.Neighbors().Select(c => c.Index).ToList();
        }

        [Route("Cell/{index}/Parent")]
        [HttpGet]
        public string Parent(string index)
        {
            var cell = Derm.Cell(index);

            return cell.Parent().Index;
        }

        [Route("Cell/{index}/Geometry")]
        [HttpGet]
        public IGeometry Geometry(string index)
        {
            var polygon = new PolygonGeometry()
            {
                Coordinates = new List<List<GeographicPosition>>()
            };
            var cell = Derm.Cell(index);

            var ring = cell.Vertices().Select(c => c.Center).ToList();            
            ring.Add(ring[0]); //close ring

            polygon.Coordinates.Add(ring);

            return polygon;
        }
    }
}