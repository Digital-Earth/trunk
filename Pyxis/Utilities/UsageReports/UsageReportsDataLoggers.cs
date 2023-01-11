/******************************************************************************
UsageReportsLogData.cs

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
    /// This file contains only the routines called by PyxNet for logging the data.
    /// 
    /// </summary>
    public partial class UsageReports
    {
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
            DatabaseObject.QueryReceived dbObj
                = new DatabaseObject.QueryReceived( sendingNodeName,
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
            //--
            //-- validate that the data source id is one that we have published.
            //--
            UsageReportsHelper.DataSourceInfo dsInfo = Helper.GetDataSourceInfo(dataSetId);
            if (dsInfo != null)
            {
                DatabaseObject.QueryMatched dbObj
                    = new DatabaseObject.QueryMatched(sendingNodeName,
                                                       sendingNodeGuid,
                                                       receivingNodeName,
                                                       receivingNodeGuid,
                                                       dataSetId
                                                       );
                Insert(dbObj);
            }
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
        public static void CoverageBytesUploaded(string sendingNodeName,
                                                  Guid sendingNodeGuid,
                                                  string receivingNodeName,
                                                  Guid receivingNodeGuid,
                                                  Guid dataSetId,
                                                  long dataSize
                                                  )
        {
            UsageReports.DatabaseObject.DataTransfer dbObj
                = new UsageReports.DatabaseObject.DataTransfer(sendingNodeName,
                                                               sendingNodeGuid,
                                                               receivingNodeName,
                                                               receivingNodeGuid,
                                                               dataSetId,
                                                               dataSize
                                                               );               
            Insert(dbObj);
        }

        /// <summary>
        /// Logs the number of bytes downloaded by a coverage pipeline.
        /// Called as every chunk is received.
        /// </summary>
        /// <param name="sendingNodeName">Name of the sending node.</param>
        /// <param name="sendingNodeGuid">The sending node GUID.</param>
        /// <param name="receivingNodeName">Name of the receiving node.</param>
        /// <param name="receivingNodeGuid">The receiving node GUID.</param>
        /// <param name="dataSetId">The dataset ID.</param>
        /// <param name="dataSize">Size of the sent chunk.</param>
        public static void CoverageBytesDownloaded(string sendingNodeName,
                                                   Guid sendingNodeGuid,
                                                   string receivingNodeName,
                                                   Guid receivingNodeGuid,
                                                   Guid dataSetId,
                                                   long dataSize
                                                  )
        {
            UsageReports.DatabaseObject.DataDownload dbObj
                = new UsageReports.DatabaseObject.DataDownload(sendingNodeName,
                                                               sendingNodeGuid,
                                                               receivingNodeName,
                                                               receivingNodeGuid,
                                                               dataSetId,
                                                               dataSize
                                                               );
            Insert(dbObj);
        }
    }
}