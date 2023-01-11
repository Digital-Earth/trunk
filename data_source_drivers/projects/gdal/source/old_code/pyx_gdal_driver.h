#ifndef PYX_GDAL_DRIVER_H
#define PYX_GDAL_DRIVER_H
/******************************************************************************
pyx_gdal_driver.h

begin		: 2005-04-01
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_gdal.h"

// pyxlib includes
#include "pyxis/data/data_driver.h"

//! Driver that accesses the GDAL library to read data files.
/*!
PYXGDALDriver uses methods in the GDAL library to read data files that can be
opened by the library. This class is named with the "PYX" prefix to avoid a
name conflict with GDALDriver which is a class in the GDAL libraries.
*/
class PYXGDALDriver : public IDataDriver
{

	PYXCOM_DECLARE_CLASS();

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

public: //IDataDriver

	//! Get the name of the driver.
	virtual  std::string STDMETHODCALLTYPE getName() const;

	//! Open a data source.
	virtual PYXPointer<PYXDataSource> STDMETHODCALLTYPE openForRead(const std::string& strDataSourceName) const;

	//! Fill supplied FileInfo structure with information about the specific file.
	bool STDMETHODCALLTYPE getFileInfo(const std::string& strURI, IDataDriver::FileInfo* pFileInfo, bool * pbExit);

public:

	//! Test method
	static void test();

	//! Constructor.
	PYXGDALDriver();

	//! Destructor
	virtual ~PYXGDALDriver();

	//! Register all drivers in GDAL.
	static void registerAllDrivers();

	//! Remove all drivers registered in GDAL.
	static void destroyDriverManager();

protected:

private:

	//! Disable copy constructor
	PYXGDALDriver(const PYXGDALDriver&);

	//! Disable copy assignment
	void operator=(const PYXGDALDriver&);
};

#endif	// GDAL_DRIVER
