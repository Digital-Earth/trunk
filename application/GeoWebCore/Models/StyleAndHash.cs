using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Pyxis.Contract.Publishing;

namespace GeoWebCore.Models
{
    /// <summary>
    /// repsonse model for styles to include style hash to be used
    /// </summary>
    public class StyleAndHash
    {
        /// <summary>
        /// Style
        /// </summary>
        public Style Style { get; set; }

        /// <summary>
        /// If present, this hash can be used to active this style
        /// </summary>
        public string Hash { get; set; }
    }
}
