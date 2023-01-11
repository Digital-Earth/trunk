/******************************************************************************
UsageReports.cs

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
    public partial class UsageReports
    {
        private const string m_fileName = "UsageReport.sqlite";
        private const long m_intervalSizeInMinutes = 5;
        private const int m_reportTimerElapseTimeInHours = 1;

        private static Pyxis.Utilities.UsageReportsHelper Helper = new Pyxis.Utilities.UsageReportsHelper();

        private static SimpleDatabase m_database = null;
        private static string m_fullDatabaseFileName  = null;
        private static string m_reportsDir = null;
        private static DateTime m_lastPurged = new DateTime();
        private static bool m_initialized = false;

        /// <summary>
        /// Report time, fires once an hour.  
        /// Creates reports to send to license server.
        /// </summary>
        private static SimpleTimer m_reportTimer = new SimpleTimer(new TimeSpan(m_reportTimerElapseTimeInHours, 0, 0));

        /// <summary>
        /// Critical section lock used to restrict concurrent acccess to sensative 
        /// areas within this module.  Most pretain to writing out to the database.
        /// </summary>
        private static object m_insertSectionLock = new Object();
        private static object m_uploadSectionLock = new Object();

        /// <summary>
        /// Simplifies access to the helper log, and
        /// hides implementation details to external calls.
        /// </summary>
        /// <value>The helper log.</value>
        public static Pyxis.Utilities.LogHelper Log
        {
            get
            {
                return Helper.Log;
            }
        }

        //------------------------------------------------------------------------
        //--
        //-- PUBLIC METHODS
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
        public static void Initialize(string libPath, Pyxis.Utilities.UsageReportsHelper inHelper )
        {    
            Initalized = true;
            
            //--
            //-- do not overwrite the default helper if one is not provided.
            //-- a null helper would be a disaster.
            //--
            if (inHelper != null)
            {
                Helper = inHelper;
            }

            m_fullDatabaseFileName = libPath + "/" + m_fileName;
            
            m_reportsDir = libPath + "/" + "UsageReports";
            if (!System.IO.Directory.Exists(m_reportsDir))
            {
                System.IO.Directory.CreateDirectory(m_reportsDir);
            }

            m_reportTimer.Timer.AutoReset = true;
            m_reportTimer.Elapsed += OnReportTimerElapsed;

            Log.Info("UsageReports:Initialize: libPath={0}", libPath);
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

        /// <summary>
        /// Generates usage reports.  
        /// Initiates upload and purge processes.
        /// </summary>
        public static void GenerateReports()
        {
            System.Threading.Thread workerThread = new System.Threading.Thread(delegate()
            {
                Log.Info("UsageReports:GenerateReports:");

                Upload();
                Purge();
            });

            workerThread.Name = "Generate Usage Reports";
            workerThread.Start();
        }

        /// <summary>
        /// Acknowledge that the report has been received by changing the Acknowledge column
        /// to True for the reports entry in the ReportsGenerated table.
        /// </summary>
        /// <param name="reportName">Name of report.</param>
        public static void AcknowledgeReport(string reportName)
        {
            Log.Info("UsageReports:AcknowledgeReport: report=" + reportName);
            
            lock (m_insertSectionLock)
            {
                try
                {
                    if (TableExists("ReportsGenerated"))
                    {
                        string whereClause = string.Format("FileName = '{0}'", reportName);
                        string sqlSelect = string.Format("UPDATE ReportsGenerated SET Acknowledged = 'true' WHERE {0}", whereClause);
                        Database.Select(sqlSelect);
                    }
                    else
                    {
                        Log.Warning("UsageReports:AcknowledgeReport: no ReportsGenerated table to update.");
                    }
                }
                catch (Exception ex)
                {
                    Log.Error("UsageReports:AcknowledgeReport:" + reportName + ": " + ex.Message);
                }
            }
        }

        /// <summary>
        /// Resends the report. Unarchive it from the reports directory,
        /// repackage it, and send it again.
        /// </summary>
        /// <param name="reportName">Name of the report.</param>
        public static void ResendReport(string reportName)
        {
            Log.Info("UsageReports:ResendReport: report=" + reportName);

            try
            {
                string fullXmlFileName = reportName;
                if (string.IsNullOrEmpty(m_reportsDir) == false)
                {
                    fullXmlFileName = string.Format("{0}/{1}", m_reportsDir, reportName);
                }

                DataSet uploadDataSet = new DataSet();
                uploadDataSet.ReadXml(fullXmlFileName);

                DataTable reportInfoTable = uploadDataSet.Tables["ReportInfo"];

                System.Diagnostics.Trace.Assert(reportInfoTable.Rows.Count == 1, "There should be one (and only one) ReportInfo row.");

                Guid receivingNodeGuid = new Guid(reportInfoTable.Rows[0]["ReceivingNodeGuid"].ToString());
                Guid sendingNodeGuid = new Guid(reportInfoTable.Rows[0]["SendingNodeGuid"].ToString());

                new System.Threading.Thread(delegate()
                {
                    Helper.SendReport(uploadDataSet, receivingNodeGuid, sendingNodeGuid);
                }).Start();
            }
            catch( Exception ex )
            {
                Log.Error("UsageReports:ResendReport:" + reportName + ": " + ex.Message);
            }            
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
        /// Gets or sets the current update time stamp,
        /// the time when the update operation started.
        /// </summary>
        /// <value>The update time stamp.</value>
        private static DateTime CurrentUpdateTimeStamp
        { get; set; }

        /// <summary>
        /// Insert usage record into database.
        /// Works for all records derived from UsageReport.DatabaseObject.UsageRecord.
        /// </summary>
        /// <param name="usageRecord">The usage record.</param>
        private static void Insert(UsageReports.DatabaseObject.UsageRecord usageRecord) 
        {
            //--
            //-- return immediately if module is not initialized.
            //--
            if (!Initalized)
            {
                return;
            }

            System.Threading.ThreadPool.QueueUserWorkItem(delegate 
            {
                bool insertRequired = false;
                DateTime intervalTimeStamp = usageRecord.TimeStamp;

                //Log.Debug("UsageReports:Insert:" + usageRecord.TableName());
                
                lock (m_insertSectionLock)
                {
                    //Log.Debug("[+]UsageReports:Insert:" + usageRecord.TableName());

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
                        Log.Warning("UsageReports:Insert:" + usageRecord.TableName() + ": " + sqlEx.Message);     
                        insertRequired = true;
                    }
                    catch (Exception ex)
                    {
                        Log.Error("UsageReports:Insert:" + usageRecord.TableName() + ": " + ex.Message);
                    }

                    if (insertRequired)
                    {
                        try
                        {
                            Database.Write(usageRecord);

                        }
                        catch (Exception ex)
                        {
                            Log.Error("UsageReports:Insert:" + usageRecord.TableName() + ": " + ex.Message);
                        }
                    }
                 
                    //Log.Debug("[-]UsageReports:Insert:" + usageRecord.TableName());
                }
            });
        }

        /// <summary>
        /// Called when the report timer elapses, this handler initiates
        /// the report generation process.
        /// </summary>
        /// <param name="sender">sender object.</param>
        /// <param name="e">The <see cref="System.Timers.ElapsedEventArgs"/> instance containing the event data.</param>
        private static void OnReportTimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            GenerateReports();
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
            //--
            //-- check for un-acknowledged reports before sending any new ones.
            //-- dont want to confuse current transactions with older ones.
            //--
            ResendUnacknowledgedReports();

            //--
            //-- set timestamp that all records generated by this operation will have
            //--
            CurrentUpdateTimeStamp = CurrentIntervalTimeStamp;

            //ForEachLicenseServer(new EnumLicenseServerCallback(DumpLicenseServerDataIds));

            //--
            //-- call upload for each known license server
            //-
            ForEachLicenseServer(new EnumLicenseServerCallback(Upload));
        }

        /// <summary>
        /// Uploads reports pertaining to a list data sets to a specified license server.
        /// </summary>
        /// <param name="serverId">License Server ID.</param>
        /// <param name="dataSetIds">List of data set IDs.</param>
        private static void Upload(Guid serverId, List<Guid> dataSetIds)
        {
            if (serverId == Guid.Empty)
            {
                Log.Warning("UsageReports:Upload: attempted to upload reports to NULL license server");
                return;
            }

            DataSet uploadDataSet = new DataSet("PyxNetUsageReport");

            Guid sendingNodeGuid = new Guid();
            Guid receivingNodeGuid = new Guid();

            string sendingNodeName = null;
            string receivingNodeName = null;
            
            lock (m_uploadSectionLock)
            {
                //--
                //-- save local node and license server node information,
                //-- used in report.
                //--
                sendingNodeGuid = Helper.FindLocalHost();
                sendingNodeName = Helper.FindNodeFriendlyName(sendingNodeGuid);

                receivingNodeGuid = serverId;
                receivingNodeName = Helper.FindNodeFriendlyName(receivingNodeGuid);
            
                //--
                //-- generate unique name for report
                //--
                string xmlFileName = null;
                string fullXmlFileName = null;

                if (string.IsNullOrEmpty(m_reportsDir) == false)
                {
                    //--
                    //-- construct filename for report using todays date.
                    //-- add counter and loop until we find an unused filename.
                    //--
                    int nLoopCnt = 0;
                    do
                    {
                        xmlFileName = string.Format("{0:yyyy-MMM-dd-}{1:000}.xml", DateTime.Now, nLoopCnt++);
                        fullXmlFileName = string.Format("{0}/{1}", m_reportsDir, xmlFileName);
                    } while (System.IO.File.Exists(fullXmlFileName) && nLoopCnt < 1000);
                }

                //--
                //-- add header record to report
                //--
                DataTable reportHeader = new DataTable("ReportInfo");

                reportHeader.Columns.Add("TimeStamp");
                reportHeader.Columns.Add("SendingNodeName");
                reportHeader.Columns.Add("SendingNodeGuid");
                reportHeader.Columns.Add("ReceivingNodeName");
                reportHeader.Columns.Add("ReceivingNodeGuid");
                reportHeader.Columns.Add("FileName");

                reportHeader.Rows.Add( new object[] { CurrentUpdateTimeStamp.ToString("s"), 
                                                      sendingNodeName, 
                                                      sendingNodeGuid,
                                                      receivingNodeName,
                                                      receivingNodeGuid,
                                                      xmlFileName
                                                      });

                uploadDataSet.Tables.Add(reportHeader);


                if (dataSetIds.Count > 0)
                {
                    //--
                    //-- add data source info to report
                    //--
                    DataTable reportDataSources = new DataTable("DataSource");

                    reportDataSources.Columns.Add("Guid");
                    reportDataSources.Columns.Add("Name");
                    reportDataSources.Columns.Add("Description");

                    foreach (Guid dataSetId in dataSetIds)
                    {
                        UsageReportsHelper.DataSourceInfo dsInfo = Helper.GetDataSourceInfo(dataSetId);
                        if (dsInfo != null)
                        {
                            reportDataSources.Rows.Add(
                                new object[] { dsInfo.Id, dsInfo.Name, dsInfo.Desc } );
                        }
                    }

                    uploadDataSet.Tables.Add(reportDataSources);
                }
          
                //--
                //-- query all database tables, and have usage data
                //-- appended to dataset that will ultimately be uploaded.
                //--
                System.DateTime lastReportTime = GetLastReportTime(serverId);
                List<string> tables = FindAllTableNames();
                foreach (string tableName in tables)
                {
                    if (TableContainsColumn(tableName, "DataSetId"))
                    {
                        Upload(tableName, dataSetIds, uploadDataSet);
                    }
                    else
                    {
                        Upload(tableName, lastReportTime, uploadDataSet);
                    }
                }

                //--
                //-- do something intelegent with the dataset.
                //-- for now, we generate an XML file and store it on the machine.
                //-- future work will upload the data set (via an XML file?)
                //-- to the license server
                //--
                if (string.IsNullOrEmpty(fullXmlFileName) == false)
                {
                    uploadDataSet.WriteXml(fullXmlFileName);
                }
                
                //--
                //-- add a reports generated record to database for 
                //-- book keeping purposes.
                //--
                UsageReports.DatabaseObject.ReportsGenerated dbObj
                    = new UsageReports.DatabaseObject.ReportsGenerated( sendingNodeName,
                                                                       sendingNodeGuid,
                                                                       receivingNodeName,
                                                                       receivingNodeGuid,
                                                                       xmlFileName
                                                                       );

                dbObj.TimeStamp = CurrentUpdateTimeStamp;
                Database.Write(dbObj);
            }

            //--
            //-- call helper function to send report to server via PyxNet
            //--
            new System.Threading.Thread(delegate()
            {
                Helper.SendReport(uploadDataSet, receivingNodeGuid, sendingNodeGuid);
            }).Start();
        }

        /// <summary>
        /// Upload process for records from specific table and matching data set IDs.  
        /// Takes list of data set IDs and where clause for SQL query.  
        /// </summary>
        /// <param name="tableName">Name of the table.</param>
        /// <param name="dataSetIds">List of data set IDs.</param>
        /// <param name="uploadDataSet">Resulting dataset.</param>
        private static void Upload(string tableName, List<Guid> dataSetIds, DataSet uploadDataSet)
        {
            if (dataSetIds == null)
                return;
            
            System.Diagnostics.Debug.Assert(dataSetIds.Count > 0);

            string commaDelimitedIdList = null;
            foreach( Guid dataSetId in dataSetIds ) 
            {
                if (string.IsNullOrEmpty(commaDelimitedIdList) == false)
                    commaDelimitedIdList += ", ";
                commaDelimitedIdList += string.Format("'{0}'", dataSetId.ToString());
            }

            string whereClause = string.Format("Uploaded = 'false' AND TimeStamp < '{0:s}' AND DataSetId IN ({1})",
                CurrentUpdateTimeStamp, commaDelimitedIdList);

            Upload(tableName, whereClause, uploadDataSet);
        }

        /// <summary>
        /// Upload process for records from specific table since last upload time.
        /// Takes timestamp and builds where clause for SQL query.
        /// </summary>
        /// <param name="tableName">Name of the table.</param>
        /// <param name="lastUploadTime">The last upload time.</param>
        /// <param name="uploadDataSet">Resulting dataset.</param>
        private static void Upload(string tableName, System.DateTime lastUploadTime, DataSet uploadDataSet)
        {
            string whereClause = string.Format("TimeStamp >= '{0:s}' AND TimeStamp < '{1:s}'", 
                lastUploadTime, CurrentUpdateTimeStamp);

            Upload(tableName, whereClause, uploadDataSet);
        }

        /// <summary>
        /// Generate usage records for a specific record type, and merge with
        /// dataset to be uploaded. The resulting records are merged to the passed
        /// in dataset, the calling routine ultimately will upload the records.
        /// </summary>
        /// <param name="tableName">Name of the table.</param>
        /// <param name="whereClause">SQL where clause.</param>
        /// <param name="uploadDataSet">Dataset to be uploaded.</param>
        private static void Upload( string tableName, string whereClause, DataSet uploadDataSet )
        {
            try
            {
                string sqlSelect = string.Format("SELECT * FROM {0} WHERE {1}", tableName, whereClause);
                DataSet ds = Database.Select(sqlSelect);

                ds.Tables[0].TableName = tableName;
                uploadDataSet.Merge(ds);

                string sqlDelete = string.Format("UPDATE {0} SET Uploaded = 'true' WHERE {1}", tableName, whereClause);
                Database.Select(sqlDelete);
            }
            catch (Exception ex)
            {
                Log.Error("UsageReports:Upload:" + tableName + ": " + ex.Message);
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
            //-- run purge procedure for each table found in database
            //--
            List<string> tables = FindAllTableNames();
            foreach (string tableName in tables)
            {
                Purge(tableName);
            }

            m_lastPurged = DateTime.Now;
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
            //lock (m_criticalSectionLock)
            {
                try
                {
                    //--
                    //-- purge uploaded records after one day
                    //--
                    DateTime purgeDate = DateTime.UtcNow - TimeSpan.FromDays(1);
                    string sqlDelete = string.Format("DELETE FROM {0} WHERE Uploaded = 'true' AND TimeStamp < '{1:s}'", tableName, purgeDate);
                    Database.Select(sqlDelete);

                    //--
                    //-- purge any record after five days
                    //--
                    purgeDate = DateTime.UtcNow - TimeSpan.FromDays(5);
                    sqlDelete = string.Format("DELETE FROM {0} WHERE TimeStamp < '{1:s}'", tableName, purgeDate);
                    Database.Select(sqlDelete);
                }
                catch (Exception ex)
                {
                    Log.Error("UsageReports:Purge:" + tableName + ": " + ex.Message);
                }
            }
        }

        /// <summary>
        /// Resends all unacknowledged reports.
        /// </summary>
        private static void ResendUnacknowledgedReports()
        {
            try
            {
                if (TableExists("ReportsGenerated"))
                {
                    //--
                    //-- count un-acknowledged reports.
                    //-- do count query first so that we do not needlessly start
                    //-- a thread when there is no need to.
                    //--
                    string sqlCount = string.Format("SELECT Count(*) FROM ReportsGenerated WHERE Acknowledged = 'false'");
                    DataSet countDS = Database.Select(sqlCount);

                    if (countDS.Tables.Count == 1 && countDS.Tables[0].Rows.Count == 1)
                    {
                        int recordCount = int.Parse(countDS.Tables[0].Rows[0][0].ToString());
                        if (recordCount > 0)
                        {
                            //--
                            //-- spawn off thread that will slowly process the 
                            //-- un-acknowledged repots.
                            //--
                            System.Threading.Thread workerThread = new System.Threading.Thread(delegate()
                            {
                                try
                                {
                                    string sqlSelect = string.Format("SELECT FileName FROM ReportsGenerated WHERE Acknowledged = 'false'");
                                    DataSet ds = Database.Select(sqlSelect);

                                    foreach (DataRow row in ds.Tables[0].Rows)
                                    {
                                        string reportFileName = row["FileName"].ToString();

                                        //--
                                        //-- pause 20 seconds before each iteration
                                        //--
                                        System.Threading.Thread.Sleep(1000 * 20);

                                        //--
                                        //-- perform one last sanity check, maybe someone
                                        //-- has acknowledged it since the original query.
                                        //--
                                        sqlSelect = string.Format("SELECT Acknowledged FROM ReportsGenerated WHERE FileName = '{0}'", reportFileName);
                                        DataSet lastCheckDS = Database.Select(sqlSelect);

                                        if (lastCheckDS.Tables.Count == 1 && lastCheckDS.Tables[0].Rows.Count == 1)
                                        {
                                            if (lastCheckDS.Tables[0].Rows[0]["Acknowledged"].ToString() == "false")
                                            {
                                                ResendReport(reportFileName);
                                            }
                                        }
                                    }
                                }
                                catch (Exception ex)
                                {
                                    Log.Error("UsageReports:ResendUnacknowledgedReports: " + ex.Message);
                                }
                            });

                            workerThread.Name = "Resending Unacknowledged Reports";
                            workerThread.Start();
                        }
                    }
                }
                else
                {
                    Log.Warning("UsageReports:ResendUnacknowledgedReports: no ReportsGenerated table to select from.");
                }
            }
            catch (Exception ex)
            {
                Log.Error("UsageReports:ResendUnacknowledgedReports: " + ex.Message);
            }
        }

        /// <summary>
        /// Find time of last generated report for a license server.
        /// </summary>
        /// <param name="serverId">License server ID.</param>
        /// <returns></returns>
        private static System.DateTime GetLastReportTime( Guid serverId )
        {
            System.DateTime lastReportTime = new System.DateTime(1900, 1, 1);

            try
            {
                if (TableExists("ReportsGenerated"))
                {
                    //--
                    //-- check ReportsGenerated to find last time a report was
                    //-- generated for to this license server.
                    //--
                    string sqlSelect = string.Format("SELECT * FROM ReportsGenerated WHERE ReceivingNodeGuid = '{0}'", serverId.ToString());
                    sqlSelect += " ORDER BY TimeStamp DESC LIMIT 1";
                    DataSet ds = Database.Select(sqlSelect);

                    System.Diagnostics.Debug.Assert(ds.Tables.Count == 1);
                    DataTable table = ds.Tables[0];

                    if (table.Rows.Count > 0)
                    {
                        string ts = (table.Rows[0]["TimeStamp"]).ToString();
                        lastReportTime = System.DateTime.Parse(ts);
                    }
                }
                else
                {
                    Log.Warning("UsageReports:GetLastReportTime: no ReportsGenerated table to select from.");
                }
            }
            catch (Exception ex)
            {
                Log.Error("UsageReports:GetLastReportTime: " + ex.Message);
            }

            return lastReportTime;
        }

        /// <summary>
        /// Finds all table names in usage reports database.
        /// </summary>
        /// <returns></returns>
        private static List<string> FindAllTableNames()
        {
            List<string> tableNames = new List<string>();

            try
            {
                //--
                //-- retrieve one row from database table
                //--
                string sqlSelect = string.Format("SELECT name FROM sqlite_master WHERE type = 'table'");
                DataSet ds = Database.Select(sqlSelect);

                //--
                //-- should only get one table back
                //--
                System.Diagnostics.Debug.Assert(ds.Tables.Count == 1);
                DataTable table = ds.Tables[0];

                //--
                //-- traverse returned rows
                //--
                foreach (DataRow row in table.Rows)
                {
                    tableNames.Add(row["name"].ToString());
                }
            }
            catch (Exception ex)
            {
                Log.Error("UsageReports:FindAllTableNames: " + ex.Message);
            }

            return tableNames;
        }

        /// <summary>
        /// Tests for existance of table.
        /// </summary>
        /// <param name="tableName">Name of the table.</param>
        /// <returns>true if table exists</returns>
        private static bool TableExists(string tableName)
        {
            List<string> tables = FindAllTableNames();
            return tables.Contains(tableName);
        }

        /// <summary>
        /// Determines if database table contains a particular column.
        /// </summary>
        /// <param name="tableName">Name of table.</param>
        /// <param name="columnName">Name of column.</param>
        /// <returns>True if column exists in table.</returns>
        private static bool TableContainsColumn(string tableName, string columnName)
        {
            bool columnFound = false;

            try
            {
                //--
                //-- retrieve one row from database table
                //--
                string sqlSelect = string.Format("SELECT * FROM {0} LIMIT 1", tableName);
                DataSet ds = Database.Select(sqlSelect);

                //--
                //-- should only get one table back
                //--
                System.Diagnostics.Debug.Assert(ds.Tables.Count == 1);
                DataTable table = ds.Tables[0];

                //--
                //-- traverse columns looking for particular column name
                //--
                foreach (DataColumn col in table.Columns)
                {
                    if (col.ColumnName == columnName)
                    {
                        //--
                        //-- stop the foreach loop once the column is found
                        //--
                        columnFound = true;
                        break;
                    }
                }
            }
            catch (Exception ex)
            {
                Log.Error("UsageReports:TableContainsColumn:" + tableName + ": " + ex.Message);
            }

            return columnFound;
        }

        /// <summary>
        /// Finds license server associated with data set.
        /// May require call to helper class.
        /// </summary>
        /// <param name="dataSetId">The data set ID.</param>
        /// <returns></returns>
        private static Guid FindLicenseServer(Guid dataSetId)
        {
            return Helper.FindLicenseServer(dataSetId);
        }

        /// <summary>
        /// Dumps to trace all the data set IDs that were found to be
        /// associated with a particular license server.
        /// </summary>
        /// <param name="serverId">The server id.</param>
        /// <param name="dataSetIds">The data set ids.</param>
        private static void DumpLicenseServerDataIds( Guid serverId, List<Guid> dataSetIds )
        {
            Log.Trace("[+]server: " + serverId.ToString());
            foreach (Guid setId in dataSetIds)
            {
                Log.Trace("== " + setId.ToString());
            }
            Log.Trace("[-]server:");
        }

        /// <summary>
        /// Delegate place holder for license server enumerator.
        /// Called for each license server discovered during analysis of usage data.
        /// </summary>
        private delegate void EnumLicenseServerCallback(Guid serverId, List<Guid> dataSetIds);

        /// <summary>
        /// Analyse the current usage report data, and derive a list of license servers with
        /// associated data source ids.  Call callback function for each license server, and
        /// pass in the list of data sources associated with the particular license server.
        /// </summary>
        /// <param name="serverCallback">License server callback.</param>
        private static void ForEachLicenseServer(EnumLicenseServerCallback serverCallback)
        {
            List<Guid> dataSetIds = new List<Guid>();
            Dictionary<Guid, List<Guid>> licenseServerTable = new Dictionary<Guid, List<Guid>>();

            try
            {
                //--
                //-- find all data set ids, and their associated license servers
                //--
                List<string> tables = FindAllTableNames();
                foreach (string tableName in tables)
                {
                    //--
                    //-- traverse all tables in database looking 
                    //-- for tables with a "DataSetId" column
                    //--
                    if (TableContainsColumn(tableName, "DataSetId"))
                    {
                        string sqlSelect = string.Format("SELECT DISTINCT DataSetId FROM {0} WHERE Uploaded = 'false'", tableName);
                        DataSet ds = Database.Select(sqlSelect);

                        //--
                        //-- should only get one table back
                        //--
                        System.Diagnostics.Debug.Assert(ds.Tables.Count == 1);
                        DataTable table = ds.Tables[0];

                        foreach (DataRow row in table.Rows)
                        {
                            Guid setId = new Guid( row["DataSetId"].ToString());
                            if (dataSetIds.Contains(setId) == false)
                            {
                                dataSetIds.Add(setId);
                                
                                Guid serverId = FindLicenseServer(setId);
                                if (licenseServerTable.ContainsKey(serverId) == false)
                                {
                                    licenseServerTable.Add(serverId, new List<Guid>());
                                }
                                licenseServerTable[serverId].Add(setId);
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Log.Error("UsageReports:ForEachLicenseServer: " + ex.Message);
            }

            //--
            //-- invoke callback for each discovered license server.
            //--
            foreach ( KeyValuePair<Guid,List<Guid>> serverDataSetListPair in licenseServerTable)
            {
                serverCallback(serverDataSetListPair.Key, serverDataSetListPair.Value);
            }
        }
    }
}
