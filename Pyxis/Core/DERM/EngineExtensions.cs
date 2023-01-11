namespace Pyxis.Core.DERM
{
    /// <summary>
    /// Engine extensions for the DERM.
    /// </summary>
    public static class EngineExtensions
    {
        /// <summary>
        /// Provide access to Pyxis DERM using the given Pyxis.Core.Engine to retrieve values
        /// </summary>
        /// <param name="engine">Engine to use to retrieve values from GeoSources.</param>
        /// <returns>Derm object</returns>
        public static Derm DERM(this Engine engine)
        {
            return new Derm(engine);
        }
    }
}
