using Pyxis.Core.IO.GeoJson;

namespace Pyxis.IO.Sources.Reflection
{
    /// <summary>
    /// Exposes a conversion into JSON representation of a geometric feature.
    /// </summary>
    public interface IFeatureConverter
    {
        /// <summary>
        /// Convert a System.Object to a Pyxis.IO.GeoJson.Feature instance.
        /// </summary>
        /// <param name="obj">The System.Object to convert.</param>
        /// <returns>Pyxis.IO.GeoJson.Feature instance representing <paramref name="obj"/>.</returns>
        Feature Convert(object obj);
    }
}
