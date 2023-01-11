/******************************************************************************
gdalinfodll.h

support for calling functions inside this GDALINFO.DLL

begin		: 2009-07-29
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#ifndef GDAL_INFO_DLL_H
#define GDAL_INFO_DLL_H

//--
//-- structure used to tranfer data out of this DLL.
//--
struct GdalSubSetNode
{
public:
	std::string m_sName;
	std::string m_sDesc;
};

//--
//-- functional interface to DLL.
//--
//-- all exportable functions are defined here as pure virtual.
//-- objects derived from this class, will have these functions/methods
//-- exposed externally to the DLL.
//--
struct IGdalInfo
{
public:
	virtual void STDMETHODCALLTYPE Init( void ) const = 0;
	virtual void STDMETHODCALLTYPE ShutDown( void ) const = 0;

	virtual std::string STDMETHODCALLTYPE VersionName( void ) const = 0;
	virtual std::string STDMETHODCALLTYPE VersionNumber( void ) const = 0;

	virtual bool STDMETHODCALLTYPE driverSupported( std::string ) const = 0;
	virtual bool STDMETHODCALLTYPE oracleSupported( void ) const = 0;

	virtual std::vector<std::string> STDMETHODCALLTYPE getSubDataSetsRaw(std::string arg) const = 0;
	virtual std::vector<GdalSubSetNode*> STDMETHODCALLTYPE getSubDataSetsNodes(std::string arg) const = 0;
 	virtual std::vector<GdalSubSetNode*> STDMETHODCALLTYPE getAllOracleRasterDataSets( std::string user, std::string password, std::string sid ) const = 0;

	virtual void STDMETHODCALLTYPE releaseSubDataSetsNodes( std::vector<GdalSubSetNode*> nodeList ) const = 0;
	virtual void STDMETHODCALLTYPE buildVrt( std::string filename, std::string connectString ) const = 0;
};

#endif 