using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Pyxis.UI;

namespace GlobeLayers
{
    public partial class Form1 : Form
    {
        private Pyxis.UI.Layers.GlobeLayer m_globe;
        private Pyxis.UI.Layers.SelectionLayer m_selection;

        public Form1()
        {
            InitializeComponent();
        }

       
        private void Form1_Load(object sender, EventArgs e)
        {
            //creating a background as default
            pyxisView1.AddLayer(new Pyxis.UI.Layers.BackgroundLayer(Color.BlueViolet));

            //wait for the engine to start
            pyxisEngineApiFactory1.WhenReady(LoadGlobe);
        }

        private void LoadGlobe()
        {
            //using CreateGlobe extension method to generate all default layers
            m_globe = pyxisView1.CreateGlobe(pyxisEngineApiFactory1.GetEngine());

            //get the selection layer using extension method GetSelectionLayer
            m_selection = pyxisView1.GetSelectionLayer();

            loadingPanel.Visible = false;
            mapLayersGroupBox.Visible = true;

            //attach bing maps - does not require PyxNet enabled
            AttachBingGeoSources();

            //attach WMS endpoint - does not require PyxNet enabled
            AttachGeoSourcetoCheckBox(toporamaCheckBox, new Guid("8cbf278e-7135-4412-a4ed-4344de518ab1"));            

            //attach lidar, coverages and features - requires PyxNet enabled
            AttachGeoSourcetoCheckBox(elevation2CheckBox, new Guid("21a3dfe0-6c19-4618-88c9-ab94bf842955"));
            AttachGeoSourcetoCheckBox(elevation30CheckBox, new Guid("8be6a2ec-5110-49cf-a295-1008f8e9a21b"));
            AttachGeoSourcetoCheckBox(lidarCheckBox, new Guid("b7baa006-b907-4e0d-b77c-2cdd6d90ca1c"),lidarGotoButton);
            AttachGeoSourcetoCheckBox(worldPoliticalCheckBox, new Guid("7664e383-9d2c-494d-b3ec-5ae708623565"));
            AttachGeoSourcetoCheckBox(globalPipelinesCheckBox, new Guid("03c2794e-269e-4ac7-a2ea-4e552aaf726f"));
            AttachGeoSourcetoCheckBox(naturalEarthCheckBox, new Guid("95fd9df9-6429-4e02-b15a-8c7832d05291"));            
        }

        private void AttachGeoSourcetoCheckBox(CheckBox checkBox, Guid guid)
        {
            AttachGeoSourcetoCheckBox(checkBox, guid, null);
        }

        private void AttachGeoSourcetoCheckBox(CheckBox checkBox, Guid guid, Button gotoButton)
        {
            Pyxis.Contract.Publishing.GeoSource geoSource = null;

            checkBox.CheckedChanged += (s, e) =>
            {
                if (geoSource == null)
                {
                    //get information about the GeoSource from WorldView.Gallery (see Pyxis.Publishing.Channel)                    
                    geoSource = pyxisEngineApiFactory1.GetEngine().GetChannel().GeoSources.GetById(guid);
                    if (gotoButton != null)
                    {
                        gotoButton.Enabled = true;
                    }
                }

                //check if the GeoSource is visible on the globe
                var id = pyxisView1.GetGlobeLayer().ViewState.GetStyledGeoSource(geoSource);

                if (checkBox.Checked)
                {
                    //the GeoSource is not visible
                    if (id == null)
                    {
                        //show it
                        pyxisView1.GetGlobeLayer().ViewState.Show(geoSource);
                    }
                }
                else
                {
                    //the GeoSource is visible
                    if (id != null)
                    {
                        //hide it
                        pyxisView1.GetGlobeLayer().ViewState.Hide(id.Value);
                    }
                }
            };

            if (gotoButton != null)
            {
                gotoButton.Enabled = false;
                gotoButton.Click += (s, e) => {
                    //zoom the globe to the GeoSource
                    pyxisView1.GetGlobeLayer().GotoPipeline(geoSource, TimeSpan.FromSeconds(5));
                };
            }
        }

        private void AttachBingGeoSources()
        {
            bingStylesComboBox.Items.Add(Pyxis.Core.IO.GDAL.MicrosoftBingImagery.Style.Aerial);
            bingStylesComboBox.Items.Add(Pyxis.Core.IO.GDAL.MicrosoftBingImagery.Style.AerialWithLabels);
            bingStylesComboBox.Items.Add(Pyxis.Core.IO.GDAL.MicrosoftBingImagery.Style.Road);
            bingStylesComboBox.SelectedIndex = 0;

            //ask for a Bing Key from WorldView.Gallery on behalf of the user
            var permit = pyxisEngineApiFactory1.GetEngine().GetUser()
                    .RequestPermit<Pyxis.Publishing.Permits.ExternalApiKeyPermit>(
                        Guid.Parse("32f9cd5d-6ef6-4601-b367-109d8d288557"));

            var currentBingId = Guid.Empty;

            bingStylesComboBox.SelectedIndexChanged += (s, e) =>
            {
                //hide all Bing layers
                if (currentBingId != Guid.Empty)
                {
                    pyxisView1.GetGlobeLayer().ViewState.Hide(currentBingId);
                    currentBingId = Guid.Empty;
                }
                //show new Bing layer if needed
                if (bingCheckBox.Checked)
                {
                    var style = (Pyxis.Core.IO.GDAL.MicrosoftBingImagery.Style)bingStylesComboBox.SelectedItem;
                    var geoSource = new Pyxis.Core.IO.GDAL.MicrosoftBingImagery(pyxisEngineApiFactory1.GetEngine())
                            .Create(style, permit.GetPermit());

                    currentBingId = pyxisView1.GetGlobeLayer().ViewState.Show(geoSource);
                }
            };

            bingCheckBox.CheckedChanged += (s, e) =>
            {
                if (currentBingId != Guid.Empty)
                {
                    pyxisView1.GetGlobeLayer().ViewState.Hide(currentBingId);
                    currentBingId = Guid.Empty;
                }
                if (bingCheckBox.Checked)
                {
                    var style = (Pyxis.Core.IO.GDAL.MicrosoftBingImagery.Style)bingStylesComboBox.SelectedItem;
                    var geoSource = new Pyxis.Core.IO.GDAL.MicrosoftBingImagery(pyxisEngineApiFactory1.GetEngine())
                            .Create(style, permit.GetPermit());

                    currentBingId = pyxisView1.GetGlobeLayer().ViewState.Show(geoSource);
                }
            };
        }

        private void saveButton_Click(object sender, EventArgs e)
        {
            //show loading bar while the GlobeLayer is loading data
            loadingPanel.Visible = true;
            pyxisView1.GetGlobeLayer().WhenLoadingCompleted(SaveToImage);
        }

        private void SaveToImage()
        {
            //this function is get called when all GlobeLayer finish loading
            loadingPanel.Visible = false;

            //extract the bitmap from PyxisView
            var bitmap = pyxisView1.SaveToBitmap();

            //allow the user to save the file.
            var saveDialog = new SaveFileDialog();
            saveDialog.DefaultExt = "*.png";
            saveDialog.Filter = "png format (*.png)|*.png|jpeg format (*.jpg)|*.jpg|All Files (*.*)|*.*";

            if (saveDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                var filename = saveDialog.FileName;
                bitmap.Save(filename);
            }
        }
    }
}
