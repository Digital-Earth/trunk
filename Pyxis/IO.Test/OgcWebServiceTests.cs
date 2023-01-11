using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using Pyxis.IO.Sources.OGC;
using Pyxis.IO.Sources.Remote;

namespace Pyxis.IO.Test
{
    /// <summary>
    /// A generic class that provides unit tests for implementation classes of IOgcWebServive
    /// </summary>
    public class OgcWebServiceTests<T> where T : IWebDataSetDiscoveryService, new()
    {
        /// <summary>
        /// The service to run the tests on
        /// </summary>
        private IWebDataSetDiscoveryService m_service;

        /// <summary>
        /// Contains lists of URLs that are each valid for a particular kind of OGC service
        /// </summary>
        private Dictionary<string, List<string>> m_validUrls;

        /// <summary>
        /// List of URLs invalid for any kind of OGC service
        /// </summary>
        private List<string> m_invalidUrls;

        [SetUp]
        public void Setup()
        {
            // Create a service to run the tests on
            m_service = new T();
            // Create test data
            m_validUrls = new Dictionary<string, List<string>>();
            {
                // Web Map Service
                var service = new OgcWebMapService();
                m_validUrls[service.ServiceIdentifier] = new List<string>();

                m_validUrls[service.ServiceIdentifier].Add("http://mrdata.usgs.gov/services/tn?service=WMS&version=1.3.0&request=GetCapabilites");
                m_validUrls[service.ServiceIdentifier].Add("http://preview.grid.unep.ch:8080/geoserver/ows?version=1.3.0&service=wms");
            }
            {
                // Web Feature Service
                var service = new OgcWebFeatureService();
                m_validUrls[service.ServiceIdentifier] = new List<string>();

                m_validUrls[service.ServiceIdentifier].Add("http://afromaison.grid.unep.ch:8080/geoserver/ows?service=WFS&version=1.1.0");
                m_validUrls[service.ServiceIdentifier].Add("http://preview.grid.unep.ch:8080/geoserver/ows?version=1.1.0&service=wfs&request=GetFeature&typeName=preview:xx0707feet");
            }
            {
                // Web Coverage Service
                var service = new OgcWebCoverageService();
                m_validUrls[service.ServiceIdentifier] = new List<string>();

                m_validUrls[service.ServiceIdentifier].Add("http://gis.csiss.gmu.edu/cgi-bin/cdl_services?version=1.0.0&service=WCS&request=GetCapabilities");
                m_validUrls[service.ServiceIdentifier].Add("http://129.194.231.213:8080/geoserver/eg_scenarios/ows?service=wcs&request=GetCoverage");
            }
            {
                // Invalid URLs
                m_invalidUrls = new List<string>();
                m_invalidUrls.Add("http://mrdata.usgs.gov/services/tn?request=WMS&version=1.3.0");
                m_invalidUrls.Add("http://preview.grid.unep.ch:8080/geoserver/ows?version=1.3.0&service=WMS,WFS");
                m_invalidUrls.Add("http://afromaison.grid.unep.ch:8080/geoserver/ows?service=WFS;version=1.1.0");
                m_invalidUrls.Add("http://preview.grid.unep.ch:8080/geoserver/ows?service=1.1.0&version=WFS&request=GetFeature&typeName=preview:xx0707feet");
                m_invalidUrls.Add("http://gis.csiss.gmu.edu/cgi-bin/cdl_services?version=1.0.0&request=GetCapabilities");
                m_invalidUrls.Add("http://129.194.231.213:8080/geoserver/eg_scenarios/ows?service=OGC&request=GetCoverage");
            }
        }

        public void IsUrlValid_ValidUrls_ReturnsTrue()
        {
            // This is a positive test.
            // Skip the unfamiliar URLs, verify all registered valid URLs.
            foreach (var url in m_validUrls.Keys
                .Where(key =>
                key == m_service.ServiceIdentifier)
                .SelectMany(key => m_validUrls[key])
                )
            {
                Assert.True(m_service.IsUriSupported(url));
            }
        }

        public void IsUrlValid_AlienValidUrls_ReturnsFalse()
        {
            // This is a false positive test
            // Skip the valid URLs, test all alien valid URLs
            foreach (var url in m_validUrls.Keys
                .Where(key =>
                key != m_service.ServiceIdentifier)
                .SelectMany(key => m_validUrls[key])
                )
            {
                Assert.False(m_service.IsUriSupported(url));
            }
        }

        public void IsUrlValid_InvalidUrls_ReturnsFalse()
        {
            // Verification should fail on every URL
            foreach (var url in m_invalidUrls)
            {
                Assert.False(m_service.IsUriSupported(url));
            }
        }
    }

    // Instantiate the tests
    [TestFixture]
    [Category("Integration")] // allows tests marked "Integration" to be included and excluded from test run
    public class OgcWebMapServiceTestses : OgcWebServiceTests<OgcWebMapService>
    {
        [Test]
        public new void IsUrlValid_ValidUrls_ReturnsTrue()
        {
            base.IsUrlValid_ValidUrls_ReturnsTrue();
        }

        [Test]
        public new void IsUrlValid_AlienValidUrls_ReturnsFalse()
        {
            base.IsUrlValid_AlienValidUrls_ReturnsFalse();
        }

        [Test]
        public new void IsUrlValid_InvalidUrls_ReturnsFalse()
        {
            base.IsUrlValid_InvalidUrls_ReturnsFalse();
        }
    }

    [TestFixture]
    [Category("Integration")] // allows tests marked "Integration" to be included and excluded from test run
    public class OgcWebCoverageServiceTestses : OgcWebServiceTests<OgcWebCoverageService>
    {
        [Test]
        public new void IsUrlValid_ValidUrls_ReturnsTrue()
        {
            base.IsUrlValid_ValidUrls_ReturnsTrue();
        }

        [Test]
        public new void IsUrlValid_AlienValidUrls_ReturnsFalse()
        {
            base.IsUrlValid_AlienValidUrls_ReturnsFalse();
        }

        [Test]
        public new void IsUrlValid_InvalidUrls_ReturnsFalse()
        {
            base.IsUrlValid_InvalidUrls_ReturnsFalse();
        }
    }

    [TestFixture]
    [Category("Integration")] // allows tests marked "Integration" to be included and excluded from test run
    public class OgcWebFeatureServiceTestses : OgcWebServiceTests<OgcWebFeatureService>
    {
        [Test]
        public new void IsUrlValid_ValidUrls_ReturnsTrue()
        {
            base.IsUrlValid_ValidUrls_ReturnsTrue();
        }

        [Test]
        public new void IsUrlValid_AlienValidUrls_ReturnsFalse()
        {
            base.IsUrlValid_AlienValidUrls_ReturnsFalse();
        }

        [Test]
        public new void IsUrlValid_InvalidUrls_ReturnsFalse()
        {
            base.IsUrlValid_InvalidUrls_ReturnsFalse();
        }
    }
}
