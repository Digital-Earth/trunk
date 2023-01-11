using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PyxNet
{
    public class PyxNetStackDefaultConfiguration : IPyxNetStackConfiguration
    {
        public PyxNetStackDefaultConfiguration()
        {
        }

        #region IPyxNetStackConfiguration Members

        public virtual Guid GetNodeID()
        {
            Guid myNodeGuid = new Guid();

            String myNodeIdInitializer = Context.NodeID;

            bool generateNewID = false;
            if (myNodeIdInitializer.Trim().Length == 0)
            {
                generateNewID = true;
            }
            else
            {
                try
                {
                    String xmlBuffer = myNodeIdInitializer.Trim().Trim('\0');
                    System.IO.StringReader input = new System.IO.StringReader(xmlBuffer);
                    System.Xml.Serialization.XmlSerializer s =
                        new System.Xml.Serialization.XmlSerializer(myNodeGuid.GetType());
                    myNodeGuid = (Guid)s.Deserialize(input);
                }
                catch
                {
                    // if anything went wrong with deserializing the NodeId, then we
                    // will just generate a new one.
                    generateNewID = true;
                }
            }

            if (generateNewID)
            {
                myNodeGuid = Guid.NewGuid();

                System.IO.StringWriter outputStream = new System.IO.StringWriter(
                    new System.Text.StringBuilder());
                System.Xml.Serialization.XmlSerializer s =
                    new System.Xml.Serialization.XmlSerializer(myNodeGuid.GetType());
                s.Serialize(outputStream, myNodeGuid);
                Context.NodeID = outputStream.ToString();
                Context.Save();
            }

            return myNodeGuid;
        }

        public virtual DLM.PrivateKey GetPrivateKey()
        {
            // Read the private key container name from Properties.Settings.Default.Settings,
            // convert to key, and set in the stack.
            // Generate (and save to settings) if not already there.
            String privateKeyContainerName = Context.PrivateKey;
            if (privateKeyContainerName.Length == 0)
            {
                privateKeyContainerName = "PyxNet";
            }

            PyxNet.DLM.PrivateKey privateKey = new PyxNet.DLM.PrivateKey(privateKeyContainerName);
            return privateKey;
        }

        public virtual string GetDefaultStackName()
        {
            return Context.StackName;
        }

        public virtual List<System.Net.IPEndPoint> GetExternalIPEndPoints()
        {
            return IPAddressAndPort.DeserializeIPEndPoints(Context.ExternalAddresses);
        }

        #endregion
    }
}
