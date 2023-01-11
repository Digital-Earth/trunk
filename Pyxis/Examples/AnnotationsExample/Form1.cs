using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using Pyxis.UI;
using Pyxis.Core.Analysis;
using Pyxis.Contract.Publishing;

namespace AnnotationsExample
{
    public partial class Form1 : Form
    {
        private Pyxis.UI.Layers.GlobeLayer m_globeLayer;
        public List<Well> Wells { get; set; } 
        public Pyxis.Contract.Publishing.GeoSource WellsGeoSource { get; set; }

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //creating a background as default
            pyxisView1.AddLayer(new Pyxis.UI.Layers.BackgroundLayer());

            //wait for the engine to start
            pyxisEngineApiFactory1.WhenReady(LoadGlobe);

            infoTableLayoutPanel.Hide();
        }

        private void LoadGlobe()
        {
            //get the engine
            var engine = pyxisEngineApiFactory1.GetEngine();

            //using CreateGlobe extension method to generate all default layers
            m_globeLayer = pyxisView1.CreateGlobe(engine);

            //create random set of wells.
            Wells = Enumerable.Range(0, 200).Select(x => WellGenerator.CreateRandomWell()).ToList();

            //Convert the Well class into a GeoSource
            WellsGeoSource = engine.CreateInMemory<Well>(Wells);

            //Show the well information on the globe
            var id = m_globeLayer.ViewState.Show(WellsGeoSource);

            //define the icon style
            m_globeLayer.ViewState.ApplyStyle(id,
                new Style()
                {
                    Icon = IconFieldStyle.FromImage(Pyxis.UI.Icons.Circle, 0.2)
                });

            //let the globe layer to define a palette based on the depth field
            //this function would check the existing style to define if to use style on
            //icons or on fill area.
            m_globeLayer.ViewState.SetStyleByField(id, "Depth");

            //zoom the globe to cover the dataset.
            m_globeLayer.GotoPipeline(WellsGeoSource, TimeSpan.FromSeconds(3));

            //make an interactive properties window appear when mouse enters/leave an annotation
            m_globeLayer.AnnotationMouseEnter += m_globeLayer_AnnotationMouseEnter;
            m_globeLayer.AnnotationMouseLeave += m_globeLayer_AnnotationMouseLeave;
        }

        

        void m_globeLayer_AnnotationMouseEnter(object sender, Pyxis.UI.Layers.Globe.GlobeAnnotationMouseEventArgs e)
        {
            //check if this is a single feature annotation
            if (e.FeaturesCount == 1)
            {
                //find the feature using the feature ID
                var id = e.GetFeaturesIds().First();
                var well = Wells.FirstOrDefault(x => x.Id.ToString() == id);

                //display the data from of the annotated feature
                if (well != null)
                {
                    wellIdLabel.Text = well.Id.ToString();
                    wellOperatorLabel.Text = well.Operator;
                    wellStatusLabel.Text = well.Status;
                    wellProductionLabel.Text = well.Production.ToString();
                    wellDepthLabel.Text = well.Depth.ToString();
                    infoTableLayoutPanel.Show();
                }
            }
            else
            {
                //simple display how many features been annotated
                wellIdLabel.Text = e.FeaturesCount + " Wells";
                wellOperatorLabel.Text = "";
                wellStatusLabel.Text = "";
                wellProductionLabel.Text = "";
                wellDepthLabel.Text = "";

                //e.GetFeaturesIds() can be used to display more information about the features

                infoTableLayoutPanel.Show();
            }
        }

        void m_globeLayer_AnnotationMouseLeave(object sender, Pyxis.UI.Layers.Globe.GlobeAnnotationMouseEventArgs e)
        {
            infoTableLayoutPanel.Hide();
        }
    }
}
