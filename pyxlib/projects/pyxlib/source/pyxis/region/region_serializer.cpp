/******************************************************************************
region_serializer.cpp

begin		: 2012-05-10
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "region_serializer.h"

#include "vector_point_region.h"
#include "circle_region.h"
#include "curve_region.h"
#include "multi_polygon_region.h"
#include "multi_curve_region.h"
#include "xy_bounds_region.h"

#include "pyxis/utility/coord_lat_lon.h"

#include "float.h"

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXVectorPointRegion> & region);
PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXCircleRegion> & region);
PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXCurveRegion> & region);
PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXXYBoundsRegion> & region);

std::string PYXRegionSerializer::serialize(const IRegion & region)
{
	PYXStringWireBuffer buffer;
	buffer << region;
	return buffer.toString();
}

PYXPointer<IRegion> PYXRegionSerializer::deserialize(const std::string & string)
{
	PYXPointer<IRegion> result;
	PYXConstWireBuffer buffer(string);
	buffer >> result;
	return result;
}

PYXPointer<IRegion> PYXRegionSerializer::deserialize(const PYXConstBufferSlice & slice)
{
	PYXPointer<IRegion> result;
	PYXConstWireBuffer buffer(slice);
	buffer >> result;
	return result;
}


template<class T,unsigned char F>
bool serializeRegionWithType(PYXWireBuffer & buffer, const IRegion & region)
{
	const T * typedRegion = dynamic_cast<const T *>(&region);

	if (typedRegion)
	{
		buffer << F << *typedRegion;
		return true;
	}
	return false;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const IRegion & region)
{
	if (!serializeRegionWithType<PYXVectorPointRegion,PYXRegionSerializer::knPointRegion>(buffer,region) &&
		!serializeRegionWithType<PYXCircleRegion,PYXRegionSerializer::knCircleRegion>(buffer,region) &&
		!serializeRegionWithType<PYXCurveRegion,PYXRegionSerializer::knCurveRegion>(buffer,region) &&
		!serializeRegionWithType<PYXXYBoundsRegion,PYXRegionSerializer::knXYBoundsRegion>(buffer,region) &&
		!serializeRegionWithType<PYXMultiCurveRegion,PYXRegionSerializer::knMultiCurveRegion>(buffer,region)&&
		!serializeRegionWithType<PYXMultiPolygonRegion,PYXRegionSerializer::knMultiPolygonRegion>(buffer,region))
	{
		PYXTHROW(PYXException,"failed to seralized region: unsupported region type");
	}
	return buffer;
}

template<class T,unsigned char F>
bool deserializeRegionWithType(PYXWireBuffer & buffer, unsigned char value, PYXPointer<IRegion> & region)
{
	if (value != F)
		return false;

	PYXPointer<T> typedRegion;
	buffer >> typedRegion;
	region = boost::dynamic_pointer_cast<IRegion>(typedRegion);
	
	return true;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<IRegion> & region)
{
	unsigned char regionType;
	buffer >> regionType;

	if (!deserializeRegionWithType<PYXVectorPointRegion,PYXRegionSerializer::knPointRegion>(buffer,regionType,region) &&
		!deserializeRegionWithType<PYXCircleRegion,PYXRegionSerializer::knCircleRegion>(buffer,regionType,region) &&
		!deserializeRegionWithType<PYXCurveRegion,PYXRegionSerializer::knCurveRegion>(buffer,regionType,region) &&
		!deserializeRegionWithType<PYXXYBoundsRegion,PYXRegionSerializer::knXYBoundsRegion>(buffer,regionType,region) &&
		!deserializeRegionWithType<PYXMultiCurveRegion,PYXRegionSerializer::knMultiCurveRegion>(buffer,regionType,region)&&
		!deserializeRegionWithType<PYXMultiPolygonRegion,PYXRegionSerializer::knMultiPolygonRegion>(buffer,regionType,region))

	{
		PYXTHROW(PYXException,"failed to seralized region: unsupported region type");
	}

	return buffer;
}


PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const CoordLatLon & ll)
{
	buffer << ll.latInDegrees() << ll.lonInDegrees();
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,CoordLatLon & ll)
{
	double lat,lon;
	buffer >> lat >> lon;
	ll.setLatInDegrees(lat);
	ll.setLonInDegrees(lon);
	return buffer;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCoord3DDouble & coord)
{
	buffer << SphereMath::xyzll(coord);
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXCoord3DDouble & coord)
{
	CoordLatLon ll;
	buffer >> ll;
	SphereMath::llxyz(ll,&coord);
	return buffer;
}

PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXIcosIndex & index)
{
	buffer << index.toString();
	return buffer;
}

PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXIcosIndex & index)
{
	std::string str;
	buffer >> str;
	index = str;
	return buffer;
}