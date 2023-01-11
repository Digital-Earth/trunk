/******************************************************************************
UsageReport.cs

begin      : August 26, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.Data;

namespace Pyxis.Utilities
{
    /// <summary>
    /// The UsageReport class is used to record PyxNet usage.
    /// The data is collected and stored in an SQLite databse file,
    /// aggregated and uploaded to the license server on a regular
    /// basis.
    /// </summary>
    public class UsageReport
    {
        private static Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(true);

        private const string m_fileName = "UsageReport.sqlite";
        private const long m_intervalSizeInMinutes = 5;
        private const int m_reportTimerElapseTimeInHours = 1;

        private static SimpleDatabase m_database = null;
        private static string m_fullDatabaseFileName  = null;
        private static string m_reportsDir = null;
        private static DateTime m_lastPurged = new DateTime();
        private static bool m_initialized = false;
        private static List<string> m_tables = new List<string>();

        /// <summary>
        /// Report time, fires once an hour.  
        /// Creates reports to send to license server.
        /// </summary>
        private static SimpleTimer m_reportTimer = new SimpleTimer(new TimeSpan(m_reportTimerElapseTimeInHours, 0, 0));

        /// <summary>
        /// Critical section lock used to restrict concurrent acccess to sensative 
        /// areas within this module.  Most pretain to writing out to the database.
        /// </summary>
        private static object m_criticalSectionLock = new Object();

        //------------------------------------------------------------------------
        //--
        //-- PUBLIC METHODS:
        //--
        //-- Stats gathering routines called from within PyxNet at critical junctures,
        //-- in order to gather the data for the usage reports.
        //--
        //------------------------------------------------------------------------

        /// <summary>
        /// Logs when a Query is received.
        /// </summary>
        /// <param name="sendingNodeName">Name of the original (sending )node.</param>
        /// <param name="sendingNodeGuid">The original (sending) node GUID.</param>
        /// <param name="receivingNodeName">Name of the local (receiving) node.</param>
        /// <param name="receivingNodeGuid">The local (receiving) node GUID.</param>
        public static void QueryReceived( string sendingNodeName,
                                          Guid sendingNodeGuid,
                                          string receivingNodeName,
                                          Guid receivingNodeGuid
                                          )
        {
            UsageReport.DatabaseObject.QueryReceived dbObj 
                = new UsageReport.DatabaseObject.QueryReceived( sendingNodeName, 
                                                                sendingNodeGuid, 
                                                                receivingNodeName, 
                                                                receivingNodeGuid
                                                                );
            Insert(dbObj);
        }

        /// <summary>
        /// Logs when a query is matched to a dataset.
        /// </summary>
        /// <param name="sendingNodeName">Name of the original (sending) node.</param>
        /// <param name="sendingNodeGuid">The original (sending) node GUID.</param>
        /// <param name="receivingNodeName">Name of the matching result (receiving) node.</param>
        /// <param name="receivingNodeGuid">The matching result (receiving) node GUID.</param>
        /// <param name="dataSetId">The matching dataset ID.</param>
        public static void QueryMatched( string sendingNodeName,
                                         Guid sendingNodeGuid,
                                         string receivingNodeName,
                                         Guid receivingNodeGuid,
                                         Guid dataSetId
                                         )
        {
            UsageReport.DatabaseObject.QueryMatched dbObj 
                = new UsageReport.DatabaseObject.QueryMatched( sendingNodeName, 
                                                               sendingNodeGuid, 
                                                               receivingNodeName, 
                                                               receivingNodeGuid, 
                                                               dataSetId
                                                               );
            Insert(dbObj);
        }

        /// <summary>
        /// Logs the number of bytes uploaded by a coverage pipeline.
        /// Called as every chunk is processed and sent.
        /// </summary>
        /// <param name="sendingNodeName">Name of the sending node.</param>
        /// <param name="sendingNodeGuid">The sending node GUID.</param>
        /// <param name="receivingNodeName">Name of the receiving node.</param>
        /// <param name="receivingNodeGuid">The receiving node GUID.</param>
        /// <param name="dataSetId">The dataset ID.</param>
        /// <param name="dataSize">Size of the sent chunk.</param>
        public static void CoverageBytesUploaded( string sendingNodeName,
                                                  Guid sendingNodeGuid,
                                                  string receivingNodeName,
                                                  Guid receivingNodeGuid,
                                                  Guid dataSetId,
                                                  long dataSize
                                                  )
        {
            UsageReport.DatabaseObject.DataTransfer dbObj 
                = new UsageReport.DatabaseObject.DataTransfer( sendingNodeName, 
                                                               sendingNodeGuid, 
                                                               receivingNodeName, 
                                                               receivingNodeGuid, 
                                                               dataSetId, 
                                                               dataSize
                                                               );
            Insert(dbObj);
        }

        //------------------------------------------------------------------------
        //--
        //-- UPLOAD/PURGE routines.
        //--
        //-- As tables are added to the usage report mechanism, the new tables must
        //-- be appropriately referenced in these two routing so that the new usage
        //-- reports are generated, and the table maintenance is performed.
        //--
        //------------------------------------------------------------------------

        /// <summary>
        /// Uploads usage reports records to license server.
        /// Currently creates XML file containing the data, ultimately that
        /// file will be uploaded (its contents actually) to the license server.
        /// </summary>
        private static void Upload()
        {
            DataSet uploadDataSet = new DataSet("PyxNetUsageReport");

            //-- 
            //-- add references to new tables here for inclusion in the 
            //-- uploaded usage report.
            //--
            foreach (string tableName in m_tables)
            {
                Upload(tableName, uploadDataSet);
            }
        //    Upload(new UsageReport.DatabaseObject.QueryReceived(), uploadDataSet);
        //    Upload(new UsageReport.DatabaseObject.QueryMatched(), uploadDataSet);
        //    Upload(new UsageReport.DatabaseObject.DataTransfer(), uploadDataSet);

            //--
            //-- do something intelegent with the dataset.
            //-- for now, we generate an XML file and store it on the machine.
            //-- future work will upload the data set (via an XML file?)
            //-- to the license server
            //--
            if( string.IsNullOrEmpty(m_reportsDir) == false )
            {
                //--
                //-- construct filename for report using todays date.
                //-- add counter and loop until we find an unused filename.
                //--
                string xmlFileName = null;
                int nLoopCnt = 0;
                do
                {
                    xmlFileName = string.Format("{0}/{1:yyyy-MMM-dd-}{2:000}.xml", m_reportsDir, DateTime.Now, nLoopCnt++);
                } while (System.IO.File.Exists(xmlFileName) && nLoopCnt < 1000 );

                if (string.IsNullOrEmpty(xmlFileName) == false)
                {
                    uploadDataSet.WriteXml(xmlFileName);
                }
            }
        }

        /// <summary>
        /// Purges database file.
        /// Only run guts once a day.
        /// </summary>
        private static void Purge()
        {
            //--
            //-- only purge once a day, determine number of days
            //-- since last purge was done.
            //--
            if ((DateTime.Now - m_lastPurged) < TimeSpan.FromDays(1))
            {
                return;
            }

            //-- 
            //-- add references to new tables here so that they
            //-- get cleaned up on a regular basis.
            //--
            foreach (string tableName in m_tables)
            {
                Purge(tableName);
            }
            
        //    Purge(new UsageReport.DatabaseObject.QueryReceived());
        //    Purge(new UsageReport.DatabaseObject.QueryMatched());
        //    Purge(new UsageReport.DatabaseObject.DataTransfer());

            m_lastPurged = DateTime.Now;
        }

        //------------------------------------------------------------------------
        //--
        //-- OTHER PUBLIC METHODS
        //--
        //------------------------------------------------------------------------

        /// <summary>
        /// Initializes the usage report module.  
        /// Provides the path to the library files.
        /// If not called by application, usage reports will not be generated.
        /// This routine is called by the WorldView and GeoStreamServer, 
        /// it is not called by the PyxNet Scanner.
        /// </summary>
        /// <param name="libPath">Absolute library path.</param>
        public static void Initialize(string libPath)
        {    
            Initalized = true;

            m_fullDatabaseFileName = libPath + "/" + m_fileName;
            
            m_reportsDir = libPath + "/" + "UsageReports";
            if (!System.IO.Directory.Exists(m_reportsDir))
            {
                System.IO.Directory.CreateDirectory(m_reportsDir);
            }

            m_reportTimer.Timer.AutoReset = true;
            m_reportTimer.Elapsed += OnReportTimerElapsed;

            //--
            //-- add table names to list
            //--
            m_tables.Add((new DatabaseObject.QueryReceived()).TableName());
            m_tables.Add((new DatabaseObject.QueryMatched()).TableName());
            m_tables.Add((new DatabaseObject.DataTransfer()).TableName());
        }

        /// <summary>
        /// Uninitializes the usage report module.
        /// </summary>
        public static void Uninitialize()
        {
            Initalized = false;

            //-- 
            //-- stop timer from inadvertently firing while shutting down.
            //--
            m_reportTimer.Timer.Stop();
        }

        //------------------------------------------------------------------------
        //--
        //-- PRIVATE METHODS
        //--
        //------------------------------------------------------------------------

        /// <summary>
        /// Gets or sets a value indicating whether this <see cref="UsageReport"/> is initalized.
        /// </summary>
        /// <value><c>true</c> if initalized; otherwise, <c>false</c>.</value>
        private static bool Initalized
        {
            get { return m_initialized; }
            set { m_initialized = value; }
        }

        /// <summary>
        /// Gets the database.
        /// </summary>
        /// <value>The database.</value>
        private static SimpleDatabase Database
        {
            get
            {
                if( m_database == null )
                {
                    m_database = new SimpleDatabase(m_fullDatabaseFileName);
                }
                return m_database;
            }
        }

        /// <summary>
        /// Gets the current interval time stamp.
        /// Typically it is the current time rounded back to the nearest 5 minute boundary.
        /// </summary>
        /// <value>current interval time stamp.</value>
        private static DateTime CurrentIntervalTimeStamp
        {
            get
            {
                //--
                //-- round current time to time interval by counting the number of ticks
                //-- into to current interval, and then subtracting that off the current time.
                //--
                DateTime now = System.DateTime.UtcNow;
                long ticksIntoInterval = now.Ticks % (m_intervalSizeInMinutes * 60L * 10000000L);
                return new DateTime(now.Ticks - ticksIntoInterval);
            }
        }

        /// <summary>
        /// Insert usage record into database.
        /// Works for all records derived from UsageReport.DatabaseObject.UsageRecord.
        /// </summary>
        /// <param name="usageRecord">The usage record.</param>
        private static void Insert( UsageReport.DatabaseObject.UsageRecord usageRecord ) 
        {
            //--
            //-- return immediately if module is not initialized.
            //--
            if (!Initalized)
            {
                return;
            }

            bool insertRequired = false;
            DateTime intervalTimeStamp = CurrentIntervalTimeStamp;

            lock (m_criticalSectionLock)
            {
                try
                {
                    string sqlSelect = string.Format("SELECT * FROM {0} WHERE {1}", usageRecord.TableName(), usageRecord.WhereClause());
                    DataSet ds = Database.Select(sqlSelect);

                    //--
                    //-- update existing record for current time intervale, 
                    //-- if we are aggregating results over the time period.
                    //--
                    if (ds.Tables.Count > 0 && ds.Tables[0].Rows.Count > 0 && string.IsNullOrEmpty(usageRecord.Aggregate()) == false)
                    {
                        string sqlUpdate = string.Format("UPDATE {0} SET {1} WHERE {2}", usageRecord.TableName(), usageRecord.Aggregate(), usageRecord.WhereClause());
                        Database.Select(sqlUpdate);
                    }
                    else
                    {
                        //
                        // no records found, insert on
                        //
                        insertRequired = true;
                    }
                }
                catch (System.Data.SQLite.SQLiteException sqlEx)
                {
                    //-- 
                    //-- most likely source of SQL failure: new database file.
                    //-- insert will force creation of table
                    //--
                    Trace.WriteLine("UsageReport:Insert:" + usageRecord.TableName() + ": " + sqlEx.Message);     
                    insertRequired = true;
                }
                catch (Exception ex)
                {
                    Trace.WriteLine("UsageReport:Insert:" + usageRecord.TableName() + ": " + ex.Message);
                }

                if (insertRequired)
                {
                    Database.Write(usageRecord);
                }
            }
        }

        /// <summary>
        /// Called when the report timer elapsed, this handler initiates
        /// the upload and purge processes.
        /// </summary>
        /// <param name="sender">sender object.</param>
        /// <param name="e">The <see cref="System.Timers.ElapsedEventArgs"/> instance containing the event data.</param>
        private static void OnReportTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            Trace.WriteLine("UsageReport:TimerElapsed");
            Upload();
            Purge();
        }

        /// <summary>
        /// Generate usage records for a specific record type, and merge with 
        /// dataset to be uploaded. The table name is derived from the dummy 
        /// usage record passed in. The resulting records are merged to the passed 
        /// in dataset, the calling routine ultimately will upload the records.
        /// </summary>
        /// <param name="usageRecord">Dummy usage record.</param>
        /// <param name="uploadDataSet">Dataset to be uploaded.</param>
        private static void Upload( string tableName, DataSet uploadDataSet )
        {
            lock (m_criticalSectionLock)
            {
                try
                {
                    string sqlSelect = string.Format("SELECT * FROM {0} WHERE Uploaded = 'false'", tableName);
                    DataSet ds = Database.Select(sqlSelect);

                    ds.Tables[0].TableName = tableName;
                    uploadDataSet.Merge(ds);

                    string sqlDelete = string.Format("UPDATE {0} SET Uploaded = 'true' WHERE Uploaded = 'false'", tableName);
                    Database.Select(sqlDelete);
                }
                catch (Exception ex)
                {
                    Trace.WriteLine("UsageReport:Upload:" + tableName + ": " + ex.Message);
                }
            }
        }

        /// <summary>
        /// Purges specified usage record type from the database.  
        /// Only uploaded records are deleted if they are older than one day.
        /// All records over five days old are deleted. Table name 
        /// is derived from the passed in usage record.
        /// </summary>
        /// <param name="usageRecord">Dummy usage record.</param>
        private static void Purge( string tableName )
        {
            lock (m_criticalSectionLock)
            {
                try
                {
                    //--
                    //-- purge uploaded records after one day
                    //--
                    DateTime purgeDate = DateTime.UtcNow - TimeSpan.FromDays(1);
                    string sqlDelete = string.Format("DELETE FROM {0} WHERE Uploaded = 'true' AND TimeStamp < {1:s}", tableName, purgeDate);
                    Database.Select(sqlDelete);

                    //--
                    //-- purge any record after five days
                    //--
                    purgeDate = DateTime.UtcNow - TimeSpan.FromDays(5);
                    sqlDelete = string.Format("DELETE FROM {0} WHERE TimeStamp < {1:s}", tableName, purgeDate);
                    Database.Select(sqlDelete);
                }
                catch (Exception ex)
                {
                    Trace.WriteLine("UsageReport:Purge:" + tableName + ": " + ex.Message);
                }
            }
        }

        //------------------------------------------------------------------------
        //--
        //-- Database Objects - TABLES
        //--
        //------------------------------------------------------------------------

        /// <summary>
        /// DatabaseObject class encapsulates all the tables that will be generated
        /// into the usage report database.  Each class within this class represents
        /// a table elemnet.
        /// 
        /// Class has to be public so that the SimpleDatabase.cs object works correctly
        /// with the sub classes in this class to automatically generate the tables
        /// inside the database file.
        /// </summary>
        public class DatabaseObject
        {
            /// <summary>
            /// Base class, contains timestamp, and logic for uploading.
            /// All subsequent classes are ultimately derived from this one.
            /// </summary>
            [Serializable]
            public class UsageRecord 
            {
                /// <summary>
                /// Class constructor, sets timestamp to interval timestamp.
                /// </summary>
                public UsageRecord()
                {
                    TimeStamp = UsageReport.CurrentIntervalTimeStamp;
                    Uploaded = false;
                }

                //--
                //-- public members become table columns
                //--
                public DateTime TimeStamp
                { get; set; }

                public bool Uploaded
                { get; set; }            

                //--
                //-- virtual functions for SQL statement clauses,
                //-- should be overwriten by derived classes as required
                //--

                /// <summary>
                /// Gets the where clause for use when generating SQL statements.
                /// </summary>
                /// <returns>Where clause.</returns>
                public virtual string WhereClause()
                {
                    return string.Format("TimeStamp = '{0:s}'", TimeStamp);
                }

                /// <summary>
                /// Aggregates clause used when.
                /// </summary>
                /// <returns>Aggregate set clause.</returns>
                public virtual string Aggregate()
                {
                    return null;
                }

                /// <summary>
                /// Get table name required when generating SQL statements.
                /// Derives table name from class name.
                /// Probably does not need to be redefined.
                /// </summary>
                /// <returns>Table name.</returns>
                public virtual string TableName()
                {
                    return this.GetType().Name;
                }
            }

            /// <summary>
            /// PyxNet usage record, always includes sending and receiving node information.
            /// </summary>
            [Serializable]
            public class PyxNetUsageRecord : UsageRecord
            {
                public PyxNetUsageRecord()
                {
                }

                public PyxNetUsageRecord( string inSendingNodeName, 
                                          Guid inSendingNodeGuid, 
                                          string inRecievingNodeName, 
                                          Guid inReceivingNodeGuid
                                          )
                    : base()
                {
                    SendingNodeName = inSendingNodeName;
                    SendingNodeGuid = inSendingNodeGuid.ToString();
                    ReceivingNodeName = inRecievingNodeName;
                    ReceivingNodeGuid = inReceivingNodeGuid.ToString();
                }

                //--
                //-- public members become table columns and
                //-- augment public members of base class
                //--
                public string SendingNodeName
                { get; set; }

                public string SendingNodeGuid
                { get; set; }

                public string ReceivingNodeName
                { get; set; }

                public string ReceivingNodeGuid
                { get; set; }

                //--
                //-- overrides for SQL statement clauses
                //--
                public override string WhereClause()
                {
                    return string.Format("{0} AND SendingNodeGuid = '{1}' AND ReceivingNodeGuid = '{2}'",
                            base.WhereClause(), SendingNodeGuid, ReceivingNodeGuid);
                }
            }

            /// <summary>
            /// PyxNet query record, counts number of queries between two specific nodes.
            /// </summary>
            [Serializable]
            public class PyxNetQueryUsageRecord : PyxNetUsageRecord
            {
                public PyxNetQueryUsageRecord()
                {
                }

                /// <summary>
                /// Query record constructor.
                /// </summary>
                public PyxNetQueryUsageRecord( string inSendingNodeName, 
                                               Guid inSendingNodeGuid, 
                                               string inRecievingNodeName, 
                                               Guid inReceivingNodeGuid
                                               ) 
                    : base(inSendingNodeName,inSendingNodeGuid,inRecievingNodeName,inReceivingNodeGuid)
                {
                    Queries = 1;
                }

                //--
                //-- public members become table columns and
                //-- augment public members of base class
                //--
                public int Queries
                { get; set; }

                //--
                //-- overrides for SQL statement clauses
                //--
                public override string Aggregate()
                {
                    return "Queries=Queries+1";
                }
            }

            /// <summary>
            /// PyxNet queries received, counts queires recieved on PyxNet between two nodes. 
            /// </summary>
            [Serializable]
            public class QueryReceived : PyxNetQueryUsageRecord
            {
                //--
                //-- straight derived class without any extras.
                //--
                public QueryReceived()
                {
                }

                public QueryReceived( string inSendingNodeName, 
                                      Guid inSendingNodeGuid, 
                                      string inRecievingNodeName, 
                                      Guid inReceivingNodeGuid
                                      ) 
                    : base(inSendingNodeName,inSendingNodeGuid,inRecievingNodeName,inReceivingNodeGuid)
                {
                }
            }

            /// <summary>
            /// Count number of matched queries between two nodes, aggregated by the matched dataset.
            /// </summary>
            [Serializable]
            public class QueryMatched : PyxNetQueryUsageRecord
            {
                public QueryMatched()
                {
                }

                public QueryMatched( string inSendingNodeName, 
                                     Guid inSendingNodeGuid, 
                                     string inRecievingNodeName, 
                                     Guid inReceivingNodeGuid, 
                                     Guid inDataSetId
                                     )
                    : base(inSendingNodeName, inSendingNodeGuid, inRecievingNodeName, inReceivingNodeGuid)
                {
                    DataSetId = inDataSetId.ToString();
                }
  
                //--
                //-- public members become table columns and
                //-- augment public members of base class
                //--
                public string DataSetId
                { get; set; }

                //--
                //-- overrides for SQL statement clauses
                //--
                public override string WhereClause()
                {
                    return string.Format("{0} AND DataSetId = '{1}'", base.WhereClause(), DataSetId);
                }
            }

            /// <summary>
            /// Tabluate data transfered between two nodes with respect to a single dataset.
            /// Aggregates based on chunks, and bytes transfered.
            /// </summary>
            [Serializable]
            public class DataTransfer : PyxNetUsageRecord
            {
                public DataTransfer()
                {
                }

                public DataTransfer( string inSendingNodeName, 
                                     Guid inSendingNodeGuid, 
                                     string inRecievingNodeName, 
                                     Guid inReceivingNodeGuid, 
                                     Guid inDataSetId, 
                                     long inBytesTransfered
                                     )
                    : base(inSendingNodeName, inSendingNodeGuid, inRecievingNodeName, inReceivingNodeGuid)
                {
                    DataSetId = inDataSetId.ToString();
                    BytesTransfered = inBytesTransfered;
                    Chunks = 1;
                }
  
                //--
                //-- public members become table columns and
                //-- augment public members of base class
                //--
                public string DataSetId
                { get; set; }

                public long BytesTransfered
                { get; set; }

                public int Chunks
                { get; set; }

                //--
                //-- overrides for SQL statement clauses
                //--
                public override string WhereClause()
                {
                    return string.Format("{0} AND DataSetId = '{1}'", base.WhereClause(), DataSetId);
                }

                public override string Aggregate()
                {
                    return string.Format("BytesTransfered=BytesTransfered+{0},Chunks=Chunks+1",BytesTransfered);
                }
            }
        }
    }

}
