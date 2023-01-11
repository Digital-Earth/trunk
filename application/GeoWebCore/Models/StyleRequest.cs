using Pyxis.Contract.Publishing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCore.Models
{
    /// <summary>
    /// Request to Generate a valid style for a GeoSource. Request can include style suggestion to be used if possible
    /// </summary>
    public class StyleRequest
    {
        /// <summary>
        /// GeoSource to style
        /// </summary>
        public GeoSource GeoSource { get; set; }

        /// <summary>
        /// Style to use if no default style has been specified for the GeoSource
        /// </summary>
        public Style Style { get; set; }
    }
}
