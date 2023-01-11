using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace GeoWebCoreRunner
{
    public partial class MainForm : Form
    {
        public RunnerConfiguration Configuration { get; set; }

        private List<GeoWebCoreInstance> Instances = new List<GeoWebCoreInstance>();
        private object m_availableNodesInfoLock = new object();
        private List<RunnerConfiguration.NodeInfo> AvailableNodesInfo;

        private bool m_shuttingDown = false;

        private RunnerConfiguration.NodeInfo GetAvailbleNode(string tag)
        {
            lock (m_availableNodesInfoLock)
            {
                //init system
                if (AvailableNodesInfo == null)
                {
                    AvailableNodesInfo = new List<RunnerConfiguration.NodeInfo>(Configuration.Nodes);
                }

                var node = AvailableNodesInfo.FirstOrDefault(x => x.Tags.Contains(tag));

                if (node != null)
                {
                    AvailableNodesInfo.Remove(node);
                }
                return node;
            }
        }

        private void AddAvilableNode(RunnerConfiguration.NodeInfo nodeInfo)
        {
            lock (m_availableNodesInfoLock)
            {
                AvailableNodesInfo.Insert(0, nodeInfo);
            }
        }


        private GeoWebCoreInstance CurrentInstance;

        public MainForm()
        {
            InitializeComponent();
        }


        private void SetCurrentInstance(GeoWebCoreInstance instance)
        {
            if (CurrentInstance != null)
            {
                CurrentInstance.OutputChanged -= CurrentInstance_OutputChanged;
            }

            CurrentInstance = instance;

            if (CurrentInstance != null)
            {
                CurrentInstance.OutputChanged += CurrentInstance_OutputChanged;
            }
        }

        void CurrentInstance_OutputChanged(object sender, EventArgs e)
        {
            BeginInvoke((MethodInvoker)UpdateConsole);
        }

        private void buttonNewInstance_Click(object sender, EventArgs e)
        {
            AttachToNewInstance(CreateNewInstance("Server"));

            listViewInstances.SelectedIndices.Clear();
            listViewInstances.SelectedIndices.Add(listViewInstances.Items.Count - 1);
            listViewInstances.Select();
        }

        private void AttachToNewInstance(GeoWebCoreInstance instance) {
            if (instance == null)
            {
                return;
            }

            var listViewItem = new ListViewItem(CreateListViewTexts(instance));

            listViewItem.Tag = instance;
            listViewInstances.Items.Add(listViewItem);
            Instances.Add(instance);

            instance.StateChanged += (s, e2) =>
            {
                BeginInvoke((MethodInvoker)(() =>
                {
                    if (instance.State == GeoWebCoreInstance.InstanceState.Exited)
                    {
                        Instances.Remove(instance);
                        AddAvilableNode(instance.Settings.NodeInfo);

                        Task.Delay(30000).ContinueWith((task) =>
                        {
                            BeginInvoke((MethodInvoker)(() =>
                            {
                                listViewInstances.Items.Remove(listViewItem);
                            }));
                        });
                    }
                }));
            };
        }

        private GeoWebCoreInstance CreateNewInstance(string tag)
        {
            return CreateNewInstance(tag, GeoWebCoreRunMode.Server, "GeoWebCore {Url}");
        }


        private GeoWebCoreInstance CreateNewDownloadInstance(string url)
        {
            var nodeInfo = GetAvailbleNode("Task");
            var instanceSettings = new GeoWebCoreInstanceSettings()
            {
                Environment = Configuration.Environment,
                Mode = GeoWebCoreRunMode.Download,
                NodeInfo = nodeInfo,
                Files = new string[] { url }
            };

            var instance = new GeoWebCoreInstance(instanceSettings, "Downloading " + url);
            return instance;
        }


        private GeoWebCoreInstance CreateNewImporterInstance(string url)
        {
            var nodeInfo = GetAvailbleNode("Task");
            var instanceSettings = new GeoWebCoreInstanceSettings()
            {
                Environment = Configuration.Environment,
                Mode = GeoWebCoreRunMode.Import,
                Verbose = true,
                NodeInfo = nodeInfo,
                Files = new string[] { url }
            };

            var instance = new GeoWebCoreInstance(instanceSettings, "Importing " + url);
            return instance;
        }

        private GeoWebCoreInstance CreateNewInstance(string tag, GeoWebCoreRunMode mode,string message)
        {
            var nodeInfo = GetAvailbleNode(tag);

            if (nodeInfo == null)
            {
                return null;
            }

            var instanceSettings = new GeoWebCoreInstanceSettings()
            {
                Environment = Configuration.Environment,
                Mode = mode,
                NodeInfo = nodeInfo,
            };

            //use first node as cluster master
            var master = GetRunningInstances("Master").FirstOrDefault();
            if (master != null && nodeInfo.Url != master.Settings.NodeInfo.Url)
            {
                instanceSettings.MasterUrl = master.Settings.NodeInfo.Url;
            }

            message = message.Replace("{Url}",nodeInfo.Url);

            var instance = new GeoWebCoreInstance(instanceSettings,message);

            return instance;
        }


        private string[] CreateListViewTexts(GeoWebCoreInstance instance)
        {
            var state = instance.State.ToString();
            
            if (instance.State == GeoWebCoreInstance.InstanceState.Running)
            {
                var idleTime = DateTime.Now - instance.GetLastCpuActivity();

                if (idleTime.TotalDays > 1)
                {
                    state = String.Format("Idle {1:0}[day]", idleTime.TotalDays);
                }
                else if (idleTime.TotalHours > 1)
                {
                    state = String.Format("Idle {0:0}[h]", idleTime.TotalHours);
                } 
                else if (idleTime.TotalMinutes > 1)
                {
                    state = String.Format("Idle {0:0}[m]", idleTime.TotalMinutes);
                }
            }

            return new[] { 
                instance.Process.Id.ToString(), 
                state, 
                (instance.GetMemory() / 1024 / 1024).ToString() + "MB", 
                ((int)(100 * instance.GetProcessUsage())).ToString() + "%" , 
                String.Join(" ",instance.Settings.NodeInfo.Tags),
                instance.Message 
            };
        }

        private void UpdateConsole()
        {
            if (CurrentInstance != null)
            {
                var lines = CurrentInstance.Output.Split(new char[]{'\n'});
                var neededLines = richTextBoxConsole.Height / 16;
                if (lines.Length > neededLines)
                {
                    richTextBoxConsole.Text = String.Join("\n", lines.Skip(lines.Length-neededLines));
                }
                else
                {
                    richTextBoxConsole.Text = String.Join("\n", lines);
                }
            }
            else
            {
                richTextBoxConsole.Clear();
            }
        }

        private void UpdateInstancesStatus()
        {
            foreach (ListViewItem item in listViewInstances.Items)
            {
                var instance = item.Tag as GeoWebCoreInstance;

                var texts = CreateListViewTexts(instance);

                item.SubItems[1].Text = texts[1];
                item.SubItems[2].Text = texts[2];
                item.SubItems[3].Text = texts[3];
            }

            if (m_shuttingDown)
            {
                if (Instances.Count == 0)
                {
                    Close();
                }
                return;
            }

            if (Configuration.MasterNodes > GetRunningInstancesCount("Master"))
            {
                //start new master instance - event hub
                AttachToNewInstance(CreateNewInstance("Master"));
                return;
            }

            if (Configuration.ServersNodes > GetRunningInstancesCount("Server"))
            {
                //start new server instance
                AttachToNewInstance(CreateNewInstance("Server"));
                return;
            }

            if (Configuration.ImportNodes > GetRunningInstancesCount("Import"))
            {
                //start new import server instance
                AttachToNewInstance(CreateNewInstance("Import"));
                return;
            }


            foreach (var instance in Instances
                .Where(instance => instance.State == GeoWebCoreInstance.InstanceState.Running)
                .Where(instance => instance.GetMemory()/1024/1024 > Configuration.Environment.MemoryLimitMB))
            {
                var idleTime = DateTime.Now - instance.GetLastCpuActivity();
                //TODO: move this into setting.
                if (idleTime.TotalMinutes > 10)
                {
                    var instanceToKill = instance;
                    Task.Factory.StartNew(() => instanceToKill.Kill());
                }
            }

          
            //handle importing requests
            if (importQueue.Count > 0 && (Instances.Count - Configuration.ImportNodes - Configuration.MasterNodes - Configuration.ServersNodes) < importInstanceCount)
            {
                var next = importQueue[0];
                importQueue.RemoveAt(0);

                if (downloadFiles)
                {
                    AttachToNewInstance(CreateNewDownloadInstance(next));
                }
                else
                {
                    AttachToNewInstance(CreateNewImporterInstance(next));
                }
            }
        }

        private IEnumerable<GeoWebCoreInstance> GetRunningInstances(string tag)
        {
            return Instances.Where(instance => instance.Settings.NodeInfo.Tags.Contains(tag));
        }

        private int GetRunningInstancesCount(string tag)
        {
            return GetRunningInstances(tag).Count();
        }

        private void timerRefresh_Tick(object sender, EventArgs e)
        {
            UpdateInstancesStatus();

            labelStatus.Text = Instances.Count + " instances, " + importQueue.Count + " items in import queue";
        }

        private void listViewInstances_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listViewInstances.SelectedItems.Count == 0)
            {
                SetCurrentInstance(null);
            }
            else
            {
                SetCurrentInstance(listViewInstances.SelectedItems[0].Tag as GeoWebCoreInstance);
            }

            UpdateConsole();
        }

        private void buttonKillInstance_Click(object sender, EventArgs e)
        {
            if (CurrentInstance != null)
            {
                Task.Factory.StartNew(() => CurrentInstance.Kill());
            }
        }

        private List<string> importQueue = new List<string>();
        private int importInstanceCount = 1;
        private bool downloadFiles = false;

        private void buttonImport_Click(object sender, EventArgs e)
        {
            var importDialog = new ImportForm()
            {
                Configuration = Configuration,
                InstanceCount = importInstanceCount
            };

            if (importDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                importInstanceCount = importDialog.InstanceCount;
                downloadFiles = importDialog.DownloadFiles;
                importQueue.AddRange(importDialog.GeoSources.Select(x=>"pyxis://"+x));
            }
        }

        private void buttonValidateChecksum_Click(object sender, EventArgs e)
        {
            AttachToNewInstance(CreateNewInstance("Task", GeoWebCoreRunMode.ValidateChecksums, "Checksumer"));
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (e.CloseReason == CloseReason.UserClosing)
            {
                if (!m_shuttingDown)
                {
                    Instances.ForEach(instnace =>
                    {
                        Task.Factory.StartNew(() => instnace.Kill());
                    });
                }

                m_shuttingDown = true;

                if (Instances.Count > 0)
                {
                    e.Cancel = true;
                }
            }
        }

        private void buttonSettings_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process.Start("nodes.json");
        }
    }
}
