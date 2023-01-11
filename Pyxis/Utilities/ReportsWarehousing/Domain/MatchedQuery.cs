/******************************************************************************
MatchedQuery.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using NHibernate.Cfg;
using NHibernate.Tool.hbm2ddl;
using NHibernate;

using NUnit.Framework;

namespace Pyxis.Utilities.ReportsWarehouse.Domain
{
    public class MatchedQuery
    {
        public virtual int id
        { get; set; }

        public virtual Report Report
        { get; set; }

        public virtual DateTime TimeStamp
        { get; set; }

        public virtual Node SendingNode
        { get; set; }

        public virtual Node ReceivingNode
        { get; set; }

        public virtual DataSet DataSet
        { get; set; }

        public virtual int Hits
        { get; set; }
    }

    #region unit testing
    
    //--
    //-- unit testing
    //--
    namespace MatchedQueryTests
    {
        [TestFixture]
        public class GenerateSchema_Fixture
        {
            private ISessionFactory m_sessionFactory;
            private Configuration m_configuration;

            [TestFixtureSetUp]
            public void TestFixtureSetUp()
            {
                m_configuration = new Configuration();
                m_configuration.Configure();
                m_configuration.AddAssembly(typeof(Node).Assembly);

                m_sessionFactory = m_configuration.BuildSessionFactory();
            }

            [SetUp]
            public void SetupContext()
            {
                new SchemaExport(m_configuration).Execute(false, true, false);
                CreateInitialData();
            }

            private void CreateInitialData()
            {
            }

            [Test]
            public void CanGenerateSchema()
            {
                var cfg = new Configuration();
                cfg.Configure();
                cfg.AddAssembly(typeof(MatchedQuery).Assembly);

                new SchemaExport(cfg).Execute(false, true, false);
            }

            [Test]
            public void CanAddNewNode()
            {
                Node sendingNode = new Node
                {
                    Ident = new Guid("C4529CB9-90AB-4d39-A647-A3766A161941"), 
                    Name = "sending.node.com" 
                };

                Node receiveingNode = new Node
                {
                    Ident = new Guid("D1D78580-8639-4a29-8E27-1174618D4F51"), 
                    Name = "receiving.node.com" 
                };

                DataSet dataSet = new DataSet 
                { 
                    Ident = new Guid("EED819BA-FEBC-4875-98CD-059C0EB60916"), 
                    Name = "blue nothing", 
                    Description = "a test dataset for the unit test "
                };

                MatchedQuery query = new MatchedQuery 
                { 
                    TimeStamp = new DateTime(2005,10,31),
                    ReceivingNode = receiveingNode,
                    SendingNode = sendingNode,
                    DataSet = dataSet,
                    Hits = 32
                };

                IMatchedQueryRepository queryRepository = new Pyxis.Utilities.ReportsWarehouse.Repositories.MatchedQueryRepository();
                queryRepository.Add(query);

                // use session to try to load the query back
                using (ISession session = m_sessionFactory.OpenSession())
                {
                    MatchedQuery fromDb = session.Get<MatchedQuery>(query.id);
                    // Test that the node was successfully inserted
                    Assert.IsNotNull(fromDb);
                    Assert.AreNotSame(query, fromDb);
                    Assert.AreEqual(query.TimeStamp, fromDb.TimeStamp);
                    Assert.AreEqual(query.SendingNode, fromDb.SendingNode);
                    Assert.AreEqual(query.ReceivingNode, fromDb.ReceivingNode);
                    Assert.AreEqual(query.DataSet, fromDb.DataSet);
                    Assert.AreEqual(query.Hits, fromDb.Hits);
                }
            }

        }

    }

    #endregion
}
