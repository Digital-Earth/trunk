/******************************************************************************
DataSet.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using NHibernate;
using NHibernate.Cfg;
using NHibernate.Tool.hbm2ddl;

using NUnit.Framework;

namespace Pyxis.Utilities.ReportsWarehouse.Domain
{
    public class DataSet
    {
        public virtual int id
        { get; set; }

        public virtual Guid Ident
        { get; set; }

        public virtual string Name
        { get; set; }

        public virtual string Description
        { get; set; }

        public override bool Equals(object obj)
        {
            // Check for null values and compare run-time types.
            if (obj == null)
                return false;

            DataSet ds = (DataSet)obj;
            return id == ds.id && Ident == ds.Ident && Name == ds.Name && Description == ds.Description;
        }

        public override int GetHashCode()
        {
            return id;
        }
    }


    namespace Tests
    {            
        [TestFixture]
        public class GenerateSchema_DataSetFixture
        {
            private ISessionFactory m_sessionFactory;
            private Configuration m_configuration;

            [TestFixtureSetUp]
            public void TestFixtureSetUp()
            {
                m_configuration = new Configuration();
                m_configuration.Configure();
                m_configuration.AddAssembly(typeof(DataSet).Assembly);
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
                cfg.AddAssembly(typeof(DataSet).Assembly);

                new SchemaExport(cfg).Execute(false, true, false);
            }

        }

    }

}
