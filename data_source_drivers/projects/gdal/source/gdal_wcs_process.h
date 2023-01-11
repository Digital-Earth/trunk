/******************************************************************************
gdal_wcs_process.h

begin      : 10/4/2007 10:34:05 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#ifndef GDAL_WCS_PROCESS_H
#define GDAL_WCS_PROCESS_H

// local includes
#include "coord_converter_impl.h"
#include "gdal_metadata.h"
#include "gdal_xy_coverage.h"
#include "module_gdal.h"
#include "ows_reference.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"

// GDAL includes
#include "ogr_spatialref.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/recursive_mutex.hpp>

// local forward declarations
class ProjectionMethod;
class PYXGeometry;

/*!
Provides access to WCS data through the GDAL library.
*/
class MODULE_GDAL_DECL GDALWCSProcess : public ProcessImpl<GDALWCSProcess>, public IXYCoverage, public IOWSReference
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)
		IUNKNOWN_QI_CASE(IOWSReference)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( GDALWCSProcess, IProcess);

public: // IRecord
	IRECORD_IMPL_PROXY(*m_spXYCoverage);

public: // IFeature

	IFEATURE_IMPL_PROXY(*m_spXYCoverage);

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL_PROXY(*m_spXYCoverage);

public: //IXYCoverage

	IXYCOVERAGE_IMPL_PROXY(*m_spXYCoverage);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IXYCoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IXYCoverage*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IOWSReference
	virtual PYXPointer<OWSFormat> getDefaultOutputFormat() const;

	virtual bool supportOutputType(const OWSFormat & format) const;

	virtual std::string getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const;


private:

	//! The GDAL XY Coverage.
	boost::intrusive_ptr<GDALXYCoverage> m_spXYCoverage;

	//! The name of the server serving the WCS datasource.
	std::string m_strServer;

	//! The version of the server serving the WCS datasource.
	std::string m_strVersion;

	//! The name of the layer (datasource) being retrieved from the server.
	std::string m_strLayer;

	std::string m_strListOfBands;
};

#endif