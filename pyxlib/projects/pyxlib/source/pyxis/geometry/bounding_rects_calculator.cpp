/******************************************************************************
bounding_rects_calculator.cpp

begin		: 2007-12-17
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/bounding_rects_calculator.h"

// pyxlib includes
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/snyder_projection.h" // TODO for temporary lat/lon swap
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/utility/tester.h"

//! Tester class
Tester<PYXBoundingRectsCalculator> gPYXBoundingRectsCalculator;

//! Test method
void PYXBoundingRectsCalculator::test()
{
	PYXRect2DDouble rect1;
	PYXRect2DDouble rect2;
	WGS84CoordConverter coordConverter;

	// spans the day/date -- should get two rects
	{
		PYXPointer<PYXCell> spTestCell = PYXCell::create(PYXIcosIndex("12-50"));
		PYXBoundingRectsCalculator test(&coordConverter, *spTestCell);
		test.addCell(*(spTestCell.get()));
		test.getBoundingRects(&rect1, &rect2);
		TEST_ASSERT(!rect1.empty());
		TEST_ASSERT(!rect2.empty());
	}

	// spans the day/date -- should get two rects
	{
		PYXPointer<PYXCell> spTestCell = PYXCell::create(PYXIcosIndex("11-60"));
		PYXBoundingRectsCalculator test(&coordConverter, *spTestCell);
		test.addCell(*(spTestCell.get()));
		test.getBoundingRects(&rect1, &rect2);
		TEST_ASSERT(!rect1.empty());
		TEST_ASSERT(!rect2.empty());
	}

	// does not span the day/date -- should get one rect
	{
		PYXPointer<PYXCell> spTestCell = PYXCell::create(PYXIcosIndex("4-60"));
		PYXBoundingRectsCalculator test(&coordConverter, *spTestCell);
		test.addCell(*(spTestCell.get()));
		test.getBoundingRects(&rect1, &rect2);
		TEST_ASSERT(!rect1.empty());
		TEST_ASSERT(rect2.empty());
	}

	double lastHeight = rect1.height();

	{
		PYXPointer<PYXCell> spTestCell = PYXCell::create(PYXIcosIndex("4-60603"));
		PYXBoundingRectsCalculator test(&coordConverter, *spTestCell);
		test.addCell(*(spTestCell.get()));
		test.getBoundingRects(&rect1, &rect2);
		TEST_ASSERT(!rect1.empty());
		TEST_ASSERT(rect2.empty());
	}

	// a higher res cell should create a smaller rectangle.
	TEST_ASSERT(lastHeight > rect1.height()); 
}

/*!
Constructor for a PYXBoundingRectsCalculator.

\param	pCoordConverter		A pointer to a coordinate convertor to use.  The client must
							ensure that the coordinate convertor lives as long as this
							calculator object, and the caller maintains ownership of the
							coordinate convertor.
\param geometry				A reference to the geometry that we are trying to convert
							into a bounds.  It is not used to create any of the extents,
							it is only is used for intersection tests.

*/
PYXBoundingRectsCalculator::PYXBoundingRectsCalculator(const ICoordConverter* pCoordConverter,
	PYXGeometry const & geometry) :
	m_pCoordConverter(pCoordConverter), m_geometry(geometry)
{
	assert((m_pCoordConverter != 0) && "Coordinate converter not available.");
}

/*
Include the point at the centre of a cell in the bounds.

\param index	The index of the cell.
*/
void PYXBoundingRectsCalculator::addIndex(const PYXIcosIndex& index)
{
	PYXCoord2DDouble xyPoint;
	if (!m_pCoordConverter->tryPyxisToNative(index, &xyPoint))
	{
		// index is out of coord convertor bounds.
		return;
	}

	// TODO 2008-02-05 mlepage SnyderProjection treats lat=x lon=y
	// so flip them here. This REALLY needs to be addressed and reconciled.
	if (m_pCoordConverter == SnyderProjection::getInstance())
	{
		double x = xyPoint.x();
		xyPoint.setX(xyPoint.y());
		xyPoint.setY(x);
	}

	addPoint(xyPoint);
}

/*
Include a cell in the bounds. The point at the centre of the cell and the points
of the cell's vertices are included.

\param cell	The cell.
*/
void PYXBoundingRectsCalculator::addCell(const PYXCell& cell)
{
	if (cell.isEmpty())
	{
		// No work required.
		return;
	}

	PYXIcosIndex cellIndex = cell.getIndex();

	// add the center
	addIndex(cellIndex);

	// add the vertex indices.
	PYXVertexIterator it(cellIndex);
	for (; !it.end(); it.next())
	{
		addIndex(it.getIndex());
	}
}

/*!
Get the bounding rectangles for the region. Rect2 will be empty unless bounds spans
fMinLon (by default the antimeridian), in which case two bounding rectangles are returned.

\param pRect1	The first rectangle (out)
\param pRect2	The second rectangle (out)
\param fMinLon	The longitude on which to split the region.
*/
void PYXBoundingRectsCalculator::getBoundingRects(
	PYXRect2DDouble* pRect1,
	PYXRect2DDouble* pRect2,
	double fMinLon) const
{
	assert((pRect1 != 0) && "Invalid argument.");
	assert((pRect2 != 0) && "Invalid argument.");

	// Clear incoming rectangles.
	pRect1->setEmpty();
	pRect2->setEmpty();

	bool overNorthPole = containsNorthPole();
	bool overSouthPole = containsSouthPole();

	if (overSouthPole || overNorthPole)
	{
		// cover the entire world left to right.
		pRect1->setXMin(-180.0);  // TODO: this isn't right for all coordinate systems.
		pRect1->setXMax(180.0); // TODO: this isn't right for all coordinate systems.

		// cover the range of points we got.
		pRect1->setYMin(m_bounds.yMin());  
		pRect1->setYMax(m_bounds.yMax());

		// extend to the south pole
		if (overSouthPole)
		{
			pRect1->setYMin(-90.00); // TODO: this isn't right for all coordinate systems.
		}

		// extend to the north pole
		if (overNorthPole)
		{
			pRect1->setYMax(90.00); // TODO: this isn't right for all coordinate systems.
		}
	}
	else
	{
		splitBounds(m_bounds, pRect1, pRect2, fMinLon);
	}
}

/*
Split bounding box (r) containing values in degrees longitude and latitude into two bounding boxes
(r1, r2) when r spans a given longitude. The resulting bounding box(es) have longitude values in
the range [fMinLon, fMinLon + 360]. This method is typically used to split bounding boxes that span
the international dateline (minLon = -180.0). The minLon parameter was added to accommodate data
sets with bounding boxes that extend slightly beyond the international dateline.

1. If r does not span split, r1 = r, r2 = empty
2. If r spans split, r1 is the portion of r in the range [minLon, minLon + 360]. r2 is
   the portion of r that falls outside [minLon, minLon + 360] but is then normalized to
   the range [minLon, minLon + 360].

\param rect		The original rectangle
\param pRect1	The first rectangle (out)
\param pRect2	The second rectangle (out)
\param fMinLon	The longitude on which to split
*/
void PYXBoundingRectsCalculator::splitBounds(
	const PYXRect2DDouble& r,
	PYXRect2DDouble* pRect1,
	PYXRect2DDouble* pRect2,
	double fMinLon)
{
	assert((r.xMin() < r.xMax() && r.xMin() > -360.0 && r.xMax() < 360.0) && "Invalid argument.");
	assert((pRect1 != 0) && "Invalid argument.");
	assert((pRect2 != 0) && "Invalid argument.");
	assert((fMinLon > -360.0 && fMinLon < 360.0) && "Invalid argument.");

	double fMaxLon = fMinLon + 360.0;

	if (r.xMin() < fMinLon)
	{
		pRect1->expand(r);
		pRect1->clip(PYXRect2DDouble(fMinLon, -90.0, fMaxLon, 90.0));

		pRect2->expand(r);
		pRect2->setXMax(pRect2->xMax() + 360.0);
		pRect2->setXMin(pRect2->xMin() + 360.0);
		pRect2->clip(PYXRect2DDouble(fMinLon, -90.0, fMaxLon, 90.0));
	}
	else if (r.xMax() > (fMaxLon))
	{
		pRect1->expand(r);
		pRect1->clip(PYXRect2DDouble(fMinLon, -90.0, fMaxLon, 90.0));

		pRect2->expand(r);
		pRect2->setXMax(pRect2->xMax() - 360.0);
		pRect2->setXMin(pRect2->xMin() - 360.0);
		pRect2->clip(PYXRect2DDouble(fMinLon, -90.0, fMaxLon, 90.0));
	}
	else
	{
		pRect1->expand(r);
		pRect2->setEmpty();
	}
}

/*
Does the geometry include the north pole.

return true if yes, otherwise false.
*/
bool PYXBoundingRectsCalculator::containsNorthPole() const
{
	PYXIcosIndex np1("E-060606060606060606060606060606060606");
	np1.setResolution(m_geometry.getCellResolution());
	if (m_geometry.intersects(PYXCell(np1)))
	{
		return true;
	}

	PYXIcosIndex np2("A-020202020202020202020202020202020202");
	np2.setResolution(m_geometry.getCellResolution());
	if (m_geometry.intersects(PYXCell(np2)))
	{
		return true;
	}

	PYXIcosIndex np3("1-202020202020202020202020202020202020");
	np3.setResolution(m_geometry.getCellResolution());
	if (m_geometry.intersects(PYXCell(np3)))
	{
		return true;
	}

	PYXIcosIndex np4("2-202020202020202020202020202020202020");
	np4.setResolution(m_geometry.getCellResolution());
	if (m_geometry.intersects(PYXCell(np4)))
	{
		return true;
	}

	return false;
}

/*
Does the geometry contain the south pole.

return true if yes, otherwise false.
*/
bool PYXBoundingRectsCalculator::containsSouthPole() const
{
	PYXIcosIndex sp1("12-101010101010101010101010101010101010");
	sp1.setResolution(m_geometry.getCellResolution());
	if (m_geometry.intersects(PYXCell(sp1)))
	{
		return true;
	}

	PYXIcosIndex sp2("9-505050505050505050505050505050505050");
	sp2.setResolution(m_geometry.getCellResolution());
	if (m_geometry.intersects(PYXCell(sp2)))
	{
		return true;
	}

	PYXIcosIndex sp3("Q-050505050505050505050505050505050505");
	sp3.setResolution(m_geometry.getCellResolution());
	if (m_geometry.intersects(PYXCell(sp3)))
	{
		return true;
	}

	PYXIcosIndex sp4("R-030303030303030303030303030303030303");
	sp4.setResolution(m_geometry.getCellResolution());
	if (m_geometry.intersects(PYXCell(sp4)))
	{
		return true;
	}

	return false;
}

/*
Expand the bounds (if required) to include a point.

\param	point	The point in native coordinates.
*/
void PYXBoundingRectsCalculator::addPoint(const PYXCoord2DDouble& point)
{
	PYXCoord2DDouble newPoint(point);
	if (!m_bounds.empty())
	{
		if (m_bounds.inside(point))
		{
			return;
		}

		if (m_bounds.xMax() > 90 && point.x() < -90)
		{
			newPoint.setX(point.x() + 360);
		}
		else if (m_bounds.xMin() < -90 && point.x() > 90)
		{
			newPoint.setX(point.x() - 360);
		}
	}

	m_bounds.expand(newPoint);
}

