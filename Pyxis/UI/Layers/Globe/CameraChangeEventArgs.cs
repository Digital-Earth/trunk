using System;

namespace Pyxis.UI.Layers.Globe
{
    /// <summary>
    /// Provides camera change event data.
    /// </summary>
    public class CameraChangeEventArgs : EventArgs
    {
        /// <summary>
        /// Gets the Pyxis.UI.Layers.Globe.Camera.
        /// </summary>
        public Contract.Publishing.Camera Camera { get; private set; }

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Globe.CameraChangeEventArgs class.
        /// </summary>
        /// <param name="camera">The Pyxis.UI.Layers.Globe.Camera when the event occurred.</param>
        public CameraChangeEventArgs(Contract.Publishing.Camera camera)
        {
            Camera = camera;
        }
    }
}
