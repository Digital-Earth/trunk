using Newtonsoft.Json;
using Pyxis.Contract.Publishing;
using Pyxis.Core.IO;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Core.Analysis
{
    /// <summary>
    /// A query to search over fields in a Pyxis.Contract.Publishing.GeoSource.
    /// </summary>
    public class FeaturesSearchQuery
    {
        /// <summary>
        /// Gets or sets the Pyxis.Contract.Publishing.GeoSource whose fields are to be searched.
        /// </summary>
        [JsonProperty("geoSource")]
        public GeoSource GeoSource { get; set;}

        /// <summary>
        /// Gets or sets the names of the fields to search.
        /// </summary>
        [JsonProperty("fields")]
        public List<string> FieldsToSearch { get; set; }

        /// <summary>
        /// Gets or sets the search string.
        /// </summary>
        [JsonProperty("search")]
        public string SearchString { get; set; }

        /// <summary>
        /// Gets or sets the Pyxis.Core.IO.IGeometry to specify the area of interest for the search.
        /// </summary>
        [JsonProperty("geometry")]
        public IGeometry Geometry { get; set; }

        /// <summary>
        /// Gets or sets the number of Pyxis.Core.IO.GeoJson.Feature to skip.
        /// </summary>
        [JsonProperty("skip")]
        public int? Skip { get; set; }

        /// <summary>
        /// Gets or sets the number of Pyxis.Core.IO.GeoJson.Feature to take.
        /// </summary>
        [JsonProperty("take")]
        public int? Take { get; set; }
    }
}
