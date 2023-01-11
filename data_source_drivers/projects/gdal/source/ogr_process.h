#ifndef OGR_PROCESS_H
#define OGR_PROCESS_H
/******************************************************************************
ogr_process.h

begin		: 2007-06-14
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_gdal.h"

// local includes
#include "coord_converter_impl.h"
#include "gdal_metadata.h"
#include "pyx_ogr_data_source.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/user_credentials_provider.h"
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
*/
//! A process for OGR feature collection.
class MODULE_GDAL_DECL OGRProcess : public ProcessImpl<OGRProcess>, public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public:

	static int knUninitializedRes;

	//! Constructor
	OGRProcess() :
		m_nRes(knUninitializedRes),
		m_spDefn(PYXTableDefinition::create()),
		m_bCanRasterize(true), //Default to being able to rasterize.
		m_nLayerIndex(0)
	{
	}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	
	IUNKNOWN_DEFAULT_CAST( OGRProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return m_spDS;
		//return static_cast<const IFeatureCollection*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return m_spDS;
		//return static_cast<IFeatureCollection*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const
	{ 
		return m_spDS->getIterator();
	} 

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const
	{
		return m_spDS->getIterator(geometry);
	}

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const 
	{
		return m_spDS->getFeatureDefinition();
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() 
	{
		return m_spDS->getFeatureDefinition();
	}

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const
	{
		return m_spDS->getFeature(strFeatureID);
	}

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const 
	{
		return m_spDS->getFeatureStyles();
	}

	virtual bool STDMETHODCALLTYPE canRasterize() const 
	{
		return m_bCanRasterize;
	}

	IFEATURECOLLECTION_IMPL_HINTS();

private:

	void open(boost::intrusive_ptr<ISRS> spSRS);

	bool close();

	std::string buildDatasetURIforWFS();

private:

	//! Lets be friendly! 
	friend class OgrPipeBuilder;

	//! Flag to indicate whether this object as an IFeatureCollection can be rasterized or not.
	bool m_bCanRasterize;

	//! Mutex to serialize concurrent access by multiple threads
	mutable boost::recursive_mutex m_mutex;

	//! Resolution. -1 means let the data source choose.
	int m_nRes;

	//! Layer index to read from the OGRDataSource.
	int m_nLayerIndex;

	//! Layer name to read from the OGRDataSource.
	std::string m_strLayerName;

	//! URL to web-based feature service (OGC WFS or GeoServices FeatureServer)
	std::string m_strUrl;

	//! Path to file.
	boost::filesystem::path m_path;

	//! Path to style.
	boost::filesystem::path m_styleURI;
	
	//! The OGR data source.
	boost::intrusive_ptr<PYXOGRDataSource> m_spDS;

	boost::intrusive_ptr<IUsernameAndPasswordCredentials> m_credentials;

	bool m_axisFlip;
};

#endif
