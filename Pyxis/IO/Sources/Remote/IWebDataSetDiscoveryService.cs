using Pyxis.IO.DataDiscovery;

namespace Pyxis.IO.Sources.Remote
{
    /// <summary>
    /// Interface class for web-based data servers (e.g. OGC, ArcGIS)
    /// </summary>
    public interface IWebDataSetDiscoveryService : IDataSetDiscoveryService
    {
        /// <summary>
        /// Gets the service identifier.  This is the (machine readable) text that identifies this service.
        /// </summary>
        /// <value>The service identifier.</value>
        string ServiceIdentifier { get; }
    }
}
