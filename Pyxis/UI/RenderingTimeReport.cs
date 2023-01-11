using System.Collections.Generic;

namespace Pyxis.UI
{
    /// <summary>
    /// RenderingTimeReport provides an hierarchal view of the PyxisView rendering timing.       
    /// </summary>
    public class RenderingTimeReport
    {
        /// <summary>
        /// Gets or Sets the name of the rendering operation - for example, the ILayer name.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Time the render took in Milliseconds.
        /// </summary>
        public double RenderTimeInMilliseconds;

        /// <summary>
        /// If set, More detailed timing information.
        /// </summary>
        public List<RenderingTimeReport> Details;
    }
}
