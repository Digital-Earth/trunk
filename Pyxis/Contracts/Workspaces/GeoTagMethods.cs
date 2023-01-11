using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace Pyxis.Contract.Workspaces
{
    public class GeoTagMethods
    {
        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)] 
        public GeoTagByLatLon LatLon { get; set; }

        [JsonProperty(NullValueHandling = NullValueHandling.Ignore)]
        public GeoTagByLookup Lookup { get; set; }
    }

    public class GeoTagByLatLon
    {
        /// <summary>
        /// Spatial Reference Systems
        /// </summary>
        public string Srs { get; set; }

        /// <summary>
        /// Gets or sets the name for latitude field.
        /// </summary>
        public string Latitude { get; set; }

        /// <summary>
        /// Gets or sets the name for longitude field.
        /// </summary>
        public string Longitude { get; set; }

        /// <summary>
        /// Pyxis Resolution to use. the default resolution is 24.
        /// </summary>
        public int Resolution { get; set; }
    }

    public class GeoTagByLookup
    {
        /// <summary>
        /// Reference of the Dataset to use for lookup
        /// </summary>
        public string Reference { get; set; }

        /// <summary>
        /// Reference field name to be indexed as a lookup field from the reference GeoSource.
        /// </summary>
        public string SourceField { get; set; }

        /// <summary>
        /// The field to be used as a lookup on the imported RecordCollection.
        /// </summary>
        public string DestinationField { get; set; }
    }
}
