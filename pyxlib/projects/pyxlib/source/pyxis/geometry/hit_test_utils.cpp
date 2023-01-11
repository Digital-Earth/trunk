/******************************************************************************
hit_test_utils.cpp

begin		: 2008-04-24
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/hit_test_utils.h"

// pyxlib includes
#include "pyxis/derm/icosahedron.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/sub_index_math.h"

// standard includes
#include <cfloat>

////////////////////////////////////////////////////////////////////////////////

const double HitTestUtils::kfEps = DBL_EPSILON;
const double HitTestUtils::kfExpIR = 1/1.13;
const double HitTestUtils::kfExpCR = 1.13;

static const double radiusScale[] = { 1.0 /* hex */, sqrt(5.0/6.0) /* pent */ };

////////////////////////////////////////////////////////////////////////////////

namespace { Tester<HitTestUtils> gTester; }

void HitTestUtils::test()
{
	PYXCoord3DDouble C;
	SnyderProjection::getInstance()->pyxisToXYZ(PYXIcosIndex("M-0"), &C);

	// M-0 is definitely in M, right near the center.
	PYXCoord3DDouble A = C;
	TEST_ASSERT(HitTestUtils::intersectPointCellCertain(A, C, 1, false) == true);

	// Although 5-40005 is definitely in M, it's close to the edge.
	SnyderProjection::getInstance()->pyxisToXYZ(PYXIcosIndex("5-40005"), &A);
	TEST_ASSERT(HitTestUtils::intersectPointCellCertain(A, C, 1, false) == false);
}

////////////////////////////////////////////////////////////////////////////////

/*!
We can already tell exactly which cell a point is in, simply by converting to
PYXIS. However, due to projection distortion, we can't be sure whether a line
segment between two points that are within a cell is itself fully contained by
the cell. The purpose of this test is to quickly determine whether points are
far enough inside the cell that we *can* be sure the line segment between them
is also fully contained by the cell, even in the presence of this distortion.
*/
bool HitTestUtils::intersectPointCellCertain(	const PYXCoord3DDouble& A,
												const PYXCoord3DDouble& C,
												int nRes,
												bool bPent	)
{
	// Note this is a direct distance from C to A, not a projected distance along a normal.
	// This may have a slight effect due to the sphere's curvature.
	double D = C.distance(A);

	double IR = kfExpIR * radiusScale[bPent] * PYXMath::calcInRadius(nRes) * Icosahedron::kfCentralAngle - kfEps; // expanded inradius

	return D <= IR;
}

/*!
This uncertain test tries to determine sure hits, sure misses, and cases where
we are unsure whether a hit or miss occurred.

The test is against a line segment between points, not against the end points in
particular. (We already can test those for sure by simply using
SnyderProjection.)

For a line segment between points A and B, this is what we are dealing with:

<pre>
             ^ N
             |
             |
    A --->---+---<--- B
	    NxA     BxN
</pre>

\param A		The first point on the line segment.
\param B		The last point on the line segment.
\param N		The normal (AxB) of the line segment.
\param C		The center point of the cell.
\param nRes		The resolution of the cell.
\param bPent	Whether the cell is a pentagon.
\return knMiss, knUnsure, knHit as appropriate.
*/
int HitTestUtils::intersectSegmentCellUncertain(	const PYXCoord3DDouble& A,
													const PYXCoord3DDouble& B,
													const PYXCoord3DDouble& N,
													const PYXCoord3DDouble& C,
													int nRes,
													bool bPent)
{
	// Assume the line segment is less than half a great circle,
	// and ensure we are on the same half of the sphere. If we don't
	// do this, we'll pick up points on the other side of the sphere.
	PYXCoord3DDouble P;
	P.set(A.x() + B.x(), A.y() + B.y(), A.z() + B.z());
	P.normalize();
	if (P.dot(C) < 0)
	{
		return knMiss;
	}

	// First project onto the line segment's normal. By comparing the
	// distance, we will know if we intersect with the line segment's
	// great circle.

	double D = N.dot(C); // distance from line segment along its normal

	double IR = kfExpIR * radiusScale[bPent] * PYXMath::calcInRadius(nRes) * Icosahedron::kfCentralAngle - kfEps; // expanded inradius
	double CR = kfExpCR * radiusScale[bPent] * PYXMath::calcCircumRadius(nRes) * Icosahedron::kfCentralAngle + kfEps; // expanded circumradius

	if (-CR <= D && D <= CR)
	{
		// We are on the great circle, but we might be beyond the line
		// segment's end points. We will know by comparing against
		// normals which point from the end points (A and B) toward each
		// other. Note that this simple test will introduce false positives
		// due to "squaring" the thick line caps; we could do a slower
		// more accurate test afterwards using Euclidean distance.
		double DA = N.cross(A).dot(C); // distance from A towards B
		double DB = B.cross(N).dot(C); // distance from B towards A
		if (-CR <= DA && -CR <= DB)
		{
			if (-IR <= D && D <= IR &&
				-IR <= DA && -IR <= DB)
			{
				return knHit;
			}
			else
			{
				return knUnsure;
			}
		}
	}

	return knMiss;
}
