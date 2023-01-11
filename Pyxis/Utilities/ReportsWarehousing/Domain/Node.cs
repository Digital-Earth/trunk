/******************************************************************************
Node.cs

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
    public class Node
    {
        public virtual int id
        { get; set; }

        public virtual Guid Ident
        { get; set; }

        public virtual string Name
        { get; set; }

        public override bool Equals(object obj) 
        { 
            // Check for null values and compare run-time types.
            if (obj == null )
                return false;

            Node n = (Node)obj;
            return id == n.id && Ident == n.Ident && Name == n.Name;
        }

        public override int GetHashCode()
        {
            return id;
        }

    }


    namespace Tests
    {
        [TestFixture]
        public class GenerateSchema_NodeFixture
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
                cfg.AddAssembly(typeof(Node).Assembly);

                new SchemaExport(cfg).Execute(false, true, false);
            }

            [Test]
            public void CanAddNewNode()
            {
                Node node = new Node 
                { 
                    Ident = new Guid("BE4A48BD-3A82-44f1-B3E4-40088A4679B3"), 
                    Name = "test.node.com" 
                };
                
                //INodeRepository repository = new Pyxis.Utilities.ReportsWarehouse.Repositories.NodeRepository();
                //repository.Add(node);

                //RepositoryHelper.

                RepositoryHelper.Node.Add(node);

                // use session to try to load the node
                using (ISession session = m_sessionFactory.OpenSession())
                {
                    Node fromDb = session.Get<Node>(node.id);
                    // Test that the node was successfully inserted
                    Assert.IsNotNull(fromDb);
                    Assert.AreNotSame(node, fromDb);
                    Assert.AreEqual(node.Ident, fromDb.Ident);
                    Assert.AreEqual(node.Name, fromDb.Name);
                }
            }

        }

    }
}
