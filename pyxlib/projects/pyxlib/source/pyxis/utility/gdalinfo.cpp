/******************************************************************************
gdalinfo.cpp

begin		: 2009-07-29
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"

#include "pyxis/utility/gdalinfo.h"
#include "pyxis/utility/trace.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"

//--
//-- windows includes
//--
#pragma warning(push)
#pragma warning(disable: 4005) // warning C4005: 'S_OK' : macro redefinition
#define _WIN32_WINNT 0x0502 // to get SetDllDirectory (requires Windows XP SP1 or greater)
#include <windows.h>
#pragma warning(pop)

//--
//-- boost includes
//--
#include <boost/filesystem/operations.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/regex.hpp>

// NOTE: Some Windows APIs require UNICODE strings. We assume
// all our strings are 8 bit clean and simply convert between
// string and wstring as needed.
#ifdef UNICODE
	typedef std::wstring tstring;
#else
	typedef std::string tstring;
#endif
	
//--
//-- support for calling the GdalInfo DLL
//-- 
#include "gdalinfodll.h"

//---------------------------------------------------------------------
//--
//-- GdalInfo class is used to proxy calls from the managed c-sharp side
//-- to the GdalInfo DLL that lives in the plugin directory.  
//--
//-- This object gets SWIG'd so that it iterface is exposed to the
//-- the c-sharp code. 
//--
//---------------------------------------------------------------------

//--
//-- initialize object with explicit loading of GdalInfo DLL.
//--
GdalInfo::GdalInfo()
{
    TRACE_DEBUG("GdalInfo::GdalInfo()");	

	m_pGdalInfo = NULL;
	m_hGdalDll = NULL;

	boost::filesystem::path plugins(AppServices::getApplicationPath() / "plugins");
	auto pluginsStr = FileUtils::pathToString(plugins);
  	tstring strPlugins(pluginsStr.begin(), pluginsStr.end());
	if (SetDllDirectory(strPlugins.c_str()) == 0)
	{
		TRACE_ERROR("SetDllDirectory failed.");
	}

	std::string gdalDllFileName = "gdalinfo.dll";
	tstring dllFileName(gdalDllFileName.begin(),gdalDllFileName.end());
	m_hGdalDll = LoadLibrary(dllFileName.c_str()); 

    if( m_hGdalDll )
	{
		FARPROC lpfnGetProcessID = GetProcAddress(m_hGdalDll,"GetGdalInfo"); 
		if( lpfnGetProcessID )
		{
			m_pGdalInfo = (IGdalInfo*)(*lpfnGetProcessID)();
			if( m_pGdalInfo )
				m_pGdalInfo->Init();
		}
	}
}

//--
//-- free up and shutdown
//--
GdalInfo::~GdalInfo()
{
    TRACE_DEBUG("GdalInfo::~GdalInfo()");	

	if( m_pGdalInfo )
	{
		m_pGdalInfo->ShutDown();
	}

	if( m_hGdalDll )
	{
		FreeLibrary(m_hGdalDll);
	}

	m_pGdalInfo = NULL;
	m_hGdalDll = NULL;
}

//--
//-- using oracle login credentials, retrieve list of all available raster datasources,
//-- returned list contains nodes that have been created on the unmanaged side of the SWIG
//--
void GdalInfo::GetRasterDataSources(
	std::vector<GdalInfoDataSourceNode*> & rasterDataSources,
	std::string username, std::string password, std::string sid)
{
	if (m_pGdalInfo)
	{
		std::vector<GdalSubSetNode*> a = m_pGdalInfo->getAllOracleRasterDataSets(username,password,sid);

		std::vector<GdalSubSetNode*>::iterator iter;
		for(iter = a.begin(); iter != a.end(); ++iter)
		{
			std::string n = (*iter)->m_sName;
			std::string d = (*iter)->m_sDesc;

			GdalInfoDataSourceNode* node = new GdalInfoDataSourceNode();
			node->m_sName = n;
			node->m_sDesc = d;

			rasterDataSources.push_back(node);
		}	

		m_pGdalInfo->releaseSubDataSetsNodes(a);
	}
}

//--
//-- release nodes created on this side of the SWIG, by previous function
//--
void GdalInfo::ReleaseRasterDataSourceNodes(
	std::vector<GdalInfoDataSourceNode*> & nodeList)
{
	std::vector<GdalInfoDataSourceNode*>::iterator iter;
	for (iter = nodeList.begin(); iter != nodeList.end(); ++iter)
	{
		GdalInfoDataSourceNode* node = *iter;
		delete node;
	}
	nodeList.clear();
}

//--
//-- gerneate vrt file from datasource connection string.
//--
void GdalInfo::GenerateVrt( std::string filename, std::string connectString )
{
	if( m_pGdalInfo )
	{
		m_pGdalInfo->buildVrt(filename,connectString);
	}
}

//--
//-- static function to determine if plugin is available,
//-- and if the driver supports oracle
//--
bool GdalInfo::OracleSupportAvailable()
{
	//--
	//-- cache result in local static, 
	//-- only do actual call once.
	//--
	enum OralceStatus { undefined, yes, no };
	static OralceStatus oracleAvailability = undefined;

	if( oracleAvailability == undefined )
	{
		bool result = false;

		boost::filesystem::path plugins(AppServices::getApplicationPath() / "plugins");
		std::string pluginsPath= FileUtils::pathToString(plugins);
  		tstring strPlugins(pluginsPath.begin(),pluginsPath.end());
		if (SetDllDirectory(strPlugins.c_str()) == 0)
		{
			TRACE_ERROR("SetDllDirectory failed.");
		}

		std::string gdalDllFileName = "gdalinfo.dll";
		tstring dllFileName(gdalDllFileName.begin(),gdalDllFileName.end());
		HINSTANCE hDLL = LoadLibrary(dllFileName.c_str()); 

		if( hDLL )
		{
			FARPROC lpfnGetProcessID = GetProcAddress(hDLL,"GetGdalInfo"); 
			IGdalInfo *pGdalInfo = (IGdalInfo*)(*lpfnGetProcessID)();

			if( pGdalInfo )
			{
				pGdalInfo->Init();
				result = pGdalInfo->oracleSupported();
				pGdalInfo->ShutDown();
			}

			FreeLibrary(hDLL);
		}

		oracleAvailability = result ? yes : no;
	}

	return oracleAvailability == yes;
}



