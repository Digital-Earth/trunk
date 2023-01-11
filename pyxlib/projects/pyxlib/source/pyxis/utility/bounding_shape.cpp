/******************************************************************************
bounding_shape.cpp

begin		: 2011-01-22
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/bounding_shape.h"
#include "pyxis/derm/snyder_projection.h"

// Nothing here yet


int PYXBoundingCircle::estimateResolutionFromRadius(double radius)
{
	static const double minRadius = 50.0/SphereMath::knEarthRadius;

	if (radius < minRadius)
	{
		return SnyderProjection::getInstance()->precisionToResolution(minRadius);
	}
	else if (radius*2 > Icosahedron::kfCentralAngle)
	{
		return 1;
	}
	return SnyderProjection::getInstance()->precisionToResolution(radius);
}