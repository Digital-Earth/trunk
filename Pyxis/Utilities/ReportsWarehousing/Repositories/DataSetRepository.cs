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
    /// Repository class for DataSet table.  
    /// Responsible for managing transactions with database.
    /// </summary>
    public class DataSetRepository : IDataSetRepository
    {
        /// <summary>
        /// Adds the DataSet record to database.
        /// </summary>
        /// <param name="ds">DataSet record.</param>
        public void Add(DataSet ds)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Save(ds);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Updates the corresponding DataSet record in the database.
        /// </summary>
        /// <param name="ds">DataSet record.</param>
        public void Update(DataSet ds)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Update(ds);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Removes the corresponding DataSet record from the database.
        /// </summary>
        /// <param name="ds">DataSet record.</param>
        public void Remove(DataSet ds)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Delete(ds);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Gets DataSet record from database by internal id.
        /// </summary>
        /// <param name="id">Internal database DataSet record id.</param>
        /// <returns>DataSet record.</returns>
        public DataSet GetById(int id)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                return session.Get<DataSet>(id);
            }
        }

        /// <summary>
        /// Gets DataSet record the DataSet's Guid identification.
        /// </summary>
        /// <param name="ident">DataSet Guid identification.</param>
        /// <returns>DataSet record.</returns>
        public DataSet GetByGuid(Guid ident)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                DataSet ds = session
                    .CreateQuery("FROM DataSet ds WHERE ds.Ident=:ident")
                    .SetString("ident", ident.ToString())
                    .UniqueResult<DataSet>();
                return ds;
            }
        }

        /// <summary>
        /// Gets all DataSet records from database.
        /// </summary>
        /// <returns>Collection of DataSet records.</returns>
        public ICollection<DataSet> GetAll()
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                var dsList = session
                    .CreateCriteria(typeof(DataSet))
                    .List<DataSet>();
                return dsList;
             }
        }
    }
}
