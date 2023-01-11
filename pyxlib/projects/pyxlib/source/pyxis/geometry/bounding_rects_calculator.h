#ifndef BOUNDING_RECTS_CALCULATOR_H
#define BOUNDING_RECTS_CALCULATOR_H
/******************************************************************************
bounding_rects_calculator.h

begin		: 2007-12-17
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/pointer.h"
#include "pyxis/utility/rect_2d.h"

// forward declarations
class PYXCell;
class ICoordConverter;
class PYXGeometry;
class PYXIcosIndex;

/*!
This is a helper class that is used to construct a bounding box (or two if we span the day/date line)
for a set of Pyxis indices.  It needs a coordinate convertor, and the geometry of the region to function.
At the moment it only works 100% for contiguous "well behaved" geometries.

IMPORTANT LIMITATION: The first implementation of this class only works for coodinate systems that go 
from -180 to 180 and -90 to 90.
*/
//! Get a rectangular bounds from a set of points.
class PYXLIB_DECL PYXBoundingRectsCalculator
{
public:
	//! Test method
	static void test();

	//! Constructor.
	PYXBoundingRectsCalculator(const ICoordConverter* coordConvertor, 
		PYXGeometry const & geometry);

	//! Adds an index (a single point) to the bounds.
	void addIndex (const PYXIcosIndex& index);

	//! Adds a cell (center and vertices) to the bounds.
	void addCell (const PYXCell& cell);

	//! Get the results.  
	void getBoundingRects(
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2,
		double fMinLon = -180.0) const;

	//! Split a rectangle r into two parts when it spans a given longitude.
	static void splitBounds(
		const PYXRect2DDouble& r,
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2,
		double fMinLon = -180.0);

private:
	//! Add a point in the native coordinates to the bounding rectangle.
	void addPoint (const PYXCoord2DDouble& point);

	//! Returns true if we have a geometry and it geometry contains the north pole.
	bool containsNorthPole() const;

	//! Returns true if we have a geometry and it geometry contains the south pole.
	bool containsSouthPole() const;

	/*! The geometry of the area that we are trying to convert into a 
	    rectangular bounds.  For instance, if two cells are added
		to the bounds at (-170, 10) and (170, -10) we could be looking at a 20X20
		rectangle that straddles the day/date line, or a 340X20 retangle that doesn't.
		To determine which one we have, we will ask if the middle point of each 
		possible rectangle is in the supplied geometry.  Without the geometry we would
		not do any processing on the result, and would get out the 340X20 rectangle.

		We also do special processing using the geometry to help around the poles.
		We check for each pole being in the geometry and if it is we will include 
		the entire top (or bottom) of the world as our bounds.  In the case of the 
		north pole being included in the geometry, we would return a rectangle that is
		(-180, 0) to (180, the southern most latitude we have).  Again, without the
		geometry, we would return probably garbage.
	*/
	//! The geometry that represents the area to be bounded.
	const PYXGeometry& m_geometry;

	//! The coordinate convertor for finding native coordinates.
	const ICoordConverter* m_pCoordConverter;

	//! The bounds around the set of cells that have been added so far.
	PYXRect2DDouble m_bounds;
};

#endif //end guard