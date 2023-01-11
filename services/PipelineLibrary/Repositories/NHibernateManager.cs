/******************************************************************************
NHibernateManager.cs

begin		: September 29, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using NHibernate;
using NHibernate.Cfg;

using Pyxis.Services.PipelineLibrary.Domain;
using System.Diagnostics;

namespace Pyxis.Services.PipelineLibrary.Repositories
{
    /// <summary>
    /// Sets up the connection to the database and manages the NHibernate 
    /// session.
    /// </summary>
    public class NHibernateManager
    {
        /// <summary>
        /// The one and only instance of the NHibernateManager.
        /// </summary>
        public static NHibernateManager Instance
        {
            get
            {
                if (s_instance == null)
                {
                    s_instance = new NHibernateManager();
                }
                return s_instance;
            }
        }
        private static NHibernateManager s_instance;
        
        /// <summary>
        /// Gets an existing NHibernate session or opens one.  Only one 
        /// session should ever be opened throughtout the execution of WorldView.
        /// </summary>
        /// <value>The session.</value>
        internal ISession Session
        {
            get
            {
                if (m_session == null || !m_session.IsOpen)
                {
                    Trace.info("Opening NHibernate Session...");
                    m_session = m_sessionFactory.OpenSession();
                    Debug.Assert(m_session != null);
                }

                return m_session;
            }            
        }
        private ISession m_session;

        private ISessionFactory m_sessionFactory;

        private Configuration m_configuration = new Configuration();

        /// <summary>
        /// Default constructor; not to be made available except for unit test, 
        /// use static instance instead.
        /// </summary>
        internal NHibernateManager()
        {
        }

        /// <summary>
        /// Initializes NHibernate with the default database configuration.
        /// </summary>
        internal void Initialize()
        {
            m_configuration.Configure();
            CreateSessionFactory();
        }

        /// <summary>
        /// Initializes NHibernate with the provided database configuration.
        /// </summary>
        internal void Initialize(IDictionary<string, string> databaseConfiguration)
        {
            m_configuration.Properties = databaseConfiguration;

            try
            {
                CreateSessionFactory();
            }
            catch (Exception ex)
            {
                Trace.error(ex.ToString());
            }
        }

        private void CreateSessionFactory()
        {
            m_configuration.AddAssembly(typeof(Pipeline).Assembly);
            m_sessionFactory = m_configuration.BuildSessionFactory();

            // this statement either creates a new database with tables 
            // specified in the .hbm.xml files or if the database already 
            // exists, updates its schema while preserving all data
            new NHibernate.Tool.hbm2ddl.SchemaUpdate(m_configuration).Execute(
                false, true);
        }
    }
}