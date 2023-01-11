#ifndef OGR_FEATURE_SERVER_PROCESS_H
#define OGR_FEATURE_SERVER_PROCESS_H

/******************************************************************************
ogr_feature_server_process.h

begin      : 2016-02-09
copyright  : (c) 2016 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "geoservices_reference.h"
#include "module_gdal.h"
#include "ogr_process.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"

/*!
Provides access to GeoServices FeatureServer data through the GDAL library.
*/
class MODULE_GDAL_DECL OGRFeatureServerProcess : public ProcessImpl<OGRFeatureServerProcess>, public IFeatureCollection, public IGeoServicesFeatureServerReference
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IGeoServicesReference)
		IUNKNOWN_QI_CASE(IGeoServicesFeatureServerReference)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecordx

	IRECORD_IMPL();

public: // IFeature

	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strUri;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_outputAsFeature->getGeometry();
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_outputAsFeature->getGeometry();
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_outputAsFeature->getStyle();
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		return m_outputAsFeature->getStyle(strStyleToGet);
	}

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const
	{
		return m_outputAsFeatureCollection->getIterator();
	}

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const
	{
		return m_outputAsFeatureCollection->getIterator(geometry);
	}

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const
	{
		return m_outputAsFeatureCollection->getFeatureDefinition();
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition()
	{
		return m_outputAsFeatureCollection->getFeatureDefinition();
	}

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const
	{
		return m_outputAsFeatureCollection->getFeature(strFeatureID);
	}

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const
	{
		return m_outputAsFeatureCollection->getFeatureStyles();
	}

	virtual bool STDMETHODCALLTYPE canRasterize() const
	{
		return m_outputAsFeatureCollection->canRasterize();
	}

	IFEATURECOLLECTION_IMPL_HINTS();


public: //IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeatureCollection*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeatureCollection*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();


public: //IGeoServicesReference

	virtual const std::string& getUri() const { return m_strUri; }

private:

	std::string m_strUri;

	PYXPointer<OGRProcess> m_ogrProcess;
	PYXPointer<IFeatureCollection> m_outputAsFeatureCollection;
	PYXPointer<IFeature> m_outputAsFeature;
};

#endif