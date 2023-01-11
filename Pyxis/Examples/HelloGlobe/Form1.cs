using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HelloGlobe
{
    public partial class Form1 : Form
    {
        private Pyxis.UI.Layers.SelectionLayer m_selection;
        private Pyxis.UI.Layers.GlobeLayer m_globeLayer;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // adding background layer with default gray color
            pyxisView1.AddLayer(new Pyxis.UI.Layers.BackgroundLayer());

            // adding globe layer
            m_globeLayer = new Pyxis.UI.Layers.GlobeLayer(pyxisView1, Program.PyxisEngine);
            pyxisView1.AddLayer(m_globeLayer);

            // adding selection layer
            m_selection = new Pyxis.UI.Layers.SelectionLayer(m_globeLayer);
            pyxisView1.AddLayer(m_selection);

            // asking api.pyxis.worldview.gallery for a Microsoft Bing Maps Key
            // this function returns PermitRetainer that would make sure the key is valid
            // while the program is running
            var bingKey = Program.PyxisEngine.GetUser()
                        .RequestPermit<Pyxis.Publishing.Permits.ExternalApiKeyPermit>(
                            Guid.Parse("32f9cd5d-6ef6-4601-b367-109d8d288557"));

            // creating a temporary GeoSource using Bing Aerial styling
            var geoSource = new Pyxis.Core.IO.GDAL.MicrosoftBingImagery(Program.PyxisEngine).Create(Pyxis.Core.IO.GDAL.MicrosoftBingImagery.Style.Aerial, bingKey.GetPermit());

            // showing the GeoSource on the globe
            m_globeLayer.ViewState.Show(geoSource);            
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //when the select button is clicked we use SelectionLayer.StartBoxSelection function
            //to start a selection of an area on the globe
            m_selection.StartBoxSelection((geometry) => {
                
                //the action can return a geometry or null.
                //it would return null in case the box selection area created by the user
                //is not overlapping with the globe.
                if (geometry != null)
                {
                    //We use CreateInMemory feature of the Pyxis Engine to create a temporary
                    //GeoSource (live in memory) using the geometry selected on earth
                    var geoSource = Program.PyxisEngine.CreateInMemory(
                        new Pyxis.Core.IO.GeoJson.FeatureCollection() { 
                            Features = new List<Pyxis.Core.IO.GeoJson.Feature>() { 
                                new Pyxis.Core.IO.GeoJson.Feature("1", geometry as Pyxis.Core.IO.GeoJson.Geometry, null) } });


                    //And then we can show it on the globe
                    m_globeLayer.ViewState.Show(geoSource);
                }
            });
        }
    }
}
