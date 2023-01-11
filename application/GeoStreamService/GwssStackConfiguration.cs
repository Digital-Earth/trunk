using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GeoStreamService
{
    class GwssStackConfiguration : PyxNet.IPyxNetStackConfiguration
    {
        public PyxNet.PyxNetStackDefaultConfiguration m_defaultConfiguration { get; set; }

        public GwssStackConfiguration()
        {
            m_defaultConfiguration = new PyxNet.PyxNetStackDefaultConfiguration();
        }

        #region IPyxNetStackConfiguration Members

        public Guid GetNodeID()
        {
            if (Properties.Settings.Default.ServerId != Guid.Empty)
            {
                return Properties.Settings.Default.ServerId;
            }
            return m_defaultConfiguration.GetNodeID();
        }

        public PyxNet.DLM.PrivateKey GetPrivateKey()
        {
            return m_defaultConfiguration.GetPrivateKey();
        }

        public string GetDefaultStackName()
        {
            return m_defaultConfiguration.GetDefaultStackName();
        }

        public List<System.Net.IPEndPoint> GetExternalIPEndPoints()
        {
            //value should be 10.10.10.10:44017,11.11.11.11:44017
            var value = Properties.Settings.Default.ServerExternalIPAddress;

            var newAddresses = new List<System.Net.IPEndPoint>();

            if (!String.IsNullOrEmpty(value))
            {
                foreach (var address in value.Split(','))
                {
                    try
                    {
                        var parts = address.Split(':');
                        var ipEndpoint = new System.Net.IPEndPoint(System.Net.IPAddress.Parse(parts[0]), int.Parse(parts[1]));

                        newAddresses.Add(ipEndpoint);
                    }
                    catch(Exception ex)
                    {
                        //we failed to parse
                        GeoStreamService.WriteLine(GeoStreamService.TraceCategory.Error, "Failed to parse External Ip address '" + address + "' due to an error: " + ex.ToString());
                    }
                }
            }
            return m_defaultConfiguration.GetExternalIPEndPoints().Concat(newAddresses).ToList();
        }

        #endregion

        
    }
}
