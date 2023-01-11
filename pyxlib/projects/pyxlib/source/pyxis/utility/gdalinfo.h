/******************************************************************************
gdalinfo.h

begin		: 2009-07-29
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#ifndef GDAL_INFO_H
#define GDAL_INFO_H

//--
//-- pyxlib includes
//--
#include "pyxlib.h"
#include <string>
#include <vector>

//--
//-- windows includes
//--
#pragma warning(push)
#pragma warning(disable: 4005) // warning C4005: 'S_OK' : macro redefinition
#define _WIN32_WINNT 0x0502 // to get SetDllDirectory (requires Windows XP SP1 or greater)
#include <windows.h>
#pragma warning(pop)


struct IGdalInfo;

//--
//-- class used to tranfer specific datasource data from the unmanaged c-plus side
//-- to the managed c-sharpe side.
//--
class PYXLIB_DECL GdalInfoDataSourceNode
{
private:
	std::string m_sName;
	std::string m_sDesc;

public:
	std::string Name() { return m_sName; }
	std::string Desc() { return m_sDesc; }

	friend class GdalInfo;
};

//--
//-- class used to proxy calls from the managed c-sharp side
//-- to the GdalInfo DLL  
//--
class PYXLIB_DECL GdalInfo 
{
public:
	GdalInfo();
	~GdalInfo();

	static bool OracleSupportAvailable();

	void GetRasterDataSources(
		std::vector<GdalInfoDataSourceNode*> & rasterDataSources,
		std::string username, std::string password, std::string sid);
	void ReleaseRasterDataSourceNodes(
		std::vector<GdalInfoDataSourceNode*> & nodeList);

	void GenerateVrt(
		std::string filename, std::string connectString);

private:
	IGdalInfo* m_pGdalInfo;
	HINSTANCE  m_hGdalDll;
};

#endif 
