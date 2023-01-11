/******************************************************************************
module_gdal.cpp

begin		: 2007-02-19
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "module_gdal.h"

// local includes
#include "gdal_file_process.h"
#include "gdal_file_converter_process.h"
#include "gdal_wcs_process.h"
#include "gdal_wcs_process_v2.h"
#include "gdal_wms_process.h"
#include "gdal_xy_coverage.h"
#include "ogr_feature_server_process.h"
#include "ogr_process.h"
#include "ogr_wfs_process.h"
#include "gdal_pipe_builder.h"
#include "ogr_pipe_builder.h"
#include "search_result_proc.h"
#include "wps_process.h"
#include "ows_network_resource.h"
#include "gdal_bing_process.h"
#include "ows_context_formatter.h"
//#include "gdal_multi_process.h"


// pyxlib includes
#include "pyxis/utility/mem_utils.h"

// ogr includes
#include "ogrsf_frmts.h"

// gdal includes
#include "gdal.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE	
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALFileProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALFileConverterProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALWCSProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALWCSProcessV2),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALWMSProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALXYCoverage),
//	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALMultiProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(OGRProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(OGRWFSProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALPipeBuilder),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(OgrPipeBuilder),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(SearchResultProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(WPSProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(OwsCoverageNetworkResourceProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(OwsVectorNetworkResourceProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(GDALBingProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(OwsContextFormatter),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(OGRFeatureServerProcess),
PYXCOM_END_CLASS_OBJECT_TABLE

namespace
{

//! Static object to manage module.
struct ModuleManager
{

	ModuleManager()
	{
		// Set GDAL environment variables.
		CPLSetConfigOption("GDAL_DATA", "plugins\\gdaldata");
		CPLSetConfigOption("GDAL_DRIVER_PATH", "plugins");

		// Register all drivers.
		GDALAllRegister();
	}

	~ModuleManager()
	{
		//--
		//-- If gdal's oracle driver (aka: "georaster") gets used, 
		//-- the clean up code for the driver manager starts throwing
		//-- lots of exceptions.  Deregistering the oracle driver 
		//-- before calling driver manager's destroy routine eliminates
		//-- the noise.  This is a work around, not a fix.
		//--
		//-- --nle-- 2009-AUG-10
		//-- 
		GDALDriverH hOracleDriver = GDALGetDriverByName("georaster");
		if( hOracleDriver )
		{
			GDALDeregisterDriver(hOracleDriver);
		}
		GDALDestroyDriverManager();
	}

} moduleManager;

}
