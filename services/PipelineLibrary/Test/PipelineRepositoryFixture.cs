/******************************************************************************
PipelineRepositoryFixture.cs

begin		: September 29, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using Iesi.Collections.Generic;

using NHibernate;
using NHibernate.Cfg;
using NHibernate.Tool.hbm2ddl;

using NUnit.Framework;

using Pyxis.Services.PipelineLibrary.Repositories;
using Pyxis.Services.PipelineLibrary.Domain;

namespace Pyxis.Services.PipelineLibrary.Test
{
    [TestFixture]
    [Category("Database Tests")]
    public class PipelineRepositoryFixture
    {
        private ISession Session
        {
            get
            {
                return m_nHibernateManager.Session;
            }
        }

        private IPipelineRepository Repository
        {
            get
            {
                return m_repository;
            }
        }
        private PipelineRepository m_repository = new PipelineRepository();

        private NHibernateManager m_nHibernateManager = new NHibernateManager();

        private string m_constCoverageGuid = "{8517369E-B91F-46be-BC8A-82E3F414D6AA}";

        #region Unit Test Setup

        /// <summary>Default constructor.</summary>
        /// <remarks>
        /// We don't use the NUnit [TestFixtureSetup] attribute because the 
        /// ManagedTestRunner does not yet support it.
        /// </remarks>
        public PipelineRepositoryFixture()
        {
            m_repository.Initialize(m_nHibernateManager);         
        }

        ~PipelineRepositoryFixture()
        {
            Repository.Uninitialize();
        }
        
        /// <summary>
        /// This method is called by every unit test 
        /// method at the beginning of their execution, so that they are 
        /// working against an empty database.
        /// </summary>
        /// <remarks>
        /// We don't use the NUnit [Setup] attribute because the 
        /// ManagedTestRunner does not yet support it.
        /// </remarks>
        public void ClearDatabase()
        {
            m_repository.ClearDatabase();
        }        

        #endregion Unit Test Setup

        #region Test CRUD Operations and Relationships

        [Test]
        public void CanAddNewPipeline()
        {
            ClearDatabase();

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);

            // get all the pipelines in the database
            IList<Pipeline> pipelines = Repository.GetAllPipelines();

            Assert.AreEqual(1, pipelines.Count);
            Assert.AreEqual(process.getProcName(), pipelines[0].Name);
            Assert.AreEqual(
                process.getProcDescription(), 
                pipelines[0].Description);
            Assert.AreEqual(
                process.getIdentity(), 
                pipelines[0].Identity.XmlIdentity);
            Assert.AreEqual(
                process.getSpec().getOutputInterfaces().Count, 
                pipelines[0].OutputTypes.Count);
        }

        [Test]
        public void CanGetExistingPipelineByOutputType()
        {
            ClearDatabase();

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);
            process = CreateTestProcess();
            Repository.Add(process);

            // outputs ICoverage
            IList<Pipeline> fromDb = Repository.GetByOutputType(new Guid(
                "8E66BE6E-B8DD-40f8-B496-3E843338D7AD"));
            Assert.AreEqual(2, fromDb.Count);

            // some random guid
            fromDb = Repository.GetByOutputType(new Guid(
                "8A55BE6E-B8DD-40f8-B496-4E843338D7AD"));
            Assert.IsNotNull(fromDb);
            Assert.AreEqual(0, fromDb.Count);
        }

        [Test]
        public void CanGetExistingPipelineByProcRef()
        {
            ClearDatabase();

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);

            Pipeline fromDb = Repository.GetByProcRef(new ProcRef(process));
            Assert.IsNotNull(fromDb);
            Assert.AreEqual(process.getProcName(), fromDb.Name);
            Assert.AreEqual(
                process.getProcDescription(),
                fromDb.Description);
            Assert.AreEqual(
                process.getIdentity(),
                fromDb.Identity.XmlIdentity);
            Assert.AreEqual(
                process.getSpec().getOutputInterfaces().Count,
                fromDb.OutputTypes.Count);            
        }

        [Test]
        public void CanGetAllExistingPipelines()
        {
            ClearDatabase();

            IList<Pipeline> fromDb = Repository.GetAllPipelines();
            Assert.IsNotNull(fromDb);
            Assert.AreEqual(0, fromDb.Count);

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);
            process = CreateTestProcess();
            Repository.Add(process);

            fromDb = Repository.GetAllPipelines();
            Assert.AreEqual(2, fromDb.Count);            
        }

        [Test]
        public void CanGetExistingPublishedPipelines()
        {
            ClearDatabase();

            IList<Pipeline> publishedPipelines =
                Repository.GetAllPublishedPipelines();
            Assert.IsNotNull(publishedPipelines);
            Assert.AreEqual(0, publishedPipelines.Count);

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);
            process = CreateTestProcess();
            Repository.Add(process);

            Repository.SetIsPublished(process, true);
            Repository.SetIsTemporary(process, false);
            Repository.SetIsHidden(process, false);

            publishedPipelines = 
                Repository.GetAllPublishedPipelines();
            Assert.AreEqual(1, publishedPipelines.Count);
            Assert.AreEqual(
                pyxlib.procRefToStr(new ProcRef(process)), 
                pyxlib.procRefToStr(publishedPipelines[0].ProcRef));
        }

        [Test]
        public void CanGetExistingDisplayablePipelines()
        {
            ClearDatabase();

            IList<Pipeline> pipelines =
                Repository.GetAllDisplayablePipelines();
            Assert.IsNotNull(pipelines);
            Assert.AreEqual(0, pipelines.Count);

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);
            process = CreateTestProcess();
            Repository.Add(process);

            Repository.SetIsTemporary(process, false);
            Repository.SetIsHidden(process, false);

            pipelines =
                Repository.GetAllDisplayablePipelines();
            Assert.AreEqual(1, pipelines.Count);
            Assert.AreEqual(
                pyxlib.procRefToStr(new ProcRef(process)),
                pyxlib.procRefToStr(pipelines[0].ProcRef));
        }

        [Test]
        public void CanGetExistingDisplayablePipelinesWithWhereClause()
        {
            ClearDatabase();

            IList<Pipeline> pipelines =
                Repository.GetAllDisplayablePipelines("WHERE Name LIKE '%Test%'");
            Assert.IsNotNull(pipelines);
            Assert.AreEqual(0, pipelines.Count);

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);
            process = CreateTestProcess();
            Repository.Add(process);

            Repository.SetIsTemporary(process, false);
            Repository.SetIsHidden(process, false);

            pipelines =
                Repository.GetAllDisplayablePipelines("WHERE Name LIKE '%Test%'");
            Assert.AreEqual(1, pipelines.Count);
        }

        [Test]
        public void CanGetExistingPipelineByIdentity()
        {
            ClearDatabase();

            IProcess_SPtr process = CreateTestProcess();

            IList<Pipeline> pipelines = Repository.GetByIdentity(
                process.getIdentity());
            Assert.IsNotNull(pipelines);
            Assert.AreEqual(0, pipelines.Count);
            
            Repository.Add(process);
            process = CreateTestProcess();
            Repository.Add(process);

            pipelines = Repository.GetByIdentity(
                process.getIdentity());
            Assert.AreEqual(2, pipelines.Count);
        }

        [Test]
        public void ExistsByProcRef()
        {
            ClearDatabase();

            IProcess_SPtr process = CreateTestProcess();
            ProcRef procRef = new ProcRef(process);

            Assert.IsFalse(Repository.Exists(procRef));
            Repository.Add(process);
            Assert.IsTrue(Repository.Exists(procRef));
        }

        [Test]
        public void ExistsByIdentity()
        {
            ClearDatabase();

            IProcess_SPtr process = CreateTestProcess();

            Assert.IsFalse(Repository.Exists(process.getIdentity()));
            Repository.Add(process);
            Assert.IsTrue(Repository.Exists(process.getIdentity()));
        }

        [Test]
        public void CanSetIsPublished()
        {
            ClearDatabase();

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);

            Assert.AreEqual(0, Repository.GetAllPublishedPipelines().Count);            
            Repository.SetIsPublished(process, true);
            Repository.SetIsHidden(process, false);
            Repository.SetIsTemporary(process, false);
            Assert.AreEqual(1, Repository.GetAllPublishedPipelines().Count);
            Repository.SetIsPublished(process, false);
            Assert.AreEqual(0, Repository.GetAllPublishedPipelines().Count);
        }

        #endregion Test CRUD Operations and Relationships

        #region Test Business Rules

        /// <summary>
        /// See if two pipelines with the same identity automatically associate 
        /// with the same identity in the database.
        /// </summary>
        [Test]
        public void DoesNewPipelineAssociateWithExistingIdentity()
        {
            ClearDatabase();

            IProcess_SPtr process = CreateTestProcess();
            Repository.Add(process);
            process = CreateTestProcess();
            Repository.Add(process);

            IList<Pipeline> allPipelines = Repository.GetAllPipelines();
            Assert.AreEqual(2, allPipelines.Count);
            Assert.AreEqual(allPipelines[0].Identity.XmlIdentity,
                allPipelines[0].Identity.XmlIdentity);            
        }

        #endregion Test Business Rules

        #region Helper Methods

        private IProcess_SPtr CreateTestProcess()
        {
            IUnknown_SPtr unknown = pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid(
                m_constCoverageGuid));
            IProcess_SPtr process = pyxlib.QueryInterface_IProcess(unknown);

            process.setProcName("Test Pipeline");
            process.setProcDescription("Test description.");

            return process;
        }

        private bool IsInCollection(Pipeline pipeline, ICollection<Pipeline> fromDb)
        {
            foreach (var item in fromDb)
            {
                if (pipeline.Id == item.Id)
                {
                    return true;
                }
            }
            return false;
        }        

        #endregion Helper Methods
    }
}
