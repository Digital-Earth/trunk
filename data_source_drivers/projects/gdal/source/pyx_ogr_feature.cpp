/******************************************************************************
ogr_feature.cpp

begin		: 2004-10-27
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

//enable this to trace polygon creation times
//#define TRACE_POLYGON_CREATION_TIME

#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "pyx_ogr_feature.h"
#include "pyx_ogr_data_source.h"
#include "pyx_shared_gdal_data_set.h"

// local includes
#include "exceptions.h"

// pyxis library includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/curve.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/multi_cell.h"
#include "pyxis/geometry/polygon.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/region/vector_point_region.h"
#include "pyxis/region/multi_curve_region.h"
#include "pyxis/region/multi_polygon_region.h"

#include "pyxis/utility/app_services.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/trace.h"
#include "pyxis/utility/exception.h"

#ifdef TRACE_POLYGON_CREATION_TIME
#include <boost/date_time/posix_time/posix_time_types.hpp>
#endif

// standard includes
#include <cassert>
#include <memory>
#include "pyxis/region/multi_polygon_region.h"

// includes for properties file
static std::string kstrScope = "PYXOGRFeature";
static std::string kstrTagFilledPolygons = "FilledPolygons";
static std::string kstrTagFilledPolygonsDescription = "Set to 1 to show filled polygons, 0 to show border only.";

////////////////////////////////////////////////////////////////////////////////
// IRecord
////////////////////////////////////////////////////////////////////////////////

PYXValue STDMETHODCALLTYPE PYXOGRFeature::getFieldValue(int nField) const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	OGRFeature * feature = m_pOGRFeature->GetFeature();
	if ((0 <= nField) && (feature->GetDefnRef()->GetFieldCount() > nField))
	{
		if (feature->IsFieldSet(nField))
		{
			OGRFieldType nOGRType = feature->GetDefnRef()->GetFieldDefn(nField)->GetType();

			switch (nOGRType)
			{
			case OFTInteger:
			case OFTIntegerList:
				return PYXValue(feature->GetFieldAsInteger(nField));

			case OFTInteger64:
			case OFTInteger64List:
				// store in a double, moving to 64 bits will require a cache update			
				return PYXValue(feature->GetFieldAsDouble(nField));

			case OFTReal:
			case OFTRealList:
				return PYXValue(feature->GetFieldAsDouble(nField));

			case OFTDate:
			case OFTDateTime:
				return PYXValue(std::string(feature->GetFieldAsString(nField)));

			case OFTString:
			case OFTStringList:
				return PYXValue(std::string(feature->GetFieldAsString(nField)));

			default:
				// unsupported data type
				PYXTHROW(	GDALProcessException,
					"Unsupported data type: '" << nOGRType << "'."	);
				break;
			}
		}
	}

	return PYXValue();
}

std::vector<PYXValue> STDMETHODCALLTYPE PYXOGRFeature::getFieldValues() const
{
	std::vector<PYXValue> result;
	result.resize(getDefinition()->getFieldCount());
	for(int nField=0;nField<getDefinition()->getFieldCount();++nField)
	{
		result[nField] = getFieldValue(nField);
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

/*!
Convert an OGR field type to a PYXIS data type.

\param	nOGRType	The OGR field type

\return	The PYXIS data type.
*/
PYXValue::eType PYXOGRFeature::convertOGRToPYXDataType(OGRFieldType nOGRType)
{
	PYXValue::eType nType;

	switch (nOGRType)
	{
	case OFTInteger:
	case OFTIntegerList:
		nType = PYXValue::knInt32;
		break;

	case OFTInteger64:
	case OFTInteger64List:
		// store in a double for now, moving to 64 bits will require invalidating the cache
		nType = PYXValue::knDouble;
		break;

	case OFTReal:
	case OFTRealList:
		nType = PYXValue::knDouble;
		break;

	case OFTDate:
	case OFTDateTime:
		nType = PYXValue::knString;
		break;

	case OFTString:
	case OFTStringList:
		nType = PYXValue::knString;
		break;

	default:
		// unsupported data type
		PYXTHROW(	GDALProcessException,
			"Unsupported data type: '" << nOGRType << "'."	);
		break;
	}

	return nType;
}

/*!
Constructor.

\param	pOGRFeature	The OGR feature (ownership transfered by caller)
\param	spDefn		The feature definition (ownership shared with caller)
\param	converter	The coordinate converter.
\param	nResolution	The PYXIS resolution for the geometry.
*/
PYXOGRFeature::PYXOGRFeature(
	const PYXPointer<OGRFeatureObject> & pOGRFeature,
	boost::intrusive_ptr<const PYXOGRDataSource> spPYXOGRDataSource,
	PYXPointer<const PYXTableDefinition> spDefn,
	const ICoordConverter& converter,
	int nResolution,
	std::string strFeatStyle) :
		m_spDefn(spDefn),
		m_pOGRFeature(pOGRFeature),
		m_spPYXOGRDataSource(spPYXOGRDataSource),
		m_strStyle(strFeatStyle)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	assert(0 != pOGRFeature);
	assert(0 != spPYXOGRDataSource);

	m_strID = StringUtils::toString(m_pOGRFeature->GetFeature()->GetFID());

	// create the geometry for the feature
	createGeometry(converter, nResolution);
}

/*!
Destructor.
*/
PYXOGRFeature::~PYXOGRFeature()
{
}

/*!
Gets the style associated with this feature.

\returns A string indicating which icon to use to display this feature or a default style.
*/
std::string PYXOGRFeature::getStyle() const
{
	return m_strStyle;

}

std::string PYXOGRFeature::getStyle(const std::string& strStyleToGet) const
{
	PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(m_strStyle);
	return styleDoc->getNodeText("/style/" + strStyleToGet);
}


#define OGRPOINT_TO_PYXCOORD3DDOUBLE(PT,CONVERTER,CRD) \
{ \
	const OGRPoint & __pt__ = PT; \
	CoordLatLon __latLon__; \
	PYXCoord2DDouble __xy__(__pt__.getX(), __pt__.getY()); \
	CONVERTER.nativeToLatLon(__xy__, &__latLon__); \
	SphereMath::llxyz(__latLon__,&(CRD)); \
}

/*!
Create the geometry for the feature at the given PYXIS resolution. Geometries that contain curves
are approximated with line segments and those containing a Z component are flattened.

\param	converter	The coordinate converter.
\param	nResolution	The resolution.
*/
void PYXOGRFeature::createGeometry(	const ICoordConverter& converter,
									int nResolution	)
{
	m_spGeometry = PYXPointer<PYXGeometry>(0);

	OGRGeometry* pOGRGeometry = m_pOGRFeature->GetFeature()->GetGeometryRef();
	OGRwkbGeometryType nGeometryType = pOGRGeometry->getGeometryType();

	try
	{
		switch (nGeometryType)
		{
			case wkbCircularString:
			case wkbCircularStringZ:
			case wkbCompoundCurve:
			case wkbCompoundCurveZ:
			case wkbLinearRing:
				{
					OGRLineString* pOGRLineString = static_cast<OGRCurve*>(pOGRGeometry)->CurveToLine();
					createGeometry(pOGRLineString, converter, nResolution);
					delete pOGRLineString;
				}
				break;

			case wkbMultiCurve:
			case wkbMultiCurveZ:
			case wkbMultiSurface:
			case wkbMultiSurfaceZ:
			case wkbCurvePolygon:
			case wkbCurvePolygonZ:
				{
					OGRGeometry* pOGRLinearGeometry = static_cast<OGRGeometry*>(pOGRGeometry)->getLinearGeometry();
					createGeometry(pOGRLinearGeometry, converter, nResolution);
					delete pOGRLinearGeometry;
				}
				break;

			default:
				createGeometry(pOGRGeometry, converter, nResolution);
				break;
		}
	}
	catch(PYXException& e)
	{
		PYXTHROW(GDALProcessException, "Failed to create geometry for feature (ID:" << getID() << "):" << e.getFullErrorString());
	}
	catch(...)
	{
		PYXTHROW(GDALProcessException, "Failed to create geometry for feature (ID:" << getID() << ")");
	}
}

/*!
Create the geometry for the feature at the given PYXIS resolution

\param	pOGRGeometry	The OGR geometry (ownership retained by caller)
\param	converter		The coordinate converter.
\param	nResolution	T	The resolution.
*/
void PYXOGRFeature::createGeometry(	
	OGRGeometry* pOGRGeometry,
	const ICoordConverter& converter,
	int nResolution	)
{
	m_spGeometry = PYXPointer<PYXGeometry>(0);

	OGRwkbGeometryType nGeometryType = pOGRGeometry->getGeometryType();
	switch (nGeometryType)
	{
		case wkbPoint:
		case wkbPoint25D:
			m_spGeometry = createVectorPointGeometry(
				static_cast<OGRPoint*>(pOGRGeometry),
				converter,
				nResolution	);
			break;

		case wkbLineString:
		case wkbLineString25D:
			m_spGeometry = createVectorLineStringGeometry(
				static_cast<OGRLineString*>(pOGRGeometry),
				converter,
				nResolution	);
			break;

		case wkbPolygon:
		case wkbPolygon25D:
			m_spGeometry = createVectorPolygonGeometry(
				static_cast<OGRPolygon*>(pOGRGeometry),
				converter,
				nResolution	);
			break;

		case wkbMultiLineString:
		case wkbMultiLineString25D:
			m_spGeometry = createVectorMultiLineStringGeometry(
				static_cast<OGRMultiLineString*>(pOGRGeometry),
				converter,
				nResolution	);
			break;

		case wkbMultiPolygon:
		case wkbMultiPolygon25D:
			m_spGeometry = createVectorMultiPolygonGeometry(
				static_cast<OGRMultiPolygon*>(pOGRGeometry),
				converter,
				nResolution	);
			break;

		case wkbMultiPoint:
		case wkbMultiPoint25D:
			{
				auto pOGRMultiPoint = static_cast<OGRMultiPoint*>(pOGRGeometry);

				if (pOGRMultiPoint->getNumGeometries() != 1)
				{
					PYXTHROW(GDALProcessException, "Unsupported multipoint with multiple geometries." );
					break;
				}

				auto onlyOnePoint = pOGRMultiPoint->getGeometryRef(0);

				m_spGeometry = createVectorPointGeometry(
					static_cast<OGRPoint*>(onlyOnePoint),
					converter,
					nResolution	);
				break;
			}
				
		case wkbGeometryCollection:
		case wkbGeometryCollection25D:
		case wkbUnknown:
		case wkbNone:
		default:
			// unsupported geometry type
			PYXTHROW(GDALProcessException, "Unsupported geometry type: '" << nGeometryType << "'." );
			break;
	}
}

/*!
Create geometry from an OGRPoint object.

\param	pOGRPoint	The OGR point geometry.
\param	converter	The coordinate converter.
\param	nResolution	The resolution of the resulting cell.

\return	The geometry.
*/
PYXPointer<PYXGeometry>
PYXOGRFeature::createVectorPointGeometry(
	OGRPoint* pOGRPoint,
	const ICoordConverter& converter,
	int nResolution	)
{
	PYXCoord3DDouble location;	
	OGRPOINT_TO_PYXCOORD3DDOUBLE(*pOGRPoint,converter,location);
	return PYXVectorGeometry2::create(PYXVectorPointRegion::create(location),nResolution);
}

/*!
Create geometry from an OGRLineString object.

\param	pOGRLineString	The OGR line string geometry.
\param	converter		The coordinate converter.
\param	nResolution		The resolution of the resulting cells.

\return	The geometry.
*/
PYXPointer<PYXGeometry>
PYXOGRFeature::createVectorLineStringGeometry(
	OGRLineString* pOGRLineString,
	const ICoordConverter& converter,
	int nResolution	)
{
	OGRPoint pt;
	CoordLatLon latLon;	

	int nNumPoints = pOGRLineString->getNumPoints();

	std::vector<PYXCoord3DDouble> nodes;
	nodes.resize(nNumPoints);

	for (int nPoint = 0; nPoint < nNumPoints; ++nPoint)
	{
		pOGRLineString->getPoint(nPoint, &pt);
		OGRPOINT_TO_PYXCOORD3DDOUBLE(pt,converter,nodes[nPoint]);		
	}

	if (nodes.size() == 1)
	{
		return PYXVectorGeometry::create(PYXVectorPointRegion::create(nodes[0]),nResolution);
	}
	else
	{
		return PYXVectorGeometry2::create(PYXCurveRegion::create(nodes),nResolution);
	}
}

/*!
Create a PYXIS geometry from an OGR polygon geometry.

\param	pOGRPolygon	The OGR polygon geometry
\param	converter	The coordinate converter
\param	nResolution	The resolution at which to perform the conversion

\return	The PYXIS geometry.
*/
PYXPointer<PYXGeometry>
PYXOGRFeature::createVectorPolygonGeometry(
	OGRPolygon* pOGRPolygon,
	const ICoordConverter& converter,
	int nResolution	)
{
	std::vector<PYXPointer<PYXCurveRegion>> regions;
	extractCurves(pOGRPolygon, converter, nResolution,regions);

	if (regions.empty())
	{
		//this could be a case of a polygon with not enough vertices.
		int nNumPoints = pOGRPolygon->getExteriorRing()->getNumPoints();

		OGRPoint pt;
		CoordLatLon latLon;	
		std::vector<PYXCoord3DDouble> points(2);
			
		switch (nNumPoints)
		{
		case 0:
			PYXTHROW(PYXException,"Polygon without any vertices");

		case 1: //1 point - its a point
			{
				pOGRPolygon->getExteriorRing()->getPoint(0, &pt);
				OGRPOINT_TO_PYXCOORD3DDOUBLE(pt,converter,points[0]);				
			}
			return PYXVectorGeometry2::create(PYXVectorPointRegion::create(points[0]),nResolution);

		case 2: //2 points - its a line
		case 3: //3 point of a polygon means that the first and last points are the same - so, its a line at the end
			{
				pOGRPolygon->getExteriorRing()->getPoint(0, &pt);
				OGRPOINT_TO_PYXCOORD3DDOUBLE(pt,converter,points[0]);

				pOGRPolygon->getExteriorRing()->getPoint(1, &pt);
				OGRPOINT_TO_PYXCOORD3DDOUBLE(pt,converter,points[1]);
			}
			return PYXVectorGeometry2::create(PYXCurveRegion::create(points),nResolution);

		default:
			PYXTHROW(PYXException,"Unexpected number of vertices on an invalid OGRPolygon outer ring");
		}
	}
	else
	{
		return PYXVectorGeometry2::create(PYXMultiPolygonRegion::create(regions),nResolution);
	}
}

/*!
Create geometry from an OGRMultiLineString object.

\param	pOGRGeometry	The OGR multi line string geometry.
\param	converter		The coordinate converter.
\param	nResolution		The resolution of the resulting cells.

\return	The geometry
*/
PYXPointer<PYXGeometry>
PYXOGRFeature::createVectorMultiLineStringGeometry(
	OGRMultiLineString* pOGRGeometry,
	const ICoordConverter& converter,
	int nResolution	)
{
	std::vector<PYXPointer<PYXCurveRegion>> regions;

	int nNumLines = pOGRGeometry->getNumGeometries();
	for (int nLine = 0; nLine < nNumLines; ++nLine)
	{
		OGRLineString* pLine =
			reinterpret_cast<OGRLineString*>(pOGRGeometry->getGeometryRef(nLine));

		PYXPointer<PYXGeometry> geom = createVectorLineStringGeometry(pLine, converter, nResolution);

		PYXPointer<PYXVectorGeometry> vector = boost::dynamic_pointer_cast<PYXVectorGeometry>(geom);

		if (vector)
		{
			auto curveRegion = boost::dynamic_pointer_cast<PYXCurveRegion>(vector->getRegion());

			if (curveRegion) 
			{
				regions.push_back(curveRegion);
			}
			else 
			{
				TRACE_DEBUG("createVectorLineStringGeometry return non curve region");
			}
		}
		else 
		{
			PYXVectorGeometry2 * vectorGeometry2 = dynamic_cast <PYXVectorGeometry2*>(geom.get()); 

			if (vectorGeometry2)
			{
				auto curveRegion = boost::dynamic_pointer_cast<PYXCurveRegion>(vectorGeometry2->getRegion());

				if (curveRegion) 
				{
					regions.push_back(curveRegion);
				} 
				else 
				{
					TRACE_DEBUG("createVectorLineStringGeometry return non curve region");
				}
			}
		}
	}

	return PYXVectorGeometry2::create(PYXMultiCurveRegion::create(regions),nResolution);
}

/*!
Create a PYXIS vector geometry from an OGR multi polygon geometry.

\param	pOGRGeometry	The OGR multi polygon geometry
\param	converter		The coordinate converter
\param	nResolution		The resolution at which to perform the conversion

\return	The PYXIS vector geometry.
*/
PYXPointer<PYXGeometry> PYXOGRFeature::createVectorMultiPolygonGeometry(
	OGRMultiPolygon* pOGRGeometry,
	const ICoordConverter& converter,
	int nResolution	)
{
	std::vector<PYXPointer<PYXCurveRegion>> regions;

	int nNumPolys = pOGRGeometry->getNumGeometries();
	for (int nPoly = 0; nPoly < nNumPolys; ++nPoly)
	{
		OGRPolygon* pPoly =
			reinterpret_cast<OGRPolygon*>(pOGRGeometry->getGeometryRef(nPoly));
		extractCurves(pPoly, converter, nResolution,regions);
	}

	return PYXVectorGeometry2::create(PYXMultiPolygonRegion::create(regions),nResolution);
}

/*!
Extract the lines from an OGR polygon geometry.

\param	pOGRPolygon	The OGR polygon
\param	converter	The coordinate converter
\param	nResolution	The resolution at which to convert
\param	regions		The curves (out)
*/
void PYXOGRFeature::extractCurves(OGRPolygon* pOGRPolygon,
								  const ICoordConverter& converter,
								  int nResolution,
								  std::vector<PYXPointer<PYXCurveRegion>> & regions)
{
	OGRPoint pt;	
	std::vector<PYXCoord3DDouble> vertices;

	// exterior ring.
	{
		// don't repeat first point as last.
		int nNumPoints = pOGRPolygon->getExteriorRing()->getNumPoints() - 1;

		// make sure the exterior ring has 3 or more vertices
		if (nNumPoints > 2)
		{
			vertices.resize(nNumPoints);

			for (int nPoint = 0; nPoint < nNumPoints; ++nPoint)
			{
				pOGRPolygon->getExteriorRing()->getPoint(nPoint, &pt);
				OGRPOINT_TO_PYXCOORD3DDOUBLE(pt, converter, vertices[nPoint]);				
			}

			PYXPointer<PYXCurveRegion> curve= PYXCurveRegion::create(vertices);
			curve->closeCurve();
			regions.push_back( curve);
		}
	}

	// interior rings.
	for (int nRing = 0; nRing != pOGRPolygon->getNumInteriorRings(); ++nRing)
	{
		// don't repeat first point as last.
		int nNumPoints = pOGRPolygon->getInteriorRing(nRing)->getNumPoints() - 1;

		// make sure the ring has 3 vertices
		if (nNumPoints <= 2)
		{
			continue;
		}
		
		vertices.clear();
		vertices.resize(nNumPoints);

		for (int nPoint = 0; nPoint < nNumPoints; ++nPoint)
		{
			pOGRPolygon->getInteriorRing(nRing)->getPoint(nPoint, &pt);
			OGRPOINT_TO_PYXCOORD3DDOUBLE(pt, converter, vertices[nPoint]);			
		}

		PYXPointer<PYXCurveRegion> curve = PYXCurveRegion::create(vertices);
		curve->closeCurve();
		regions.push_back( curve);
	}
}
