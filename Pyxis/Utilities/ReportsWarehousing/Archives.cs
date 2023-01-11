/******************************************************************************
Archives.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

using NHibernate;
using NHibernate.Cfg;
using NHibernate.Tool.hbm2ddl;

using NUnit.Framework;

using Pyxis.Utilities.ReportsWarehouse.Domain;


namespace Pyxis.Utilities.ReportsWarehouse
{

    /// <summary>
    /// Encapsulates all the operations needed to walk over an XML report
    /// and generate the necessary records in the reports warehouse.
    /// Class is disposable so that it can be release as soon as it
    /// </summary>
    public class Archives : IDisposable
    {
        //--
        //-- link to logging utility, and associated property.
        //---
        static Pyxis.Utilities.LogHelper m_log = null;

        /// <summary>
        /// Gets the log utility.
        /// </summary>
        /// <value>Log utility.</value>
        public static Pyxis.Utilities.LogHelper Log
        {
            get
            {
                if( m_log == null )
                {
                    m_log = new Pyxis.Utilities.LogHelper(null);
                }
                return m_log;
            }
        }

        /// <summary>
        /// Process XML report.
        /// </summary>
        /// <param name="reportXml">Report XML.</param>
        public static void ProcessReports(string reportXml)
        {
            Log.Trace("[+]UsageReports:ReportsWarehouse:ProcessReports: start");
           
            try
            {
                using (Archives ar = new Archives())
                {
                    //-- 
                    //-- read passed in xml and create data set 
                    //--
                    System.Data.DataSet ds = new System.Data.DataSet();
                    using (StringReader textReader = new StringReader(reportXml))
                    {
                        ds.ReadXml(textReader);
                    }

                    //--
                    //-- iterate over each row in each table found in xml report
                    //-- 
                    foreach (System.Data.DataTable table in ds.Tables)
                    {
                        System.Data.DataColumnCollection cols = table.Columns;
                        foreach (System.Data.DataRow row in table.Rows)
                        {
                            ar.ProcessRowData(table.TableName, cols, row);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Log.Error("[-]UsageReports:ReportsWarehouse:ProcessReports: " + ex.Message);
                throw ex;
            }

            Log.Trace("[-]UsageReports:ReportsWarehouse:ProcessReports: done");
        }


        /// <summary>
        /// Initializes a new instance of the <see cref="Archives"/> class.
        /// Cleans up any cached static data.
        /// </summary>
        public Archives()
        {
            //--
            //-- flush cached data 
            //--
            CommonRowData.CleanUp();
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
        }

        /// <summary>
        /// Gets or sets Report record.
        /// </summary>
        /// <value>Report record</value>
        private Report Report
        { get; set; }

        /// <summary>
        /// Process row data from XML report.
        /// </summary>
        /// <param name="tableName">Name of the table.</param>
        /// <param name="cols">The cols.</param>
        /// <param name="row">The row.</param>
        private void ProcessRowData(string tableName, System.Data.DataColumnCollection cols, System.Data.DataRow row)
        {
            switch (tableName)
            {
                case "ReportInfo":
                    ProcessReportInfo(cols, row);
                    break;

                case "DataSource":
                    ProcessDataSource(cols, row);
                    break;

                case "QueryMatched":
                    ProcessMatchedQuery(cols, row);
                    break;
                
                case "QueryReceived":
                    ProcessReceivedQuery(cols, row);
                    break;

                case "DataTransfer":
                    ProcessDataTransferred(cols, row);
                    break;

                default:
                    break;
            }
        }

        /// <summary>
        /// Process row of data from ReportInfo table.
        /// </summary>
        /// <param name="cols">The cols.</param>
        /// <param name="row">The row.</param>
        private void ProcessReportInfo(System.Data.DataColumnCollection cols, System.Data.DataRow row)
        {
            //--
            //-- extract common data elments from row
            //-- 
            CommonRowData common = new CommonRowData(cols, row);

            string reportName = null;
            if (cols.Contains("FileName"))
            {
                reportName = row["FileName"].ToString();
            }

            //--
            //-- remove all previous occurances of this report that have been saved.
            //-- normally, there should not be an existing occurance of the same report,
            //-- unless the report has been resent for some reason.
            //-- 
            //-- if there are multiple occurances of the same report in the database,
            //-- then we have a catastrophic condition that needs to be delt with.
            //-- 
            ICollection<Report> reportList = RepositoryHelper.Report.Get(common.SendingNode,reportName);
            foreach( Report report in reportList )
            {
                RepositoryHelper.Report.Remove(report);
            }

            if (reportList.Count > 1)
            {
                Log.Warning("UsageReports:ReportsWarehouse:ProcessReportInfo: mulitple({0}) occurances of report=\"{1}\" from=\"{2}\"",
                    reportList.Count, reportName, common.SendingNode.Name);
            }

            Report = new Report()
            {
                TimeStamp = common.TimeStamp,
                SendingNode = common.SendingNode,
                Name = reportName,
                Processed = System.DateTime.UtcNow
            };

            Log.Trace("UsageReports:ReportsWarehouse:ProcessReportInfo: report=\"{0}\" from=\"{1}\"",
                reportName, common.SendingNode.Name);

            RepositoryHelper.Report.Add(Report);
        }

        /// <summary>
        /// Process row of data from DataSource table.
        /// </summary>
        /// <param name="cols">The cols.</param>
        /// <param name="row">The row.</param>
        private void ProcessDataSource(System.Data.DataColumnCollection cols, System.Data.DataRow row)
        {
            Guid id = Guid.Empty;
            string name = null;
            string desc = null;

            if (cols.Contains("Guid"))
            {
                id = new Guid(row["Guid"].ToString());
            }
            if (cols.Contains("Name"))
            {
                name = row["Name"].ToString();
            }

            if (cols.Contains("Description"))
            {
                desc = row["Description"].ToString();
            }

            DataSet ds = RepositoryHelper.DataSet.GetByGuid(id);
            if (ds == null)
            {
                ds = new DataSet()
                {
                    Ident = id,
                    Name = name,
                    Description = desc
                };
                RepositoryHelper.DataSet.Add(ds);
            }
            else
            {
                bool updateFlag = false;
                if (ds.Name != name)
                {
                    ds.Name = name;
                    updateFlag = true;
                }
                if (ds.Description != desc)
                {
                    ds.Description = desc;
                    updateFlag = true;
                }
                if (updateFlag)
                {
                    RepositoryHelper.DataSet.Update(ds);
                }
            }
        }

        /// <summary>
        /// Process row for data from QueryMatched table.
        /// </summary>
        /// <param name="cols">The cols.</param>
        /// <param name="row">The row.</param>
        private void ProcessMatchedQuery(System.Data.DataColumnCollection cols, System.Data.DataRow row)
        {
            //--
            //-- extract common data elments from row
            //-- 
            CommonRowData common = new CommonRowData(cols, row);

            int hitCount = 0;
            if (cols.Contains("Queries"))
            {
                hitCount = System.Convert.ToInt32(row["Queries"]);
            }

            MatchedQuery query = new MatchedQuery()
            {
                TimeStamp = common.TimeStamp,
                DataSet = common.DataSet,
                SendingNode = common.SendingNode,
                ReceivingNode = common.ReceivingNode,
                Hits = hitCount,
                Report = Report
            };

            RepositoryHelper.MatchedQuery.Add(query);
        }

        /// <summary>
        /// Process row of data from QueryReceived table.
        /// </summary>
        /// <param name="cols">The cols.</param>
        /// <param name="row">The row.</param>
        private void ProcessReceivedQuery(System.Data.DataColumnCollection cols, System.Data.DataRow row)
        {
            //--
            //-- extract common data elments from row
            //-- 
            CommonRowData common = new CommonRowData(cols, row);

            int hitCount = 0;
            if (cols.Contains("Queries"))
            {
                hitCount = System.Convert.ToInt32(row["Queries"]);
            }

            ReceivedQuery query = new ReceivedQuery()
            {
                TimeStamp = common.TimeStamp,
                SendingNode = common.SendingNode,
                ReceivingNode = common.ReceivingNode,
                Hits = hitCount,
                Report = Report
            };

            RepositoryHelper.ReceivedQuery.Add(query);
        }

        /// <summary>
        /// Processes row of data from DataTransferred table.
        /// </summary>
        /// <param name="cols">The cols.</param>
        /// <param name="row">The row.</param>
        private void ProcessDataTransferred(System.Data.DataColumnCollection cols, System.Data.DataRow row)
        {
            //--
            //-- extract common data elments from row
            //-- 
            CommonRowData common = new CommonRowData(cols, row);

            int chunksCount = 0;
            if (cols.Contains("Chunks"))
            {
                chunksCount = System.Convert.ToInt32(row["Chunks"]);
            }

            int bytesTransferred = 0;
            if (cols.Contains("BytesTransferred"))
            {
                bytesTransferred = System.Convert.ToInt32(row["BytesTransferred"]);
            }

            DataTransferred dataRecord = new DataTransferred()
            {
                TimeStamp = common.TimeStamp,
                DataSet = common.DataSet,
                SendingNode = common.SendingNode,
                ReceivingNode = common.ReceivingNode,
                BytesTransferred = bytesTransferred,
                Chunks = chunksCount,
                Report = Report
            };

            RepositoryHelper.DataTransferred.Add(dataRecord);
        }
    }

    //--------------------------------------------------------------
    //--
    //-- unit tests
    //--
    //--------------------------------------------------------------

    namespace WarehouseTest
    {
        [TestFixture]
        public class TestWarehouseOperations
        {
            [Test]
            public void ProcessFile()
            {
                string filename = "C:\\UsageReports\\WarehouseTest.xml";

                System.Data.DataSet ds = new System.Data.DataSet();
                ds.ReadXml(filename);

                Archives.ProcessReports(ds.GetXml());
            }
        }
    }
}
