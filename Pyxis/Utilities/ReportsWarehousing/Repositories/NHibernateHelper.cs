/******************************************************************************
NHibernateHelper.cs

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

using Pyxis.Utilities.ReportsWarehouse.Domain;

namespace Pyxis.Utilities.ReportsWarehouse.Repositories
{
    /// <summary>
    /// Hibrernate helper class, encapsulates the session information
    /// and ensures that only one session is open at a time.
    /// </summary>
    public class NHibernateHelper
    {
        private static ISessionFactory m_sessionFactory = null;

        private static ISessionFactory SessionFactory
        {
            get
            {
                if (m_sessionFactory == null)
                {
                    var configuration = new Configuration();
                    configuration.Configure();
                    configuration.AddAssembly(typeof(Node).Assembly);
                    m_sessionFactory = configuration.BuildSessionFactory();

                    new SchemaUpdate(configuration).Execute(false,true);
                }
                return m_sessionFactory;
            }
        }

        /// <summary>
        /// Opens hibernate database session.
        /// </summary>
        /// <returns></returns>
        public static ISession OpenSession()
        {
            return SessionFactory.OpenSession();
        }
    }
 }
