using System.Linq;

namespace Pyxis.UI.Layers.Globe
{
    /// <summary>
    /// Describes the position of a camera viewing the globe.
    /// </summary>
    public static class CameraExtensions
    {
        internal static Contract.Publishing.Camera ToCamera(this Camera camera)
        {
            var settings = view_model_swig.camToCookieStr(camera).Split(' ').Select(double.Parse).ToArray();

            return new Contract.Publishing.Camera
            {
                Latitude = settings[0],
                Longitude = settings[1],
                Heading = settings[2],
                Altitude = settings[3],
                Tilt = settings[4],
                Range = settings[5]
            };
        }

        internal static Camera ToPYXCamera(this Contract.Publishing.Camera camera)
        {
            var pyxCamera = new Camera();
            view_model_swig.camFromSettings(pyxCamera, camera.Latitude, camera.Longitude, camera.Heading, camera.Altitude, camera.Tilt, camera.Range);
            return pyxCamera;
        }

      
        /// <summary>
        /// Gets an instance of a Pyxis.UI.Layers.Globe.Camera at a default position.
        /// </summary>
        public static Contract.Publishing.Camera Default
        {
            get
            {
                return new Contract.Publishing.Camera() { Range = 13300000 };
            }
        }
    }
}
