/******************************************************************************
ReportRepository.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using NHibernate;

using Pyxis.Utilities.ReportsWarehouse.Domain;

namespace Pyxis.Utilities.ReportsWarehouse.Repositories
{
    /// <summary>
    /// Repository class for Report table.  
    /// Responsible for managing transactions with database.
    /// </summary>
    public class ReportRepository : IReportRepository
    {
        /// <summary>
        /// Adds specified Report record to database.
        /// </summary>
        /// <param name="report">Report record.</param>
        public void Add(Report report)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Save(report);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Updates corresponding Report record in database.
        /// </summary>
        /// <param name="report">Report record.</param>
        public void Update(Report report)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Update(report);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Removes corresponding Report record from database.
        /// </summary>
        /// <param name="report">Report record.</param>
        public void Remove(Report report)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Delete(report);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Gets Report record from database by internal id.
        /// </summary>
        /// <param name="id">Internal database Report record id.</param>
        /// <returns>Report record.</returns>
        public Report Get(int id)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                return session.Get<Report>(id);
            }
        }

        /// <summary>
        /// Get all Report records from database with matching sending 
        /// node and report name.
        /// </summary>
        /// <param name="sendingNode">The sending node.</param>
        /// <param name="reportName">Name of the report.</param>
        /// <returns>Collection of Report records.</returns>
        public ICollection<Report> Get(Node sendingNode, string reportName)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                var reportList = session
                    .CreateQuery("FROM Report r WHERE r.SendingNode=:sn AND r.Name=:rn")
                    .SetString("sn", sendingNode.id.ToString())
                    .SetString("rn", reportName)
                    .List<Report>();
                return reportList;
            }
        }

        /// <summary>
        /// Get all Report records from the database.
        /// </summary>
        /// <returns>Collection of Report records.</returns>
        public ICollection<Report> GetAll()
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                var reportList = session
                    .CreateCriteria(typeof(Report))
                    .List<Report>();
                return reportList;
             }
        }
    }
}
