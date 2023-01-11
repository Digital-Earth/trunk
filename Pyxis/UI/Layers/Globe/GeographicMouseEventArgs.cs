using System;
using System.Windows.Forms;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;

namespace Pyxis.UI.Layers.Globe
{
    /// <summary>
    /// Provides geographic data about mouse events on the globe.
    /// </summary>
    public class GeographicMouseEventArgs : MouseEventArgs
    {
        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Globe.GeographicMouseEventArgs class.
        /// </summary>
        /// <param name="mouseEvent">System.Windows.Forms.MouseEventArgs providing mouse event data.</param>
        /// <param name="index">PYXIS index of the geographic event.</param>
        public GeographicMouseEventArgs(MouseEventArgs mouseEvent, string index)
            : base(mouseEvent.Button, mouseEvent.Clicks, mouseEvent.X, mouseEvent.Y, mouseEvent.Delta)
        {
            if (!String.IsNullOrEmpty(index))
            {
                CellOnGlobe = new CellGeometry { Index = index };
            }
        }

        /// <summary>
        /// Gets or sets the location of the cell on the globe where the event occurred.
        /// </summary>
        public CellGeometry CellOnGlobe { get; private set; }

        /// <summary>
        /// Gets the corresponding Pyxis.Core.IO.GeoJson.GeographicPosition of the cell.
        /// </summary>
        public GeographicPosition PointOnEarth
        {
            get
            {
                return CellOnGlobe!=null?CellOnGlobe.AsGeographicPosition():null;
            }
        }
    }
}
