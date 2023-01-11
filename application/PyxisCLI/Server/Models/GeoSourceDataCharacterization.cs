using Pyxis.Core.IO.GeoJson.Specialized;

namespace PyxisCLI.Server.Models
{
    /// <summary>
    /// General Characterization of the data of a given GeoSource
    /// </summary>
    public class GeoSourceDataCharacterization
    {
        /// <summary>
        /// estimation what is the spatial resolution of the data based on Pyxis DERM resolution 
        /// </summary>
        public int NativeResolution { get; set; }
                
        /// <summary>
        /// bounding circle of the data
        /// </summary>
        public CircleGeometry BoundingCircle { get; set; }
    }
}
