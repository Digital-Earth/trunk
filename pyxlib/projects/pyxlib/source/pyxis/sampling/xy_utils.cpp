/******************************************************************************
xy_utils.cpp

begin		: 2007-02-14
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/sampling/xy_utils.h"

/*!
Convert from native coordinates to the nearest X,Y data index using rounding.

\param	bounds			The bounds.
\param	stepSize		The step size.
\param	native			The native coordinate.
\param	pRaster			The raster coordinates (out)
*/
void XYUtils::nativeToNearestRaster(	const PYXRect2DDouble& bounds,
										const PYXCoord2DDouble& stepSize,
										const PYXCoord2DDouble& native,
										PYXCoord2DInt* pRaster	)
{
	int nX;
	if (stepSize.x() < 0)
	{
		nX = MathUtils::round(
				(native.x() - bounds.xMax()) /
				stepSize.x());
	}
	else
	{
		nX = MathUtils::round(
				(native.x() - bounds.xMin()) /
				stepSize.x());
	}
	pRaster->setX(nX);
		
	int nY;
	if (stepSize.y() < 0)
	{
		nY = MathUtils::round(
				(native.y() - bounds.yMax()) /
				stepSize.y());
	}
	else
	{
		nY = MathUtils::round(
				(native.y() - bounds.yMin()) /
				stepSize.y());
	}

	pRaster->setY(nY);
}

/*!
Convert from native coordinates to the lower X,Y data index using truncation.

\param	bounds			The bounds.
\param	stepSize		The step size.
\param	native			The native coordinate.
\param	pRaster			The raster coordinates (out)
*/
void XYUtils::nativeToLowerRaster(	const PYXRect2DDouble& bounds,
									const PYXCoord2DDouble& stepSize,
									const PYXCoord2DDouble& native,
									PYXCoord2DInt* pRaster	)
{
	int nX;
	if (stepSize.x() < 0)
	{
		nX = static_cast<int>(floor(
				(native.x() - bounds.xMax()) /
				stepSize.x()));
	}
	else
	{
		nX = static_cast<int>(floor(
				(native.x() - bounds.xMin()) /
				stepSize.x()));
	}
    pRaster->setX(nX);

	int nY;
	if (stepSize.y() < 0)
	{
		nY = static_cast<int>(floor(
				(native.y() - bounds.yMax()) /
				stepSize.y()));
	}
	else
	{
		nY = static_cast<int>(floor(
				(native.y() - bounds.yMin()) /
				stepSize.y()));
	}
    pRaster->setY(nY);
}


/*!
Convert from native coordinates to the lower X,Y data index using truncation.

\param	bounds			The bounds.
\param	stepSize		The step size.
\param	native			The native coordinate.
\param	pRaster			The raster coordinates (out)
*/
void XYUtils::nativeToLowerRasterSubPixel(	const PYXRect2DDouble& bounds,
									const PYXCoord2DDouble& stepSize,
									const PYXCoord2DDouble& native,
									PYXCoord2DDouble* pRaster	)
{
	double nX;
	if (stepSize.x() < 0)
	{
		nX = (native.x() - bounds.xMax()) /
			stepSize.x();
	}
	else
	{
		nX = (native.x() - bounds.xMin()) /
			stepSize.x();
	}
	pRaster->setX(nX);

	double nY;
	if (stepSize.y() < 0)
	{
		nY = (native.y() - bounds.yMax()) /
			stepSize.y();
	}
	else
	{
		nY = (native.y() - bounds.yMin()) /
			stepSize.y();
	}
	pRaster->setY(nY);
}


/*!
Convert from raster coordinates to native coordinates.

\param	bounds			The bounds.
\param	stepSize		The step size.
\param	raster			The raster coordinates. 
\param	pNative			The native coordinate. (out)
*/
void XYUtils::rasterToNative(	const PYXRect2DDouble& bounds,
								const PYXCoord2DDouble& stepSize,
								const PYXCoord2DInt& raster,
								PYXCoord2DDouble* pNative	)
{
	double xBound;
	if (stepSize.x() < 0)
	{
		xBound = bounds.xMax();
	}
	else
	{
		xBound = bounds.xMin();
	}
    pNative->setX((raster.x() * stepSize.x()) + xBound);

	double yBound;
	if (stepSize.y() < 0)
	{
		yBound = bounds.yMax();
	}
	else
	{
		yBound = bounds.yMin();
	}
    pNative->setY((raster.y() * stepSize.y()) + yBound);
}
