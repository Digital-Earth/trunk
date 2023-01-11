using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.Hosting;
using System.Web.Http.Routing;
using LicenseServer.Controllers;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using LicenseServer.Tests.Models;
using NUnit.Framework;

namespace LicenseServer.Tests.Controllers
{
    [TestFixture]
    class GwssNotificationControllerTests
    {
        private MockMongoDBEntities m_context;
        private GwssNotificationController m_testController;

        private void Setup(HttpMethod method)
        {
            m_context = MockMongoDBEntities.SetupContext();
            var setup = new TestMongoSetup
            {
                Context = m_context,
                Method = method,
                Configuration = new HttpConfiguration(),
                RouteName = "DefaultApi",
                BaseUri = "http://localhost/api/v1/GwssNotification",
                RouteTemplate = "api/v1/GwssNotification/{id}"
            };

            m_testController = new GwssNotificationController(setup);
        }

        [Test]
        public void Get_Empty_ReturnsAllGwsses()
        {
            Setup(HttpMethod.Get);

            var geoSources = m_testController.Get().ToList();

            Assert.AreEqual(m_context.Resources.AsEnumerable().Where(r => r.Type == Pyxis.Contract.Publishing.ResourceType.GeoSource).Count(), geoSources.Count);
        }

        [Test]
        public void Get_ExistingId_ReturnsGwssWithSpecifiedId()
        {
            Setup(HttpMethod.Get);

            var gwss = m_testController.Get(MockMongoDBEntities.GwssId);

            Assert.IsTrue(gwss != null);
            Assert.IsTrue(gwss.Id == MockMongoDBEntities.GwssId);
        }

        [Test]
        public void Get_NonexistentId_ReturnsNull()
        {
            Setup(HttpMethod.Get);

            var gwss = m_testController.Get(new Guid(9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9));

            Assert.IsTrue(gwss == null);
        }

        [Test]
        public void Post_Gwss_ReturnsCreated()
        {
            Setup(HttpMethod.Post);

            var response = m_testController.Post(MockMongoDBEntities.GwssId, m_context.Gwsses.First().Status);

            Assert.IsTrue(response.StatusCode == HttpStatusCode.Created);
        }

        [Test]
        public void Delete_ExistingGwss_ReturnsOK()
        {
            Setup(HttpMethod.Delete);
            
            var gwsses = m_context.Gwsses;
            var numGwssesBefore = gwsses.Count;

            var response = m_testController.Delete(MockMongoDBEntities.GwssId);

            Assert.IsTrue(response.StatusCode == HttpStatusCode.OK);
            Assert.AreEqual(numGwssesBefore - 1, gwsses.Count);
        }

        [Test]
        public void Delete_NonexistentGwss_ReturnsOk()
        {
            Setup(HttpMethod.Delete);

            var gwsses = m_context.Gwsses;
            var numGwssesBefore = gwsses.Count;

            var response = m_testController.Delete(new Guid(9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9));

            Assert.IsTrue(response.StatusCode == HttpStatusCode.OK);
            Assert.AreEqual(numGwssesBefore, gwsses.Count);
        }
    }
}