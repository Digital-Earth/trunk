/******************************************************************************
OracleConnectData.cs

project    : Oracle Pipeline Wizard

begin      : July 21, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

using OSGeo;
using OSGeo.GDAL;

namespace OraclePipelineWizard
{
    /// <summary>
    /// Class for managing oracle connection data that accesses the raster
    /// files on an oracle database.  All communications with oracle are done
    /// through gdal.
    /// </summary>
    public class OracleConnectData
    {
        //-------------------------------------------------
        //--
        //-- oracle connection data
        //--
        //-------------------------------------------------

        private string m_userName;
        private string m_password;
        private string m_sid;

        /// <summary>
        /// oracle user name
        /// </summary>
        public string UserName
        {
            set { m_userName = value; }
            get { return m_userName; }
        }

        /// <summary>
        /// oracle password
        /// </summary>
        public string Password
        {
            set { m_password = value; }
            get { return m_password; }
        }

        /// <summary>
        /// oracle connection name
        /// </summary>
        public string SID
        {
            set { m_sid = value; }
            get { return m_sid; }
        }

        //-------------------------------------------------
        //--
        //-- selected data information
        //--
        //-------------------------------------------------

        private DatabaseNodeInfo m_spatialTable;
        private DatabaseNodeInfo m_rasterColumn;
        private DatabaseNodeInfo m_rasterDataTable;

        /// <summary>
        /// selected spatial table
        /// </summary>
        public DatabaseNodeInfo SpatialTable
        {
            set { m_spatialTable = value; }
            get { return m_spatialTable; }
        }

        /// <summary>
        /// selected raster column name
        /// </summary>
        public DatabaseNodeInfo RasterColumnName
        {
            set { m_rasterColumn = value; }
            get { return m_rasterColumn; }
        }

        /// <summary>
        /// selected raster data table
        /// </summary>
        public DatabaseNodeInfo RasterDataTable
        {
            set { m_rasterDataTable = value; }
            get { return m_rasterDataTable; }
        }

        //-------------------------------------------------
        //--
        //-- retrieve tables, columns, and raster data tables
        //--
        //-------------------------------------------------

        /// <summary>
        /// list all spatial tables in oracle
        /// </summary>
        /// <returns>list of normal oracle tables</returns>
        public List<DatabaseNodeInfo> FindTables()
        {
            string connectString = "geor:" + UserName + "," + Password + "," + SID;
            Console.WriteLine("FindTables connectString: " + connectString);
            return GDalInfoDataSets(connectString);
        }

        /// <summary>
        /// list all georaster columns in selected spatial table 
        /// </summary>
        /// <returns>list of oracle columns</returns>
        public List<DatabaseNodeInfo> FindColumns()
        {
            return FindColumns(SpatialTable);
        }

        /// <summary>
        /// list all georaster columns in an oracle spatail table
        /// </summary>
        /// <param name="ds">oracle spatial table node</param>
        /// <returns>list of oracle columns</returns>
        public List<DatabaseNodeInfo> FindColumns(DatabaseNodeInfo ds)
        {
            if (ds == null)
            {
                return null;
            }
            Console.WriteLine("FindColumns connectString: " + ds.Name);
            return GDalInfoDataSets(ds.Name);
        }

        /// <summary>
        /// list all raster data tables in selected spatial table column
        /// </summary>
        /// <returns>list of raster data tables</returns>
        public List<DatabaseNodeInfo> FindRasterData()
        {
            return FindRasterData(RasterColumnName);
        }

        /// <summary>
        /// list all raster data tables is spatial table column
        /// </summary>
        /// <param name="ds">spatial table column</param>
        /// <returns>list of raster data tables</returns>
        public List<DatabaseNodeInfo> FindRasterData(DatabaseNodeInfo ds)
        {
            if (ds == null)
            {
                return null;
            }
            Console.WriteLine("FindRasterData connectString: " + ds.Name);
            return GDalInfoDataSets(ds.Name);
        }

        //-------------------------------------------------
        //--
        //-- generalized gdal infor query
        //--
        //-------------------------------------------------

        /// <summary>
        /// return list of subset from a basic gdal connect query
        /// </summary>
        /// <param name="connectString">gdal connect string</param>
        /// <returns>list of subset</returns>
        public List<DatabaseNodeInfo> GDalInfoDataSets(string connectString)
        {
            List<DatabaseNodeInfo> list = new List<DatabaseNodeInfo>();

            try
            {
                //--
                //-- register drivers         
                //--
                Gdal.AllRegister();

                //--
                //-- open dataset
                //--
                Dataset ds = Gdal.Open(connectString, Access.GA_ReadOnly);

                if (ds == null)
                {
                    Console.WriteLine("Can't open: " + connectString);
                    throw new Exception("Can't open: " + connectString);
                }

                Console.WriteLine("Open: " + connectString);

                //--
                //-- find and traverse all sub data sets
                //--
                string[] metadata = ds.GetMetadata("");

                metadata = ds.GetMetadata("SUBDATASETS");
                if (metadata.Length > 0)
                {
                    string name = null;
                    string desc = null;

                    //--
                    //-- extract name and description of data sub-sets from meta data
                    //--
                    Console.WriteLine("  Subdatasets:");
                    for (int iMeta = 0; iMeta < metadata.Length; iMeta++)
                    {
                        Console.WriteLine("    " + iMeta + ":  " + metadata[iMeta]);

                        String[] mainSections = metadata[iMeta].Split('=');
                        string[] attributeSections = mainSections[0].Split('_');

                        if (attributeSections[2] == "NAME")
                        {
                            name = mainSections[1];
                        }

                        if (attributeSections[2] == "DESC")
                        {
                            desc = mainSections[1];

                            OracleConnectData.DatabaseNodeInfo subdataset = new OracleConnectData.DatabaseNodeInfo(name, desc);
                            list.Add(subdataset);
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                Console.WriteLine("Application error: " + ex.Message);
                MessageBox.Show("Failed: " + ex.Message.ToString(), Application.ProductName);
            }

            return list;
        }

        //-------------------------------------------------
        //--
        //-- Projection Data
        //--
        //-------------------------------------------------

        /// <summary>
        /// get projection data for selected raster data table
        /// </summary>
        /// <returns></returns>
        public string GetProjectionData()
        {
            return GetProjectionData(RasterDataTable);
        }

        /// <summary>
        /// get projection data for raster data table
        /// </summary>
        /// <param name="ds">raster data table</param>
        /// <returns>WKT record for projection data</returns>
        public string GetProjectionData(DatabaseNodeInfo ds)
        {
            if (ds == null)
            {
                return null;
            }
            Console.WriteLine("GetProjectionData connectString: " + ds.Name);
            return GetProjectionData(ds.Name);
        }

        /// <summary>
        /// get projection data from gdal for raster data set
        /// </summary>
        /// <param name="connectString">fully qualified gdal connect string for a raster data set</param>
        /// <returns>WKT record describing the projection</returns>
        public string GetProjectionData(string connectString)
        {
            string projcs = null;

            try
            {
                //--
                //-- register drivers         
                //--
                Gdal.AllRegister();

                //--
                //-- open dataset
                //--
                OSGeo.GDAL.Dataset ds = Gdal.Open(connectString, Access.GA_ReadOnly);

                if (ds == null)
                {
                    Console.WriteLine("Can't open: " + connectString);
                    throw new Exception("Can't open: " + connectString);
                }

                Console.WriteLine("Projection: " + ds.GetProjection());
                Console.WriteLine("ProjectionRef: " + ds.GetProjectionRef());

                //--
                //-- pull projection data from gdal data source.
                //-- usually a PROJCS or GEOGCS record
                //--
                projcs = ds.GetProjectionRef();
            }
            catch( Exception ex)
            {
                Console.WriteLine("Application error: " + ex.Message);
                MessageBox.Show("Failed: " + ex.Message.ToString(), Application.ProductName);
            }

            return projcs;
        }

        //-------------------------------------------------
        //--
        //-- Generate VRT
        //--
        //-------------------------------------------------

        /// <summary>
        /// generate vrt file for "RasterDataTable", use generated file to connect worldview.
        /// </summary>
        /// <param name="filename">output file (vrt)</param>
        public void GenerateVRT(string filename)
        {
            try
            {
                //--
                //-- register drivers         
                //--
                Gdal.AllRegister();

                //--
                //-- open destination driver 
                //--
                const string outputDriverName = "VRT";

                Driver vrtDriver = Gdal.GetDriverByName(outputDriverName);
                if (vrtDriver == null)
                {
                    Console.WriteLine("Output driver is not recognized: " + outputDriverName);
                    throw new Exception("Output driver is not recognized: " + outputDriverName);
                }

                //--
                //-- open source data set
                //--
                OSGeo.GDAL.Dataset sourceDataset = Gdal.OpenShared(RasterDataTable.Name, Access.GA_ReadOnly);
                if (sourceDataset == null)
                {
                    Console.WriteLine("Unable to open source data set: " + RasterDataTable.Name);
                    throw new Exception("Unable to open source data set: " + RasterDataTable.Name);
                }

                //--
                //-- copy data source to virtual data source.
                //--
                OSGeo.GDAL.Dataset destDataset = vrtDriver.CreateCopy(filename, sourceDataset, 1, null, null, null);
                if (destDataset == null)
                {
                    Console.WriteLine("Unable to generate virtual data table file: " + filename);
                    throw new Exception("Unable to generate virtual data table file: " + filename);
                }

                MessageBox.Show("VRT file created: " + filename, Application.ProductName);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Application error: " + ex.Message);
                MessageBox.Show("Failed: " + ex.Message.ToString(), Application.ProductName);
            }
            finally
            {
                Gdal.GDALDestroyDriverManager();
            }
        }

        //-------------------------------------------------
        //--
        //--
        //--
        //-------------------------------------------------
        
        /// <summary>
        /// Class encompassing set data return by gdal about oracle nodes.
        /// Each node contains "name" data, which is the gdal connect string to access the node,
        /// and a description field.
        /// </summary>
        public class DatabaseNodeInfo
        {
            private string m_name;
            private string m_desc;

            public DatabaseNodeInfo(string name, string desc)
            {
                Name = name;
                Desc = desc;
            }

            public string Name
            {
                set { m_name = value; }
                get { return m_name; }
            }

            public string Desc
            {
                set { m_desc = value; }
                get { return m_desc; }
            }

            public override string ToString()
            {
                return Desc;
            }
        }

    }
}
