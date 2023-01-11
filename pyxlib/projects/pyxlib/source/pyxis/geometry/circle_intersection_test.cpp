/******************************************************************************
circle_intersection_test.cpp

begin		: 2006-05-10
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "circle_intersection_test.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/utility/great_circle_arc.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/derm/index_math.h"

// standard includes
#include <cassert>

/*!
Constructor initializes member variables.
*/
CircleIntersectionTest::CircleIntersectionTest(
	const ProjectionMethod* pProjection	) :
	m_fRadius(0),
	m_pProjection(pProjection)
{
	assert(pProjection != 0);
}

/*!
Set the index that is at the centre of the circular area to be defined. The index
that is defined in this method should be at the data resolution of the test.

param pyxIndex	The index at the centre of the circle geometry.
*/
void CircleIntersectionTest::setCentre(const PYXIcosIndex& pyxIndex)
{
	if (pyxIndex.isNull())
	{
		PYXTHROW(	CircularIntersectionException, 
					"Null index not a valid centre index for test."	);
	}
	m_pyxIndex = pyxIndex;
	m_pProjection->pyxisToNative(m_pyxIndex, &m_centreCoord);
}

/*! 
Set the radius of the circle defined in metres along a great circle arc.
The radius should always be set after the target resolution is set.

\param fRadius	The radius in metres of the great circle arc that defines
				the circle geometry.
*/
void CircleIntersectionTest::setRadiusInMetres(double fRadius)
{
	if (fRadius < 0)
	{
		PYXTHROW(	CircularIntersectionException, 
					"Radius value in metres '" << fRadius << "' is invalid."	);
	}

	// convert the metres to an angle
	m_fRadius = fRadius / ReferenceSphere::kfRadius;
}

/*!
Set the radius of the circular geometry in radians. The radius should always 
be set after the target resolutionis set.

\param fAngle	The angle at the centre of the reference sphere that defines
				the radius of the circular geometry.
*/
void CircleIntersectionTest::setRadiusInRadians(double fAngle)
{
	if (fAngle < 0 || MathUtils::kf360Rad < fAngle)
	{
		PYXTHROW(	CircularIntersectionException, 
					"Radius angle '" << fAngle << "' is invalid."	);
	}
		
	assert(verifySize(fAngle) && "Defined radius is smaller than resolution.");
	m_fRadius = fAngle;
}

/*!
Test an individual index for inclusion in the circular geometry.

\param	index			The index to test.
\param	pbAbort			true to stop the test immediately, initialized to
						false on entry (out).

\return	The test result.
*/
PYXTest::eTestResult CircleIntersectionTest::doTestIndex(
												const PYXIcosIndex& index,
												bool* pbAbort	)
{
	// verify member variables
	assert(!m_pyxIndex.isNull());
	assert(!index.isNull());
	assert(0 < m_fRadius);

	bool bDataResolution = (index.getResolution() == getDataResolution());
	if (!bDataResolution && index.hasVertexChildren())
	{
		return PYXTest::knMaybe;
	}

	// count the number of vertices inside the circle
	eVertexResult nResult = calcIncludedVertices(index);

	switch (nResult)
	{
		case knNoVertices:
		{
			return PYXTest::knNo;
		}
		case knAllVertices:
		{
			return PYXTest::knYesComplete;
		}
		case knAreaContained:
		{
			return bDataResolution ? 
				PYXTest::knNo : PYXTest::knMaybe;
		}
		case knSomeVertices:
		{
			return bDataResolution ? 
				PYXTest::knYes : PYXTest::knMaybe;
		}
		default:
		{
			assert("Invalid return type");
			return PYXTest::knNo;
		}
	}
}

/*!
Calculate the distance of each vertex of the hexagon to the centre of the
circular geometry along a great circle arc. The method will return the number
of vertices of the cell that are smaller than the specified radius of the 
circular geometry.

\param pyxIndex	The index to test for cell vertex inclusion.

\return An indication of the number of vertices that are included in the geometry.
*/
CircleIntersectionTest::eVertexResult
CircleIntersectionTest::calcIncludedVertices(const PYXIcosIndex& pyxIndex)
{
	CoordLatLon coordCentre;
	m_pProjection->pyxisToNative(pyxIndex, &coordCentre);
	double distanceToCellCenter = GreatCircleArc::calcDistance(
														m_centreCoord,
														coordCentre	);

	double cellRadius = PYXIcosMath::UnitSphere::calcCellCircumRadius(pyxIndex);

	if (distanceToCellCenter-cellRadius > m_fRadius)
	{
		return knNoVertices;
	}

	if (distanceToCellCenter+cellRadius < m_fRadius)
	{
		return knAreaContained;
	}

	return knSomeVertices;
}

/*!
Compare the size of the defined area to the size of the cells at the 
cell resolution. 

\param fAngle	The proposed angle (radius) to examine for size against the
				data resolution hexagon size.

\return true if the radius of the area is larger than the circumradius of the 
		cells at the data resolution, otherwise false.
*/
bool CircleIntersectionTest::verifySize(double fAngle)
{
	if (!m_pyxIndex.isNull())
	{
		double fHexAngle;
		PYXIcosIndex pyxIndex = m_pyxIndex;
		pyxIndex.setResolution(getDataResolution());
		PYXVertexIterator itVertex(pyxIndex);

		// calculate the centre position
		CoordLatLon coordCentre;
		m_pProjection->pyxisToNative(pyxIndex, &coordCentre);

		// calculate the angle to each vertex
		CoordLatLon coordVertex;
		for (; !itVertex.end(); ++itVertex)
		{
			m_pProjection->pyxisToNative(itVertex.getIndex(), &coordVertex);
			fHexAngle = GreatCircleArc::calcDistance(coordCentre, coordVertex);
			if (fAngle < fHexAngle)
			{
				return false;
			}
		}
		return true;
	}

	return false;
}
