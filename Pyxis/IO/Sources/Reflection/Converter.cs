using Pyxis.Core.IO.GeoJson;

namespace Pyxis.IO.Sources.Reflection
{
    /// <summary>
    /// Provides the ability to register Pyxis.IO.Reflection.IFeatureConverter implementations and convert objects using registered feature converters.
    /// </summary>
    public static class Converter
    {
        /// <summary>
        /// Convert an object into Pyxis.IO.GeoJson.Feature.
        /// </summary>
        /// <typeparam name="T">Type of the object.</typeparam>
        /// <param name="obj">The object to convert.</param>
        /// <returns>Pyxis.IO.GeoJson.Feature object or throw an exception if object type can not be converted.</returns>
        public static Feature Convert<T>(T obj)
        {
            return TypeConversionRepository.GetConverter<T>().Convert(obj);
        }

        /// <summary>
        /// Register custom feature converter for a specific type.
        /// </summary>
        /// <typeparam name="T">Type to register.</typeparam>
        /// <param name="converter">Converter class to use.</param>
        public static void Register<T>(IFeatureConverter converter) 
        {
            TypeConversionRepository.RegisterType<T>(converter);
        }
    }
}
