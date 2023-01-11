using System;
using System.Collections.Generic;
using System.Web.Http;
using GeoWebCore.Models;
using GeoWebCore.Utilities;
using System.Text;
using System.Net.Http;
using GeoWebCore.WebConfig;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// Controller that generate vertices of a PYXIS rhombus
    /// </summary>
    [RoutePrefix("api/v1/Rhombus")]
    [Obsolete("client now generate meshses locally")]
    public class MeshController : ApiController
    {
        /// <summary>
        /// Generate a Mesh for a given Rhombus
        /// </summary>
        /// <param name="key">Rhombus key</param>
        /// <param name="size">Mesh size (need to be a valid Rhombus size)</param>
        /// <returns>Mesh model (vertices latice)</returns>
        [HttpGet]
        [Route("Mesh")]
        [ApiCache]
        [TimeTrace("key,size")]
        public Mesh Mesh(string key, int size)
        {
            if (!RhombusHelper.IsValidSize(size))
            {
                return null;
            }

            var mesh = new Mesh
            {
                Vertices = new double[size * size][]
            };

            double[] verticies = RhombusHelper.GetRhombusVerticies(key, size);
            for (var i = 0; i < mesh.Vertices.Length; i++)
            {
                mesh.Vertices[i] = new[] { verticies[3*i + 0], verticies[3*i + 1], verticies[3*i + 2] };
            }

            return mesh;
        }

        /// <summary>
        /// Create a rhombus geomerty using obj file syntax
        /// 
        /// https://en.wikipedia.org/wiki/Wavefront_.obj_file
        /// </summary>
        /// <param name="key">The rhombus key</param>
        /// <param name="size">The size of the mesh to create</param>
        /// <returns>Rhombus geometry in obj file format</returns>
        [HttpGet]
        [Route("Obj")]
        [TimeTrace("key,size")]
        public HttpResponseMessage ObjMesh(string key, int size)
        {
            var stringBuilder = new StringBuilder();

            if (!RhombusHelper.IsValidSize(size))
            {
                return null;
            }

            double[] verticies = RhombusHelper.GetRhombusVerticies(key, size);
            for (var i = 0; i < verticies.Length; i += 3)
            {
                stringBuilder.AppendFormat("v {0} {1} {2}\n", verticies[i + 0], verticies[i + 1], verticies[i + 2]);
            }

            for (var i = 0; i < size - 1; i++)
            {
                for (var j = 0; j < size - 1; j++)
                {
                    stringBuilder.AppendFormat("f {0} {1} {2}\n", (j * size + i) + 1, (j * size + i + 1) + 1, ((j + 1) * size + i) + 1);
                    stringBuilder.AppendFormat("f {0} {1} {2}\n", ((j + 1) * size + i + 1) + 1, ((j + 1) * size + i) + 1, (j * size + i + 1) + 1);
                }
            }

            var response = new HttpResponseMessage(System.Net.HttpStatusCode.OK)
            {
                Content = new StringContent(stringBuilder.ToString())
            };
            response.AddCache(Request);
            return response;
        }
    }
}