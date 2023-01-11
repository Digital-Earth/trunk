/******************************************************************************
OleDbDataSource.cs

begin		: 2010-02-03
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Data;
using System.Data.OleDb;

namespace ApplicationUtility
{
    /// <summary>
    /// A connection to an OLE DB data source.
    /// </summary>
    public class OleDbDataSource : IDisposable
    {
        #region Fields

        /// <summary>
        /// The file path of the data source.
        /// </summary>
        readonly System.IO.FileInfo m_fileInfo;

        /// <summary>
        /// The OLE DB connection to the data source.
        /// According to the documentation, this is not thread-safe
        /// and all access must be locked.
        /// </summary>
        readonly OleDbConnection m_connection;

        #endregion

        #region Properties

        /// <summary>
        /// Gets the file information for the data source file.
        /// </summary>
        public System.IO.FileInfo FileInfo
        {
            get { return m_fileInfo; }
        }

        #endregion

        #region Construction

        /// <summary>
        /// Opens a connection to the OLE DB data source at the file path.
        /// Can throw.
        /// </summary>
        /// <param name="fileInfo">The source file.</param>
        public OleDbDataSource(System.IO.FileInfo fileInfo, string connectionString)
        {
            this.m_fileInfo = fileInfo;

            this.m_connection = new OleDbConnection(connectionString);
            this.m_connection.Open();
        }

        /// <summary>
        /// Closes the connection to the data source.
        /// </summary>
        ~OleDbDataSource()
        {
            Dispose();
        }

        #endregion

        #region IDisposable

        /// <summary>
        /// Closes the connection to the data source.
        /// </summary>
        /// <returns></returns>
        public void Dispose()
        {
            try
            {
                lock (this.m_connection)
                {
                    // This can throw an "invalid handle" exception if called
                    // a second time.
                    this.m_connection.Close();
                }
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine(
                    String.Format("Closing connection threw exception: {0}",
                    e.Message));
            }
        }

        #endregion

        #region Methods

        /// <summary>
        /// Runs a non-query database command such as UPDATE, INSERT or DELETE.
        /// Can throw.
        /// </summary>
        /// <param name="nonQuery">The required query</param>
        /// <returns>Count of rows affected.</returns>
        public int RunNonQuery(string nonQuery)
        {
            OleDbCommand nonQueryCommand = new OleDbCommand(nonQuery);
            lock (this.m_connection)
            {
                nonQueryCommand.Connection = this.m_connection;
                nonQueryCommand.CommandText = nonQuery;
                return nonQueryCommand.ExecuteNonQuery();
            }
        }

        /// <summary>
        /// Runs a query that reads data into a DataTable, which is returned.
        /// Can throw.
        /// </summary>
        /// <param name="query">The query.</param>
        /// <returns>DataTable, or null if there was an exception.</returns>
        public DataTable RunQuery(string query)
        {
            DataTable returnDataObject = new DataTable();
            lock (this.m_connection)
            {
                OleDbCommand selectCommand = new OleDbCommand(query);
                selectCommand.Connection = this.m_connection;

                OleDbDataAdapter adapter = new OleDbDataAdapter();
                adapter.SelectCommand = selectCommand;
                adapter.Fill(returnDataObject);
            }
            return returnDataObject;
        }

        /// <summary>
        /// Gets the schema table in which each row represents a table.
        /// </summary>
        /// <returns></returns>
        public DataTable GetTables()
        {
            lock (this.m_connection)
            {
                return this.m_connection.GetSchema("Tables");
            }
        }

        #endregion
    }
}
