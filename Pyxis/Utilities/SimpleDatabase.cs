/******************************************************************************
SimpleDatabase.cs

begin      : March 12, 2008
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Data;

namespace Pyxis.Utilities
{
    public class SimpleDatabase: IDisposable
    {
        #region Static
        public static Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(false);

        /// <summary>
        /// Displays all providers.  This is a helper function to simply dump 
        /// all the provider info to the trace log.  For demonstration purposes.
        /// </summary>
        public static void DisplayAllProviders()
        {
            DataTable factories = System.Data.Common.DbProviderFactories.GetFactoryClasses();
            Trace.WriteLine("{0},\t{1},\t{2},\t{3}",
                    "Name",
                    "Description",
                    "InvariantName",
                    "AssemblyQualifiedName");

            foreach (DataRow r in factories.Rows)
            {
                Trace.WriteLine("{0},\t{1},\t{2},\t{3}",
                    r["Name"],
                    r["Description"],
                    r["InvariantName"],
                    r["AssemblyQualifiedName"]);
            }
        }

        /// <summary>
        /// Displays the table contents to the diagnostic window.
        /// </summary>
        /// <param name="schema">The schema.</param>
        private static void DisplayTableContents(DataTable schema)
        {
            Trace.WriteLine("Table '{0}' has {1} row(s).", schema.TableName, schema.Rows.Count);
            bool firstline = true;
            foreach (DataColumn c in schema.Columns)
            {
                if (!firstline)
                    Trace.Write(", ");
                Trace.Write(c.ColumnName);
                firstline = false;
            }
            Trace.WriteLine();

            foreach (DataRow r in schema.Rows)
            {
                firstline = true;
                foreach (object o in r.ItemArray)
                {
                    if (!firstline)
                        Trace.Write(", ");
                    Trace.Write("{0}", o.ToString());
                    firstline = false;
                }
                Trace.WriteLine();
            }
        }

        /// <summary>
        /// Gets the default provider factory.
        /// </summary>
        /// <returns></returns>
        public static System.Data.Common.DbProviderFactory GetDefaultProviderFactory()
        {
            try
            {
                return System.Data.Common.DbProviderFactories.GetFactory("System.Data.SQLite");
            }
            catch (ArgumentException ex)
            {
                // Hard-coded class is pretty clumsy.  But it works.
                Trace.WriteLine("Error accessing SQLite data provider.  {0}", ex.Message);
                return new System.Data.SQLite.SQLiteFactory();
            }
        }

        #endregion Static

        #region Constructors, Destructors

        /// <summary>
        /// Initializes a new instance of the <see cref="SimpleDatabase"/> class.
        /// </summary>
        /// <param name="filename">The filename.</param>
        public SimpleDatabase(string filename)
            :
            this(filename, SimpleDatabase.GetDefaultProviderFactory())
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SimpleDatabase"/> class.
        /// </summary>
        /// <param name="filename">The filename.</param>
        /// <param name="dbProvider">The db provider.</param>
        public SimpleDatabase(string filename, System.Data.Common.DbProviderFactory dbProvider)
        {
            if (dbProvider == null)
            {
                throw new ArgumentNullException("dbProvider");
            }

            m_dbProviderFactory = dbProvider;
            m_filename = filename;

            // Read schema...
            //DataTable schema = DbConnection.GetSchema();
            //DisplayTableContents(schema);
            //DisplayTableContents( DbConnection.GetSchema( "MetaDataCollections"));

            m_dataTypes = DbConnection.GetSchema("DataTypes");
            //DisplayTableContents(m_dataTypes);
        }

        /// <summary>
        /// Destructor
        /// </summary>
        ~SimpleDatabase()
        {
            Dispose(false);
        }

        #region IDisposable
        private bool m_disposed = false;

        /// <summary>
        /// Dispose of this object (as per IDisposable)
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Implementation of Dispose - will be called from Dispose or destructor.
        /// </summary>
        /// <remarks>Do NOT touch member variables if disposing is false!</remarks>
        /// <param name="disposing"></param>
        private void Dispose(bool disposing)
        {
            if (!this.m_disposed)
            {
                if (disposing)
                {
                    if (m_dbConnection != null)
                    {
                        m_dbConnection.Dispose();
                        m_dbConnection = null;
                    }
                }
            }
            m_disposed = true;
        }
        #endregion /* IDispose */

        #endregion Constructors, Destructors

        #region Properties, Fields

        private System.Data.Common.DbProviderFactory m_dbProviderFactory;
        private DataTable m_dataTypes;
        private string m_filename;

        /// <summary>
        /// Gets the filename that the database is stored in.
        /// </summary>
        /// <value>The filename.</value>
        public string Filename
        {
            get { return m_filename; }
        }

        private System.Data.Common.DbConnection m_dbConnection = null;

        /// <summary>
        /// Gets the db connection.
        /// </summary>
        /// <value>The db connection.</value>
        public System.Data.Common.DbConnection DbConnection
        {
            get 
            {
                if (m_dbConnection == null)
                {
                    m_dbConnection = m_dbProviderFactory.CreateConnection();
                    // Note that the connection string is SQLite-specific
                    m_dbConnection.ConnectionString = string.Format(
                        "Data Source={0};Version=3;FailIfMissing=False;", m_filename);
                    m_dbConnection.Open();
                }
                return m_dbConnection;
            }
        }

        #endregion Properties, Fields

        /// <summary>
        /// Creates a table adapter for the given table.
        /// </summary>
        /// <param name="table">The table.</param>
        /// <returns></returns>
        private System.Data.Common.DbDataAdapter CreateTableAdapter( DataTable table)
        {
            // TODO: Enable table "extensions/modifications".

            System.Data.Common.DbDataAdapter result = m_dbProviderFactory.CreateDataAdapter();
            result.SelectCommand = m_dbProviderFactory.CreateCommand();
            result.SelectCommand.Connection = DbConnection;
            result.SelectCommand.CommandText = string.Format("SELECT * FROM {0}", table.TableName);
            result.InsertCommand = m_dbProviderFactory.CreateCommand();
            result.InsertCommand.Connection = DbConnection;
            StringBuilder insertCommand = new StringBuilder();
            insertCommand.AppendFormat( "INSERT OR REPLACE INTO {0} (", table.TableName);
            StringBuilder valuesLine = new StringBuilder();
            bool firstline = true;
            foreach( DataColumn column in table.Columns)
            {
                if (!firstline)
                {
                    insertCommand.Append(", ");
                    valuesLine.Append(", ");
                }
                insertCommand.Append(column.ColumnName);
                valuesLine.Append( "?");
                DataRow typeMap = GetDataTypeRow( column.DataType);
                System.Data.Common.DbParameter parameter = m_dbProviderFactory.CreateParameter();
                parameter.DbType = (DbType) typeMap["ProviderDbType"];
                parameter.ParameterName = column.ColumnName;
                parameter.SourceColumn = column.ColumnName;
                result.InsertCommand.Parameters.Add( parameter);
                firstline = false;
            }
            insertCommand.AppendFormat( ") VALUES ({0})", valuesLine.ToString());

            result.InsertCommand.CommandText = insertCommand.ToString();
            result.TableMappings.Add("Table", table.TableName);
            return result;
        }

        /// <summary>
        /// Selects the records matching the specified select string.
        /// </summary>
        /// <param name="selectString">The select string.</param>
        /// <returns></returns>
        public DataSet Select(string selectString)
        {
            System.Data.Common.DbDataAdapter adapter = m_dbProviderFactory.CreateDataAdapter();
            adapter.SelectCommand = m_dbProviderFactory.CreateCommand();
            adapter.SelectCommand.Connection = DbConnection;
            adapter.SelectCommand.CommandText = selectString;
            DataSet result = new DataSet();
            adapter.Fill(result);
            return result;
        }

        /// <summary>
        /// Creates the table iff it doesn't already exist.
        /// </summary>
        /// <param name="table">The table.</param>
        private void CreateTable( DataTable table)
        {
            // TODO: Enable table "extensions/modifications".

            DataTable tableSchema = DbConnection.GetSchema("Tables");
            //DisplayTableContents(tableSchema);

            // This select statement is provider-specific.
            DataRow[] match = tableSchema.Select(string.Format("TABLE_NAME='{0}'", table.TableName));
            if (match.Length == 0)
            {
                using (System.Data.Common.DbCommand command = m_dbProviderFactory.CreateCommand())
                {
                    command.Connection = DbConnection;
                    StringBuilder commandText = new StringBuilder();
                    commandText.AppendFormat("CREATE TABLE {0} (", table.TableName);
                    bool firstline = true;
                    foreach( DataColumn column in table.Columns)
                    {
                        if (!firstline)
                        {
                            commandText.Append( ", ");
                        }
                        commandText.AppendFormat( "{0} {1}", column.ColumnName, GetDataType( column.DataType));
                        foreach (DataColumn keyColumn in table.PrimaryKey)
                        {
                            if (keyColumn == column)
                            {
                                commandText.Append(" PRIMARY KEY");
                            }
                        }
                        firstline = false;
                    }
                    commandText.Append( ")");
                    command.CommandText = commandText.ToString(); 
                    command.ExecuteNonQuery();
                }
            }

            //// Debugging help.  Don't check this in.
            //bool dontcheckthisin;
            //tableSchema = DbConnection.GetSchema("Tables");
            //DisplayTableContents(tableSchema);
        }

        /// <summary>
        /// Gets the data type row.
        /// </summary>
        /// <param name="managedDataType">Type of the managed data.</param>
        /// <returns></returns>
        private DataRow GetDataTypeRow( Type managedDataType)
        {
            DataRow[] result = m_dataTypes.Select( 
                string.Format("(DataType='{0}') AND (IsBestMatch='True')", managedDataType.FullName));
            if (result.Length == 0)
            {
                throw new InvalidOperationException( string.Format(
                    "Unable to find a match for type '{0}'.", managedDataType.FullName));
            }
            return result[0];
        }

        /// <summary>
        /// Gets the DDL text to create a column of the given type.
        /// </summary>
        /// <param name="managedDataType">Type of the managed data.</param>
        /// <returns></returns>
        private string GetDataType(Type managedDataType)
        {
            // Look in the datatypes table for the best type matching the given managedDataType
            DataRow typeMap = GetDataTypeRow(managedDataType);

            // Extract the text to be passed to CREATE TABLE (Data Definition Language).
            string createFormat = typeMap["CreateFormat"].ToString();

            // If there are parameters for the create, set them.
            switch (typeMap["CreateParameters"].ToString())
            {
                case "max length":
                    {
                        return string.Format(createFormat, 5000);
                    }
                case "":
                    {
                        return createFormat;
                    }
                default:
                    {
                        throw new NotImplementedException( String.Format(
                            "Unexpected parameter ({0}) for creation of type {1}.",
                            typeMap["CreateParameters"].ToString(),
                            managedDataType.FullName));
                    }
            }
        }

        /// <summary>
        /// Updates the specified data set, copying it into the database.  
        /// Tables will be created if necessary.
        /// </summary>
        /// <param name="dataSet">The data set.</param>
        public void Update( DataSet dataSet)
        {
            System.Collections.Generic.List<DataTable> tablesToUpdate =
                new System.Collections.Generic.List<DataTable>();

            // Find the tables that need to be updated (those with data).
            foreach (DataTable table in dataSet.Tables)
            {
                // Create the table, if necessary.
                CreateTable(table);

                if (table.Rows.Count > 0)
                {
                    tablesToUpdate.Add(table);
                }
            }

            // Copy that into tablesRemaining.
            System.Collections.Generic.List<DataTable> tablesRemaining = 
                new System.Collections.Generic.List<DataTable>();
            tablesRemaining.AddRange(tablesToUpdate);

            DataSet dataToWrite = dataSet.Clone(); // Only copies the structure/schema.

            // Create the tables and copy the data into the dataset.
            for (int i = 2; (i >= 0) && (tablesRemaining.Count > 0); --i)
            {
                System.Collections.Generic.List<DataTable> failures =
                    new System.Collections.Generic.List<DataTable>();

                foreach (DataTable table in tablesRemaining)
                {
                    // Copy the data into the table.
                    try
                    {
                        DataTable destinationTable = dataToWrite.Tables[table.TableName];
                        foreach (DataRow r in table.Rows)
                        {
                            DataRow newRow = destinationTable.NewRow();
                            for (int col = 0; col < newRow.ItemArray.Length; ++col)
                            {
                                newRow[col] = r[col];
                            }
                            destinationTable.Rows.Add(newRow);
                        }
                    }
                    catch (Exception)
                    {
                        if (i == 0)
                        {
                            throw;
                        }
                        failures.Add(table);
                    }
                }
                tablesRemaining = failures;
            }

            for (int j = 2; (j >= 0) && (tablesToUpdate.Count > 0); --j)
            {
                System.Collections.Generic.List<DataTable> failures =
                    new System.Collections.Generic.List<DataTable>();
                foreach (DataTable table in tablesToUpdate)
                {
                    try
                    {
                        System.Data.Common.DbDataAdapter adapter = this.CreateTableAdapter(table);
                        adapter.Update(dataToWrite);
                    }
                    catch (Exception)
                    {
                        if (j == 0)
                        {
                            throw;
                        }
                        failures.Add(table);
                    }
                }
                tablesToUpdate = failures;
            }
        }

        /// <summary>
        /// Writes the specified object.
        /// </summary>
        /// <param name="objectToWrite">The object to write.</param>
        public void Write(object objectToWrite)
        {
            string xml = Pyxis.Utilities.XmlTool.ToXml(objectToWrite);
            DataSet dataSet = new DataSet(objectToWrite.GetType().Name);
            System.IO.StringReader reader = new System.IO.StringReader(xml);
            dataSet.ReadXml(reader);
            Update(dataSet);
        }

        private const string IdColumn = "Id";
        private const string XmlColumn = "XmlContent";

        private DataSet CreateFlatDataSet<TypeToEncode>()
        {
            DataSet image = new DataSet("Document");
            DataTable table = new DataTable(typeof(TypeToEncode).Name);
            DataColumn key = table.Columns.Add(IdColumn, typeof(string));
            table.Columns.Add(XmlColumn, typeof(string));
            DataColumn[] keys = new DataColumn[] { key };
            table.PrimaryKey = keys;
            image.Tables.Add(table);
            
            return image;
        }

        public void Write<ObjectType>(ObjectType objectToWrite, Guid id)
        {
            Write(objectToWrite, id.ToString());
        }

        public void Write<ObjectType>(ObjectType objectToWrite, string id)
        {
            string xml = Pyxis.Utilities.XmlTool.ToXml(objectToWrite);

            DataSet image = CreateFlatDataSet<ObjectType>();
            DataTable table = image.Tables[0];
            DataRow row = table.NewRow();
            row[IdColumn] = id.ToString();
            row[XmlColumn] = xml;
            table.Rows.Add(row);

            Update(image);
        }

        /// <summary>
        /// Reads the specified id.  Returns null if it isn't found.
        /// </summary>
        /// <param name="id">The id.</param>
        /// <returns></returns>
        public ReturnType Read<ReturnType>(Guid id) where ReturnType:class
        {
            DataSet image = CreateFlatDataSet<ReturnType>();

            try
            {
                System.Data.Common.DbDataAdapter adapter = m_dbProviderFactory.CreateDataAdapter();
                adapter.SelectCommand = m_dbProviderFactory.CreateCommand();
                adapter.SelectCommand.Connection = DbConnection;
                // TODO: Better error handling for missing key here.
                adapter.SelectCommand.CommandText = string.Format(
                    "SELECT * FROM {0} WHERE {1}='{2}'", image.Tables[0].TableName,
                    image.Tables[0].PrimaryKey[0].ColumnName,
                    id);
                if (adapter.Fill(image.Tables[0]) == 1)
                {
                    string xml = image.Tables[0].Rows[0][XmlColumn].ToString();

                    return Pyxis.Utilities.XmlTool.FromXml<ReturnType>(xml);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(
                    string.Format("Error reading {0} from database {1}: {2}.",
                    typeof(ReturnType).Name, m_filename, ex.Message));

            }
            return default(ReturnType);
        }

        public IEnumerable<ReturnType> ReadAll<ReturnType>() where ReturnType : class
        {
            DataSet image = CreateFlatDataSet<ReturnType>();

            try 
            {
                System.Data.Common.DbDataAdapter adapter = m_dbProviderFactory.CreateDataAdapter();
                adapter.SelectCommand = m_dbProviderFactory.CreateCommand();
                adapter.SelectCommand.Connection = DbConnection;
                adapter.SelectCommand.CommandText = string.Format(
                    "SELECT * FROM {0}", image.Tables[0].TableName);
                if (adapter.Fill(image.Tables[0]) == 0)
                {
                    image = null;
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(
                    string.Format("Error reading all {0}'s from database {1}: {2}.",
                    typeof(ReturnType).Name, m_filename, ex.Message));
            }

            if (image != null)
            {
                foreach (DataRow r in image.Tables[0].Rows)
                {
                    string xml = r[XmlColumn].ToString();

                    yield return Pyxis.Utilities.XmlTool.FromXml<ReturnType>(xml);
                }
            }
        }

        public void CreateTable<TypeToEncode>()
        {
            DataSet emptydataset = this.CreateFlatDataSet<TypeToEncode>();
            this.Update(emptydataset);
        }

        public System.Data.Common.DbTransaction BeginTransaction()
        {
            return m_dbConnection.BeginTransaction();
        }
    }
}
