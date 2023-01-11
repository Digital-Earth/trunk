namespace Pyxis.IO.Import.GeoTagging
{
    /// <summary>
    /// Base interface that represents a geo tag method of a record collection into a feature collection.
    /// </summary>
    public interface IGeoTagMethod
    {
        /// <summary>
        /// The GeoTag method.
        /// </summary>
        /// <param name="recordCollectionProcess">The process that provides the record collection.</param>
        /// <returns>The geo tag process.</returns>
        IProcess_SPtr GeoTag(IProcess_SPtr recordCollectionProcess);
    }
}
