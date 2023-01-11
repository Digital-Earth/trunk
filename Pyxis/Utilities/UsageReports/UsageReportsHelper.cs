/******************************************************************************
UsageReportsHelper.cs

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
    /// The UsageReports Helper class is used by the UsageReports utility to 
    /// make calls to PyxNet and external tracing devices with out having specific
    /// information about their implementation details.
    /// 
    /// The details are hidden by the classes virtual functions that need to be 
    /// defined by the encompassing application.  Applications generating usage
    /// reports will need to instantiate a derivative helper class object, and
    /// pass the object to the UsageReports utility with it is initialized.
    /// </summary>
    public class UsageReportsHelper
    {
        //------------------------------------------------------------------------
        //--
        //-- VIRTUAL FUNCTIONS
        //--
        //-- The following functions should be defined by the application using
        //-- the UsageReports utility.  Ultimately, the utility does not understand
        //-- anything about PyxNet, so it must use this helper class to call out
        //-- and have someone else perform PyxNet operations on its behalf.
        //--
        //------------------------------------------------------------------------

        /// <summary>
        /// Finds the license server for a data source.
        /// </summary>
        /// <param name="dataSetId">Data Source ID.</param>
        /// <returns></returns>
        public virtual Guid FindLicenseServer(Guid dataSetId)
        {
            #warning FIXME: FindLicenseServer is hardcoded to return a private GUID.
            Guid licenseServerId = new Guid("B59EEA07-D6AE-4402-BCBE-6E6DEC155A08");
            return licenseServerId;
        }

        /// <summary>
        /// Find Guid for local host.
        /// </summary>
        /// <returns>Guid for local host.</returns>
        public virtual Guid FindLocalHost()
        {
            Guid localHost = new Guid();
            return localHost;
        }

        /// <summary>
        /// Finds the friendly name of a node.  
        /// </summary>
        /// <param name="nodeId">Node ID.</param>
        /// <returns>Friendly name for node ID</returns>
        public virtual string FindNodeFriendlyName(Guid nodeId)
        {
            return "";
        }

        /// <summary>
        /// Send report.
        /// </summary>
        /// <param name="dataSet">Report data.</param>
        /// <param name="toNodeId">Destination node ID.</param>
        /// <param name="fromNodeId">Source node ID.</param>
        public virtual void SendReport(DataSet dataSet, Guid toNodeId, Guid fromNodeId)
        {
        }

        /// <summary>
        /// Route log messages to application trace log.
        /// </summary>
        /// <param name="level">Message severity level.</param>
        /// <param name="message">The message.</param>
        public virtual void Logger(Pyxis.Utilities.LogHelper.LogLevel level, string message)
        {
        }

        public class DataSourceInfo
        {
            public Guid Id { get; set; }
            public string Name { get; set; }
            public string Desc { get; set; }
        }

        public virtual DataSourceInfo GetDataSourceInfo(Guid dataSourceId)
        {
            return null;
        }

        //------------------------------------------------------------------------
        //--
        //-- Encapsulated logging routines.
        //--
        //------------------------------------------------------------------------

        private void HelperLoggerCallback(Pyxis.Utilities.LogHelper.LogLevel serverId, string msg)
        {
            Logger(serverId,msg);
        }


        public LogHelper Log = null;


        public UsageReportsHelper()
        {
            Log = new LogHelper(HelperLoggerCallback);
        }
    }
}


