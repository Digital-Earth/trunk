using GeoWebCore.Services.Cache;
using Pyxis.Core.IO;

namespace GeoWebCore.Services.Cluster
{
    public class GeometryClusterResolver : IClusterResolver
    {
        private Cluster m_cluster;
        private string m_host;
        private IGeometry m_geometry;

        public GeometryClusterResolver(Cluster cluster, string geomtryHash)
        {
            m_cluster = cluster;
            Key = geomtryHash;
            m_host = cluster.GetEndpointForGeometry(Key);
            m_geometry = GeometryCacheSingleton.Get(Key);
        }

        public string Key { get; private set; }

        public bool IsLocal
        {
            get { return m_cluster.IsLocal(m_cluster.ServersRing, m_host); }
        }

        public bool IsReady { get; private set; }

        public bool IsFaulted { get; private set; }

        public PYXGeometry_SPtr GetPYXGeometry()
        {
            return GeoSourceInitializer.Engine.ToPyxGeometry(m_geometry);
        }
    }
}