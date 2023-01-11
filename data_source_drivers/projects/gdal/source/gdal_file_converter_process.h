/******************************************************************************
gdal_file_converter_process.h

begin      : 1/4/2008 2:44:00 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#ifndef GDAL_FILECONVERTER_PROCESS_H
#define GDAL_FILECONVERTER_PROCESS_H

// local includes
#include "module_gdal.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/procs/data_processor.h"
#include "pyxis/utility/rect_2d.h"

// boost includes
#include <boost/filesystem/path.hpp>

// gdal includes
#include "gdal_priv.h"

/*
Converts the input Process' coverage to JPG format using the GDAL library. The conversion
is executed through the IDataProcessor interface (processData).
*/
class MODULE_GDAL_DECL GDALFileConverterProcess : 
	public ProcessImpl<GDALFileConverterProcess>, 
	public IDataProcessor
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IDataProcessor)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IDataProcessor*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IDataProcessor*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

public: // IDataProcessor

	//! Convert the coverage according to the settings
	virtual void STDMETHODCALLTYPE processData();

private:

	//! Converts the coverage and writes to file.
	bool writeToFile(GDALDataset* pDS);

	//! Create the pgd file needed by the PYXISGDAL driver.
	boost::filesystem::path createPGDFile(PYXRect2DDouble pRect1, int nXSize, int nYSize);	

	//! The path to save the converted data to.
	std::string m_strSavePath;
};

#endif