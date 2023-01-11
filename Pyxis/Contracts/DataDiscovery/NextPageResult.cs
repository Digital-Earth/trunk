using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Contract.DataDiscovery
{
    /// <summary>
    /// Represents the next page in a paged result set.
    /// </summary>
    public class NextPageResult
    {
        /// <summary>
        /// Uri of the next page.
        /// </summary>
        public string Uri { get; set; }

        /// <summary>
        /// Total number of results in the set.
        /// </summary>
        public int? TotalResults { get; set; }
    }
}
