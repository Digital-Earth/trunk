#ifndef PYXIS__SAMPLING__XY_UTILS_H
#define PYXIS__SAMPLING__XY_UTILS_H
/******************************************************************************
xy_utils.h

begin		: 2007-02-14
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/rect_2d.h"

/*!
*/
//! Utilities related to XY coordinate systems.
class PYXLIB_DECL XYUtils
{
public:

	//! Convert from native coordinates to the nearest X,Y data index using rounding.
	static void nativeToNearestRaster(	const PYXRect2DDouble& bounds,
										const PYXCoord2DDouble& stepSize,
										const PYXCoord2DDouble& native,
										PYXCoord2DInt* pRaster	);

	//! Convert from native coordinates to the lower X,Y data index using truncation.
	static void nativeToLowerRaster(	const PYXRect2DDouble& bounds,
										const PYXCoord2DDouble& stepSize,
										const PYXCoord2DDouble& native,
										PYXCoord2DInt* pRaster	);

	//! Convert from native coordinates to the X,Y data index with fraction for subpixle operation.
	static void nativeToLowerRasterSubPixel(	const PYXRect2DDouble& bounds,
										const PYXCoord2DDouble& stepSize,
										const PYXCoord2DDouble& native,
										PYXCoord2DDouble* pRaster	);

	//! Convert from raster coordinates to native coordinates.
	static void rasterToNative(	const PYXRect2DDouble& bounds,
								const PYXCoord2DDouble& stepSize,
								const PYXCoord2DInt& raster,
								PYXCoord2DDouble* pNative	);
};

#endif // guard
