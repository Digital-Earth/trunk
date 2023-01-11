using System;
using System.Collections.Generic;
using System.Net.Http;
using System.Web.Http;
using GeoWebCore.Models;
using GeoWebCore.Utilities;
using GeoWebCore.WebConfig;

namespace GeoWebCore.Controllers
{
    /// <summary>
    /// Controller that generate three.js geometries for PYXIS rhombus
    /// </summary>
    [RoutePrefix("api/v1/Rhombus")]
    public class ThreeJsGeometryController : ApiController
    {
        /// <summary>
        /// Return a valid ThreeJS geometry object for the given rhombus.
        /// </summary>
        /// <param name="key">rhombus key</param>
        /// <param name="size">rhombus size</param>
        /// <returns>ThreeJsGeometry geometry object</returns>
        [HttpGet]
        [Route("Geometry")]
        [ApiCache]
        [TimeTrace("key,size")]
        public ThreeJsGeometry Geometry(string key, int size)
        {
            if (!RhombusHelper.IsValidSize(size))
            {
                return null;
            }

            double[] vertices = RhombusHelper.GetRhombusVerticies(key, size);
            ThreeJsGeometry threeJsGeometry = CreateThreeJsGeometryFromVerticesQuad(size, vertices);

            return threeJsGeometry;
        }

        private ThreeJsGeometry CreateThreeJsGeometryFromVerticesQuad(int size, double[] vertices)
        {
            var threeJsGeometry = new ThreeJsGeometry();

            int[] fs = new int[(size - 1)*(size - 1)*13]; //new List<int> ();
            int fi = 0;

            for (int i = 0; i < size - 1; i++)
            {
                for (int j = 0; j < size - 1; j++)
                {
                    //Three JS face format
                    //https://github.com/mrdoob/three.js/wiki/JSON-Model-format-3
                    fs[fi++] = 41; //QUAD | FACE_UV | FACE_VERTEX_NORMAL
                    fs[fi++] = (j*size + i);
                    fs[fi++] = (j*size + i + 1);
                    fs[fi++] = ((j + 1)*size + i + 1);
                    fs[fi++] = ((j + 1)*size + i);
                    fs[fi++] = (j*size + i);
                    fs[fi++] = (j*size + i + 1);
                    fs[fi++] = ((j + 1)*size + i + 1);
                    fs[fi++] = ((j + 1)*size + i);
                    fs[fi++] = (j*size + i);
                    fs[fi++] = (j*size + i + 1);
                    fs[fi++] = ((j + 1)*size + i + 1);
                    fs[fi++] = ((j + 1)*size + i);
                }
            }

            float[] uvs = new float[size*size*2];
            int uvi = 0;
            for (int i = 0; i < size; i++)
            {
                for (int j = 0; j < size; j++)
                {
                    uvs[uvi++] = (j/(float) (size - 1));
                    uvs[uvi++] = 1.0f - (i/(float) (size - 1));
                }
            } //^ uv correction?

            threeJsGeometry.Vertices = vertices;
            threeJsGeometry.Normals = vertices;

            threeJsGeometry.Faces = fs;
            threeJsGeometry.Uvs = new[] {uvs};
            threeJsGeometry.Metadata.Uvs = size*size;
            threeJsGeometry.Metadata.Vertices = size*size;
            threeJsGeometry.Metadata.Faces = (size - 1)*(size - 1);
            return threeJsGeometry;
        }
    }
}