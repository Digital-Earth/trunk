using System;
using System.Collections.Generic;
using Pyxis.Contract;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Sources.OGC
{
    public abstract class OgcWebServiceBase : WebDataSetServiceBase
    {
        /// <summary>
        /// The category of the discovery service (corresponds to "DataType" in Catalog)
        /// </summary>
        public override string Category { get { return DataSetType.OGC.ToString(); } }

        /// <summary>
        /// Checks if a URI presumably represents a resource of this OGC kind.
        /// </summary>
        /// <param name="uri">URI of the resource</param>
        /// <returns>True if the url represents a resource of this kind, otherwise false.</returns>
        public override bool IsUriSupported(string uri)
        {
            if (string.IsNullOrEmpty(uri))
            {
                return false;
            }
            try
            {
                // May throw an exception if the URL value is incorrect
                var ogcUrl = new OgcInvariantUrl(uri);
                // Verify that creation of an OGC URL succeeded
                return ogcUrl.Service != null
                    // and the URL corresponds to this particular type of an OGC service
                       && ogcUrl.Service.Equals(ServiceIdentifier, StringComparison.InvariantCultureIgnoreCase)
                    // TODO: figure out why IsWellFormedUriString always returns false when a data set url is being verified
                    /*&& Uri.IsWellFormedUriString(query.ToString(), UriKind.Absolute)*/;
            }
            catch (Exception e)
            {
                Trace.error(e.Message);
                return false;
            }
        }

        /// <summary>
        /// Get the catalogs accessible from this uri (non-recursive).
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The catalogs</returns>
        public override List<DataSetCatalog> GetCatalogs(string uri, IPermit permit)
        {
            var catalog = BuildCatalog(uri, permit);
            if (catalog != null && catalog.DataSets != null)
            {
                return catalog.SubCatalogs;
            }
            return new List<DataSetCatalog>();
        }

        /// <summary>
        /// Get the data sets accessible from this uri.
        /// </summary>
        /// <param name="uri">The uri.</param>
        /// <param name="permit">The permit.</param>
        /// <returns>The data sets</returns>
        public override List<DataSet> GetDataSets(string uri, IPermit permit)
        {
            var catalog = BuildCatalog(uri, permit);
            if (catalog != null && catalog.DataSets != null)
            {
                return catalog.DataSets;
            }

            return new List<DataSet>();
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="title">Service description</param>
        /// <param name="identifier">Unique identifier of the service</param>
        protected OgcWebServiceBase(string title, string identifier)
        {
            Title = title;
            ServiceIdentifier = identifier;
        }
    }
}
