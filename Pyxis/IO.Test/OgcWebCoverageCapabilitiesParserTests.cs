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
    class OgcWebCoverageCapabilitiesParserTests
    {
        [Test]
        public void ParsingVersion201DetectLayersCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wcs_2.0.1_capabilities.xml");

            var url = new OgcWebCoverageUrl("http://sedac.ciesin.org/geoserver/ows?service=WCS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebCoverageCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("2.0.1", url.Version);
            Assert.AreEqual(191, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("anthromes__anthromes-anthropogenic-biomes-world-v1", catalog.DataSets[0].Layer);
            Assert.AreEqual("anthromes  anthromes-anthropogenic-biomes-world-v1", catalog.DataSets[0].Metadata.Name);

            Assert.AreEqual("Web Coverage Service", catalog.Metadata.Name);
            Assert.AreEqual(
                "This server implements the WCS specification 1.0 and 1.1.1, it's reference implementation of WCS 1.1.1. All layers published by this service are available on WMS also.",
                catalog.Metadata.Description);
            Assert.AreEqual(new[] {"WCS", "WMS", "GEOSERVER"}, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion112DetectLayersCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wcs_1.1.2_capabilities_1_coverage_with_abstract.xml");

            var url =
                new OgcWebCoverageUrl(
                    "http://kgs.uky.edu/usgin/services/aasggeothermal/KYUpperOrdovician_WCS/MapServer/WCSServer?service=WCS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebCoverageCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.1.2", url.Version);
            Assert.AreEqual(1, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("1", catalog.DataSets[0].Layer);
            Assert.AreEqual("Surface_1", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual(
                "These data were analyzed with Petra mapping software using available in-house oil-and-gas well locations (KY single zone projection, NAD 83, state plane feet) from the KGS well database and some selected nonstate supplied well data. Tops were picked for the Upper part of Ordovician System. A smoothed (unfaulted) raster dataset was generated from a total of 1178 wells examined. An ascii tabular grid file (cell size 4000 feet) was exported from Petra and loaded into ArcMap. Raster dataset then clipped to Silurian outcrop limit for central Kentucky; remainder clipped to buffered state outline.",
                catalog.DataSets[0].Metadata.Description);

            Assert.AreEqual("KYUpperOrdovician_WCS", catalog.Metadata.Name);
            Assert.AreEqual(null, catalog.Metadata.Description);
            Assert.AreEqual(
                new[]
                {
                    "GIS", "Kentucky", "KGS", "top of Upper part of Ordovician System", "Upper Ordovician",
                    "stratigraphy", "gridded surface", "geothermal"
                }, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion100DetectLayersCorrectly()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wcs_1.0.0_capabilities.xml");

            var url =
                new OgcWebCoverageUrl(
                    "http://geobrain.laits.gmu.edu/cgi-bin/gbwcs-dem?service=WCS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebCoverageCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("1.0.0", url.Version);
            Assert.AreEqual(6, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("SRTM_30m_USA", catalog.DataSets[0].Layer);
            Assert.AreEqual("SRTM USA 30m spatial resolution GeoTiff (Int16)", catalog.DataSets[0].Metadata.Name);
            Assert.AreEqual(null, catalog.DataSets[0].Metadata.Description);

            Assert.AreEqual("GMU LAITS Web Coverage Server", catalog.Metadata.Name);
            Assert.AreEqual("GeoBrain Web Coverage Server for DEM data", catalog.Metadata.Description);
            Assert.AreEqual(new[] {"GMU", "LAITS", "GeoBrain", "WCS", "DEM"}, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion201WithEoExtensionsDetectSubCatalogs()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wcs_2.0.1_EO_capabilities.xml");

            var url =
                new OgcWebCoverageUrl("http://ows.eox.at/testbed-12/eoxserver/ows?service=WCS&request=GetCapabilities");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebCoverageCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("2.0.1", url.Version);
            Assert.AreEqual(0, catalog.DataSets.Count);
            Assert.AreEqual(3, catalog.SubCatalogs.Count);

            Assert.AreEqual("dlr_fire_emission_dispersion_california_20160223", catalog.SubCatalogs[0].Metadata.Name);
            Assert.AreEqual("http://ows.eox.at/testbed-12/eoxserver/ows?version=2.0.1&service=WCS&request=DescribeEOCoverageSet&eoid=dlr_fire_emission_dispersion_california_20160223", catalog.SubCatalogs[0].Uri);

            Assert.AreEqual("EVO-ODAS standards demonstration instance of EOxServer", catalog.Metadata.Name);
            Assert.AreEqual("EVO-ODAS standards demonstration instance of EOxServer", catalog.Metadata.Description);
            Assert.AreEqual(new[] {"EVO-ODAS", "EOxServer", "standards", "demonstration"}, catalog.Metadata.Tags);
        }

        [Test]
        public void ParsingVersion201DescribeEOCoverageSetDetectLayers()
        {
            var doc = new XmlDocument();
            doc.Load("../../Resources/OGC/wcs_2.0.1_DescribeEOCoverageSet.xml");

            var url =
                new OgcWebCoverageUrl("http://ows.eox.at/testbed-12/eoxserver/ows?version=2.0.1&service=WCS&request=DescribeEOCoverageSet&eoid=dlr_fire_emission_dispersion_california_20160223");
            var catalog = new DataSetCatalog();
            var parser = new OgcWebCoverageCapabilitiesParser(url, catalog);
            parser.Parse(doc);

            Assert.AreEqual("2.0.1", url.Version);
            //the parser can now break a dataset with multi bands into several datasets with Domain
            Assert.AreEqual(32 * 10, catalog.DataSets.Count);
            Assert.AreEqual(0, catalog.SubCatalogs.Count);

            Assert.AreEqual("dlr_fire_emission_dispersion_california_20160223_1", catalog.DataSets[0].Layer);
            Assert.AreEqual("dlr fire emission dispersion california 20160223 1 (fire_emission_b1)", catalog.DataSets[0].Metadata.Name);
            
            //validate domain extraction
            Assert.AreEqual(2, catalog.DataSets[0].Domains.Count);
            Assert.AreEqual("Time", catalog.DataSets[0].Domains[0].Name);
            Assert.AreEqual("2016-02-23T03:00:00Z", catalog.DataSets[0].Domains[0].Value);

            Assert.AreEqual("Band", catalog.DataSets[0].Domains[1].Name);
            Assert.AreEqual("fire_emission_b1", catalog.DataSets[0].Domains[1].Value);
            Assert.AreEqual(10, catalog.DataSets[0].Domains[1].Values.Count);
            Assert.AreEqual("fire_emission_b1", catalog.DataSets[0].Domains[1].Values[0]);

            //valide that next dataset is the same coverage, second band
            Assert.AreEqual("dlr_fire_emission_dispersion_california_20160223_1", catalog.DataSets[1].Layer);
            Assert.AreEqual("fire_emission_b2", catalog.DataSets[1].Domains[1].Value);
            Assert.AreEqual(10, catalog.DataSets[1].Domains[1].Values.Count);
            Assert.AreEqual("fire_emission_b2", catalog.DataSets[1].Domains[1].Values[1]);

            //validate domain extraction extrac t the time domain correctly
            Assert.AreEqual("dlr_fire_emission_dispersion_california_20160223_10", catalog.DataSets[10].Layer);
            Assert.AreEqual(2, catalog.DataSets[10].Domains.Count);
            Assert.AreEqual("Time", catalog.DataSets[10].Domains[0].Name);
            Assert.AreEqual("2016-02-24T06:00:00Z", catalog.DataSets[10].Domains[0].Value);
            Assert.AreEqual("Band", catalog.DataSets[10].Domains[1].Name);
            Assert.AreEqual("fire_emission_b1", catalog.DataSets[10].Domains[1].Value);
        }
    }
}
