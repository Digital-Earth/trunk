/******************************************************************************
gdal_wcs_process_v2.h

begin      : 10/4/2011 10:34:05 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#ifndef GDAL_WCS_PROCESS_V2_H
#define GDAL_WCS_PROCESS_V2_H

// local includes
#include "module_gdal.h"
#include "ows_reference.h"
#include "gdal_xy_coverage.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/data/coverage_base.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/recursive_mutex.hpp>

// local forward declarations
class ProjectionMethod;
class PYXGeometry;

/*!
Provides access to WCS data through the GDAL library.
*/
class MODULE_GDAL_DECL GDALWCSProcessV2 : public ProcessImpl<GDALWCSProcessV2>, public CoverageBase, public IOWSReference
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
		IUNKNOWN_QI_CASE(ICoverage)
		IUNKNOWN_QI_CASE(IOWSReference)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( GDALWCSProcessV2, IProcess);

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(		const PYXIcosIndex& index,
																			int nRes,
																			int nFieldIndex = 0	) const;

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex = 0	) const;

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

private:
	std::string buildGdalXml() const;

	virtual void createGeometry() const;

	const boost::intrusive_ptr<ICoverage> & getCoverageForResolution(int resolution) const;

public: //IOWSReference
	virtual PYXPointer<OWSFormat> getDefaultOutputFormat() const;

	virtual bool supportOutputType(const OWSFormat & format) const;

	virtual std::string getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const;

private:
	boost::intrusive_ptr<IProcess> createCoverageFromOverview(boost::intrusive_ptr<GDALXYCoverage> coverage, const std::string & samplingType);

	boost::intrusive_ptr<IProcess> addResolutionFilter(boost::intrusive_ptr<IProcess> pipeline,int minRes,int maxRes);

private:

	//! The overview Coverages.
	std::vector<boost::intrusive_ptr<IProcess>> m_overviewPipelines;
	std::vector<boost::intrusive_ptr<ICoverage>> m_overviewCoverages;
	std::vector<int> m_overviewCoveragesResolutions;

	//! The name of the server serving the WCS datasource.
	std::string m_strServer;

	//! The version of the server serving the WCS datasource.
	std::string m_strVersion;

	//! The name of the layer (datasource) being retrieved from the server.
	std::string m_strLayer;

	//! The name of the server serving the WCS datasource.
	std::string m_overSampling;

	//! Allow the coverage to select a given field Select Field 
	std::string m_strListOfBands;

	//! prefered format of the results
	std::string m_strFormat;

	//! Allow the coverage to slice multi-domain coverage (3d++) into 2d coverages
	std::map<std::string,std::string> m_subsets;
};


class MODULE_GDAL_DECL GDALCoverageProcessWrapper : public ProcessImpl<GDALCoverageProcessWrapper>, public IXYCoverage
{
	PYXCOM_DECLARE_CLASS();

public:

	GDALCoverageProcessWrapper(const boost::intrusive_ptr<IXYCoverage> & coverage);

	static boost::intrusive_ptr<GDALCoverageProcessWrapper> create(const boost::intrusive_ptr<IXYCoverage> & coverage)
	{
		return boost::intrusive_ptr<GDALCoverageProcessWrapper>(new GDALCoverageProcessWrapper(coverage));
	}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)		
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( GDALCoverageProcessWrapper, IProcess);

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

private:

	//! The output Coverage.
	boost::intrusive_ptr<IXYCoverage> m_spXYCoverage;	
};

#endif