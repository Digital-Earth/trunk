/******************************************************************************
UsageReportsDataObjects.cs

begin      : September 24, 2009
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
    /// 
    /// This file contains only the database objects for the reports.
    /// 
    /// </summary>
    public partial class UsageReports
    {

        //------------------------------------------------------------------------
        //--
        //-- Database Objects - TABLES
        //--
        //------------------------------------------------------------------------

        /// <summary>
        /// DatabaseObject class encapsulates all the tables that will be generated
        /// into the usage report database.  Each class within this class represents
        /// a table element.
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
                    TimeStamp = UsageReports.CurrentIntervalTimeStamp;
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

                public PyxNetUsageRecord(string inSendingNodeName,
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
                public PyxNetQueryUsageRecord(string inSendingNodeName,
                                               Guid inSendingNodeGuid,
                                               string inRecievingNodeName,
                                               Guid inReceivingNodeGuid
                                               )
                    : base(inSendingNodeName, inSendingNodeGuid, inRecievingNodeName, inReceivingNodeGuid)
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

                public QueryReceived(string inSendingNodeName,
                                      Guid inSendingNodeGuid,
                                      string inRecievingNodeName,
                                      Guid inReceivingNodeGuid
                                      )
                    : base(inSendingNodeName, inSendingNodeGuid, inRecievingNodeName, inReceivingNodeGuid)
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
            /// Tabluate data transferred between two nodes with respect to a single dataset.
            /// Aggregates based on chunks, and bytes transferred.
            /// </summary>
            [Serializable]
            public class DataTransfer : PyxNetUsageRecord
            {
                public DataTransfer()
                {
                }

                public DataTransfer(string inSendingNodeName,
                                     Guid inSendingNodeGuid,
                                     string inRecievingNodeName,
                                     Guid inReceivingNodeGuid,
                                     Guid inDataSetId,
                                     long inBytesTransferred
                                     )
                    : base(inSendingNodeName, inSendingNodeGuid, inRecievingNodeName, inReceivingNodeGuid)
                {
                    DataSetId = inDataSetId.ToString();
                    BytesTransferred = inBytesTransferred;
                    Chunks = 1;
                }

                //--
                //-- public members become table columns and
                //-- augment public members of base class
                //--
                public string DataSetId
                { get; set; }

                public long BytesTransferred
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
                    return string.Format("BytesTransferred=BytesTransferred+{0},Chunks=Chunks+1", BytesTransferred);
                }
            }

            /// <summary>
            /// Same as data transfer, but with a different name.
            /// Audits coverage data downloaded from PyxNet.
            /// </summary>
            [Serializable]
            public class DataDownload : DataTransfer
            {
                public DataDownload()
                {
                }

                public DataDownload(string inSendingNodeName,
                                     Guid inSendingNodeGuid,
                                     string inRecievingNodeName,
                                     Guid inReceivingNodeGuid,
                                     Guid inDataSetId,
                                     long inBytesTransferred
                                     )
                    : base( inSendingNodeName, inSendingNodeGuid, 
                            inRecievingNodeName, inReceivingNodeGuid, 
                            inDataSetId,inBytesTransferred 
                            )
                {
                }
           }

            /// <summary>
            /// Keep record of every report generated.
            /// </summary>
            [Serializable]
            public class ReportsGenerated : PyxNetUsageRecord
            {
                public ReportsGenerated()
                {
                }

                public ReportsGenerated(string inSendingNodeName,
                                         Guid inSendingNodeGuid,
                                         string inRecievingNodeName,
                                         Guid inReceivingNodeGuid,
                                         string inFileName
                                         )
                    : base(inSendingNodeName, inSendingNodeGuid, inRecievingNodeName, inReceivingNodeGuid)
                {
                    FileName = inFileName;
                    Acknowledged = false;
                }

                //--
                //-- public members become table columns and
                //-- augment public members of base class
                //--
                public string FileName
                { get; set; }

                public bool Acknowledged
                { get; set; }
            }

        }
    }
}