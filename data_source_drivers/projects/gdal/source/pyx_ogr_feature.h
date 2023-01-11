#ifndef PYX_OGR_FEATURE_H
#define PYX_OGR_FEATURE_H
/******************************************************************************
pyx_ogr_feature.h

begin		: 2004-10-27
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_gdal.h"

// pyxlib includes
#include "pyxis/data/feature.h"
#include "pyxis/geometry/multi_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/region/curve_region.h"
// OGR includes
#include "ogr_core.h"
#include "ogr_feature.h"

// standard includes
#include <string>

// OGR forward declarations
class OGRFeature;
class OGRGeometryCollection;
class OGRLineString;
class OGRMultiLineString;
class OGRMultiPoint;
class OGRMultiPolygon;
class OGRPoint;
class OGRPolygon;

// local forward declarations
class ICoordConverter;
class PYXCell;
class PYXCurve;
class PYXPolygon;
class PYXMultiCell;

class OGRFeatureObject;
/*!
PYXOGRFeature wraps an OGR feature to provide a PYXFeature interface.
*/
//! Provides access to features through the OGR class library.
class PYXOGRFeature : public IFeature
{
// TODO should have PYXCOM class etc.?
	friend class PYXOGRDataSource;

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IRecord

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const
	{
		return m_spDefn;
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition()
	{
		// TODO: should PYXTableDefinitions ever be non-const?
		return boost::const_pointer_cast<PYXTableDefinition, const PYXTableDefinition>(m_spDefn);
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const;

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		// TODO what to do here?
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const
	{
		int nFieldIndex = getDefinition()->getFieldIndex(strName);
		PYXValue value;
		if (0 <= nFieldIndex)
		{
			value = getFieldValue(nFieldIndex);
		}
		return value;
	}

	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName)
	{
		int nFieldIndex = getDefinition()->getFieldIndex(strName);
		if (0 <= nFieldIndex)
		{
			setFieldValue(value, nFieldIndex);
		}
	}

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const;

	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues)
	{
		// TODO what to do here?
	}

	virtual void STDMETHODCALLTYPE addField(	const std::string& strName,
												PYXFieldDefinition::eContextType nContext,
												PYXValue::eType nType,
												int nCount = 1,
												PYXValue value = PYXValue()	)
	{
		// TODO what to do here?
	}

public: // IFeature

	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spGeometry;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_spGeometry;
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const;

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const;
	
	
public:

	//! Convert an OGR field type to a PYXIS data type
	static PYXValue::eType convertOGRToPYXDataType(OGRFieldType nOGRType);

public:

	//! Constructor
	PYXOGRFeature(
		const PYXPointer<OGRFeatureObject> & pOGRFeature,
		boost::intrusive_ptr<const PYXOGRDataSource> spDataSource,
		PYXPointer<const PYXTableDefinition> spDefn,
		const ICoordConverter& converter,
		int nResolution,
		std::string strFeatStyle="");

	//! Destructor
	~PYXOGRFeature();

private:

	//! Disable copy constructor
	PYXOGRFeature(const PYXOGRFeature&);

	//! Disable copy assignment
	void operator=(const PYXOGRFeature&);

	//! Create the geometry for the feature at the given PYXIS resolution.
	void createGeometry(const ICoordConverter& converter, int nResolution);

	//! Create the PYXIS geometry for the OGR geometry at the given PYXIS resolution.
	void createGeometry(OGRGeometry* pOGRGeometry, const ICoordConverter& converter, int nResolution);

	//! Create geometry from an OGRPoint object.
	PYXPointer<PYXGeometry> createVectorPointGeometry(
		OGRPoint* pOGRPoint,
		const ICoordConverter& converter,
		int nResolution	);

	//! Create geometry from an OGRLineString object.
	PYXPointer<PYXGeometry> createVectorLineStringGeometry(
		OGRLineString* pOGRLineString,
		const ICoordConverter& converter,
		int nResolution	);

	//! Create geometry from an OGRPolygon object.
	PYXPointer<PYXGeometry> createVectorPolygonGeometry(
		OGRPolygon* pOGRPolygon,
		const ICoordConverter& converter,
		int nResolution	);

	//! Create geometry from an OGRMultiLineString object.
	PYXPointer<PYXGeometry> createVectorMultiLineStringGeometry(
		OGRMultiLineString* pOGRGeometry,
		const ICoordConverter& converter,
		int nResolution	);

	//! Create geometry from an OGRMultiPolygon object.
	PYXPointer<PYXGeometry> createVectorMultiPolygonGeometry(
		OGRMultiPolygon* pOGRGeometry,
		const ICoordConverter& converter,
		int nResolution	);

	//! Extract the curves from an OGR polygon geometry
	void extractCurves(
		OGRPolygon* pOGRPolygon,
		const ICoordConverter& converter,
		int nResolution,
		std::vector<PYXPointer<PYXCurveRegion>> & regions	);

private:
	
	//! The style used to display this feature.
	std::string m_strStyle;

	//! Table definition.
	PYXPointer<const PYXTableDefinition> m_spDefn;

	//! Pointer to PTXDataSource for thread safety
	boost::intrusive_ptr<const PYXOGRDataSource> m_spPYXOGRDataSource;

	//! The OGR feature.
	PYXPointer<OGRFeatureObject> m_pOGRFeature;

	//! Storage for ID of the feature.
	std::string m_strID;

	//! Geometry.
	PYXPointer<PYXGeometry> m_spGeometry;

	//! The styles associated with the feature.
	std::vector<FeatureStyle> m_vecStyles;

};

#endif
