#ifndef PYXIS__DERM__PROJECTION_METHOD_H
#define PYXIS__DERM__PROJECTION_METHOD_H
/******************************************************************************
projection_method.h

begin		: 2004-03-11
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/utility/coord_3d.h"
#include "pyxis/utility/coord_lat_lon.h"

/*!
ProjectionMethod is the abstract base for all classes that project from a PYXIS
index to a point on the surface of the sphere specified in lat/lon coordinates.
*/
//! Abstract base class for projection methods.
class PYXLIB_DECL ProjectionMethod : public PYXCoordConverter
{
public:

	//! Destructor
	virtual ~ProjectionMethod() {;}

	//! Convert geocentric lat lon values to PYXIS indices.
	virtual void nativeToPYXIS(	const PYXCoord2DDouble& native,
								PYXIcosIndex* pIndex,
								int nResolution	) const = 0;

	//! Convert PYXIS indices to geocentric lat lon.
	virtual void pyxisToNative(	const PYXIcosIndex& index,
								PYXCoord2DDouble* pNative	) const = 0;

	//! Convert geocentric lat lon values to PYXIS indices.
	virtual void nativeToPYXIS(	const CoordLatLon& ll,
								PYXIcosIndex* pIndex,
								int nResolution	) const = 0;

	//! Convert PYXIS indices to geocentric lat lon.
	virtual void pyxisToNative(	const PYXIcosIndex& index,
								 CoordLatLon* pll	) const = 0;

	//! Convert 3d coordinates to PYXIS indices.
	virtual void xyzToPYXIS(	const PYXCoord3DDouble& coord,
								PYXIcosIndex* pIndex,
								int nResolution	) const = 0;

	//! Convert PYXIS indices to x, y, z coordinates.
	virtual void pyxisToXYZ(	const PYXIcosIndex& index,
								PYXCoord3DDouble* pCoord	) const = 0;

	//! Convert precision specified in arc radians to a resolution
	virtual int precisionToResolution(double fPrecision) const = 0;

	//! Convert a PYXIS resolution to a precision in arc radians.
	virtual double resolutionToPrecision(int nResolution) const = 0;
};

#endif // guard
