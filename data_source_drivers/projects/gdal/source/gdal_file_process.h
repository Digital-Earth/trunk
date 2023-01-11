/******************************************************************************
gdal_file_process.h

begin      : 9/25/2007 2:31:35 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#ifndef GDAL_FILE_PROCESS_H
#define GDAL_FILE_PROCESS_H

// local includes
#include "gdal_xy_coverage.h"
#include "module_gdal.h"

// pyxlib includes
#include "pyxis/pipe/process.h"

/*!
Provides access to a GDAL coverage.
*/
class MODULE_GDAL_DECL GDALFileProcess : public ProcessImpl<GDALFileProcess>
{
	PYXCOM_DECLARE_CLASS();

public:
	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IProcess)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IFeature

	IFEATURE_IMPL();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return m_spXYCoverage;
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return m_spXYCoverage;
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

private:

	//! The GDAL XY Coverage.
	boost::intrusive_ptr<GDALXYCoverage> m_spXYCoverage;

	PYXValue m_forcedNullValue;

	std::string m_layerName;
	std::string m_selectedBands;
};

#endif
