using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using NUnit.Framework;
using Pyxis.Contract.DataDiscovery;
using Pyxis.IO.Sources.OGC;

namespace Pyxis.IO.Test
{
    [TestFixture]
    class OgcWebMapCapabilitiesParserTests
    {
        [Test]
        public void ParsingVersion130DetectsLayerCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wms_1.3.0_capabilities_simple.xml");

            var url = new OgcWebMapUrl("http://129.194.231.213:8080/geoserver/eg_basemap/ows?service=WMS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebMapCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.3.0", url.Version);
            Assert.AreEqual(9, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("eg_basemap:Esri93_cities2_c", catalog.DataSets[0].Layer);
            Assert.AreEqual("Europe Cities", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual("Europe Cities represents the cities of Europe including national capitals, major population centers, and landmark cities.", catalog.DataSets[0].Metadata.Description);
            Assert.AreEqual("Esri93_cities2_c features".Split(' '), catalog.DataSets[0].Metadata.Tags);

            Assert.AreEqual("EnviroGRIDS Web Map Service", catalog.Metadata.Name);
            Assert.AreEqual("EnviroGRIDS WMS", catalog.Metadata.Description);
            Assert.AreEqual(new[] { "WFS", "WMS", "GEOSERVER", "ENVIROGRIDS" }, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion132DetectsLayerCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wms_1.3.2_capabilities_simple.xml");

            var url = new OgcWebMapUrl("http://demo.cubewerx.com/demo/cubeserv/simple?service=WMS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebMapCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.3.2", url.Version);
            Assert.AreEqual(14, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("Foundation.aerofacp_1m", catalog.DataSets[0].Layer);
            Assert.AreEqual("Airport Facility Points", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual("VPF Narrative Table for \"Airport Facilities Points\":\n\nInformation on airports/airfields (GB005) was derived from the DAFIF (Digital Aeronautical Flight Information File) and TINT (Target Intelligence) in areas where such data was available. Each airfield's DAFIF reference number was placed in the 'na3' (classification name) attribute field. Only airfields which had at least one hard surface runway longer that 3,000 feet (910 meters) were collected.", catalog.DataSets[0].Metadata.Description);
            Assert.AreEqual(null, catalog.DataSets[0].Metadata.Tags);

            Assert.AreEqual("CubeSERV Demo WMS", catalog.Metadata.Name);
            Assert.AreEqual("OGC-compliant cascading web map server (WMS) by CubeWerx Inc.", catalog.Metadata.Description);
            Assert.AreEqual(null, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion111DetectsLayerCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wms_1.1.1_capabilities_nasa_missing_styles.xml");

            var url = new OgcWebMapUrl("http://disc1.sci.gsfc.nasa.gov/daac-bin/wms_airs?service=WMS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebMapCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.1.1", url.Version);
            Assert.AreEqual(79, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("AIRX3STD_TOTCLDLIQH2O_A", catalog.DataSets[0].Layer);
            Assert.AreEqual("Daily Mean Total Integrated Column Cloud Liquidwater Ascending", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual("Level 3 Version 006 daily, physical retrieval standard product, without HSB. Surface and Atmospheric Quantities in a 1x1 Lat/Lon Grid Box. The geophysical parameter: AIRS daily Mean total integrated column cloud liquidwater (kg/m^2)", catalog.DataSets[0].Metadata.Description);
            Assert.AreEqual("EARTH SCIENCE,ATMOSPHERE,ALTITUDE,TROPOPAUSE,ATMOSPHERIC CHEMISTRY".Split(','),catalog.DataSets[0].Metadata.Tags);

            Assert.AreEqual("Atmospheric Infrared Sounder (AIRS) Data from NASA Goddard Earth Sciences Data and Information Services Center (GES DISC)", catalog.Metadata.Name);
            Assert.AreEqual(null, catalog.Metadata.Description);
            Assert.AreEqual(null, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion130DetectsLayerWithMixDepthCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wms_1.3.0_capbilitites_mixed_depth.xml");

            var url = new OgcWebMapUrl("http://map.sitr.regione.sicilia.it/ArcGIS/services/CTR_10000_AGG-2008/MapServer/WMSServer?service=WMS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebMapCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.3.0", url.Version);
            Assert.AreEqual(42, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("1", catalog.DataSets[0].Layer);
            Assert.AreEqual("Acque - Area idrica", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual("Acque - Area idrica", catalog.DataSets[0].Metadata.Description);
            Assert.AreEqual(null, catalog.DataSets[0].Metadata.Tags);

            Assert.AreEqual("CTR_10000_AGG-2008", catalog.Metadata.Name);
            Assert.AreEqual("WMS", catalog.Metadata.Description);
            Assert.AreEqual(null, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion130DetectsLayerWithDeepHierarchyCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wms_1.3.2_capabilities_deep_hierarchy.xml");

            var url = new OgcWebMapUrl("http://portal.cubewerx.com/cubewerx/cubeserv/haiti?service=WMS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebMapCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.3.2", url.Version);
            Assert.AreEqual(51, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("GeoEye.ge1_2010_01_13_e", catalog.DataSets[0].Layer);
            Assert.AreEqual("GeoEye 1 - Jan 13, 2010", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual(null, catalog.DataSets[0].Metadata.Description);
            Assert.AreEqual(null, catalog.DataSets[0].Metadata.Tags);

            Assert.AreEqual("CubeSERV Demo WMS", catalog.Metadata.Name);
            Assert.AreEqual("OGC-compliant cascading web map server (WMS) by CubeWerx Inc.", catalog.Metadata.Description);
            Assert.AreEqual(null, catalog.Metadata.Tags);
        }
    }
}
