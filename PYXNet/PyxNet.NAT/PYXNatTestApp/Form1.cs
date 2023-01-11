using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WindowsApplication1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void addItem(string strItem)
        {
            listBox1.Items.Add(strItem);
            listBox1.SelectedIndex = listBox1.Items.Count - 1;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int nPort = int.Parse(textBox1.Text);

            string strProtocol = this.comboBox1.Text;

            PYXNATTraversal.PYXAdapterInfo adapterInfo = new PYXNATTraversal.PYXAdapterInfo();

            string strIPAddress = null;
            if (adapterInfo != null)
            {
                strIPAddress = adapterInfo.getIPAddress();
            }

            PYXNATTraversal.PYXNatControl pyxNatControl = new PYXNATTraversal.PYXNatControl();

            if (pyxNatControl != null)
            {
                addItem("Adding Port Mapping...");

                NatTraversal.PortMappingInfo newPortMapping;

                System.Net.IPAddress ByteIPAddress = new System.Net.IPAddress(0x00000000);

                newPortMapping = new NatTraversal.PortMappingInfo("PYXNet TCPIP Port", strProtocol, strIPAddress, nPort, ByteIPAddress, nPort, true);

                pyxNatControl.OnPYXNatAddComplete += mappingComplete;

                pyxNatControl.AddPortMappingWithThread(newPortMapping);

            }
            else
            {
                addItem("Unable to create port mapper");
            }
        }

        private delegate void mappingCompleteDelegate(object sender, PYXNATTraversal.PYXNatControl.PYXNatFunctionCompleteEventArgs natProperty);

        private void mappingComplete(object sender, PYXNATTraversal.PYXNatControl.PYXNatFunctionCompleteEventArgs natProperty)
        {
            if (listBox1.InvokeRequired)
            {
                this.BeginInvoke( new mappingCompleteDelegate(mappingComplete), new object[] { sender, natProperty});
                return;
            }
            else
            {
                if (natProperty.PYXNatProperty.success)
                {
                    addItem("Add port completed successfully.");
                    addItem(natProperty.PYXNatProperty.ipAddress + ":" + natProperty.PYXNatProperty.port.ToString());
                }
                else
                {
                    addItem("Add port failed");
                }
            }
        }

        private delegate void removeMappingCompleteDelegate(object sender, PYXNATTraversal.PYXNatControl.PYXNatFunctionCompleteEventArgs natProperty);

        private void removeMappingComplete(object sender, PYXNATTraversal.PYXNatControl.PYXNatFunctionCompleteEventArgs natProperty)
        {
            if (listBox1.InvokeRequired)
            {
                this.BeginInvoke(new removeMappingCompleteDelegate(removeMappingComplete), new object[] { sender, natProperty });
                return;
            }
            else
            {
                if (natProperty.PYXNatProperty.success)
                {
                    addItem("Remove port mapping completed successfully.");
                }
                else
                {
                    addItem("Remove port failed.");
                }
            }
        }

        private void getMappingComplete(object sender, PYXNATTraversal.PYXNatControl.PYXNatFunctionCompleteEventArgs natProperty)
        {
            if (listBox1.InvokeRequired)
            {
                this.BeginInvoke(new mappingCompleteDelegate(getMappingComplete), new object[] { sender, natProperty });
                return;
            }
            else
            {
                if (natProperty.PYXNatProperty.success)
                {
                    addItem("Get Port Mapping completed successfully.");
                    string strEnabled = "OFF";
                    if (natProperty.PYXNatProperty.mappings[0].Enabled)
                    {
                        strEnabled = "ON";
                    }
                    addItem(natProperty.PYXNatProperty.mappings[0].ExternalIPAddress + ":" +
                        natProperty.PYXNatProperty.mappings[0].ExternalPort.ToString() + " to " + 
                        natProperty.PYXNatProperty.mappings[0].InternalHostName + ":" +
                        natProperty.PYXNatProperty.mappings[0].InternalPort.ToString() + " " +
                        natProperty.PYXNatProperty.mappings[0].Description + " " +
                        strEnabled);
                }
                else
                {
                    addItem("Get port mapping failed");
                }
            }
        }

        private void getMappingsComplete(object sender, PYXNATTraversal.PYXNatControl.PYXNatFunctionCompleteEventArgs natProperty)
        {
            if (listBox1.InvokeRequired)
            {
                this.BeginInvoke(new mappingCompleteDelegate(getMappingsComplete), new object[] { sender, natProperty });
                return;
            }
            else
            {
                if (natProperty.PYXNatProperty.success)
                {
                    addItem("Get Port Mappings completed successfully.");
                    for (int x = 0; x < natProperty.PYXNatProperty.count; ++x)
                    {
                        string strEnabled = "OFF";
                        if (natProperty.PYXNatProperty.mappings[x].Enabled)
                        {
                            strEnabled = "ON";
                        }

                        addItem(natProperty.PYXNatProperty.mappings[x].ExternalIPAddress + ":" +
                            natProperty.PYXNatProperty.mappings[x].ExternalPort.ToString() + "  to  " +
                            natProperty.PYXNatProperty.mappings[x].InternalHostName + ":" +
                            natProperty.PYXNatProperty.mappings[x].InternalPort.ToString() + " " +
                            natProperty.PYXNatProperty.mappings[x].Description + " " +
                            strEnabled);
                    }
                }
                else
                {
                    addItem("Get port mappings failed");
                }
            }
        }

        private void Add_Click(object sender, EventArgs e)
        {
            PYXNATTraversal.PYXAdapterInfo adapterInfo = new PYXNATTraversal.PYXAdapterInfo();

            int nPort = int.Parse(textBox1.Text);

            string strProtocol = this.comboBox1.Text;

            string strIPAddress = null;
            if (adapterInfo != null)
            {
                strIPAddress = adapterInfo.getIPAddress();

                addItem("IP Address = " + strIPAddress);
            }

            addItem("Adding mapping for " + strProtocol + " port " + nPort.ToString() + " IP Address " + strIPAddress);

            NatTraversal.PortMappingInfo newPortMapping;

            System.Net.IPAddress ByteIPAddress = new System.Net.IPAddress(0x00000000);

            newPortMapping = new NatTraversal.PortMappingInfo("PYXNet TCPIP Port", strProtocol, strIPAddress, nPort, ByteIPAddress, nPort, true);

            PYXNATTraversal.PYXNatControl pyxNatControl = new PYXNATTraversal.PYXNatControl();

            pyxNatControl.OnPYXNatAddComplete += mappingComplete;

            pyxNatControl.AddPortMappingWithThread(newPortMapping);
        }

        private void Remove_Click(object sender, EventArgs e)
        {
            PYXNATTraversal.PYXAdapterInfo adapterInfo = new PYXNATTraversal.PYXAdapterInfo();

            int nPort = int.Parse(textBox1.Text);

            string strProtocol = this.comboBox1.Text;

            addItem("Remove mapping for " + strProtocol + " port " + nPort.ToString() );

            NatTraversal.PortMappingInfo newPortMapping;

            System.Net.IPAddress ByteIPAddress = new System.Net.IPAddress(0x00000000);

            newPortMapping = new NatTraversal.PortMappingInfo("PYXNet TCPIP Port", strProtocol, null, nPort, ByteIPAddress, nPort, true);

            PYXNATTraversal.PYXNatControl pyxNatControl = new PYXNATTraversal.PYXNatControl();

            pyxNatControl.OnPYXNatRemoveComplete += removeMappingComplete;

            pyxNatControl.RemovePortMappingWithThread(newPortMapping);
        }

        // Get port mapping
        private void button4_Click(object sender, EventArgs e)
        {
            int nPort = int.Parse(textBox1.Text);

            string strProtocol = this.comboBox1.Text;

            addItem("Get mapping for " + strProtocol + " protocol on port " + nPort.ToString());

            PYXNATTraversal.PYXNatControl pyxNatControl = new PYXNATTraversal.PYXNatControl();

            if (pyxNatControl != null)
            {
                pyxNatControl.OnPYXNatGetMappingComplete += getMappingComplete;

                pyxNatControl.getPortMappingWithThread(nPort, strProtocol);
            }
        }

        // Get Port Mappings
        private void button7_Click(object sender, EventArgs e)
        {
            addItem("Get port mappings");

            PYXNATTraversal.PYXNatControl pyxNatControl = new PYXNATTraversal.PYXNatControl();

            if (pyxNatControl != null)
            {
                pyxNatControl.OnPYXNatGetMappingsComplete += getMappingsComplete;

                pyxNatControl.getPortMappingsWithThread();
            }
        }
   
        // Get Host Name
        private void button5_Click(object sender, EventArgs e)
        {
            PYXNATTraversal.PYXAdapterInfo adapterInfo = new PYXNATTraversal.PYXAdapterInfo();

            if (adapterInfo != null)
            {
                addItem("Host Name = " + adapterInfo.getHostName());
            }
        }

        // Get IP Address
        private void button6_Click(object sender, EventArgs e)
        {
            PYXNATTraversal.PYXAdapterInfo adapterInfo = new PYXNATTraversal.PYXAdapterInfo();

            string strIPAddress = null;
            if (adapterInfo != null)
            {
                strIPAddress = adapterInfo.getIPAddress();

                addItem("IP Address = " + strIPAddress);
            }
        }
    }
}