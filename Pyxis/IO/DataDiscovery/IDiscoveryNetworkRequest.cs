using System.Threading.Tasks;
using Pyxis.Contract;

namespace Pyxis.IO.DataDiscovery
{
    public interface IDiscoveryNetworkRequest
    {
        string Uri { get; }

        IPermit Permit { get; }

        Task<IDiscoveryNetworkResult> SendAsync();
    }

    public interface IDiscoveryNetworkResult
    {
        IDiscoveryNetworkRequest Request { get; }
    }

}