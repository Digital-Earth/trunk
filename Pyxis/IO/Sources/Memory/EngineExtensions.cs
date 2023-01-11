using System.Collections.Generic;
using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO.GeoJson;
using Pyxis.IO.Sources.Reflection;

namespace Pyxis.IO.Sources.Memory
{
    /// <summary>
    /// Extension methods for Pyxis.Core.Engine.
    /// </summary>
    public static class EngineExtensions
    {

        /// <summary>
        /// Creates a Pyxis.Contract.Publishing.GeoSource in memory created from a collection of items convertible to Pyxis.IO.GeoJson.Feature.
        /// </summary>
        /// <see cref="Converter"/>
        /// <see cref="Pyxis.Core.IO.GeoJson.Feature"/>
        /// <typeparam name="T">Type convertible to Pyxis.IO.GeoJson.Feature.</typeparam>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="items">Items to be converted to Pyxis.IO.GeoJson.Feature to create the Pyxis.Contract.Publishing.GeoSource.</param>
        /// <returns>The created Pyxis.Contract.Publishing.GeoSource.</returns>
        /// <exception cref="System.InvalidOperationException">The engine must be started.</exception>
        public static GeoSource CreateInMemory<T>(this Engine engine, IEnumerable<T> items)
        {
            engine.ValidateEngineState(CoreServiceState.Running, "Can't get process when engine is not running");

            return InMemoryGeoSourceCreator.Create(engine, items);
        }

        /// <summary>
        /// Creates a Pyxis.Contract.Publishing.GeoSource in memory created from a Pyxis.IO.GeoJson.FeatureCollection.
        /// </summary>
        /// <see cref="Pyxis.Core.IO.GeoJson.FeatureCollection"/>
        /// <param name="engine">Pyxis.Core.Engine object</param>
        /// <param name="featureCollection">The Pyxis.IO.GeoJson.FeatureCollection used to create the Pyxis.Contract.Publishing.GeoSource</param>
        /// <returns>The created Pyxis.Contract.Publishing.GeoSource.</returns>
        /// <exception cref="System.InvalidOperationException">The engine must be started.</exception>
        public static GeoSource CreateInMemory(
            this Engine engine,
            FeatureCollection featureCollection
            )
        {
            engine.ValidateEngineState(CoreServiceState.Running, "Can't get process when engine is not running");

            return InMemoryGeoSourceCreator.Create(engine, featureCollection);
        }
    }
}
