/******************************************************************************
reference_sphere.cpp

begin		: 2004-04-01
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/reference_sphere.h"

// pyxlib includes
#include "pyxis/utility/great_circle_arc.h"
#include "pyxis/utility/trace.h"

//! The radius of the sphere in metres from GRS 1980 Authalic Sphere (EPSG_code=7048)
const double ReferenceSphere::kfRadius = 6371007.0;

//! Singleton instance
ReferenceSphere* ReferenceSphere::m_pInstance = 0;

/*!
Get the instance of the datum.

\return	The instance of the datum (ownership retained)
*/
ReferenceSphere const * ReferenceSphere::getInstance()
{
	assert(m_pInstance);
	return m_pInstance;	
}

/*!
Initialize static data when application starts.
*/
void ReferenceSphere::initStaticData()
{
	m_pInstance = new ReferenceSphere();
	TRACE_INFO("Instance of 'ReferenceSphere' created");
}

/*!
Free static data when application exits.
*/
void ReferenceSphere::freeStaticData()
{
	delete m_pInstance;
	m_pInstance = 0;
	TRACE_DEBUG("Instance of 'ReferenceSphere' destroyed");
}

/*!
Calculate the distance in metres between two points.

\param	pt1	The first point.
\param	pt2	The second point.

\return	The distance in metres.
*/
double ReferenceSphere::calcDistance(	const CoordLatLon& pt1,
										const CoordLatLon& pt2	) const
{
	return GreatCircleArc::calcDistance(pt1, pt2, kfRadius);
}
