using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Builder;
using System.Web.Http.OData.Query;
using LicenseServer.Controllers;
using LicenseServer.Extensions;
using LicenseServer.Models;
using LicenseServer.Tests.Models;
using Newtonsoft.Json.Linq;
using NUnit.Framework;
using Pyxis.Contract.Publishing;
using MultiDomainGeoSource = LicenseServer.Models.Mongo.MultiDomainGeoSource;
using Resource = Pyxis.Contract.Publishing.Resource;

namespace LicenseServer.Tests.Controllers
{
    [TestFixture]
    class MetadataControllerTests
    {
        private MockMongoDBEntities m_context;
        private MetadataController m_testController;
        
        private static ODataConventionModelBuilder s_modelBuilder = new ODataConventionModelBuilder();
        private static ODataQueryContext s_queryContext;
        private static ODataQueryOptions<Resource> s_queryOptions;
        private static ODataQueryContext s_queryStringContext;
        private static ODataQueryOptions<string> s_queryStringOptions;
        static MetadataControllerTests()
        {
            s_modelBuilder.EntitySet<Resource>("Resource");
            s_modelBuilder.EntitySet<Metadata>("Metadata");
            s_modelBuilder.EntitySet<ImmutableMetadata>("ImmutableMetadata");
            s_modelBuilder.EntitySet<SimpleMetadata>("SimpleMetadata");
            s_modelBuilder.EntitySet<PipelineSpecification>("PipelineSpecification");
            s_modelBuilder.EntitySet<PipelineSpecification.FieldSpecification>("PipelineSpecification+FieldSpecification");
            s_modelBuilder.EntitySet<Map.Group>("Map+Group");
            s_modelBuilder.EntitySet<Map.Item>("Map+Item");
            s_modelBuilder.EntitySet<Dashboard>("Dashboard");
            s_modelBuilder.EntitySet<Dashboard.StyledSelection>("Dashboard+StyledSelection");
            s_modelBuilder.EntitySet<Dashboard.Widget>("Dashboard+Widget");
            s_modelBuilder.EntitySet<MultiDomainGeoSource>("MultiDomainGeoSource");
            s_modelBuilder.EntitySet<Domain>("Domain");
            s_modelBuilder.EntitySet<Style>("Style");
            s_modelBuilder.EntitySet<JToken>("JToken");
            s_modelBuilder.EntitySet<JObject>("JObject");
            s_modelBuilder.EntitySet<JContainer>("JContainer");
            s_modelBuilder.EntitySet<JArray>("JArray");
            s_modelBuilder.EntitySet<JConstructor>("JConstructor");
            s_modelBuilder.EntitySet<JProperty>("JProperty");
            s_modelBuilder.EntitySet<JValue>("JValue");
            s_modelBuilder.EntitySet<JRaw>("JRaw");
            s_modelBuilder.EntitySet<AggregateComment>("AggregateComment");
            s_modelBuilder.EntitySet<Comment>("Comment");
            s_queryContext = new ODataQueryContext(s_modelBuilder.GetEdmModel(), typeof(Resource));
            s_queryOptions = new ODataQueryOptions<Resource>(s_queryContext, new HttpRequestMessage());
            s_queryStringContext = new ODataQueryContext(new ODataModelBuilder().GetEdmModel(), typeof(string));
            s_queryStringOptions = new ODataQueryOptions<string>(s_queryStringContext, new HttpRequestMessage());
        }

        private void Setup(HttpMethod method)
        {
            m_context = MockMongoDBEntities.SetupContext();
            var setup = new TestMongoSetup
            {
                Context = m_context,
                Method = method,
                Configuration = new HttpConfiguration(),
                RouteName = "DefaultApi",
                RouteTemplate = "api/v1/Metadata/{id}"
            };
            m_testController = new MetadataController(setup);
        }

        [Test]
        public void Get_MetaData_ReturnsAllResources()
        {
            Setup(HttpMethod.Get);

            var resources = m_context.Resources;
            var numResourcesBefore = resources.Count;

            var metadata = m_testController.Get(new Pyxis.Contract.Publishing.ResourceType[]{}, s_queryOptions).ToList();

            Assert.AreEqual(numResourcesBefore, metadata.Count);
        }

        [Test]
        public void Get_MetaDataTypes_ReturnsOnlyResourcesOfSpecifiedTypes()
        {
            Setup(HttpMethod.Get);
            var validTypes = new [] { Pyxis.Contract.Publishing.ResourceType.GeoSource, Pyxis.Contract.Publishing.ResourceType.License };

            var resources = m_context.Resources;
            var numOfValidTypeResources = resources.Count(r => validTypes.Contains(r.Type));
            
            var metadata = m_testController.Get(validTypes, s_queryOptions).ToList();

            Assert.AreEqual(numOfValidTypeResources, metadata.Count);
            foreach(var resource in metadata)
            {
                Assert.Contains((resource as Resource).Type, validTypes);
            }
        }

        [Test]
        public void Get_MetaDataSearchString_ReturnsMatchingResources()
        {
            Setup(HttpMethod.Get);

            var resources = m_context.Resources;
            var categoryString = "Test";
            var matchingResources = resources.Where(m => m.Metadata.Category.IndexOf(categoryString, StringComparison.OrdinalIgnoreCase) != -1).ToList();

            var metadata = m_testController.Get(new Pyxis.Contract.Publishing.ResourceType[]{}, categoryString, s_queryOptions).Cast<Resource>().ToList();

            Assert.AreEqual(matchingResources.Count, metadata.Count);
            foreach (var metadatum in metadata)
            {
                Assert.IsTrue(metadatum.Metadata.Category.IndexOf(categoryString, StringComparison.OrdinalIgnoreCase) != -1);
            }
        }

        [Test]
        public void GetTerms_NonEmptySearch_ReturnsTerms()
        {
            Setup(HttpMethod.Get);
            var search = "test";

            var suggestions = m_testController.GetTerms(new Pyxis.Contract.Publishing.ResourceType[] { }, search, s_queryStringOptions);

            Assert.IsNotNull(suggestions);
            Assert.IsInstanceOf(typeof(IEnumerable<string>), suggestions.Items);
            Assert.IsTrue(suggestions.Items.Any());
        }

        [Test]
        public void GetTerms_NullString_ReturnsEmptyResults()
        {
            Setup(HttpMethod.Get);
            string search = null;

            var suggestions = m_testController.GetTerms(new Pyxis.Contract.Publishing.ResourceType[] { }, search, s_queryStringOptions);

            Assert.IsNotNull(suggestions);
            Assert.IsInstanceOf(typeof(IEnumerable<string>), suggestions.Items);
            Assert.IsFalse(suggestions.Items.Any());
        }

        [Test]
        public void GetCompletiones_NullString_ReturnsEmptyResults()
        {
            Setup(HttpMethod.Get);
            string search = null;

            var suggestions = m_testController.GetCompletions(new Pyxis.Contract.Publishing.ResourceType[] { }, search, s_queryStringOptions);

            Assert.IsNotNull(suggestions);
            Assert.IsInstanceOf(typeof(IEnumerable<string>), suggestions.Items);
            Assert.IsFalse(suggestions.Items.Any());
        }

        [Test]
        public void GetMatches_NullString_ReturnsEmptyResults()
        {
            Setup(HttpMethod.Get);
            string search = null;

            var suggestions = m_testController.GetMatches(new[] { Pyxis.Contract.Publishing.ResourceType.GeoSource }, search, s_queryStringOptions);

            Assert.IsNotNull(suggestions);
        }
    }
}