#ifndef GDAL_MULTI_DRIVER_H
#define GDAL_MULTI_DRIVER_H
/******************************************************************************
gdal_multi_driver.h

begin		: 2005-04-01
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_gdal.h"

// pyxlib includes
#include "pyxis/data/data_driver.h"

// standard includes
#include <map>

//! Driver that accesses the GDAL library to read data files.
/*!
GDALMultiDriver uses methods in the GDAL library to read data files that can be
opened by the library.
*/
class GDALMultiDriver : public IDataDriver
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

	//! Constructor.
	GDALMultiDriver() {;}

	//! Destructor
	virtual ~GDALMultiDriver() {;}

public: //IDataDriver

	//! Get the name of the driver.
	virtual std::string STDMETHODCALLTYPE getName() const;

	//! Open a data source.
	virtual PYXPointer<PYXDataSource> STDMETHODCALLTYPE openForRead(const std::string& strDataSourceName) const;

	bool STDMETHODCALLTYPE getFileInfo(const std::string& strURI, IDataDriver::FileInfo* pFileInfo, bool* pbExit);

public: //IUnknown

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppObject)
	{
		// See Essential COM p65
		assert(ppObject != 0);

		if (riid == IDataDriver::iid || riid == IUnknown::iid)
		{
			*ppObject = static_cast<IDataDriver*>(this);
		}
		else
		{
			*ppObject = 0;
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}

	
	RC_IMPL();

private:

	//! Disable copy constructor
	GDALMultiDriver(const GDALMultiDriver&);

	//! Disable copy assignment
	void operator=(const GDALMultiDriver&);

	//! Open the files located in the directory specified.  
	int openFiles( const std::string& strDir,
		 		   IDataDriver::FileInfo* pFileInfo, 
				   const std::string& strFileExt,
				   bool* pbExit);
	
	bool openForCheckInfo(	const std::string& strDir,
							IDataDriver::FileInfo* pFileInfo,
							const std::string& strFileExt,
							bool* pbExit);

	//! Read directory structure and record all file endings and count of each type.
	void getFileEndings(const std::string strDirectory, std::map<std::string, int>& endingMap, bool* pbExit) const;
};

#endif	// GDAL_MULTI_DRIVER
