using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.Hosting;
using System.Web.Http.OData;
using System.Web.Http.OData.Builder;
using System.Web.Http.OData.Query;
using System.Web.Http.Routing;
using LicenseServer.Controllers;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using LicenseServer.Tests.Models;
using MongoDB.AspNet.Identity;
using NUnit.Framework;
using Pyxis.Contract.Publishing;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using MultiDomainGeoSource = LicenseServer.Models.Mongo.MultiDomainGeoSource;

namespace LicenseServer.Tests.Controllers
{
    [TestFixture]
    class GeoSourceControllerTests
    {
        private MockMongoDBEntities m_context;
        private GeoSourceController m_testController;
        
        private static ODataConventionModelBuilder s_modelBuilder = new ODataConventionModelBuilder();
        private static ODataQueryContext s_queryContext;
        private static ODataQueryOptions<MultiDomainGeoSource> s_queryOptions;

        static GeoSourceControllerTests()
        {
            s_modelBuilder.EntitySet<MultiDomainGeoSource>("MultiDomainGeoSource");
            s_modelBuilder.EntitySet<Domain>("Domain");
            s_modelBuilder.EntitySet<Style>("Style");
            s_modelBuilder.EntitySet<GeoSource>("GeoSource");
            s_modelBuilder.EntitySet<Metadata>("Metadata");
            s_modelBuilder.EntitySet<ImmutableMetadata>("ImmutableMetadata");
            s_modelBuilder.EntitySet<SimpleMetadata>("SimpleMetadata");
            s_modelBuilder.EntitySet<PipelineSpecification>("PipelineSpecification");
            s_modelBuilder.EntitySet<PipelineSpecification.FieldSpecification>("PipelineSpecification+FieldSpecification");
            s_modelBuilder.EntitySet<AggregateComment>("AggregateComment");
            s_modelBuilder.EntitySet<Comment>("Comment");
            s_queryContext = new ODataQueryContext(s_modelBuilder.GetEdmModel(), typeof(MultiDomainGeoSource));
            s_queryOptions = new ODataQueryOptions<MultiDomainGeoSource>(s_queryContext, new HttpRequestMessage());
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
                BaseUri = "http://localhost/api/v1/GeoSource",
                RouteTemplate = "api/v1/GeoSource/{id}",
                CurrentUserIdentity = MockMongoDBEntities.GenerateUserIdentity()
            };

            m_testController = new GeoSourceController(setup);
        }

        [Test]
        public void Get_GeoSources_ReturnsAllGeoSources()
        {
            Setup(HttpMethod.Get);

            var geoSources = m_testController.Get(s_queryOptions).ToList();

            Assert.AreEqual(m_context.Resources.AsEnumerable().Count(r => r.Type == ResourceType.GeoSource), geoSources.Count);
        }

        [Test]
        public void Get_ExistingGeoSourceId_ReturnsGeoSource()
        {
            Setup(HttpMethod.Get);

            var geoSourceResponse = m_testController.Get(MockMongoDBEntities.GeoSourceId);

            Assert.IsTrue(geoSourceResponse.StatusCode == HttpStatusCode.OK);
            MultiDomainGeoSource geoSource;
            Assert.IsTrue(geoSourceResponse.TryGetContentValue(out geoSource) != false);
            Assert.IsTrue(geoSource.Version == MockMongoDBEntities.Version);
        }

        [Test]
        public void Get_NonexistentGeoSourceId_ReturnsNotFound()
        {
            Setup(HttpMethod.Get);

            var geoSourceResponse = m_testController.Get(new Guid(9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9));

            Assert.IsTrue(geoSourceResponse.StatusCode == HttpStatusCode.NotFound);
        }

        [Test]
        public void Get_ExistingGeoSourceName_ReturnsGeoSource()
        {
            Setup(HttpMethod.Get);
            var existingGeoSource = m_context.Resources.First(r => r.Type == ResourceType.GeoSource);

            var geoSourceResponse = m_testController.Get(existingGeoSource.Metadata.Name);

            Assert.IsTrue(geoSourceResponse.StatusCode == HttpStatusCode.OK);
            MultiDomainGeoSource geoSource;
            Assert.IsTrue(geoSourceResponse.TryGetContentValue(out geoSource));
            Assert.IsTrue(existingGeoSource.Metadata.Name == geoSource.Metadata.Name);
        }

        [Test]
        public void Get_NonexistentGeoSourceName_ReturnsNotFound()
        {
            Setup(HttpMethod.Get);

            var geoSourceResponse = m_testController.Get("Nonexistent GeoSource Name");

            Assert.IsTrue(geoSourceResponse.StatusCode == HttpStatusCode.NotFound);
            GeoSource geoSource;
            Assert.IsFalse(geoSourceResponse.TryGetContentValue(out geoSource));
        }

        [Test]
        public void Post_GeoSource_ReturnsCreated()
        {
            Setup(HttpMethod.Post);

            var resources = m_context.Resources;
            var newVersion = new Guid(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);

            var numGeoSourcesBefore = resources.Count(r => r.Type == ResourceType.GeoSource);
            var geoSource = new MultiDomainGeoSource(new List<LicenseReference>(),
                new Metadata("New GeoSource", "New Description", MockMongoDBEntities.UserInformation, 
                    new List<Provider>(), "Test", new List<string>(), new List<string>(),
                    DateTime.MinValue, DateTime.MinValue, null, new List<ExternalUrl>(), VisibilityType.Public,
                    new LinkedList<AggregateComment>(), new AggregateRatings()), 
                    newVersion,
                    "", "", new List<ResourceReference>(),
                    PipelineDefinitionState.Active,
                    54321, new List<ResourceReference>(), new List<Guid>(), new List<Guid>());
            var response = m_testController.Post(geoSource);

            Assert.AreEqual(HttpStatusCode.Created, response.StatusCode);
            Assert.AreEqual(numGeoSourcesBefore + 1, resources.Count(r => r.Type == ResourceType.GeoSource));
        }

        [Test]
        public void Post_InvalidGeoSource_ReturnsBadRequest()
        {
            Setup(HttpMethod.Post);

            var resources = m_context.Resources;
            var newVersion = new Guid(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);

            var numGeoSourcesBefore = resources.Count(r => r.Type == ResourceType.GeoSource);
            // null name
            var geoSource = new MultiDomainGeoSource(Guid.Empty, new List<LicenseReference>(),
                new Metadata(null, "New Description", MockMongoDBEntities.UserInformation, 
                    new List<Provider>(), "Test", new List<string>(), new List<string>(),
                    DateTime.MinValue, DateTime.MinValue, null, new List<ExternalUrl>(), VisibilityType.Public,
                    new LinkedList<AggregateComment>(), new AggregateRatings()),
                    newVersion,
                    "", "", new List<ResourceReference>(),
                    PipelineDefinitionState.Active,
                    54321, new List<ResourceReference>(), new List<Guid>(), new List<Guid>());
            var response = m_testController.Post(geoSource);

            Assert.AreEqual(HttpStatusCode.BadRequest, response.StatusCode);
        }
        

        [Test]
        public void Post_ExistingGeoSource_ReturnsConflict()
        {
            Setup(HttpMethod.Post);

            var resources = m_context.Resources;

            var numGeoSourcesBefore = resources.Count(r => r.Type == ResourceType.GeoSource);
            var geoSource = new MultiDomainGeoSource(MockMongoDBEntities.GeoSourceId, new List<LicenseReference>(),
                new Metadata("New GeoSource", "New Description", MockMongoDBEntities.UserInformation, 
                    new List<Provider>(), "Test", new List<string>(), new List<string>(),
                    DateTime.MinValue, DateTime.MinValue, null, new List<ExternalUrl>(), VisibilityType.Public,
                    new LinkedList<AggregateComment>(), new AggregateRatings()),
                    Guid.Empty,
                    "", "", new List<ResourceReference>(),
                    PipelineDefinitionState.Active,
                    1234, new List<ResourceReference>(), new List<Guid>(), new List<Guid>());
            var response = m_testController.Post(geoSource);

            Assert.AreEqual(HttpStatusCode.Conflict, response.StatusCode);
            Assert.AreEqual(numGeoSourcesBefore, resources.Count(r => r.Type == ResourceType.GeoSource));
        }

        [Test]
        public void Put_ExistingGeoSource_ReturnsOk()
        {
            Setup(HttpMethod.Put);

            var resources = m_context.Resources;

            var numGeoSourcesBefore = resources.Count(r => r.Type == ResourceType.GeoSource);
            var putId = MockMongoDBEntities.GeoSourceId;
            var putVersion = MockMongoDBEntities.Version;
            var putDescription = "Test Description";
            var putDataSize = 1200;
            var geoSource = new MultiDomainGeoSource(new List<LicenseReference>(),
                new Metadata(null, putDescription, MockMongoDBEntities.UserInformation, new List<Provider>(), 
                    "Test", new List<string>(), new List<string>(),
                    DateTime.MinValue, DateTime.MinValue, null, new List<ExternalUrl>(), VisibilityType.Public,
                    new LinkedList<AggregateComment>(), new AggregateRatings()),
                Guid.Empty, null, null, new List<ResourceReference>(), null, putDataSize, new List<ResourceReference>(), new List<Guid>(), new List<Guid>());
            var response = m_testController.Put(putId, putVersion, geoSource);

            var modifiedGeoSource = resources.FirstOrDefault(r => r.Id == putId && r.Version != putVersion);
            Assert.AreEqual(HttpStatusCode.OK, response.StatusCode);
            Assert.AreEqual(numGeoSourcesBefore, resources.Count(r => r.Type == ResourceType.GeoSource));
            Assert.AreNotEqual(null, modifiedGeoSource);
            Assert.AreEqual(ResourceType.GeoSource, modifiedGeoSource.Type);
            Assert.AreEqual(putDescription, (modifiedGeoSource as MultiDomainGeoSource).Metadata.Description);
            Assert.AreEqual(putDataSize, (modifiedGeoSource as MultiDomainGeoSource).DataSize);
        }

        [Test]
        public void Put_NonexistentGeoSource_ReturnsNotFound()
        {
            Setup(HttpMethod.Put);

            var resources = m_context.Resources;

            var numGeoSourcesBefore = resources.Count(r => r.Type == ResourceType.GeoSource);
            var putId = new Guid(9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9);
            var putVersion = MockMongoDBEntities.Version;
            var putDescription = "Test Description";
            var putDataSize = 1200;
            var geoSource = new MultiDomainGeoSource(new List<LicenseReference>(),
                new Metadata(null, putDescription, MockMongoDBEntities.UserInformation, new List<Provider>(), 
                    "Test", new List<string>(), new List<string>(),
                    DateTime.MinValue, DateTime.MinValue, null, new List<ExternalUrl>(), VisibilityType.Public,
                    new LinkedList<AggregateComment>(), new AggregateRatings()),
                Guid.Empty, null, null, new List<ResourceReference>(), null, putDataSize, new List<ResourceReference>(), new List<Guid>(), new List<Guid>());
            var response = m_testController.Put(putId, putVersion, geoSource);

            Assert.AreEqual(HttpStatusCode.NotFound, response.StatusCode);
            Assert.AreEqual(numGeoSourcesBefore, resources.Count(r => r.Type == ResourceType.GeoSource));
        }

        [Test]
        public void Delete_ExistingGeoSource_ReturnsOk()
        {
            Setup(HttpMethod.Delete);

            var resources = m_context.Resources;

            var numGeoSourcesBefore = resources.Count(r => r.Type == ResourceType.GeoSource);
            var response = m_testController.Delete(MockMongoDBEntities.GeoSourceId);

            Assert.AreEqual(HttpStatusCode.OK, response.StatusCode);
            Assert.AreEqual(numGeoSourcesBefore - 1, resources.Count(r => r.Type == ResourceType.GeoSource));
        }

        [Test]
        public void Delete_NonexistentGeoSource_ReturnsNotFound()
        {
            Setup(HttpMethod.Delete);

            var resources = m_context.Resources;

            var numGeoSourcesBefore = resources.Count(r => r.Type == ResourceType.GeoSource);
            var id = new Guid(9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9);
            var response = m_testController.Delete(id);

            Assert.AreEqual(HttpStatusCode.NotFound, response.StatusCode);
            Assert.AreEqual(numGeoSourcesBefore, resources.Count(r => r.Type == ResourceType.GeoSource));
        }
    }
}
