using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PyxNet
{
    public interface IPyxNetStackConfiguration
    {
        /// <summary>
        /// Get the node ID.
        /// </summary>
        /// <returns>The node ID.</returns>
        Guid GetNodeID();

        /// <summary>
        /// Get the local private key.
        /// </summary>
        /// <returns>The private key.</returns>
        PyxNet.DLM.PrivateKey GetPrivateKey();

        /// <summary>
        /// Get the stack name eg: "PyxNet"
        /// </summary>
        /// <returns>stack name</returns>
        String GetDefaultStackName();

        /// <summary>
        /// Get predefined External IPEndPoints
        /// </summary>
        /// <returns>List of IPEndPoint</returns>
        List<System.Net.IPEndPoint> GetExternalIPEndPoints();
    }
}
