using System;
using System.Collections.Generic;
using System.Linq;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Query;
using Pyxis.Core.DERM;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using PyxisCLI.Server.WebConfig;

namespace PyxisCLI.Server.Controllers
{
    [RoutePrefix("api/v1/Dggs")]
    public class DggsController : ApiController
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

        [Route("Cell")]
        [HttpGet]
        public List<string> Roots()
        {
            return Derm.PrimeCells().Select(cell => cell.Index).ToList();
        }


        [Route("Cell")]
        [HttpGet]
        public string Cell(double latitude, double longitude, int resolution)
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
        public string Cell(double latitude,double longitude, double radius)
        {
            var resolution = Derm.ResolutionFromDistance(radius);
            return Cell(latitude, longitude, resolution);
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