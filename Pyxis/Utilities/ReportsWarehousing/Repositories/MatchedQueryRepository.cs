/******************************************************************************
ReceivedQuery.cs

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
    /// Repository class for MatchedQuery table.
    /// Responsible for managing transactions with database.
    /// </summary>
    public class MatchedQueryRepository : IMatchedQueryRepository
    {
        /// <summary>
        /// Adds the specified MatchedQuery record to database.
        /// </summary>
        /// <param name="query">MatchedQuery record.</param>
        public void Add(MatchedQuery query)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Save(query);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Updates the corresponding MatchedQuery record in the database.
        /// </summary>
        /// <param name="query">MatchedQuery record.</param>
        public void Update(MatchedQuery query)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Update(query);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Removes the corresponding matchedQuery record from the database.
        /// </summary>
        /// <param name="query">MatchedQuery record.</param>
        public void Remove(MatchedQuery query)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Delete(query);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Gets MatchedQuery record from database by internal id.
        /// </summary>
        /// <param name="id">Internal database MatchedQuery record id.</param>
        /// <returns>MatchedQuery record.</returns>
        public MatchedQuery GetById(int id)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                return session.Get<MatchedQuery>(id);
            }
        }

        /// <summary>
        /// Gets all MatchedQuery records from database.
        /// </summary>
        /// <returns>Collection of MatchedQuery records.</returns>
        public ICollection<MatchedQuery> GetAll()
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                var queryList = session
                    .CreateCriteria(typeof(MatchedQuery))
                    .List<MatchedQuery>();
                return queryList;
             }
        }
    }
}
