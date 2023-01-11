#ifndef PYXIS__REGION__REGION_SERIALIZER_H
#define PYXIS__REGION__REGION_SERIALIZER_H
/******************************************************************************
region_serializer.h

begin		: 2012-05-12
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/region/region.h"
#include "pyxis/utility/wire_buffer.h"


class PYXLIB_DECL PYXRegionSerializer
{
public:
	enum RegionType
	{
		knPointRegion = 1,
		knCircleRegion = 2,
		knCurveRegion = 3,
		knPolygonRegion = 4,
		knMultiCurveRegion = 5,
		knMultiPolygonRegion = 6,
		knXYBoundsRegion = 7,
	};

	static std::string serialize(const IRegion & region);
	static PYXPointer<IRegion> deserialize(const std::string & string);
	static PYXPointer<IRegion> deserialize(const PYXConstBufferSlice & slice);

	static std::string serialize(const IRegionVisitor & visitor);
	static PYXPointer<IRegionVisitor> deserialize(const IRegion & region, const std::string & string);
};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const IRegion & region);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<IRegion> & region);

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const CoordLatLon & ll);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,CoordLatLon & ll);

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXCoord3DDouble & coord);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXCoord3DDouble & coord);

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXIcosIndex & index);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXIcosIndex & index);

#endif // guard
