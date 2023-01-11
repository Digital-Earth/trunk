/******************************************************************************
CommonRowData.cs

begin      : October 9, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using NHibernate;
using NHibernate.Cfg;

using Pyxis.Utilities.ReportsWarehouse;
using Pyxis.Utilities.ReportsWarehouse.Domain;

namespace Pyxis.Utilities.ReportsWarehouse
{
    /// <summary>
    /// Extarct common data that is most rows of data passed to the reports warehose.
    /// That information usually includes, but is not limitted to node and data set 
    /// information, as well as timestamps.
    /// </summary>
    class CommonRowData
    {
        public System.DateTime TimeStamp
        { get; set; }

        public DataSet DataSet
        { get; set; }

        public Node SendingNode
        { get; set; }

        public Node ReceivingNode
        { get; set; }

        /// <summary>
        /// This class maintains a list of all nodes and data sets.
        /// As long as all node and data set references are vetted through
        /// this module, then normallization of these objects will be done
        /// correctly.
        /// </summary>
        static Dictionary<Guid, Node> m_nodeList = new Dictionary<Guid, Node>();
        static Dictionary<Guid, DataSet> m_dataSetList = new Dictionary<Guid, DataSet>();

        /// <summary>
        /// Cleans static structures up, in particular the node and data set lists.
        /// </summary>
        static public void CleanUp()
        {
            m_nodeList.Clear();
            m_dataSetList.Clear();
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="CommonRowData"/> class.
        /// </summary>
        /// <param name="cols">Column descriptions for the current data table.</param>
        /// <param name="row">Row of data from current table.</param>
        public CommonRowData(System.Data.DataColumnCollection cols, System.Data.DataRow row)
        {  
            //--
            //-- extract data from row
            //--
            DataSet = GetDataSet(cols, row, "DataSetId");
            SendingNode = GetNode(cols, row, "SendingNodeGuid", "SendingNodeName");
            ReceivingNode = GetNode(cols, row, "ReceivingNodeGuid", "ReceivingNodeName");

            //--
            //-- extract timestamp
            //--
            TimeStamp = new System.DateTime();
            if (cols.Contains("TimeStamp"))
            {
                TimeStamp = System.DateTime.Parse(row["TimeStamp"].ToString());
            }
        }

        /// <summary>
        /// Extract node data from row data.
        /// </summary>
        /// <param name="cols">Column header information.</param>
        /// <param name="row">Row data</param>
        /// <param name="guidColumnName">Node Guid column name.</param>
        /// <param name="nameColumnName">Node name column name.</param>
        /// <returns></returns>
        private Node GetNode(System.Data.DataColumnCollection cols, System.Data.DataRow row, string guidColumnName, string nameColumnName)
        {
            Node node = null;

            try
            {
                if (cols.Contains(guidColumnName))
                {
                    //--
                    //-- extract data from row
                    //--
                    Guid nodeGuid = new Guid(row[guidColumnName].ToString());
                    string nodeName = null;

                    if (cols.Contains(nameColumnName))
                    {
                        nodeName = row[nameColumnName].ToString();
                    }

                    //--
                    //-- vet against database and static list
                    //--
                    node = GetNode(nodeGuid, nodeName);
                }
            }
            catch (Exception ex)
            {
                Archives.Log.Error("UsageReports:Warehouse:GetNode: " + ex.Message);
            }

            return node;
        }

        /// <summary>
        /// Gets the node, either from static list or database.
        /// Insert into database if necessary.
        /// </summary>
        /// <param name="nodeGuid">Node Guid.</param>
        /// <param name="nodeName">Node Name.</param>
        /// <returns></returns>
        private Node GetNode(Guid nodeGuid, string nodeName)
        {
            Node node = null;

            try
            {
                //-- 
                //-- check list
                //--
                if (m_nodeList.ContainsKey(nodeGuid))
                {
                    node = m_nodeList[nodeGuid];
                }
                else
                {
                    //-- 
                    //-- access database
                    //--
                    node = RepositoryHelper.Node.GetByGuid(nodeGuid);
                    if (node == null)
                    {
                        //--
                        //-- add to database
                        //--
                        node = new Node()
                        {
                            Ident = nodeGuid,
                            Name = nodeName
                        };
                        RepositoryHelper.Node.Add(node);
                    }
                    //--
                    //-- add to static list
                    //--
                    m_nodeList.Add(nodeGuid, node);
                }

                //--
                //-- update name if different
                //--
                if (string.IsNullOrEmpty(nodeName) == false && node.Name != nodeName)
                {
                    node.Name = nodeName;
                }
            }
            catch (Exception ex)
            {
                Archives.Log.Error("UsageReports:Warehouse:GetNode: " + ex.Message);
            }
         
            return node;
        }

        /// <summary>
        /// Extract data set data from row data.
        /// </summary>
        /// <param name="cols">Column header information.</param>
        /// <param name="row">Row data.</param>
        /// <param name="guidColumnName">Data set Guid column name.</param>
        /// <returns></returns>
        private DataSet GetDataSet(System.Data.DataColumnCollection cols, System.Data.DataRow row, string guidColumnName)
        {
            DataSet dataSet = null;

            try
            {
                if (cols.Contains(guidColumnName))
                {
                    //--
                    //-- extract data from row, and vet against database
                    //--
                    Guid dataSetGuid = new Guid(row[guidColumnName].ToString());
                    dataSet = GetDataSet(dataSetGuid);
                }
            }
            catch (Exception ex)
            {
                Archives.Log.Error("UsageReports:Warehouse:GetDataSet: " + ex.Message);
            }

            return dataSet;
        }


        /// <summary>
        /// Gets the data set, either from static list or database.
        /// Insert into database if necessary.
        /// </summary>
        /// <param name="dataSetGuid">Data set Guid.</param>
        /// <returns></returns>
        private DataSet GetDataSet(Guid dataSetGuid)
        {
            DataSet dataSet = null;

            try
            {
                //-- 
                //-- check list
                //--
                if (m_dataSetList.ContainsKey(dataSetGuid))
                {
                    dataSet = m_dataSetList[dataSetGuid];
                }
                else
                {
                    //-- 
                    //-- access database
                    //--
                    dataSet = RepositoryHelper.DataSet.GetByGuid(dataSetGuid);
                    if (dataSet == null)
                    {
                        //--
                        //-- add to database
                        //--
                        dataSet = new DataSet()
                        {
                            Ident = dataSetGuid
                        };
                        RepositoryHelper.DataSet.Add(dataSet);
                    }
                    //--
                    //-- add to static list
                    //--
                    m_dataSetList.Add(dataSetGuid, dataSet);
                }
            }
            catch (Exception ex)
            {
                Archives.Log.Error("UsageReports:Warehouse:GetDataSet: " + ex.Message);
            }
                
            return dataSet;
        }
    }

}
