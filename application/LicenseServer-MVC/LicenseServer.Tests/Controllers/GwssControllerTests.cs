/******************************************************************************
GwssControllerTests.cs

begin		: Sept. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.OData;
using System.Web.Http.OData.Builder;
using System.Web.Http.OData.Query;
using LicenseServer.Controllers;
using LicenseServer.DTOs;
using LicenseServer.Models;
using LicenseServer.Models.Mongo;
using LicenseServer.Tests.Models;
using NUnit.Framework;

namespace LicenseServer.Tests.Controllers
{
    [TestFixture]
    class GwssControllerTests
    {
        private MockMongoDBEntities m_context;
        private GwssController m_testController;
        
        private static ODataConventionModelBuilder s_modelBuilder = new ODataConventionModelBuilder();
        private static ODataQueryContext s_queryContext;
        private static ODataQueryOptions<GwssSummaryDTO> s_queryOptions;

        static GwssControllerTests()
        {
            s_modelBuilder.EntitySet<GwssSummaryDTO>("GwssSummaryDTO");
            s_queryContext = new ODataQueryContext(s_modelBuilder.GetEdmModel(), typeof(GwssSummaryDTO));
            s_queryOptions = new ODataQueryOptions<GwssSummaryDTO>(s_queryContext, new HttpRequestMessage());
        }

        [SetUp]
        protected void Setup()
        {
            m_context = MockMongoDBEntities.SetupContext();
            var setup = new TestMongoSetup
            {
                Context = m_context,
                Method = HttpMethod.Get,
                Configuration = new HttpConfiguration(),
                RouteName = "DefaultApi",
                BaseUri = "http://localhost/api/v1/Gwss",
                RouteTemplate = "api/v1/Gwss/{id}"
            };

            m_testController = new GwssController(setup);
        }

        [Test]
        public void GetServers_Empty_ReturnsAllServers()
        {
            var response = m_testController.GetServers(s_queryOptions);

            Assert.AreEqual(m_context.Gwsses.Count(), response.Count());
        }

        [Test]
        public void GetServer_Id_ReturnsGwssWithSpecifiedId()
        {
            var serverId = MockMongoDBEntities.GwssId;
            var response = m_testController.GetServer(serverId);

            Assert.AreEqual(serverId, Guid.Parse(response.NodeID));
        }
    }
}
