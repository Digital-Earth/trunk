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
    public partial class ImportForm : Form
    {
        public ImportForm()
        {
            InitializeComponent();
        }

        public RunnerConfiguration Configuration;

        public List<Guid> GeoSources
        {
            get
            {
                var result = new List<Guid>();

                foreach (var item in listBoxGeoSources.Items)
                {
                    Guid guid;
                    if (Guid.TryParse(item.ToString(), out guid))
                    {
                        result.Add(guid);
                    }
                }
                return result; 
            }
        }

        public bool DownloadFiles
        {
            get
            {
                return checkBoxDownload.Checked;
            }
        }

        public int InstanceCount
        {
            get
            {
                return (int)numericUpDownInstanceCount.Value;
            }
            set
            {
                numericUpDownInstanceCount.Value = value;
            }
        }

        private void ImportForm_Load(object sender, EventArgs e)
        {

        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (textBoxGeoSourceId.Text != "")
            {
                listBoxGeoSources.Items.Add(textBoxGeoSourceId.Text);
            }

            DialogResult = System.Windows.Forms.DialogResult.OK;
        }

        private void textBoxGeoSourceId_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBoxGeoSourceId_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                listBoxGeoSources.Items.Add(textBoxGeoSourceId.Text);
                textBoxGeoSourceId.Text = "";
            }
        }

        private void listBoxGeoSources_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete || e.KeyCode == Keys.Back)
            {
                while (listBoxGeoSources.SelectedIndices.Count > 0)
                {
                    listBoxGeoSources.Items.RemoveAt(listBoxGeoSources.SelectedIndices[listBoxGeoSources.SelectedIndices.Count - 1]);
                }
            }
        }

        private void buttonVectors_Click(object sender, EventArgs e)
        {
            var channel = new Pyxis.Publishing.Channel(Pyxis.Publishing.ApiUrl.ProductionLicenseServerRestAPI);

            if (!String.IsNullOrEmpty(Configuration.GwcKey))
            {
                channel = channel.Authenticate(new Pyxis.Publishing.ApiKey(Configuration.GwcKey, Configuration.GwcUser));
            }

            foreach (var geoSource in channel.GeoSources.Filter("State eq 'Active' and DataSize gt 0"))
            {
                //if (geoSource.State != Pyxis.Contract.Publishing.PipelineDefinitionState.Active) continue;
                //if (geoSource.DataSize == 0) continue;

                if (geoSource.Specification == null || geoSource.Specification.OutputType != Pyxis.Contract.Publishing.PipelineSpecification.PipelineOutputType.Feature) continue;

                if (listBoxGeoSources.Items.IndexOf(geoSource.Id) == -1)
                {
                    listBoxGeoSources.Items.Add(geoSource.Id);
                }
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            listBoxGeoSources.Items.Clear();
        }

        private void buttonCoverages_Click(object sender, EventArgs e)
        {
            var channel = new Pyxis.Publishing.Channel(Pyxis.Publishing.ApiUrl.ProductionLicenseServerRestAPI);

            if (!String.IsNullOrEmpty(Configuration.GwcKey))
            {
                channel = channel.Authenticate(new Pyxis.Publishing.ApiKey(Configuration.GwcKey, Configuration.GwcUser));
            }

            foreach (var geoSource in channel.GeoSources.Filter("State eq 'Active' and DataSize gt 0"))
            {
                //if (geoSource.State != Pyxis.Contract.Publishing.PipelineDefinitionState.Active) continue;
                //if (geoSource.DataSize == 0) continue;

                if (geoSource.Specification == null || geoSource.Specification.OutputType != Pyxis.Contract.Publishing.PipelineSpecification.PipelineOutputType.Coverage) continue;

                if (listBoxGeoSources.Items.IndexOf(geoSource.Id) == -1)
                {
                    listBoxGeoSources.Items.Add(geoSource.Id);
                }
            }
        }

        private DateTime m_weekAgo = DateTime.Now;

        private void buttonLastWeek_Click(object sender, EventArgs e)
        {
            m_weekAgo = m_weekAgo.AddDays(-7);

            var channel = new Pyxis.Publishing.Channel(Pyxis.Publishing.ApiUrl.ProductionLicenseServerRestAPI);

            if (!String.IsNullOrEmpty(Configuration.GwcKey))
            {
                channel = channel.Authenticate(new Pyxis.Publishing.ApiKey(Configuration.GwcKey, Configuration.GwcUser));
            }

            foreach (var geoSource in channel.GeoSources.Filter("State eq 'Active' and DataSize gt 0 and Metadata/Created gt DateTime'" + m_weekAgo.ToString("s") + "'"))
            {
                //if (geoSource.State != Pyxis.Contract.Publishing.PipelineDefinitionState.Active) continue;
                //if (geoSource.DataSize == 0) continue;

                if (geoSource.Metadata.Created < m_weekAgo) continue;

                if (listBoxGeoSources.Items.IndexOf(geoSource.Id) == -1)
                {
                    listBoxGeoSources.Items.Add(geoSource.Id);
                }
            }
        }


        private bool handled = false;

        private void textBoxGeoSourceId_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                handled = true;
            }
            else
            {
                handled = false;
            }

        }

        private void textBoxGeoSourceId_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (handled)
            {
                e.Handled = true;
            }
        }

        private void textBoxGeoSourceId_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                e.IsInputKey = true;
            }
        }
    }
}
