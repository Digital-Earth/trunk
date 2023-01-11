/******************************************************************************
multi_geometry.cpp

begin		: 2006-05-30
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/multi_geometry.h"

// pyxlib includes
#include "pyxis/geometry/cell.h"
#include "pyxis/utility/tester.h"

/*
For ease of testing, test a container of PYXCells. In practice the more
efficient PYXMultiCell is used when a container of multiple cells is required.
*/

//! Tester class
Tester< PYXMultiGeometry<PYXCell> > gTester;

//! Specialization for test method
template<> static void PYXMultiGeometry<PYXCell>::test()
{
	// verify a multi cell can be created on the base class
	PYXMultiGeometry<PYXGeometry> testMulti;

	// test an empty multi-cell
	PYXMultiGeometry<PYXCell> emptyMultiCell;

	TEST_ASSERT(emptyMultiCell.isEmpty());
	TEST_ASSERT(emptyMultiCell.getCellResolution() < 0);

	{
		// test an empty iterator
		PYXPointer<PYXIterator> spIt(emptyMultiCell.getIterator());
		TEST_ASSERT(spIt->end());
		PYXIcosIndex index = spIt->getIndex();
		TEST_ASSERT(index.isNull());
	}

	{
		// test copying and equality
		PYXPointer< PYXMultiGeometry<PYXCell> > spCopy = PYXMultiGeometry<PYXCell>::create(emptyMultiCell);
		assert(0 != spCopy);
		TEST_ASSERT(spCopy->isEmpty());
		TEST_ASSERT(emptyMultiCell == *spCopy);

		// test intersection
		PYXPointer<PYXGeometry> spGeometry(emptyMultiCell.intersection(*spCopy));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!emptyMultiCell.intersects(*spCopy));
	}

	// test a multi-cell with values
	PYXIcosIndex index1 = "A-0";
	PYXIcosIndex index2 = "B-0";
	PYXIcosIndex index3 = "C-0";
	PYXIcosIndex index4 = "D-0";
	PYXIcosIndex index5 = "E-0";
	PYXIcosIndex index6 = "F-0";

	PYXMultiGeometry<PYXCell> multiCell;
	PYXPointer<PYXCell> spCell2(PYXCell::create(index2));
	multiCell.addGeometry(spCell2);
	PYXPointer<PYXCell> spCell3(PYXCell::create(index3));
	multiCell.addGeometry(spCell3);
	PYXPointer<PYXCell> spCell1(PYXCell::create(index1));
	multiCell.addGeometry(spCell1);
	TEST_ASSERT(!multiCell.isEmpty());
	TEST_ASSERT(multiCell.getCellResolution() == index1.getResolution());

#if 0 // ordering is no longer maintained in the geometry
	{
		// test iterator and sorting
		PYXPointer<PYXIterator> spIt(multiCell.getIterator());
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(spIt->getIndex() == index1);
		spIt->next();
		TEST_ASSERT(spIt->getIndex() == index2);
		spIt->next();
		TEST_ASSERT(spIt->getIndex() == index3);
		spIt->next();
		TEST_ASSERT(spIt->end());
		PYXIcosIndex index = spIt->getIndex();
		TEST_ASSERT(index.isNull());
	}
#endif

#if 0 // TODO mlepage disabled this on 2008-03-28 until the tests can be fixed
	{
		// test copying and equality
		PYXPointer< PYXMultiGeometry<PYXCell> > spCopy = PYXMultiGeometry<PYXCell>::create(multiCell);
		assert(0 != spCopy);
		TEST_ASSERT(!spCopy->isEmpty());
		TEST_ASSERT(multiCell == *spCopy);

		// test intersection with the copy
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(*spCopy));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			TEST_ASSERT(multiCell == dynamic_cast<const PYXMultiGeometry<PYXCell>&>(*spGeometry));
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}
		TEST_ASSERT(!emptyMultiCell.intersects(*spCopy));
	}

	{
		// test intersection with the empty set
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(emptyMultiCell));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!multiCell.intersects(emptyMultiCell));
	}

	{
		// test intersection with an intersecting cell
		PYXCell cell;
		cell.setIndex(index1);
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(cell));
		TEST_ASSERT(!spGeometry->isEmpty());
		try
		{
			const PYXMultiGeometry<PYXCell>& multiCell = dynamic_cast<const PYXMultiGeometry<PYXCell>&>(*spGeometry);
			PYXPointer<PYXIterator> spIt = multiCell.getIterator();
			TEST_ASSERT(!spIt->end());
			if (!spIt->end())
			{
				TEST_ASSERT(spIt->getIndex() == index1);
				spIt->next();
				TEST_ASSERT(spIt->end());
			}
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}
		TEST_ASSERT(multiCell.intersects(cell));
	}
#endif

	{
		// test intersection with a non-intersecting cell
		PYXCell cell;
		cell.setIndex(index4);
		PYXPointer<PYXGeometry> spGeometry(multiCell.intersection(cell));
		TEST_ASSERT(spGeometry->isEmpty());
		TEST_ASSERT(!multiCell.intersects(cell));
	}

	{
		// test non-intersection with another multicell
		PYXMultiGeometry<PYXCell> multiCell2;
		PYXPointer<PYXCell> spCell5(PYXCell::create(index5));
		multiCell2.addGeometry(spCell5);
		PYXPointer<PYXCell> spCell6(PYXCell::create(index6));
		multiCell2.addGeometry(spCell6);
		PYXPointer<PYXCell> spCell4(PYXCell::create(index4));
		multiCell2.addGeometry(spCell4);
		TEST_ASSERT(!multiCell2.isEmpty());
		TEST_ASSERT(multiCell2.getCellResolution() == index1.getResolution());
		TEST_ASSERT(!multiCell.intersects(multiCell2));
		TEST_ASSERT(!multiCell2.intersects(multiCell));
	}

	{
		// test intersection with another multicell
		PYXMultiGeometry<PYXCell> multiCell2;
		PYXPointer<PYXCell> spCell5(PYXCell::create(index5));
		multiCell2.addGeometry(spCell5);
		PYXPointer<PYXCell> spCell6(PYXCell::create(index6));
		multiCell2.addGeometry(spCell6);
		PYXPointer<PYXCell> spCell2(PYXCell::create(index2));
		multiCell2.addGeometry(spCell2);
		TEST_ASSERT(!multiCell2.isEmpty());
		TEST_ASSERT(multiCell2.getCellResolution() == index1.getResolution());
		TEST_ASSERT(multiCell.intersects(multiCell2));
		TEST_ASSERT(multiCell2.intersects(multiCell));
	}

	// empty the geometry and test
	multiCell.clear();
	TEST_ASSERT(multiCell.isEmpty());
}
