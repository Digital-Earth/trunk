using System;
using NUnit.Framework;
using Pyxis.IO.Sources.OGC;

namespace Pyxis.IO.Test
{
    /// <summary>
    /// An abstract class that provides unit tests for implementation classes of Pyxis.IO.OGC.IOgcUrl
    /// </summary>
    [TestFixture]
    [Category("Integration")]   // allows tests marked "Integration" to be included and excluded from test run
    public abstract class OgcUrlTests
    {
        protected string m_urlFull;
        protected string m_urlNoVersion;
        protected string m_urlNoRequest;
        protected string m_service;
        protected string m_version;
        protected string m_request;

        protected abstract IOgcUrl CreateOgcUrl(string url);

        [Test]
        public void Service_FullUrl_ValueRetrieved()
        {
            var url = CreateOgcUrl(m_urlFull);
            Assert.True(m_service.Equals(url.Service, StringComparison.InvariantCultureIgnoreCase));
        }

        [Test]
        public void Version_FullUrl_ValueRetrieved()
        {
            var url = CreateOgcUrl(m_urlFull);
            Assert.True(m_version.Equals(url.Version, StringComparison.InvariantCultureIgnoreCase));
        }

        [Test]
        public void Request_FullUrl_ValueRetrieved()
        {
            var url = CreateOgcUrl(m_urlFull);
            Assert.True(m_request.Equals(url.Request, StringComparison.InvariantCultureIgnoreCase));
        }

        [Test]
        public void ServerUrl_FullUrl_PublicValuesNotIncluded()
        {
            var url = CreateOgcUrl(m_urlFull);
            // Recreate an OGC URL from the server URL part
            var serverUrl = CreateOgcUrl(url.ServerUrl);
            // Verify that the request is not there anymore
            // (service and version are fundamental attributes and might be set automatically)
            Assert.True(string.IsNullOrEmpty(serverUrl.Request));
        }

        [Test]
        public void ToString_FullUrl_FieldsMatch()
        {
            var url = CreateOgcUrl(m_urlFull);
            // Create a new URL from the string value of the first one
            var newUrl = CreateOgcUrl(url.ToString());
            // Verify that the public fields match (except ServerUrl, which might have been reformatted)
            Assert.AreEqual(url.Service, newUrl.Service);
            Assert.AreEqual(url.Version, newUrl.Version);
            Assert.AreEqual(url.Request, newUrl.Request);
        }

        [Test]
        public void Version_UrlNoVersion_ValueEmpty()
        {
            var url = CreateOgcUrl(m_urlNoVersion);
            Assert.True(string.IsNullOrEmpty(url.Version));
        }

        [Test]
        public void Request_UrlNoRequest_ValueEmpty()
        {
            var url = CreateOgcUrl(m_urlNoRequest);
            Assert.True(string.IsNullOrEmpty(url.Request));
        }

        [Test]
        public void Version_ResetVersion_ValueEmpty()
        {
            var url = CreateOgcUrl(m_urlFull);
            url.Version = null;
            Assert.True(string.IsNullOrEmpty(url.Version));
            // Recreate the URL to verify that the version is vanished
            var newUrl = CreateOgcUrl(url.ToString());
            Assert.True(string.IsNullOrEmpty(newUrl.Version));
        }

        [Test]
        public void Request_ResetRequest_ValueEmpty()
        {
            var url = CreateOgcUrl(m_urlFull);
            url.Request = null;
            Assert.True(string.IsNullOrEmpty(url.Request));
            // Recreate the URL to verify that the request is vanished
            var newUrl = CreateOgcUrl(url.ToString());
            Assert.True(string.IsNullOrEmpty(newUrl.Request));
        }

        [Test]
        public void Version_SetVersion_ValueRetrieved()
        {
            var url = CreateOgcUrl(m_urlNoVersion);
            url.Version = m_version;
            Assert.True(m_version.Equals(url.Version, StringComparison.InvariantCultureIgnoreCase));
            // Recreate the URL to verify that the version was memorized
            var newUrl = CreateOgcUrl(url.ToString());
            Assert.AreEqual(url.Version, newUrl.Version);
        }

        [Test]
        public void Request_SetRequest_ValueRetrieved()
        {
            var url = CreateOgcUrl(m_urlNoRequest);
            url.Request = m_request;
            Assert.True(m_request.Equals(url.Request, StringComparison.InvariantCultureIgnoreCase));
            // Recreate the URL to verify that the version was memorized
            var newUrl = CreateOgcUrl(url.ToString());
            Assert.AreEqual(url.Request, newUrl.Request);
        }
    }

    // Instantiate the tests

    public class OgcWebMapUrlTests : OgcUrlTests
    {
        [SetUp]
        public void Setup()
        {
            m_urlFull = "http://mrdata.usgs.gov/services/tn?service=WMS&version=1.3.0&request=GetCapabilites";
            m_urlNoVersion = "http://mrdata.usgs.gov/services/tn?service=WMS&request=GetCapabilites";
            m_urlNoRequest = "http://mrdata.usgs.gov/services/tn?service=WMS&version=1.3.0";
            m_service = "wms";
            m_version = "1.3.0";
            m_request = "GetCapabilites";
        }

        protected override IOgcUrl CreateOgcUrl(string url)
        {
            return new OgcWebMapUrl(url);
        }
    }

    public class OgcWebFeatureUrlTests : OgcUrlTests
    {
        [SetUp]
        public void Setup()
        {
            m_urlFull = "http://preview.grid.unep.ch:8080/geoserver/ows?service=WFS&version=1.1.0&request=GetFeature&typeName=preview:xx0707feet";
            m_urlNoVersion = "http://preview.grid.unep.ch:8080/geoserver/ows?service=WFS&request=GetFeature&typeName=preview:xx0707feet";
            m_urlNoRequest = "http://preview.grid.unep.ch:8080/geoserver/ows?service=WFS&version=1.1.0";
            m_service = "wfs";
            m_version = "1.1.0";
            m_request = "GetFeature";
        }

        protected override IOgcUrl CreateOgcUrl(string url)
        {
            return new OgcWebFeatureUrl(url);
        }
    }

    public class OgcWebCoverageUrlTests : OgcUrlTests
    {
        [SetUp]
        public void Setup()
        {
            m_urlFull = "http://preview.grid.unep.ch:8080/geoserver/ows?service=WCS&version=1.0.0&request=GetCoverage&identifier=preview:cs_ecoexp";
            m_urlNoVersion = "http://preview.grid.unep.ch:8080/geoserver/ows?service=WCS&request=GetCoverage&identifier=preview:cs_ecoexp";
            m_urlNoRequest = "http://preview.grid.unep.ch:8080/geoserver/ows?service=WCS&version=1.0.0";
            m_service = "wcs";
            m_version = "1.0.0";
            m_request = "GetCoverage";
        }

        protected override IOgcUrl CreateOgcUrl(string url)
        {
            return new OgcWebCoverageUrl(url);
        }

        [Test]
        public void SingleAxesSubsetEncodedCorrectly()
        {
            var url = "http://preview.grid.unep.ch:8080/geoserver/ows?version=2.0.0&service=WCS&request=GetCoverage&coverageid=preview:cs_ecoexp&subset=time(1)";
            var esacpedUrl = "http://preview.grid.unep.ch:8080/geoserver/ows?version=2.0.0&service=WCS&request=GetCoverage&coverageid=preview%3Acs_ecoexp&subset=time%281%29";

            var wcsUrl = new OgcWebCoverageUrl(url);

            var subsets = wcsUrl.Subset;

            Assert.AreEqual(1,subsets.Count);
            Assert.AreEqual(new[] {"time"}, subsets.Keys);
            Assert.AreEqual(new[] { "1" }, subsets.Values);
            Assert.AreEqual(esacpedUrl, wcsUrl.ToString());
        }

        [Test]
        public void MultipleAxesSubsetEncodedCorrectly()
        {
            var url = "http://preview.grid.unep.ch:8080/geoserver/ows?version=2.0.0&service=WCS&request=GetCoverage&coverageid=preview:cs_ecoexp&subset=time(1),elevation(2)";
            var esacpedUrl = "http://preview.grid.unep.ch:8080/geoserver/ows?version=2.0.0&service=WCS&request=GetCoverage&coverageid=preview%3Acs_ecoexp&subset=time%281%29%2Celevation%282%29";

            var wcsUrl = new OgcWebCoverageUrl(url);

            var subsets = wcsUrl.Subset;

            Assert.AreEqual(2, subsets.Count);
            Assert.AreEqual(new[] { "time", "elevation" }, subsets.Keys);
            Assert.AreEqual(new[] { "1" , "2" }, subsets.Values);
            Assert.AreEqual("time(1),elevation(2)", wcsUrl.EncodedSubset );
            Assert.AreEqual(esacpedUrl, wcsUrl.ToString());
        }
    }

    public class OgcGeneralizedUrlTests : OgcUrlTests
    {
        [SetUp]
        public void Setup()
        {
            // inherited
            m_urlFull = "http://mrdata.usgs.gov/services/tn?service=WMS&version=1.3.0&request=GetCapabilites&width=100";
            m_urlNoVersion = "http://mrdata.usgs.gov/services/tn?service=WMS&request=GetCapabilites";
            m_urlNoRequest = "http://mrdata.usgs.gov/services/tn?service=WMS&version=1.3.0";
            m_service = "wms";
            m_version = "1.3.0";
            m_request = "GetCapabilites";
            // custom
            m_width_key = "width";
            m_width_value = "100";
        }

        protected override IOgcUrl CreateOgcUrl(string url)
        {
            return new OgcInvariantUrl(url);
        }

        // Custom tests for the functionality specific to this class
        string m_width_key;
        string m_width_value;

        [Test]
        public void GetAttribute_UrlFull_ValueRetrieved()
        {
            var url = new OgcInvariantUrl(m_urlFull);
            Assert.True(m_width_value.Equals(url.GetAttribute(m_width_key), StringComparison.InvariantCultureIgnoreCase));
        }

        [Test]
        public void SetAttribute_ExistingAttribute_ValueSet()
        {
            var url = new OgcInvariantUrl(m_urlFull);
            var newValue = "200";
            // Set and verify the new value
            url.SetAttribute(m_width_key, newValue);
            Assert.True(newValue.Equals(url.GetAttribute(m_width_key), StringComparison.InvariantCultureIgnoreCase));
            // Recreate the url and verify that the value is still there
            var newUrl = new OgcInvariantUrl(url.ToString());
            Assert.True(newValue.Equals(newUrl.GetAttribute(m_width_key), StringComparison.InvariantCultureIgnoreCase));
        }

        [Test]
        public void SetAttribute_NewAttribute_ValueSet()
        {
            var url = new OgcInvariantUrl(m_urlFull);
            var newKey = "height";
            var newValue = "200";
            // Set and verify the new value
            url.SetAttribute(newKey, newValue);
            Assert.True(newValue.Equals(url.GetAttribute(newKey), StringComparison.InvariantCultureIgnoreCase));
            // Recreate the url and verify that the value is still there
            var newUrl = new OgcInvariantUrl(url.ToString());
            Assert.True(newValue.Equals(newUrl.GetAttribute(newKey), StringComparison.InvariantCultureIgnoreCase));
        }

        [Test]
        public void SetAttribute_ResetAttribute_ValueEmpty()
        {
            var url = new OgcInvariantUrl(m_urlFull);
            // Set and verify the new value
            url.SetAttribute(m_width_key, null);
            Assert.True(string.IsNullOrEmpty(url.GetAttribute(m_width_key)));
            // Recreate the url and verify that the attribute was vanished
            var newUrl = new OgcInvariantUrl(url.ToString());
            Assert.True(string.IsNullOrEmpty(newUrl.GetAttribute(m_width_key)));
        }

        [Test]
        public void ClearAttributes_FullUrlAndNewAttribute_ValueEmpty()
        {
            var url = new OgcInvariantUrl(m_urlFull);
            // First add a new attribute
            var newKey = "height";
            var newValue = "200";
            url.SetAttribute(newKey, newValue);
            // Remove all attributes
            url.ClearAttributes();
            // Verify that both the initial and the new attributes are gone
            Assert.True(string.IsNullOrEmpty(url.GetAttribute(m_width_key)));
            Assert.True(string.IsNullOrEmpty(url.GetAttribute(newKey)));
            // Recreate the url and verify that the attributes are not there
            var newUrl = new OgcInvariantUrl(url.ToString());
            Assert.True(string.IsNullOrEmpty(newUrl.GetAttribute(m_width_key)));
            Assert.True(string.IsNullOrEmpty(newUrl.GetAttribute(newKey)));
        }
    }
}
