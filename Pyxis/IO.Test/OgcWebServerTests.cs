using System;
using System.Collections.Generic;
using System.Xml;
using NUnit.Framework;
using Pyxis.Contract.DataDiscovery;
using Pyxis.Contract.Publishing;
using Pyxis.IO.Sources.OGC;
using File = System.IO.File;

namespace Pyxis.IO.Test
{
    /// <summary>
    /// Helper class that defines tests for implementation classses of OgcWebServerBase
    /// </summary>
    [TestFixture]
    [Category("Integration")]   // allows tests marked "Integration" to be included and excluded from test run
    public abstract class OgcWebServerTests
    {
        protected OgcWebServerTests()
        {
            m_expectedServerMetadata = new SimpleMetadata();
            m_expectedDataSetsMetadata = new List<SimpleMetadata>();
        }

        protected OgcWebServerBase m_server;
        protected SimpleMetadata m_expectedServerMetadata;
        protected List<SimpleMetadata> m_expectedDataSetsMetadata;

        [Test]
        public void Metadata_ParseCapabilities_ValueExpected()
        {
            if (m_server == null)
            {
                return;
            }
            Assert.AreEqual(m_expectedServerMetadata.Name, m_server.Catalog.Metadata.Name);
            Assert.AreEqual(m_expectedServerMetadata.Description, m_server.Catalog.Metadata.Description);
        }

        [Test]
        public void DataSets_ParseCapabilities_ValueExpected()
        {
            if (m_server == null)
            {
                return;
            }
            IEnumerator<DataSet> serverDataset = m_server.Catalog.DataSets.GetEnumerator();
            //Assert.AreEqual(m_expectedDataSetsMetadata.Count, m_server.DataSets.Count);
            foreach (var expected in m_expectedDataSetsMetadata)
            {
                serverDataset.MoveNext();
                Assert.IsNotNull(serverDataset.Current);
                Assert.AreEqual(expected.Name, serverDataset.Current.Metadata.Name);
                Assert.AreEqual(expected.Description, serverDataset.Current.Metadata.Description);
            }
        }
    }

    // Initialize the tests

    public class WmsEmptyCapabilitiesTest : OgcWebServerTests
    {
        public WmsEmptyCapabilitiesTest()
        {
            m_server = OgcWebMapServerMock.Create("<WMS_Capabilities></WMS_Capabilities>");
        }
    }

    public class WmsValidCapabilitiesTest : OgcWebServerTests
    {
        public WmsValidCapabilitiesTest()
        {
            // Server
            var staticCapabilities = File.ReadAllText("../../Resources/OGC/wms_valid_capabilities.xml");
            var server = OgcWebMapServerMock.Create(staticCapabilities);
            m_expectedServerMetadata.Name = "Tennessee_Geology";
            m_expectedServerMetadata.Description = null;
            m_server = server;
            // Datasets

            //his dataset has no styles
            //m_expectedDataSetsMetadata.Add(new SimpleMetadata
            //{
            //    Name = "Tennessee_Geology",
            //    Description = "Tennessee Geology"
            //});
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "Tennessee_Faults",
                Description = "Tennessee Faults"
            });
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "Tennessee_Faults",
                Description = "USGS Web Mapping Service: Faults derived from the Tennessee State Geologic Map"
            });
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "Tennessee_Faults",
                Description = "USGS Web Mapping Service: Faults derived from the Tennessee State Geologic Map"
            });
        }
    }

    public class WfsEmptyCapabilitiesTest : OgcWebServerTests
    {
        public WfsEmptyCapabilitiesTest()
        {
            m_server = OgcWebFeatureServerMock.Create("<WFS_Capabilities></WFS_Capabilities>");
        }
    }

    public class WfsValidCapabilitiesTest : OgcWebServerTests
    {
        public WfsValidCapabilitiesTest()
        {
            // Server
            var staticCapabilities = File.ReadAllText("../../Resources/OGC/wfs_valid_capabilities.xml");
            var server = OgcWebFeatureServerMock.Create(staticCapabilities);
            m_expectedServerMetadata.Name = "GeoServer Web Feature Service";
            m_expectedServerMetadata.Description =
                "This is the reference implementation of WFS 1.0.0 and WFS 1.1.0, supports all WFS operations including Transaction.";
            m_server = server;
            // Datasets
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "ETH_case_study",
                Description = "ethiopia:ETH_case_study"
            });
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "ETH_case_study_bounding_box",
                Description = ""
            });
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "ETH_hydroshed",
                Description = ""
            });
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "ETH_hydroshed_rivers",
                Description = ""
            });
        }
    }

    public class WcsEmptyCapabilitiesTest : OgcWebServerTests
    {
        public WcsEmptyCapabilitiesTest()
        {
            m_server = OgcWebCoverageServerMock.Create("<WCS_Capabilities></WCS_Capabilities>");
        }
    }

    public class WcsValidCapabilitiesTest : OgcWebServerTests
    {
        public WcsValidCapabilitiesTest()
        {
            // Server
            var staticCapabilities = File.ReadAllText("../../Resources/OGC/wcs_valid_capabilities.xml");
            m_server = OgcWebCoverageServerMock.Create(staticCapabilities);
            m_expectedServerMetadata.Name = "CropScape WCS Server";
            m_expectedServerMetadata.Description = "Cropland Data Layer WCS Service";
            // Datasets
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "the 2009 Cropland Data Layer",
            });
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "the 2014 Cropland Data Layer",
            });
            m_expectedDataSetsMetadata.Add(new SimpleMetadata
            {
                Name = "the 2013 Cropland Data Layer",
            });
        }
    }

    #region Mock classes

    public class OgcWebMapServerMock : OgcWebMapServer
    {
        protected OgcWebMapServerMock(string url, string staticCapabilities) :
            base(url, null, new OgcWebMapService())
        {
            // Clean the data
            Catalog.Metadata = new SimpleMetadata();
            Catalog.DataSets = new List<DataSet>();
            // Instead of communicating with a real server, mock the server capabilities
            m_capabilities = new XmlDocument();
            m_capabilities.LoadXml(staticCapabilities);
            ParseCapabilites();
        }

        public static OgcWebMapServerMock Create(string staticCapabilities)
        {
            try
            {
                // Use a wwell known test url so the base constructor shouldn't throw
                return new OgcWebMapServerMock(
                    "http://mrdata.usgs.gov/services/tn?service=WMS&version=1.3.0&request=GetCapabilites",
                    staticCapabilities
                    );
            }
            catch (Exception)
            {
                // The base constructor has thrown an exception
                return null;
            }
        }
    }

    public class OgcWebFeatureServerMock : OgcWebFeatureServer
    {
        protected OgcWebFeatureServerMock(string url, string staticCapabilities) :
            base(url, null, new OgcWebFeatureService())
        {
            // Clean the data
            Catalog.Metadata = new SimpleMetadata();
            Catalog.DataSets = new List<DataSet>();
            // Instead of communicating with a real server, mock the server capabilities
            m_capabilities = new XmlDocument();
            m_capabilities.LoadXml(staticCapabilities);
            ParseCapabilites();
        }

        public static OgcWebFeatureServerMock Create(string staticCapabilities)
        {
            try
            {
                // Use a well known test url so the base constructor shouldn't throw
                return new OgcWebFeatureServerMock(
                    "http://afromaison.grid.unep.ch:8080/geoserver/ows?service=WFS&version=1.1.0",
                    staticCapabilities
                    );
            }
            catch (Exception)
            {
                // The base constructor has thrown an exception
                return null;
            }
        }

        public new string DataSetDescription(string title, string abst)
        {
            return base.DataSetDescription(title, abst);
        }
    }

    public class OgcWebCoverageServerMock : OgcWebCoverageServer
    {
        protected OgcWebCoverageServerMock(string url,string staticCapabilities) :
            base(url, null, new OgcWebCoverageService())
        {
            // Clean the data
            Catalog.Metadata = new SimpleMetadata();
            Catalog.DataSets = new List<DataSet>();
            // Instead of communicating with a real server, mock the server capabilities
            m_capabilities = new XmlDocument();
            m_capabilities.LoadXml(staticCapabilities);
            ParseCapabilites();
        }

        public static OgcWebCoverageServerMock Create(string staticCapabilities)
        {
            try
            {
                // Use a well known test url so the base constructor shouldn't throw
                return new OgcWebCoverageServerMock(
                    "http://gis.csiss.gmu.edu/cgi-bin/cdl_services?service=WCS&version=1.0.0&request=GetCapabilities",
                    staticCapabilities
                    );
            }
            catch (Exception)
            {
                // The base constructor has thrown an exception
                return null;
            }
        }

        public new string DataSetDescription(string title, string abst)
        {
            return base.DataSetDescription(title, abst);
        }
    }
    #endregion // Mock classes
}
