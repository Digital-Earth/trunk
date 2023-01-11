#ifndef PYXIS__GEOMETRY__HIT_TEST_UTILS_H
#define PYXIS__GEOMETRY__HIT_TEST_UTILS_H
/******************************************************************************
hit_test_utils.h

begin		: 2008-04-24
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/coord_3d.h"
#include "pyxis/utility/tester.h"

// forward declarations
class PYXIterator;

/*!
Various utilties for hit testing line segments against PYXIS cells.
*/
//! Utilties for hit testing.
class PYXLIB_DECL HitTestUtils
{
public:

	static void test(); // unit test

public:

	enum
	{
		knMiss,
		knUnsure,
		knHit
	};

	//! Epsilon for tests.
	static const double kfEps;

	//! Expansion factor for cell inradius (to account for distortion due to Snyder projection).
	static const double kfExpIR;

	//! Expansion factor for cell cirumradius (to account for distortion due to Snyder projection).
	static const double kfExpCR;

public:

	static bool intersectPointCellCertain(	const PYXCoord3DDouble& A,
											const PYXCoord3DDouble& C,
											int nRes,
											bool bPent	);

	static bool intersectPointCellCertain(	const PYXCoord3DDouble& A,
											const PYXIcosIndex cell		)
	{
		PYXCoord3DDouble C;
		SnyderProjection::getInstance()->pyxisToXYZ(cell, &C);
		return intersectPointCellCertain(A, C, cell.getResolution(), cell.isPentagon());
	}

	//! Intersect a line segment with a cell allowing for uncertainty.
	static int intersectSegmentCellUncertain(		const PYXCoord3DDouble& A,
													const PYXCoord3DDouble& B,
													const PYXCoord3DDouble& N,
													const PYXCoord3DDouble& C,
													int nRes,
													bool bPent	);

	static int intersectSegmentCellUncertain(		const PYXCoord3DDouble& A,
													const PYXCoord3DDouble& B,
													const PYXCoord3DDouble& N,
													const PYXIcosIndex cell		)
	{
		PYXCoord3DDouble C;
		SnyderProjection::getInstance()->pyxisToXYZ(cell, &C);
		return intersectSegmentCellUncertain(A, B, N, C, cell.getResolution(), cell.isPentagon());
	}

};

#endif // guard
