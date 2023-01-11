using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using Pyxis.Core.Analysis;
using Pyxis.UI;
using Pyxis.Contract.Publishing;

namespace CountWells
{
    public partial class Form1 : Form
    {
        private Pyxis.UI.Layers.GlobeLayer m_globeLayer;
        private Pyxis.UI.Layers.SelectionLayer m_selection;

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
        }

        private void LoadGlobe()
        {
            //get the engine
            var engine = pyxisEngineApiFactory1.GetEngine();

            //using CreateGlobe extension method to generate all default layers
            m_globeLayer = pyxisView1.CreateGlobe(engine);

            //get the selection later using extension method GetSelectionLayer
            m_selection = pyxisView1.GetSelectionLayer();
            
            //create random set of wells.
            Wells = Enumerable.Range(0, 200).Select(x => WellGenerator.CreateRandomWell()).ToList();

            //Convert the Well class into a GeoSource
            WellsGeoSource = engine.CreateInMemory<Well>(Wells);

            //Show the well information on the globe
            var id = m_globeLayer.ViewState.Show(WellsGeoSource);

            //define the icon style
            m_globeLayer.ViewState.ApplyStyle(id, 
                new Style() { 
                    Icon = IconFieldStyle.FromImage(Pyxis.UI.Icons.Circle, 0.2) 
                });

            //let the globe layer to define a palette based on the depth field
            //this function would check the existing style and see if it needs to 
            //style on icons or on fill area.
            m_globeLayer.ViewState.SetStyleByField(id, "Depth");

            //zoom the globe to cover the dataset.
            m_globeLayer.GotoPipeline(WellsGeoSource, TimeSpan.FromSeconds(3));
        }

        public List<Well> Wells { get; set; }
        public Pyxis.Contract.Publishing.GeoSource WellsGeoSource { get; set; }

        private void button1_Click(object sender, EventArgs e)
        {
            //start selection
            button1.Enabled = false;
            pyxisView1.Cursor = Cursors.Cross;
            m_selection.StartBoxSelection(FinishSelection);
        }

        private void FinishSelection(Pyxis.Core.IO.GeoJson.Geometry geometry)
        {
            //finish selection
            button1.Enabled = true;
            pyxisView1.Cursor = Cursors.Default;

            if (geometry != null)
            {
                UpdateWellsListView(geometry);
            }
        }

        private void UpdateWellsListView(Pyxis.Core.IO.GeoJson.Geometry geometry)
        {
            listView1.Items.Clear();
            listView1.Groups.Clear();

            //Use Pyxis.Engine.GetAsFeatures(GeoSource) to use the data inside that GeoSource
            //Use GetFeature(IGeometry) to get only the features inside a given area
            var features = pyxisEngineApiFactory1.GetEngine().GetAsFeature(WellsGeoSource).GetFeatures(geometry);            

            //We want to display wells based on company.
            var groups = features.Features
                    .Select(x => x.Properties["Operator"].ToString())
                    .Distinct()
                    .ToList();

            //Create ListViewGroups
            foreach (var group in groups)
            {
                listView1.Groups.Add(group, group);
            }

            //Add ListViewItem for each feature
            foreach (var feature in features.Features)
            {
                listView1.Items.Add(
                    new ListViewItem(
                        new string[] { 
                                    feature.Id, 
                                    feature.Properties["Status"].ToString(), 
                                    feature.Properties["Production"].ToString(), 
                                    feature.Properties["Depth"].ToString() })
                    {
                        Group = listView1.Groups[feature.Properties["Operator"].ToString()]
                    }
                );
            }
        }

        private void listView1_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            var palettes = new List<Color[]>()
            {
                new Color[] { Color.Black,Color.White },
                new Color[] { Color.Red,Color.Green },
                new Color[] { Color.Blue,Color.Yellow },
                new Color[] { Color.Cyan,Color.Pink}
            };

            //Get the ViewState Id for the WellsGeoSource.            
            var id = m_globeLayer.ViewState.GetStyledGeoSource(WellsGeoSource);

            //the id is being used to update the style for the GeoSource
            m_globeLayer.ViewState.SetStyleByField(id.Value, 
                listView1.Columns[e.Column].Text, 
                Palette.Create(palettes[e.Column]));
        }
    }
}
