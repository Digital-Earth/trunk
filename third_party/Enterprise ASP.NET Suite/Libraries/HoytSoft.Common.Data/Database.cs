#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2006 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 ***************************************************/
#endregion

using System;
using System.Data;
using System.Data.SqlClient;
using HoytSoft.Common.Configuration;
using HoytSoft.Common.Configuration.Internal;

namespace HoytSoft.Common.Data {
	public abstract class Database {
		#region Initialization
		private static string defaultConnStr;
		private static ConnectionStringSettings connStrObj = null;
		protected const string 
			NULL_CONNECTION_OBJ = "The connection object cannot be null";

		static Database() {
			Reload();
		}

		public static string ProviderName { get { return "System.Data.SqlClient"; } }
		public static string DefaultConnectionString { get { return defaultConnStr; } }

		protected static string getConnStr(string Name) {
			if (string.IsNullOrEmpty(Name))
				if (!string.IsNullOrEmpty(defaultConnStr))
					return defaultConnStr;
				else throw new ArgumentException("Could not find a valid connection string. Please define one in your configuration file.");
			if (connStrObj != null)
				return connStrObj[Name];
			else
				throw new ArgumentException("Could not find a valid connection string. Please define one in your configuration file.");
		}

		protected static IDbConnection getConnection(string ConnectionString) {
			return new SqlConnection(ConnectionString);
		}

		protected static IDbCommand getCommand(string CommandText, ref IDbConnection Connection) {
			if (!(Connection is SqlConnection))
				throw new ArgumentException("Invalid connection object. Must be of type " + typeof(SqlConnection).FullName);
			return new SqlCommand(CommandText, (SqlConnection)Connection);
		}

		protected static IDbCommand getCommand(string CommandText, ref IDbConnection Connection, ref IDbTransaction Transaction) {
			if (!(Connection is SqlConnection))
				throw new ArgumentException("Invalid connection object. Must be of type " + typeof(SqlConnection).FullName);
			if (!(Transaction is SqlTransaction))
				throw new ArgumentException("Invalid transaction object. Must be of type " + typeof(SqlTransaction).FullName);
			return new SqlCommand(CommandText, (SqlConnection)Connection, (SqlTransaction)Transaction);
		}

		protected static IDbCommand getCommandAsStoredProcedure(string CommandText, ref IDbConnection Connection) {
			if (!(Connection is SqlConnection))
				throw new ArgumentException("Invalid connection object. Must be of type " + typeof(SqlConnection).FullName);
			IDbCommand cmd = new SqlCommand(CommandText, (SqlConnection)Connection);
			cmd.CommandType = CommandType.StoredProcedure;
			return cmd;
		}

		protected static IDbCommand getCommandAsStoredProcedure(string CommandText, ref IDbConnection Connection, ref IDbTransaction Transaction) {
			if (!(Connection is SqlConnection))
				throw new ArgumentException("Invalid connection object. Must be of type " + typeof(SqlConnection).FullName);
			if (!(Transaction is SqlTransaction))
				throw new ArgumentException("Invalid transaction object. Must be of type " + typeof(SqlTransaction).FullName);
			IDbCommand cmd = new SqlCommand(CommandText, (SqlConnection)Connection, (SqlTransaction)Transaction);
			cmd.CommandType = CommandType.StoredProcedure;
			return cmd;
		}

		protected static IDbDataAdapter getDataAdapter() {
			return new SqlDataAdapter();
		}

		protected static IDbDataAdapter getDataAdapter(ref IDbCommand Command) {
			if (!(Command is SqlCommand))
				throw new ArgumentException("Command must by of type " + typeof(SqlCommand).FullName);
			return new SqlDataAdapter((SqlCommand)Command);
		}

		protected static IDbDataAdapter getDataAdapter(string CommandText, ref IDbConnection Connection) {
			if (!(Connection is SqlConnection))
				throw new ArgumentException("Connection must by of type " + typeof(SqlConnection).FullName);
			return new SqlDataAdapter(CommandText, (SqlConnection)Connection);
		}

		protected static IDbDataAdapter getDataAdapter(string CommandText, string ConnectionString) {
			return new SqlDataAdapter(CommandText, ConnectionString);
		}

		protected static IDbDataAdapter getDataAdapterByName(string CommandText, string Name) {
			return new SqlDataAdapter(CommandText, getConnStr(Name));
		}

		protected static IDataParameter getParameter() {
			return new SqlParameter();
		}

		protected static IDataParameter getParameter(string ParameterName, object Value) {
			return new SqlParameter(ParameterName, Value);
		}

		protected static IDataParameter getParameter(string ParameterName, SqlDbType SqlDbType) {
			return new SqlParameter(ParameterName, SqlDbType);
		}

		protected static IDataParameter getParameter(string ParameterName, SqlDbType SqlDbType, object Value) {
			IDataParameter p = new SqlParameter(ParameterName, SqlDbType);
			p.Value = Value;
			return p;
		}

		protected static IDataParameter getParameter(string ParameterName, SqlDbType SqlDbType, int Size) {
			return new SqlParameter(ParameterName, SqlDbType, Size);
		}

		protected static IDataParameter getParameter(string ParameterName, SqlDbType SqlDbType, byte Precision, byte Scale) {
			return new SqlParameter(ParameterName, SqlDbType, 0, ParameterDirection.Input, false, Precision, Scale, null, DataRowVersion.Current, null);
		}

		protected static IDataParameter getParameter(string ParameterName, SqlDbType SqlDbType, int Size, bool IsNullable, object Value) {
			return new SqlParameter(ParameterName, SqlDbType, Size, ParameterDirection.Input, IsNullable, 0, 0, null, DataRowVersion.Current, Value);
		}

		protected static IDataParameter addOutputParameterToCommand(ref IDbCommand Command, string ParameterName, SqlDbType SqlDbType) {
			IDataParameter p = getParameter(ParameterName, SqlDbType);
			p.Direction = ParameterDirection.Output;
			Command.Parameters.Add(p);
			return p;
		}

		protected static IDataParameter addOutputParameterToCommand(ref IDbCommand Command, string ParameterName, SqlDbType SqlDbType, int Size) {
			IDataParameter p = getParameter(ParameterName, SqlDbType, Size);
			p.Direction = ParameterDirection.Output;
			Command.Parameters.Add(p);
			return p;
		}

		protected static IDataParameter addOutputParameterToCommand(ref IDbCommand Command, string ParameterName, SqlDbType SqlDbType, byte Precision, byte Scale) {
			IDataParameter p = getParameter(ParameterName, SqlDbType, Precision, Scale);
			p.Direction = ParameterDirection.Output;
			Command.Parameters.Add(p);
			return p;
		}

		protected static void addParameterToCommand(ref IDbCommand Command) {
			Command.Parameters.Add(getParameter());
		}

		protected static void addParameterToCommand(ref IDbCommand Command, string ParameterName, object Value) {
			Command.Parameters.Add(getParameter(ParameterName, Value));
		}

		protected static void addParameterToCommand(ref IDbCommand Command, string ParameterName, SqlDbType SqlDbType, object Value) {
			IDataParameter p = getParameter(ParameterName, SqlDbType);
			p.Value = Value;
			Command.Parameters.Add(p);
		}

		protected static void addParameterToCommand(ref IDbCommand Command, string ParameterName, SqlDbType SqlDbType, int Size, object Value) {
			IDataParameter p = getParameter(ParameterName, SqlDbType, Size);
			p.Value = Value;
			Command.Parameters.Add(p);
		}

		protected static void addParameterToCommand(ref IDbCommand Command, string ParameterName, SqlDbType SqlDbType, byte Precision, byte Scale, object Value) {
			IDataParameter p = getParameter(ParameterName, SqlDbType, Precision, Scale);
			p.Value = Value;
			Command.Parameters.Add(p);
		}

		public static void Reload() {
			connStrObj = null;
			defaultConnStr = null;

			if ((connStrObj = Settings.From<ConnectionStringSettings>(Settings.Section.ConnectionString)) != null) {
				defaultConnStr = connStrObj.LookUp(null);
			}
		} //Reload

		public static IDbConnection NewConnection() {
			return Database.getConnection(Database.DefaultConnectionString);
		}

		public static IDbConnection NewConnection(string ConnectionString) {
			return Database.getConnection(ConnectionString);
		}

		public static IDbConnection NewConnectionByName(string Name) {
			return Database.getConnection(Database.getConnStr(Name));
		}
		#endregion

		public static IDataReader ExecuteReaderSQL(string SQL) { return ExecuteReaderSQL(getConnection(DefaultConnectionString), SQL, null); }
		public static IDataReader ExecuteReaderSQL(string SQL, IDataParameter[] Params) { return ExecuteReaderSQL(getConnection(DefaultConnectionString), SQL, Params); }
		public static IDataReader ExecuteReaderSQL(string ConnectionString, string SQL) { return ExecuteReaderSQL(getConnection(ConnectionString), SQL, null); }
		public static IDataReader ExecuteReaderSQL(string ConnectionString, string SQL, IDataParameter[] Params) { return ExecuteReaderSQL(getConnection(ConnectionString), SQL, Params); }
		public static IDataReader ExecuteReaderSQLByName(string Name, string SQL) { return ExecuteReaderSQL(getConnection(getConnStr(Name)), SQL, null); }
		public static IDataReader ExecuteReaderSQLByName(string Name, string SQL, IDataParameter[] Params) { return ExecuteReaderSQL(getConnection(getConnStr(Name)), SQL, Params); }
		protected static IDataReader ExecuteReaderSQL(IDbConnection cn, string SQL, IDataParameter[] Params) {
			if (cn == null) throw new NullReferenceException(NULL_CONNECTION_OBJ);
			if (string.IsNullOrEmpty(SQL)) return null;
			IDataReader dr = null;
			IDbCommand cmd = null;
			try {
				cmd = getCommand(SQL, ref cn);
				cmd.CommandType = CommandType.Text;
				if (Params != null)
					foreach(IDataParameter p in Params)
						cmd.Parameters.Add(p);

				if (cn.State == ConnectionState.Closed) cn.Open();
				dr = cmd.ExecuteReader(CommandBehavior.CloseConnection);
				return dr;
			} catch (Exception) {
				if (dr != null && !dr.IsClosed) dr.Close();
				if (dr != null) dr.Dispose();
				if (cn != null && cn.State != ConnectionState.Closed) cn.Close();
				if (cn != null) cn.Dispose();
				if (Settings.DebugMode)
					throw;
			}
			return null;

		}

		public static object ExecuteScalarSQL(string SQL) { return ExecuteScalarSQL(getConnection(DefaultConnectionString), SQL, null); }
		public static object ExecuteScalarSQL(string SQL, IDataParameter[] Params) { return ExecuteScalarSQL(getConnection(DefaultConnectionString), SQL, Params); }
		public static object ExecuteScalarSQL(string ConnectionString, string SQL) { return ExecuteScalarSQL(getConnection(ConnectionString), SQL, null); }
		public static object ExecuteScalarSQL(string ConnectionString, string SQL, IDataParameter[] Params) { return ExecuteScalarSQL(getConnection(ConnectionString), SQL, Params); }
		public static object ExecuteScalarSQLByName(string Name, string SQL) { return ExecuteScalarSQL(getConnection(getConnStr(Name)), SQL, null); }
		public static object ExecuteScalarSQLByName(string Name, string SQL, IDataParameter[] Params) { return ExecuteScalarSQL(getConnection(getConnStr(Name)), SQL, Params); }
		protected static object ExecuteScalarSQL(IDbConnection cn, string SQL, IDataParameter[] Params) {
			if (cn == null) throw new NullReferenceException(NULL_CONNECTION_OBJ);
			if (string.IsNullOrEmpty(SQL)) return null;
			object ret = null;
			IDbCommand cmd = null;
			try {
				cmd = getCommand(SQL, ref cn);
				cmd.CommandType = CommandType.Text;
				if (Params != null)
					foreach (IDataParameter p in Params)
						cmd.Parameters.Add(p);

				if (cn.State == ConnectionState.Closed) cn.Open();
				ret = cmd.ExecuteScalar();
			} catch (Exception) {
				if (Settings.DebugMode)
					throw;
			} finally {
				if (cn != null && cn.State != ConnectionState.Closed) cn.Close();
				if (cn != null) cn.Dispose();
			}
			return ret;
		}

		public static int ExecuteNonQuerySQL(string SQL) { return ExecuteNonQuerySQL(getConnection(DefaultConnectionString), SQL, null); }
		public static int ExecuteNonQuerySQL(string SQL, IDataParameter[] Params) { return ExecuteNonQuerySQL(getConnection(DefaultConnectionString), SQL, Params); }
		public static int ExecuteNonQuerySQL(string ConnectionString, string SQL) { return ExecuteNonQuerySQL(getConnection(ConnectionString), SQL, null); }
		public static int ExecuteNonQuerySQL(string ConnectionString, string SQL, IDataParameter[] Params) { return ExecuteNonQuerySQL(getConnection(ConnectionString), SQL, Params); }
		public static int ExecuteNonQuerySQLByName(string Name, string SQL) { return ExecuteNonQuerySQL(getConnection(getConnStr(Name)), SQL, null); }
		public static int ExecuteNonQuerySQLByName(string Name, string SQL, IDataParameter[] Params) { return ExecuteNonQuerySQL(getConnection(getConnStr(Name)), SQL, Params); }
		protected static int ExecuteNonQuerySQL(IDbConnection cn, string SQL, IDataParameter[] Params) {
			if (cn == null) throw new NullReferenceException(NULL_CONNECTION_OBJ);
			if (string.IsNullOrEmpty(SQL)) return 0;
			int ret = 0;
			IDbCommand cmd = null;
			try {
				cmd = getCommand(SQL, ref cn);
				cmd.CommandType = CommandType.Text;
				if (Params != null)
					foreach (IDataParameter p in Params)
						cmd.Parameters.Add(p);

				if (cn.State == ConnectionState.Closed) cn.Open();
				ret = cmd.ExecuteNonQuery();
			} catch (Exception) {
				//if (Settings.DebugMode)
					throw;
			} finally {
				if (cn != null && cn.State != ConnectionState.Closed) cn.Close();
				if (cn != null) cn.Dispose();
			}
			return ret;
		}
	} //class
} //namespace
