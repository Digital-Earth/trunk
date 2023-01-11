using Newtonsoft.Json;

namespace GeoWebCore.Models
{
    /// <summary>
    /// Represnt the ThreeJs geometry format
    /// </summary>
    public class ThreeJsGeometry
    {
        /// <summary>
        /// Create a ThreeJsGeometry (version 3.1)
        /// </summary>
        public ThreeJsGeometry()
        {
            Metadata.Version = 3.1f;
            Metadata.Type = "geometry";
            Metadata.Generator = "Pyxis";
            Vertices = new double[0];
            Normals = new double[0];
            // uvs = new float[1][]{new float[0]};
            Faces = new int[0];
            Colors = new float[0];
            Materials = new MaterialStruct[1];
            Metadata.Materials = 1;
            Materials[0] = new MaterialStruct
            {
                ColorDiffuse = new[] {1f, 1f, 1f},
                Shading = "Phong"
            };
        }

        /// <summary>
        /// ThreeJs MaterialStruct
        /// </summary>
        public struct MaterialStruct
        {
            /// <summary>
            /// Diffuse color
            /// </summary>
            [JsonProperty(PropertyName = "colorDiffuse")] 
            public float[] ColorDiffuse;

            /// <summary>
            /// Shading
            /// </summary>
            [JsonProperty(PropertyName = "shading")] 
            public string Shading;
        }

        /// <summary>
        /// ThreeJs MetaDataStruct
        /// </summary>
        public struct MetaDataStruct
        {
            /// <summary>
            /// Version
            /// </summary>
            [JsonProperty(PropertyName = "version")] 
            public float Version;

            /// <summary>
            /// Type
            /// </summary>
            [JsonProperty(PropertyName = "type")] 
            public string Type;

            /// <summary>
            /// Generator
            /// </summary>
            [JsonProperty(PropertyName = "generator")] 
            public string Generator;

            /// <summary>
            /// Materials
            /// </summary>
            [JsonProperty(PropertyName = "materials")] 
            public int Materials;

            /// <summary>
            /// Faces
            /// </summary>
            [JsonProperty(PropertyName = "faces")] 
            public int Faces;

            /// <summary>
            /// Vertices
            /// </summary>
            [JsonProperty(PropertyName = "vertices")] 
            public int Vertices;

            /// <summary>
            /// UV coodinates
            /// </summary>
            [JsonProperty(PropertyName = "uvs")] 
            public int Uvs;
        };

        /// <summary>
        /// Metadata
        /// </summary>
        [JsonProperty(PropertyName = "metadata")] 
        public MetaDataStruct Metadata;

        /// <summary>
        /// Scale
        /// </summary>
        [JsonProperty(PropertyName = "scale")] 
        public float Scale = 1f;

        /// <summary>
        /// Materials
        /// </summary>
        [JsonProperty(PropertyName = "materials")] 
        public MaterialStruct[] Materials;

        /// <summary>
        /// Vertices
        /// </summary>
        [JsonProperty(PropertyName = "vertices")] 
        public double[] Vertices;

        /// <summary>
        /// Normals
        /// </summary>
        [JsonProperty(PropertyName = "normals")] 
        public double[] Normals;

        /// <summary>
        /// Colors
        /// </summary>
        [JsonProperty(PropertyName = "colors")] 
        public float[] Colors;

        /// <summary>
        /// UVs
        /// </summary>
        [JsonProperty(PropertyName = "uvs")] 
        public float[][] Uvs;

        /// <summary>
        /// Faces
        /// </summary>
        [JsonProperty(PropertyName = "faces")] 
        public int[] Faces;
    }
}