/******************************************************************************
PipelineControllerTests.cs

begin		: Sept. 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System.Linq;
using System.Net.Http;
using System.Web.Http;
using LicenseServer.Controllers;
using LicenseServer.Models.Mongo;
using LicenseServer.Tests.Models;
using NUnit.Framework;
using Pyxis.Contract.Publishing;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using MultiDomainGeoSource = LicenseServer.Models.Mongo.MultiDomainGeoSource;

namespace LicenseServer.Tests.Controllers
{
    [TestFixture]
    public class PipelineControllerTests
    {
        private MockMongoDBEntities m_context;
        private PipelineController m_testController;

        private void Setup(HttpMethod method)
        {
            m_context = MockMongoDBEntities.SetupContext();
            var setup = new TestMongoSetup
            {
                Context = m_context,
                Method = method,
                Configuration = new HttpConfiguration(),
                RouteName = "DefaultApi",
                RouteTemplate = "api/v1/Pipeline/{id}"
            };
            m_testController = new PipelineController(setup);
        }

        [Test]
        public void GetPipelineInfoes_Empty_ReturnsAllPipelines()
        {
            Setup(HttpMethod.Get);

            var pipelines = m_testController.GetPipelineInfoes().ToList();

            Assert.AreEqual(m_context.Resources.Where(x => x.Type == ResourceType.GeoSource 
                && (x as MultiDomainGeoSource).State.Value != PipelineDefinitionState.Removed).Count(), pipelines.Count);
        }
    }
}
