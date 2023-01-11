using System.Net;
using Pyxis.Contract;

namespace Pyxis.IO.Sources.Remote
{
    /// <summary>
    /// Represents a permission for accessing an external resource with network credentials.
    /// </summary>
    public interface INetworkPermit: IPermit
    {
        /// <summary>
        /// Network credentials for accessing the external resource
        /// </summary>
        NetworkCredential Credentials { get; }
    }
}
