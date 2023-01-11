using System;

namespace PyxNet.Test
{
    /// <summary>
    /// Helper for unit tests.
    /// </summary>
    public static class NodeInfoHelper
    {
        /// <summary>
        /// Creates a (random) node info with the appropriate settings.  
        /// (Don't even think of using this in production code!)
        /// </summary>
        /// <param name="m"></param>
        /// <param name="hubCount"></param>
        /// <param name="leafCount"></param>
        /// <returns></returns>
        static public NodeInfo CreateNodeInfo(NodeInfo.OperatingMode m,
            int hubCount, int leafCount)
        {
            NodeInfo info = new NodeInfo();
            info.Mode = m;
            info.HubCount = hubCount;
            info.LeafCount = leafCount;
            info.NodeGUID = Guid.NewGuid();
            info.PublicKey = new PyxNet.DLM.PrivateKey().PublicKey;
            return info;
        }
    }
}