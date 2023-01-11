using System.Collections.Generic;

namespace Pyxis.IO.Sources.OGC
{
    /// <summary>
    /// Interface for a class for an attribute of an URL of a OGC server or a resource on it
    /// </summary>
    public interface IOgcUrlAttribute
    {
        /// <summary>
        /// Name of the attribute (case insensitive)
        /// </summary>
        string Name { get; }

        /// <summary>
        /// Value of the attribute
        /// </summary>
        string Value { get; }
    }

    /// <summary>
    /// Interface for a class for a URL of a OGC server or a resource on it
    /// </summary>
    public interface IOgcUrl
    {
        /// <summary>
        /// URL of the server (contains the protocol, domain name and constant attributes)
        /// </summary>
        string ServerUrl { get; }

        /// <summary>
        /// Identifier of the related OGC service
        /// </summary>
        string Service { get; }

        /// <summary>
        /// OGC version of the server
        /// </summary>
        string Version { get; set; }

        /// <summary>
        /// Type of a OGC request that this URL represents
        /// </summary>
        string Request { get; set; }

        /// <summary>
        /// Attributes of the URL that may vary
        /// </summary>
        List<IOgcUrlAttribute> Attributes { get; }

        /// <summary>
        /// Converts the object data to a URL string
        /// </summary>
        string ToString();
    }
}
