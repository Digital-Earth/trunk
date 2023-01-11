using System.Collections.Generic;
using Pyxis.Contract.Publishing;

namespace Pyxis.IO.MultiDomain
{
    /// <summary>
    /// An interface to allow clients resolve domain into a GeoSource object
    /// </summary>
    public interface IMultiDomainGeoSourceResolver
    {
        /// <summary>
        /// Get list of available domains
        /// </summary>
        List<Domain> Domains { get; }

        /// <summary>
        /// Resolve a given set of domain and values into a geoSource object
        /// </summary>
        /// <param name="domains">Dictionary of domain name and domain valu.</param>
        /// <returns>resolve GeoSource object or throw an exception</returns>
        ResourceReference Resolve(Dictionary<string, object> domains);
    }
}