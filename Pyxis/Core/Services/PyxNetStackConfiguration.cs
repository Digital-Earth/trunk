using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Pyxis.Core.Services
{
    class PyxNetStackCustomConfiguration : PyxNet.IPyxNetStackConfiguration
    {
        public Guid NodeId { get; private set; }
        public string StackName { get; private set;}

        private PyxNet.PyxNetStackDefaultConfiguration m_defaultConfiguration { get; set; }

        public PyxNetStackCustomConfiguration()
        {
            m_defaultConfiguration = new PyxNet.PyxNetStackDefaultConfiguration();
        }

        public PyxNetStackCustomConfiguration(Guid nodeId)
            : this()
        {
            NodeId = nodeId;
        }

        public PyxNetStackCustomConfiguration(string stackName)
            : this()
        {
            StackName = stackName;
        }

        public PyxNetStackCustomConfiguration(Guid nodeId,string stackName)
            : this()
        {
            NodeId = nodeId;
            StackName = stackName;
        }

        #region IPyxNetStackConfiguration Members

        public Guid GetNodeID()
        {
            if (NodeId != Guid.Empty)
            {
                return NodeId;
            }
            return m_defaultConfiguration.GetNodeID();
        }

        public PyxNet.DLM.PrivateKey GetPrivateKey()
        {
            return m_defaultConfiguration.GetPrivateKey();
        }

        public string GetDefaultStackName()
        {
            if (!String.IsNullOrEmpty(StackName))
            {
                return StackName;
            }
            return m_defaultConfiguration.GetDefaultStackName();
        }
       
        public List<System.Net.IPEndPoint> GetExternalIPEndPoints()
        {
            return m_defaultConfiguration.GetExternalIPEndPoints();
        }

        #endregion       
        
    }
}
