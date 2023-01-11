/******************************************************************************
RepositoryHelper.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using NHibernate;
using NHibernate.Cfg;

using Pyxis.Utilities.ReportsWarehouse.Domain;


namespace Pyxis.Utilities.ReportsWarehouse
{
    /// <summary>
    /// Access all the repository classes through this helper.
    /// Keeps overhead at a minimum with repository creation and
    /// management.  Each repository links to a database table.
    /// </summary>
    class RepositoryHelper
    {
        private static IReportRepository m_reportRepository = null;
        private static INodeRepository m_nodeRepository = null;
        private static IDataSetRepository m_dataSetRepository = null;
        private static IMatchedQueryRepository m_matchedQueryRepository = null;
        private static IReceivedQueryRepository m_receivedQueryRepository = null;
        private static IDataTransferredRepository m_dataTransferredRepository = null;

        /// <summary>
        /// Gets the Report repository.
        /// </summary>
        /// <value>The Report repository.</value>
        public static IReportRepository Report
        {
            get
            {
                if (m_reportRepository == null)
                {
                    m_reportRepository = new Repositories.ReportRepository();
                }
                return m_reportRepository;
            }
        }

        /// <summary>
        /// Gets the Node repository.
        /// </summary>
        /// <value>The Node.</value>
        public static INodeRepository Node
        {
            get
            {
                if (m_nodeRepository == null)
                {
                    m_nodeRepository = new Repositories.NodeRepository();
                }
                return m_nodeRepository;
            }
        }

        /// <summary>
        /// Gets the DataSet repository.
        /// </summary>
        /// <value>The DataSet repository.</value>
        public static IDataSetRepository DataSet
        {
            get
            {
                if (m_dataSetRepository == null)
                {
                    m_dataSetRepository = new Repositories.DataSetRepository();
                }
                return m_dataSetRepository;
            }
        }

        /// <summary>
        /// Gets the MatchedQuery repository
        /// </summary>
        /// <value>The MatchedQuery repository.</value>
        public static IMatchedQueryRepository MatchedQuery
        {
            get
            {
                if (m_matchedQueryRepository == null)
                {
                    m_matchedQueryRepository = new Repositories.MatchedQueryRepository();
                }
                return m_matchedQueryRepository;
            }
        }

        /// <summary>
        /// Gets the ReceivedQuery repository.
        /// </summary>
        /// <value>The ReceivedQuery repository.</value>
        public static IReceivedQueryRepository ReceivedQuery
        {
            get
            {
                if (m_receivedQueryRepository == null)
                {
                    m_receivedQueryRepository = new Repositories.ReceivedQueryRepository();
                }
                return m_receivedQueryRepository;
            }
        }

        /// <summary>
        /// Gets the DataTransferred repository.
        /// </summary>
        /// <value>The DataTransferred repository.</value>
        public static IDataTransferredRepository DataTransferred
        {
            get
            {
                if (m_dataTransferredRepository == null)
                {
                    m_dataTransferredRepository = new Repositories.DataTransferredRepository();
                }
                return m_dataTransferredRepository;
            }
        }
    }
}
