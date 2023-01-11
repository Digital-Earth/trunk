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
    /// Repository class for DataTransferred table.  
    /// Responsible for managing transactions with database.
    /// </summary>
    public class DataTransferredRepository : IDataTransferredRepository
    {
        /// <summary>
        /// Adds the specified DataTransferred record to database.
        /// </summary>
        /// <param name="dataRecord">DataTransferred record.</param>
        public void Add(DataTransferred dataRecord)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Save(dataRecord);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Updates the corresponding DataTransferred record in the database.
        /// </summary>
        /// <param name="dataRecord">DataTransferred record.</param>
        public void Update(DataTransferred dataRecord)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Update(dataRecord);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Removes the corresponding DataTransferred record from database.
        /// </summary>
        /// <param name="dataRecord">DataTransferred record.</param>
        public void Remove(DataTransferred dataRecord)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Delete(dataRecord);
                transaction.Commit();
            }
        }

        /// <summary>
        ///  Gets DataTransferred record from database by internal id.
        /// </summary>
        /// <param name="id">Internal database DataTransferred record id.</param>
        /// <returns>DataTransferred record.</returns>
        public DataTransferred GetById(int id)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                return session.Get<DataTransferred>(id);
            }
        }

        /// <summary>
        /// Gets all DataTransferred records from the database.
        /// </summary>
        /// <returns>Collection for DataTranferred records.</returns>
        public ICollection<DataTransferred> GetAll()
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                var recordList = session
                    .CreateCriteria(typeof(DataTransferred))
                    .List<DataTransferred>();
                return recordList;
             }
        }
    }
}
