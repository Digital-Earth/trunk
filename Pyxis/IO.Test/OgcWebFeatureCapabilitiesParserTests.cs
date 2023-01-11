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
    class OgcWebFeatureCapabilitiesParserTests
    {
        [Test]
        public void ParsingVersion200DetectLayersCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wfs_2.0.0_capabilities.xml");

            var url = new OgcWebFeatureUrl("http://data.usgin.org/arizona/ows?service=WFS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebFeatureCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("2.0.0", url.Version);
            Assert.AreEqual(6, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("azgs:activefaults", catalog.DataSets[0].Layer);
            Assert.AreEqual("Arizona Active Faults", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual("Active faults throughout Arizona", catalog.DataSets[0].Metadata.Description);
            Assert.AreEqual(new[] { "activefaults", "features" }, catalog.DataSets[0].Metadata.Tags);

            Assert.AreEqual("GeoServer Web Feature Service", catalog.Metadata.Name);
            Assert.AreEqual("This is the reference implementation of WFS 1.0.0 and WFS 1.1.0, supports all WFS operations including Transaction.",catalog.Metadata.Description);
            Assert.AreEqual(new[] {"WFS", "WMS", "GEOSERVER"}, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion110DetectLayersCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wfs_1.1.0_capabilities(ArcGIS).xml");

            var url = new OgcWebFeatureUrl("http://geothermal.isgs.illinois.edu/ArcGIS/services/aasggeothermal/PARockChemistry/MapServer/WFSServer?service=WFS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebFeatureCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.1.0", url.Version);
            Assert.AreEqual(4, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("aasg:WRMajorElements", catalog.DataSets[0].Layer);
            Assert.AreEqual("WRMajorElements", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual(null, catalog.DataSets[0].Metadata.Description);
            Assert.AreEqual(null, catalog.DataSets[0].Metadata.Tags);

            Assert.AreEqual("CTRockChemistry", catalog.Metadata.Name);
            Assert.AreEqual("RockChemistry in the state of Connecticut", catalog.Metadata.Description);
            Assert.AreEqual(new[] { "rock specimen analysis","rare earths","trace elements","chemical analysis","whole rock chemical analysis","Connecticut","goethermal" }, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion100DetectLayersCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wfs_1.0.0_capabilities.xml");

            var url = new OgcWebFeatureUrl("http://arcms30.tor.ec.gc.ca/cgi-bin/wem_en?service=WFS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebFeatureCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.0.0", url.Version);
            Assert.AreEqual(14, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("Active_Station_View", catalog.DataSets[0].Layer);
            Assert.AreEqual("Real-time Hydrometric Monitoring Stations in Canada", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual("The purpose is to identify active hydrometric stations in Canada that provide Water Level data in near real-time. The real-time water level data is received via satellite (data collection platforms) or land-line (telephone) transmissions from hydrometric stations across Canada. Satellite transmissions of water level data are made on a scheduled basis, typically every 1 to 3 hours. Land-line hydrometric stations are polled on a scheduled basis varying from every hour to once a day, depending on the station.", catalog.DataSets[0].Metadata.Description);
            Assert.AreEqual("Water Freshwater Hydrology Sediments Water Water levels Water level Water quantity Water resources".Split(' '), catalog.DataSets[0].Metadata.Tags);

            Assert.AreEqual("Environment Canada MSC Weather and Environmental Monitoring OGC Web Services", catalog.Metadata.Name);
            Assert.AreEqual("This service contains a number of map layers that represent Canadian climate and hydrometric (water quantity) monitoring stations, both active and inactive (discontinued)", catalog.Metadata.Description);
            Assert.AreEqual(new[] { "environment","weather","msc","wem","monitoring","hydrometric","water","levels","ec","cise","ogc","cgdi" }, catalog.Metadata.Tags);
        }

    }
}
