/******************************************************************************
ReceivedQueryRepository.cs

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
    /// Repository class for ReceivedQuery table.  
    /// Responsible for managing transactions with database.
    /// </summary>
    public class ReceivedQueryRepository : IReceivedQueryRepository
    {
        /// <summary>
        /// Adds the specified ReceivedQuery record to database.
        /// </summary>
        /// <param name="query">ReceivedQuery record.</param>
        public void Add(ReceivedQuery query)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Save(query);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Updates corresponding ReceivedQuery record in database.
        /// </summary>
        /// <param name="query">ReceivedQuery record.</param>
        public void Update(ReceivedQuery query)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Update(query);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Removes corresponding ReceivedQuery record from database.
        /// </summary>
        /// <param name="query">ReceivedQuery record.</param>
        public void Remove(ReceivedQuery query)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Delete(query);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Gets ReceivedQuery record from database by internal id.
        /// </summary>
        /// <param name="id">Internal database ReceivedQuery record id.</param>
        /// <returns>ReceivedQuery record.</returns>
        public ReceivedQuery GetById(int id)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                return session.Get<ReceivedQuery>(id);
            }
        }

        /// <summary>
        /// Gets all ReceivedQuery records from database.
        /// </summary>
        /// <returns>Collection of ReceivedQuery records.</returns>
        public ICollection<ReceivedQuery> GetAll()
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                var queryList = session
                    .CreateCriteria(typeof(ReceivedQuery))
                    .List<ReceivedQuery>();
                return queryList;
             }
        }
    }
}
