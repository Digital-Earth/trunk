/******************************************************************************
NodeRepository.cs

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
    /// Repository class for Node table.  
    /// Responsible for managing transactions with database.
    /// </summary>
    public class NodeRepository : INodeRepository
    {
        /// <summary>
        /// Adds the specified Node record to the database.
        /// </summary>
        /// <param name="node">Node record.</param>
        public void Add(Node node)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Save(node);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Updates the corresponding Node record in the database.
        /// </summary>
        /// <param name="node">Node record.</param>
        public void Update(Node node)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Update(node);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Removes the corresponding Node record from the database.
        /// </summary>
        /// <param name="node">Node record.</param>
        public void Remove(Node node)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            using (ITransaction transaction = session.BeginTransaction())
            {
                session.Delete(node);
                transaction.Commit();
            }
        }

        /// <summary>
        /// Gets Node record from database by internal id.
        /// </summary>
        /// <param name="id">Internal database Node record id.</param>
        /// <returns>Node record.</returns>
        public Node GetById(int id)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                return session.Get<Node>(id);
            }
        }

        /// <summary>
        /// Gets Node record associated with Node Guid identification.
        /// </summary>
        /// <param name="ident">Node Guid identification.</param>
        /// <returns>Node record.</returns>
        public Node GetByGuid(Guid ident)
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                Node node = session
                    .CreateQuery("FROM Node n WHERE n.Ident=:ident")
                    .SetString("ident", ident.ToString())
                    .UniqueResult<Node>();
                return node;
            }
        }

        /// <summary>
        /// Gets all Node records from database.
        /// </summary>
        /// <returns>Collection of Node records.</returns>
        public ICollection<Node> GetAll()
        {
            using (ISession session = NHibernateHelper.OpenSession())
            {
                var nodeList = session
                    .CreateCriteria(typeof(Node))
                    .List<Node>();
                return nodeList;
             }
        }
    }
}
