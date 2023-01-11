/******************************************************************************
tile_collection.cpp

begin		: 2004-11-16
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/inner_tile_intersection_iterator.h"

// pyxlib includes
#include "pyxis/derm/hexagon.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/pentagon.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/geometry/exceptions.h"
#include "pyxis/geometry/polygon.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"

// standard includes
#include <algorithm>
#include <iomanip>
#include <set>
#include <sstream>

//! Tester class
Tester<PYXTileCollection> gTester;

//! Test method
void PYXTileCollection::test()
{
#if NDEBUG // Performance tests.  These take more than a moment to run, and are only useful in release.
	// Serialize a large polygon.
	{
		PYXTileCollection tc;
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));

		// Initialize.
		{
			int nRes = 16;
			PYXPolygon poly;
			{
				WGS84CoordConverter cc;
				PYXIcosIndex index;
				cc.nativeToPYXIS(PYXCoord2DDouble(30, 30), &index, nRes); poly.addVertex(index);
				cc.nativeToPYXIS(PYXCoord2DDouble(40, 40), &index, nRes); poly.addVertex(index);
				cc.nativeToPYXIS(PYXCoord2DDouble(50, 25), &index, nRes); poly.addVertex(index);
				poly.closeRing();
			}
			poly.copyTo(&tc);
		}

		// Intersection and intersects.
		{
			int nRes = 16;
			PYXPolygon poly;
			{
				WGS84CoordConverter cc;
				PYXIcosIndex index;
				cc.nativeToPYXIS(PYXCoord2DDouble(30, 30), &index, nRes); poly.addVertex(index);
				cc.nativeToPYXIS(PYXCoord2DDouble(40, 40), &index, nRes); poly.addVertex(index);
				cc.nativeToPYXIS(PYXCoord2DDouble(40, 25), &index, nRes); poly.addVertex(index);
				poly.closeRing();
			}
			PYXTileCollection tc2;
			poly.copyTo(&tc2);

			PYXPointer<PYXGeometry> spGeometry;
			{
				clock_t nStart = clock();
				const int nCount = 100;
				for (int i = 0; i < nCount; ++i)
				{
					spGeometry = tc.intersection(tc2);
				}
				double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
				TRACE_TEST("PYXTileCollection intersection " << nCount << " times: " << std::setprecision(2) << fSeconds << " seconds.");
			}
			{
				clock_t nStart = clock();
				const int nCount = 10000;
				for (int i = 0; i < nCount; ++i)
				{
					TEST_ASSERT(spGeometry->intersects(tc));
				}
				double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
				TRACE_TEST("PYXTileCollection intersects tc " << nCount << " times: " << std::setprecision(2) << fSeconds << " seconds.");
			}
			{
				clock_t nStart = clock();
				const int nCount = 10000;
				for (int i = 0; i < nCount; ++i)
				{
					TEST_ASSERT(spGeometry->intersects(tc2));
				}
				double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
				TRACE_TEST("PYXTileCollection intersects tc2 " << nCount << " times: " << std::setprecision(2) << fSeconds << " seconds.");
			}
		}

		// Serialize.
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);

			clock_t nStart = clock();
			out << tc;
			double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
			TRACE_TEST("PYXTileCollection serialization to " << strPath << ": " << std::setprecision(2) << fSeconds << " seconds.");
		}

		// Query.
		{
			clock_t nStart = clock();
			for (PYXTileSet::Iterator it(*tc.m_spTileSet); !it.end(); ++it)
			{
				TEST_ASSERT(tc.m_spTileSet->contains(*it));
			}
			double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
			TRACE_TEST("PYXTileCollection containment queries: " << std::setprecision(2) << fSeconds << " seconds.");
		}

		PYXTileCollection tcLoaded;

		// Deserialize.
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			clock_t nStart = clock();
			in >> tcLoaded;
			double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
			TRACE_TEST("PYXTileCollection deserialization from " << strPath << ": " << std::setprecision(2) << fSeconds << " seconds.");
		}

		// Compare to original.
		{
			clock_t nStart = clock();
			TEST_ASSERT(tcLoaded == tc);
			double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
			TRACE_TEST("PYXTileCollection comparison: " << std::setprecision(2) << fSeconds << " seconds.");
		}
	}

	// Serialize a big tile collection.
	{
		const bool bAggregate = false;

		PYXTileCollection tcOriginal(bAggregate);
		const int nResolution = 11;
		{
			clock_t nStart = clock();
			for (PYXIcosIterator it(nResolution); !it.end(); it.next())
			{
				tcOriginal.addTile(PYXTile::create(it.getIndex(), nResolution));
			}
			double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
			TRACE_TEST("PYXTileCollection 2 creation: " << std::setprecision(2) << fSeconds << " seconds.");
		}

		// Serialize.
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
			clock_t nStart = clock();
			out << tcOriginal;
			double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
			TRACE_TEST("PYXTileCollection 2 serialization to " << strPath << ": " << std::setprecision(2) << fSeconds << " seconds.");
		}

		// Deserialize.
		PYXTileCollection tcLoaded(bAggregate);
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			clock_t nStart = clock();
			in >> tcLoaded;
			double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
			TRACE_TEST("PYXTileCollection 2 deserialization from " << strPath << ": " << std::setprecision(2) << fSeconds << " seconds.");
		}

		// Compare to original.
		{
			clock_t nStart = clock();
			TEST_ASSERT(tcLoaded == tcOriginal);
			double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
			TRACE_TEST("PYXTileCollection 2 comparison: " << std::setprecision(2) << fSeconds << " seconds.");
		}
	}
#endif

	PYXTileCollection tileCollection;

	// basic tests on empty collection
	TEST_ASSERT(tileCollection.isCollection());
	TEST_ASSERT(tileCollection.isEmpty());
	TEST_ASSERT(tileCollection.getGeometryCount() == 0);

	PYXIcosIndex indexA("A-0");
	PYXIcosIndex indexB("B-0");
	const int knTestResolution = 10;

	// test add of single tile
	tileCollection.addTile(indexA, knTestResolution);

	// basic tests on collection of one tile
	TEST_ASSERT(!tileCollection.isEmpty());
	TEST_ASSERT(tileCollection.getGeometryCount() == 1);

	// test add of duplicate tile
	PYXPointer<PYXTile> spTile = PYXTile::create(indexA, knTestResolution);
	tileCollection.addTile(spTile);
	TEST_ASSERT(tileCollection.getGeometryCount() == 1);

	// test add of tile with different cell resolution
	TEST_ASSERT_EXCEPTION(tileCollection.addTile(indexB, knTestResolution + 1), PYXException);

	// test sorting
	tileCollection.addTile(indexB, knTestResolution);
	TEST_ASSERT(tileCollection.getGeometryCount() == 2);

	PYXPointer<PYXTileCollectionIterator> spIt(tileCollection.getTileIterator());
	TEST_ASSERT(!spIt->end());

	spTile = spIt->getTile();
	TEST_ASSERT(spTile != 0);
	TEST_ASSERT(spTile->getRootIndex() == PYXIcosIndex("A"));
	TEST_ASSERT(spTile->getCellResolution() == knTestResolution);

	spIt->next();
	TEST_ASSERT(!spIt->end());

	spTile = spIt->getTile();
	TEST_ASSERT(spTile != 0);
	TEST_ASSERT(spTile->getRootIndex() == PYXIcosIndex("B"));
	TEST_ASSERT(spTile->getCellResolution() == knTestResolution);
	TEST_ASSERT(spTile->intersects((const PYXGeometry&)PYXCell(spTile->getRootIndex())));
	TEST_ASSERT(spTile->intersects((const PYXGeometry&)*spTile));
	try
	{
		PYXCell cell(indexB);
		PYXPointer<PYXGeometry> spIntersection = spTile->intersection((const PYXGeometry&)cell);
		TEST_ASSERT(cell == dynamic_cast<const PYXCell&>(*spIntersection));
		TEST_ASSERT(spTile->intersects((const PYXGeometry&)cell));
	}
	catch (...)
	{
		TEST_ASSERT(false && "Incorrect type.");
	}

	spIt->next();
	TEST_ASSERT(spIt->end());

	tileCollection.clear();
	TEST_ASSERT(tileCollection.isEmpty());
	TEST_ASSERT(tileCollection.getGeometryCount() == 0);

	// Test intersects
	{
		PYXTileCollection t1;
		t1.addTile(PYXIcosIndex("1-02"), 15);
		t1.addTile(PYXIcosIndex("1-03"), 15);
		t1.addTile(PYXIcosIndex("3-02"), 15);
		t1.addTile(PYXIcosIndex("3-03"), 15);

		PYXTileCollection t2;
		t2.addTile(PYXIcosIndex("2-0200"), 14);
		t2.addTile(PYXIcosIndex("3-0200"), 14);
		t2.addTile(PYXIcosIndex("3-0400"), 14);
		t2.addTile(PYXIcosIndex("4-0200"), 14);

		PYXTileCollection t3;
		t3.addTile(PYXIcosIndex("1-040000"), 16);
		t3.addTile(PYXIcosIndex("2-040000"), 16);
		t3.addTile(PYXIcosIndex("3-040000"), 16);
		t3.addTile(PYXIcosIndex("4-040000"), 16);

		TEST_ASSERT(t1.intersects(t2));
		TEST_ASSERT(t2.intersects(t1));
		TEST_ASSERT(t1.intersects((const PYXGeometry&)t2));
		TEST_ASSERT(t2.intersects((const PYXGeometry&)t1));
		TEST_ASSERT(!(t1.intersection(t2)->isEmpty()));
		TEST_ASSERT(!(t2.intersection((const PYXGeometry&)t1)->isEmpty()));

		TEST_ASSERT(!t1.intersects(t3));
		TEST_ASSERT(!t3.intersects(t1));
		TEST_ASSERT(!t1.intersects((const PYXGeometry&)t3));
		TEST_ASSERT(!t3.intersects((const PYXGeometry&)t1));

		TEST_ASSERT(t2.intersects(t3));
		TEST_ASSERT(t3.intersects(t2));
		TEST_ASSERT(t2.intersects((const PYXGeometry&)t3));
		TEST_ASSERT(t3.intersects((const PYXGeometry&)t2));

		{
			PYXTile tile(PYXIcosIndex("3-040000"), 16);
			PYXPointer<PYXGeometry> spIntersection = t3.intersection((const PYXGeometry&)tile);
			try
			{
				PYXTile& intersection = dynamic_cast<PYXTile&>(*spIntersection);
				TEST_ASSERT(!intersection.isEmpty());
				TEST_ASSERT(intersection == tile);
			}
			catch (...)
			{
				TEST_ASSERT(false && "Incorrect type.");
			}

			PYXCell cell(PYXIcosIndex("3-040000000000000"));
			TEST_ASSERT(t3.intersects((const PYXGeometry&)cell));
			PYXPointer<PYXGeometry> spCellIntersection = cell.intersection((const PYXGeometry&)t3);
			try
			{
				PYXCell& intersection = dynamic_cast<PYXCell&>(*spCellIntersection);
				TEST_ASSERT(!intersection.isEmpty());
				TEST_ASSERT(intersection == cell);
			}
			catch (...)
			{
				TEST_ASSERT(false && "Incorrect type.");
			}
		}

		// Each of these tests for intersection of a tile at a lower
		// and higher cell resolution than the tile collection.
		{
			PYXTile tile(PYXIcosIndex("3-0400"), 13);
			TEST_ASSERT(!t1.intersects(tile));
			TEST_ASSERT(t2.intersects(tile));
			TEST_ASSERT(t3.intersects(tile));
			TEST_ASSERT(!t1.intersects((const PYXGeometry&)tile));
			TEST_ASSERT(t2.intersects((const PYXGeometry&)tile));
			TEST_ASSERT(t3.intersects((const PYXGeometry&)tile));
		}
		{
			PYXTile tile(PYXIcosIndex("3-0400"), 17);
			TEST_ASSERT(!t1.intersects(tile));
			TEST_ASSERT(t2.intersects(tile));
			TEST_ASSERT(t3.intersects(tile));
			TEST_ASSERT(!t1.intersects((const PYXGeometry&)tile));
			TEST_ASSERT(t2.intersects((const PYXGeometry&)tile));
			TEST_ASSERT(t3.intersects((const PYXGeometry&)tile));
		}
		{
			PYXTile tile(PYXIcosIndex("1-0"), 13);
			TEST_ASSERT(t1.intersects(tile));
			TEST_ASSERT(!t2.intersects(tile));
			TEST_ASSERT(t3.intersects(tile));
			TEST_ASSERT(((const PYXGeometry&)t1).intersects(tile));
			TEST_ASSERT(!((const PYXGeometry&)t2).intersects(tile));
			TEST_ASSERT(((const PYXGeometry&)t3).intersects(tile));
			TEST_ASSERT(!tile.intersection((const PYXGeometry&)tile)->isEmpty());
		}
		{
			PYXTile tile(PYXIcosIndex("1-0"), 17);
			TEST_ASSERT(t1.intersects(tile));
			TEST_ASSERT(!t2.intersects(tile));
			TEST_ASSERT(t3.intersects(tile));
			TEST_ASSERT(((const PYXGeometry&)t1).intersects((const PYXGeometry&)tile));
			TEST_ASSERT(!((const PYXGeometry&)t2).intersects((const PYXGeometry&)tile));
			TEST_ASSERT(((const PYXGeometry&)t3).intersects((const PYXGeometry&)tile));
		}
		{
			PYXTile tile(PYXIcosIndex("3-02"), 13);
			TEST_ASSERT(t1.intersects(tile));
			TEST_ASSERT(t2.intersects(tile));
			TEST_ASSERT(!t3.intersects(tile));
		}
		{
			PYXTile tile(PYXIcosIndex("3-02"), 17);
			TEST_ASSERT(t1.intersects(tile));
			TEST_ASSERT(t2.intersects(tile));
			TEST_ASSERT(!t3.intersects(tile));
		}
		{
			PYXTile tile(PYXIcosIndex("2-03"), 13);
			TEST_ASSERT(!t1.intersects(tile));
			TEST_ASSERT(!t2.intersects(tile));
			TEST_ASSERT(!t3.intersects(tile));
		}
		{
			PYXTile tile(PYXIcosIndex("2-03"), 17);
			TEST_ASSERT(!t1.intersects(tile));
			TEST_ASSERT(!t2.intersects(tile));
			TEST_ASSERT(!t3.intersects(tile));
		}

		// Each of these tests for intersection of a tile with the same cell
		// resolution as the tile collection.
		{
			PYXTile tile(PYXIcosIndex("3-0"), 15);
			TEST_ASSERT(t1.intersects(tile));
		}
		{
			PYXTile tile(PYXIcosIndex("3-020"), 15);
			TEST_ASSERT(t1.intersects(tile));
		}
		{
			PYXTile tile(PYXIcosIndex("3-0"), 14);
			TEST_ASSERT(t2.intersects(tile));
		}
		{
			PYXTile tile(PYXIcosIndex("3-040000"), 14);
			TEST_ASSERT(t2.intersects(tile));
		}
		{
			PYXTile tile(PYXIcosIndex("3-0"), 16);
			TEST_ASSERT(t3.intersects(tile));
		}
		{
			PYXTile tile(PYXIcosIndex("3-04000000"), 16);
			TEST_ASSERT(t3.intersects(tile));
		}
	}

	{
		// Test intersection.
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("M-02"), 10);
		tc.addTile(PYXIcosIndex("A-02"), 10);
		tc.addTile(PYXIcosIndex("R-02"), 10);
		tc.addTile(PYXIcosIndex("C-02"), 10);
		tc.addTile(PYXIcosIndex("C-03"), 10);
		tc.addTile(PYXIcosIndex("C-04"), 10);
		TEST_ASSERT(tc.getGeometryCount() == 6);

		// Table of test cases.
		// Each test case contains the query tile root index, then zero or more result root indices, then a null terminator.
		// The test cases are themselves terminated by a null.
		const int knMaxIntersection = 3;
		const char* const testTable[][1 + knMaxIntersection + 1] =
		{
			{ "1-02", 0 },
			{ "A-0", "A-02", 0 },
			{ "A-01", 0 },
			{ "A-02", "A-02", 0 },
			{ "A-0203", "A-0203", 0 },
			{ "A-06", 0 },
			{ "B-0", 0 },
			{ "C-0", "C-02", "C-03", "C-04", 0 },
			{ "C-01", 0 },
			{ "C-02", "C-02", 0 },
			{ "C-0203", "C-0203", 0 },
			{ "C-03", "C-03", 0 },
			{ "C-0304", "C-0304", 0 },
			{ "C-04", "C-04", 0 },
			{ "C-0405", "C-0405", 0 },
			{ "C-06", 0 },
			{ "L-0", 0 },
			{ "M-0", "M-02", 0 },
			{ "M-01", 0 },
			{ "M-02", "M-02", 0 },
			{ "M-0203", "M-0203", 0 },
			{ "M-06", 0 },
			{ "Q-0", 0 },
			{ "R-0", "R-02", 0 },
			{ "R-01", 0 },
			{ "R-02", "R-02", 0 },
			{ "R-06", 0 },
			{ "S-0", 0 },
			{ 0 } // terminate test cases
		};

		struct X
		{
			// Function for testing cases.
			static bool doTestCase(const PYXTileCollection& tc, const char* const indices[])
			{
				// Tile to query.
				PYXTile tile(PYXIcosIndex(*indices), 10);

				// Expected result.
				PYXTileCollection tc2;
				while (*++indices)
				{
					tc2.addTile(PYXIcosIndex(*indices), 10);
				}

				PYXPointer<PYXGeometry> spInt = tc.intersection(tile);

				// Perform the test. One of these lines must be true. (Excepting perverse code, exactly one line will be true.)
				bool bTest =
					(tc2.isEmpty() && boost::dynamic_pointer_cast<PYXEmptyGeometry, PYXGeometry>(spInt) && !tc.intersects(tile))
					|| (tc2.getGeometryCount() == 1 && *boost::dynamic_pointer_cast<PYXTile, PYXGeometry>(spInt) == *tc2.getTileIterator()->getTile() && tc.intersects(tile))
					|| (tc2.getGeometryCount() > 1 && *boost::dynamic_pointer_cast<PYXTileCollection, PYXGeometry>(spInt) == tc2 && tc.intersects(tile));
				TEST_ASSERT(bTest);
				if (!bTest)
				{
					TRACE_INFO("PYXTileCollection unit test case " << tile.getRootIndex() << " failed.");
				}
				return bTest;
			}
		};

		// Perform test cases.
		int nCase = 0;
		while (*testTable[nCase])
		{
			X::doTestCase(tc, testTable[nCase++]);
		}
	}

	{
		// Test union (disjunction).
		PYXTileCollection tc1;
		tc1.addTile(PYXIcosIndex("C-04"), 3);
		tc1.addTile(PYXIcosIndex("C-03"), 3);

		PYXTileCollection tc2;
		tc2.addTile(PYXIcosIndex("C-02"), 4);
		tc2.addTile(PYXIcosIndex("R-02"), 4);
		tc2.addTile(PYXIcosIndex("A-02"), 4);
		tc2.addTile(PYXIcosIndex("M-02"), 4);
		{
			PYXPointer<PYXGeometry> spResult = tc1.disjunction(tc2);
			TEST_ASSERT(0 != spResult);
			try
			{
				PYXTileCollection& tcResult = dynamic_cast<PYXTileCollection&>(*spResult);
				
				TEST_ASSERT(tcResult.getGeometryCount() == 6);
				TEST_ASSERT(tcResult.getCellResolution() == 4);
				TEST_ASSERT(tcResult.intersects(tc1));
				TEST_ASSERT(tcResult.intersects(tc2));
				
				// test the add geometry method
				tc1.addGeometry(tc2);
				TEST_ASSERT(tcResult.isEqual(tc1));

			}
			catch (...)
			{
				TEST_ASSERT(0 && "Should have been a tile collection.");
			}
		}
	}

	{
		// Test serialization
		PYXTileCollection t1;
		t1.addTile(PYXIcosIndex("1-02"), 15);
		t1.addTile(PYXIcosIndex("1-03"), 15);
		t1.addTile(PYXIcosIndex("3-02"), 15);
		t1.addTile(PYXIcosIndex("3-03"), 15);
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
			out << t1;
		}

		PYXTileCollection t1copy;
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			in >> t1copy;
		}

		TEST_ASSERT(t1copy == t1);
	}

	{
		// Test collection of resolution 1 roots
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("A"), PYXIcosIndex::knResolution1);
		tc.addTile(PYXIcosIndex("B"), PYXIcosIndex::knResolution1);
		tc.addTile(PYXIcosIndex("C"), PYXIcosIndex::knResolution1);
		tc.addTile(PYXIcosIndex("D"), PYXIcosIndex::knResolution1);
		tc.addTile(PYXIcosIndex("E"), PYXIcosIndex::knResolution1);
		tc.addTile(PYXIcosIndex("F"), PYXIcosIndex::knResolution1);
		tc.addTile(PYXIcosIndex("G"),  PYXIcosIndex::knResolution1);
		TEST_ASSERT(tc.getGeometryCount() == 7);
	}

	{
		// Test single tile aggregation.
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("B-0200"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0201"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0202"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0203"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0204"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0205"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0206"), knTestResolution);
		TEST_ASSERT(tc.getGeometryCount() == 1);
		PYXPointer<PYXTileCollectionIterator> tileIt = tc.getTileIterator();
		PYXPointer<PYXTile> spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("B-02") == spTile->getRootIndex());
	}

	{
		// Test pentagon in North Hemisphere.
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("1-0000"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0002"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0003"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0004"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0005"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0006"), knTestResolution);
		PYXPointer<PYXTileCollectionIterator> tileIt = tc.getTileIterator();
		PYXPointer<PYXTile> spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("1-000") == spTile->getRootIndex());
	}

	{
		// Test pentagon in South Hemisphere.
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("12-0000"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0001"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0002"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0003"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0005"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0006"), knTestResolution);
		TEST_ASSERT(tc.getGeometryCount() == 1);
		PYXPointer<PYXTileCollectionIterator> tileIt = tc.getTileIterator();
		PYXPointer<PYXTile> spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("12-000") == spTile->getRootIndex());
	}

	{
		// Test multiple aggregations (same resolution)
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("B-0200"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0201"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0202"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0002"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0003"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0203"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0204"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0000"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0001"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0205"), knTestResolution);
		tc.addTile(PYXIcosIndex("B-0206"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0000"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0004"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0005"), knTestResolution);
		tc.addTile(PYXIcosIndex("1-0006"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0002"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0003"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0005"), knTestResolution);
		tc.addTile(PYXIcosIndex("12-0006"), knTestResolution);
		TEST_ASSERT(tc.getGeometryCount() == 3);
		PYXPointer<PYXTileCollectionIterator> tileIt = tc.getTileIterator();
		PYXPointer<PYXTile> spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("1-000") == spTile->getRootIndex());
		tileIt->next();
		spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("12-000") == spTile->getRootIndex());
		tileIt->next();
		spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("B-02") == spTile->getRootIndex());
	}

	{
		// Test aggregation to resolution 1 pentagon
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("1-0"), 2);
		tc.addTile(PYXIcosIndex("1-2"), 2);
		tc.addTile(PYXIcosIndex("1-3"), 2);
		tc.addTile(PYXIcosIndex("1-4"), 2);
		tc.addTile(PYXIcosIndex("1-5"), 2);
		tc.addTile(PYXIcosIndex("1-6"), 2);
		TEST_ASSERT(tc.getGeometryCount() == 1);
		PYXPointer<PYXTileCollectionIterator> tileIt = tc.getTileIterator();
		PYXPointer<PYXTile> spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("1") == spTile->getRootIndex());
	}

	{
		// Test aggregation to resolution 1 hexagon
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("M-0"), 2);
		TEST_ASSERT(tc.getGeometryCount() == 1);
		PYXPointer<PYXTileCollectionIterator> tileIt = tc.getTileIterator();
		PYXPointer<PYXTile> spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("M") == spTile->getRootIndex());
	}

	{
		// Test multi-level aggregation
		PYXTileCollection tc;
		tc.addTile(PYXIcosIndex("M-00"), 6);
		tc.addTile(PYXIcosIndex("M-0100"), 6);
		tc.addTile(PYXIcosIndex("M-0102"), 6);
		tc.addTile(PYXIcosIndex("M-0103"), 6);
		tc.addTile(PYXIcosIndex("M-0104"), 6);
		tc.addTile(PYXIcosIndex("M-0105"), 6);
		tc.addTile(PYXIcosIndex("M-0106"), 6);
		tc.addTile(PYXIcosIndex("M-02"), 6);
		tc.addTile(PYXIcosIndex("M-03"), 6);
		tc.addTile(PYXIcosIndex("M-04"), 6);
		tc.addTile(PYXIcosIndex("M-05"), 6);
		tc.addTile(PYXIcosIndex("M-06"), 6);
		TEST_ASSERT(tc.getGeometryCount() == 12); // shouldn't have aggregated yet
		tc.addTile(PYXIcosIndex("M-0101"), 6);
		TEST_ASSERT(tc.getGeometryCount() == 1); // should have aggregated
		PYXPointer<PYXTileCollectionIterator> tileIt = tc.getTileIterator();
		PYXPointer<PYXTile> spTile = tileIt->getTile();
		TEST_ASSERT(PYXIcosIndex("M") == spTile->getRootIndex());
	}
}

/*!
Constructor initializes member variables.
*/
PYXTileCollection::PYXTileCollection(bool bAutoAggregate) :
	m_spTileSet(new PYXTileSet()),
	m_bAutoAggregate(bAutoAggregate)
{
}

/*!
Copy constructor.
*/
PYXTileCollection::PYXTileCollection(const PYXTileCollection& source) :
	m_spTileSet(new PYXTileSet(*(source.m_spTileSet))), // m_spTileSet is never null
	m_bAutoAggregate(source.m_bAutoAggregate)
{
}

/*!
Copy constructor.
*/
PYXTileCollection::PYXTileCollection(const PYXTileCollection& source, bool bAutoAggregate) :
	m_spTileSet(new PYXTileSet((source.m_spTileSet)->resolution())), // m_spTileSet is never null
	m_bAutoAggregate(bAutoAggregate)
{
	assert(0 != source.m_spTileSet.get());
	for (PYXTileSet::Iterator iTiles(*source.m_spTileSet); !iTiles.end(); ++iTiles)
	{
		m_spTileSet->insert(*iTiles, bAutoAggregate);
	}
}

/*!
Deserialization constructor.
*/
PYXTileCollection::PYXTileCollection(std::basic_istream< char>& in) :
	m_spTileSet(),
	m_bAutoAggregate(true)
{
	deserialize(in);
	assert(0 != m_spTileSet.get());
}

//! Destructor.
PYXTileCollection::~PYXTileCollection()
{
}

/*!
Create a copy of the geometry.

\return a copy of the geometry (ownership transferred).
*/
PYXPointer<PYXGeometry> PYXTileCollection::clone() const
{
	return create(*this);
}

//! Serialize the instance.
void PYXTileCollection::serialize(std::basic_ostream< char>& out) const
{
	m_spTileSet->serialize(out);
}

//! Deserialize the instance.
void PYXTileCollection::deserialize(std::basic_istream< char>& in)
{
	// Create the new tile set from the stream.
	m_spTileSet.reset(new PYXTileSet(in));

	// Set resolution.
	const int nResolution = m_spTileSet->resolution();
	setCellResolution(nResolution);
	assert(getCellResolution() == nResolution);
}

/*!
Is the geometry empty.

\return	true if the geometry is empty (no cells) or false otherwise.
*/
//! Determine if a collection has any tiles
bool PYXTileCollection::isEmpty() const
{
	assert(0 != m_spTileSet.get());
	return m_spTileSet->empty();
}

//! Clear all entries in the collection.
void PYXTileCollection::clear()
{
	assert(0 != m_spTileSet.get());
	m_spTileSet->clear();
}

//! Get the PYXIS resolution of cells in the geometry.
int PYXTileCollection::getCellResolution() const
{
	assert(0 != m_spTileSet.get());
	return m_spTileSet->resolution();
}

/*!
Get the intersection of this geometry and the specified geometry.

\param	geometry		The specified geometry.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection geometry.
*/
PYXPointer<PYXGeometry> PYXTileCollection::intersection(
	const PYXGeometry& geometry,
	bool bCommutative	) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return intersection(*pCell);
	}

	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (0 != pTile)
	{
		return intersection(*pTile);
	}

	const PYXTileCollection* const pTileCollection = 
		dynamic_cast<const PYXTileCollection*>(&geometry);
	if (0 != pTileCollection)
	{
		return intersection(*pTileCollection);
	}

	return PYXGeometry::intersection(geometry, bCommutative);
}

/*!
Get the intersection of this geometry and the specified geometry.

\param	collection	The specified tile collection.

\return	The intersection geometry.
*/
PYXPointer<PYXGeometry> PYXTileCollection::intersection(const PYXTileCollection& collection) const
{
	if (&collection == this)
	{
		return collection.clone();
	}

	PYXPointer<PYXTileCollection> spCollection = PYXTileCollection::create();

	int nResolution = std::max(getCellResolution(), collection.getCellResolution());

	assert(0 != m_spTileSet.get());
	assert(0 != collection.m_spTileSet.get());
	for (PYXTileSet::Iterator iTiles(*m_spTileSet); iTiles; ++iTiles)
	{
		if (collection.m_spTileSet->contains(*iTiles))
		{
			spCollection->addTile(*iTiles, nResolution);
		}
	}
	for (PYXTileSet::Iterator iTiles(*collection.m_spTileSet); iTiles; ++iTiles)
	{
		if (m_spTileSet->contains(*iTiles))
		{
			spCollection->addTile(*iTiles, nResolution);
		}
	}

	if (!spCollection->isEmpty())
	{
		return spCollection;
	}

	return PYXEmptyGeometry::create();
}

/*!
Get the intersection of this geometry and the specified geometry.

\param	tile	The specified tile.

\return	The intersection geometry.
*/
PYXPointer<PYXGeometry> PYXTileCollection::intersection(const PYXTile& tile) const
{
	int nResolution = std::max(getCellResolution(), tile.getCellResolution());

	assert(0 != m_spTileSet.get());
	if (m_spTileSet->contains(tile.getRootIndex()))
	{
		// Return a clone of the tile, with resolution set to the maximum of the two.
		PYXPointer<PYXGeometry> spTile = tile.clone();
		spTile->setCellResolution(nResolution);
		return spTile;
	}

	PYXTileCollection tc;
	tc.addTile(tile.getRootIndex(), nResolution);
	PYXPointer<PYXGeometry> spCollection = intersection(tc);

	const PYXTileCollection* const pCollection = 
		dynamic_cast<const PYXTileCollection*>(&*spCollection);
	if (0 != pCollection)
	{
		PYXTileCollection::Iterator iTiles(*pCollection);
		if (iTiles.end())
		{
			// If the intersection is an empty tile collection,
			// the intersection method should have returned a PYXEmptyGeometry instance.
			assert(0);
			return PYXEmptyGeometry::create();
		}
		PYXPointer<PYXTile> spTile = PYXTile::create(*iTiles, nResolution);
		if ((++iTiles).end())
		{
			return spTile;
		}
	}
	return spCollection;
}

/*!
Get the intersection of this geometry and the specified geometry.

\param	cell	The specified cell.

\return	The intersection geometry.
*/
PYXPointer<PYXGeometry> PYXTileCollection::intersection(const PYXCell& cell) const
{
	PYXPointer<PYXTileCollection> spCollection = PYXTileCollection::create();

	// Cycle through each of the tiles in the collection
	for (Iterator it(*this); !it.end(); ++it)
	{
		PYXPointer<PYXTile> spTile = PYXTile::create(*it, getCellResolution());
		assert(0 != spTile);

		PYXPointer<PYXGeometry> spIntersection = spTile->intersection(cell);
		assert(0 != spIntersection);
		if (!spIntersection->isEmpty())
		{
			return spIntersection;
		}
	}

	return PYXEmptyGeometry::create();
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXTileCollection::intersects(	const PYXGeometry& geometry,
									bool bCommutative	) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return intersects(*pCell);
	}

	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (0 != pTile)
	{
		return intersects(*pTile);
	}

	const PYXTileCollection* const pTileCollection = 
		dynamic_cast<const PYXTileCollection*>(&geometry);
	if (0 != pTileCollection)
	{
		return intersects(*pTileCollection);
	}

	return PYXGeometry::intersects(geometry, bCommutative);
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param collection	The tile collection to intersect with this one.
 
\return true if any intersection exists or false if none is found.
*/
bool PYXTileCollection::intersects(const PYXTileCollection& collection) const
{
	if (&collection == this)
	{
		return true;
	}

	// rely on fact that tiles are sorted to optimize test
	Iterator it1(*this);
	Iterator it2(collection);

	// cycle through each of the tiles in the collection
	while (!it1.end() && !it2.end())
	{
		PYXPointer<const PYXTile> spTile1 = PYXTile::create(*it1, getCellResolution());
		PYXPointer<const PYXTile> spTile2 = PYXTile::create(*it2, collection.getCellResolution());

		if (spTile1->intersects(*spTile2))
		{
			return true;
		}

		// Advance correct iterator by comparing index values (not pointers)
		if (*it1 < *it2)
		{
			++it1;
		}
		else
		{
			++it2;
		}
	}

	return false;
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param tile		The tile to intersect with this tile collection.
 
\return true if any intersection exists or false if none is found.
*/
bool PYXTileCollection::intersects(const PYXTile& tile) const
{
	return m_spTileSet->intersects(tile.getRootIndex());
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param cell		The cell to intersect with this tile collection.
 
\return true if any intersection exists or false if none is found.
*/
bool PYXTileCollection::intersects(const PYXCell& cell) const
{
	return m_spTileSet->intersects(cell.getIndex());
}
/*!
Determine if this geometry has any intersection with the specified geometry.

\param cell		The cell to intersect with this tile collection.

\return true if any intersection exists or false if none is found.
*/
bool PYXTileCollection::intersects(const PYXIcosIndex& cell) const
{
	return m_spTileSet->intersects(cell);
}


bool PYXTileCollection::contains(const PYXGeometry& geometry) const
{
	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (0 != pCell)
	{
		return m_spTileSet->contains(pCell->getIndex());
	}

	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (0 != pTile)
	{
		return m_spTileSet->contains(pTile->getRootIndex());
	}

	const PYXTileCollection* const pTileCollection = 
		dynamic_cast<const PYXTileCollection*>(&geometry);
	if (0 != pTileCollection)
	{
		return contains(*pTileCollection);
	}

	const PYXVectorGeometry2* const vectorGeometry2 = 
		dynamic_cast<const PYXVectorGeometry2*>(&geometry);

	if (0 != vectorGeometry2)
	{
		//check if we contains the low resolution version of the geometry
		int resolution = PYXBoundingCircle::estimateResolutionFromRadius(vectorGeometry2->getBoundingCircle().getRadius());

		PYXTileCollection tcLowRes;
		geometry.copyTo(&tcLowRes,resolution);

		if (contains(tcLowRes))
		{
			//yes we are
			return true;
		}
		if (!intersects(tcLowRes))
		{
			//we not even inerests this geometry - return false
			return false;
		}

		//TODO: we should realy allways use getCellResolution()+2 - but that can be to much work.
		//we need to make it iterative process
		if (resolution+11 < PYXMath::knMaxAbsResolution)
		{
			resolution+=11;
		}
		else
		{
			resolution = PYXMath::knMaxAbsResolution-1;
		}

		//no need to make to high resolution of that geometry
		if (resolution > getCellResolution()+2)
		{
			resolution = getCellResolution()+2;
		}

		//create better resolution of the geometry
		PYXTileCollection tcHighRes;
		geometry.copyTo(&tcHighRes,resolution);
		return geometry.contains(tcHighRes);
	}

	PYXTileCollection tc2;
	geometry.copyTo(&tc2);
	return contains(tc2);
}

/*!
Determine whether this geometry completely contains the specified geometry.

\param collection	The cell to intersect with this tile collection.
 
\return true if this geometry completely contains the specified geometry, otherwise false.
*/
bool PYXTileCollection::contains(const PYXTileCollection& collection) const
{
	for (Iterator it(collection); !it.end(); ++it)
	{
		if(!m_spTileSet->contains(*it))
		{
			return false;
		}
	}

	return true;
}

/*!
Get the disjunction (union) of this geometry and the specified geometry as a new
geometry.

\param	geometry		The geometry to unite with this one.

\return	The disjunction (union) of geometries.
*/
PYXPointer<PYXGeometry> PYXTileCollection::disjunction(const PYXGeometry& geometry) const
{
	PYXTileCollection collection;
	geometry.copyTo(&collection);
	assert(0 != this);
	return collection.disjunction(*this);
}

//! Return the number of geometries in the collection.
/*!
Get the number of geometries in this collection.

\return	The number of geometries in this geometry.
*/
int PYXTileCollection::getGeometryCount() const
{
	assert(0 != m_spTileSet.get());
	size_t nCount = m_spTileSet->count();
	assert(nCount <= INT_MAX);
	return static_cast<int>(nCount);
}

/*!
Get the disjunction (union) of this tile collection and the specified tile collection as a new
geometry.

\param	tileCollection	The tile collection to unite with this geometry.

\return	The disjunction (union) of geometries.
*/
PYXPointer<PYXGeometry> PYXTileCollection::disjunction(const PYXTileCollection& tileCollection) const
{
	// Create the new tile collection from the one passed in.
	PYXPointer<PYXTileCollection> spResult = PYXTileCollection::create(tileCollection);
	assert(0 != spResult);
	assert(0 != spResult->m_spTileSet.get());
	assert(tileCollection == *spResult);

	// If the resolutions are different, set to the highest (for consistency with intersection).
	if (spResult->getCellResolution() < getCellResolution())
	{
		spResult->setCellResolution(getCellResolution());
	}

	// Insert the tile set.
	spResult->m_spTileSet->insert(*m_spTileSet, getAutoAggregate());

	return spResult;
}

/*!
Add a tile to the collection.

\param	spTile	The tile.
*/
void PYXTileCollection::addTile(PYXPointer<PYXTile> spTile)
{
	assert((spTile != 0) && "Invalid pointer.");
	addTile(spTile->getRootIndex(), spTile->getCellResolution());
}

/*!
Add a tile to the collection.
Check to ensure that the tile doesn't already exist before attempting to 
add it.

\param	index			The index of the tile.
\param	nCellResolution	The cell resolution of the tile.
*/
void PYXTileCollection::addTile(const PYXIcosIndex& index, int nCellResolution)
{
	assert(0 != m_spTileSet.get());
	assert(nCellResolution > 0);
	if (isEmpty())
	{
		// save cell resolution from first tile added
		m_spTileSet->setResolution(nCellResolution);
	}
	else
	{
		// check that the cell resolutions match
		if (nCellResolution != getCellResolution())
		{
			PYXTHROW(	PYXGeometryException,
						"Cell resolutions don't match: '" << getCellResolution() <<
						"' and '" << nCellResolution << "'."	);
		}
	}
	m_spTileSet->insert(index, getAutoAggregate());
}

//! Returns whether tiles are automatically aggregated.
bool PYXTileCollection::getAutoAggregate() const
{
	return m_bAutoAggregate;
}

/*!
Add a geometry to the collection. Convenience method.
Check to ensure that the tile doesn't already exist before attempting to 
add it.

\param	geometry		The geometry to add.
*/
void PYXTileCollection::addTile(const PYXGeometry& geometry)
{
	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (0 != pTile)
	{
		addTile(pTile->getRootIndex(), pTile->getCellResolution());
		return;
	}

	assert(0 && "Addition of non-tile types is currently unsupported.");
}

/*! 
Insert the passed geometry this tile collection. This would have 
the same result as the disjunction of these two collections. The
cell resolution of this tile collection will be set to the value
of the higher resolution of the two collections.

\param tc	The tile collection to add to this one.
*/
void PYXTileCollection::addGeometry(const PYXTileCollection& tc)
{
	// set the resolution
	if (getCellResolution() < tc.getCellResolution())
	{
		setCellResolution(tc.getCellResolution());
	}
	m_spTileSet->insert(*(tc.m_spTileSet.get()), getAutoAggregate());
}

/*!
Get an iterator to the geometries in this collection.

\return	An iterator to the geometries in this collection (ownership transferred).
*/
PYXPointer<PYXGeometryIterator>PYXTileCollection::getGeometryIterator() const
{
	return getTileIterator();
}

//! Get an iterator to the tiles in this collection.
PYXPointer<PYXTileCollectionIterator> PYXTileCollection::getTileIterator() const
{
	return PYXTileCollectionIterator::create(*this);
}

/*!
Set the cell resolution.
*/
void PYXTileCollection::setCellResolution(int nCellResolution)
{
	m_spTileSet->setResolution(nCellResolution);
}

/*! 
Calculate a series of PYXIS indices around a geometry.

\param pVecIndex	The container to hold the returned indices.
*/
void PYXTileCollection::calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const
{
	assert(pVecIndex != 0 && "Null pointer.");
	pVecIndex->clear();

	if (isEmpty())
	{
		return;
	}

	// Find a bounding cell for the entire tile collection.
	// TODO this doesn't work properly in cases where tiles cross
	// tessellations (e.g. "A-0" and "1-1") or in cases where the
	// tessellation is a top level pentagon (e.g. "1").
	Iterator it(*this);
	
	assert(!it.end());
	int nResolution = (*it).getResolution();

	PYXIcosIndex indexBounds = (nResolution == 1) ?
		*it : // Either root index is already bounds, or there is none anyways.
		PYXTile::create(*it, nResolution)->getBoundingCell().getIndex();

	for (++it; !it.end(); ++it)
	{
		nResolution = (*it).getResolution();

		const PYXIcosIndex index = (nResolution == 1) ?
			*it : // Either root index is already bounds, or there is none anyways.
			PYXTile::create(*it, nResolution)->getBoundingCell().getIndex();

		const PYXIcosIndex indexAncestor =
			PYXIcosMath::calcAncestorIndex(indexBounds, index);
		if (indexAncestor.isNull())
		{
			break;
		}
		indexBounds = indexAncestor;
	}

	// Copy bounds vertices to container.
	for (PYXVertexIterator itVertex(indexBounds); !itVertex.end(); itVertex.next())
	{
		pVecIndex->push_back(itVertex.getIndex());
	}
}

/*!
The copy assignment of the container class.  This method
will be used to perform a deep copy of all of the elements in the collection.

\param tileCollection	The tile collection to copy from.
*/
PYXTileCollection& PYXTileCollection::operator =(const PYXTileCollection& tileCollection)
{
	if (this != &tileCollection) 
	{
		m_spTileSet.reset(new PYXTileSet(*(tileCollection.m_spTileSet)));
		m_bAutoAggregate = tileCollection.m_bAutoAggregate;
	}

	return *this;
}

/*! 
Allows PYXIS tile collection to be written to streams.

\param out					The stream to write to.
\param pyxTileCollection	The tile collection to write to the stream.

\return The stream after the operation.
*/
std::basic_ostream< char>& operator <<(	std::basic_ostream< char>& out, 
							const PYXTileCollection& pyxTileCollection	)
{
	pyxTileCollection.serialize(out);
	return out;
}

/*!
Allows PYXIS tile collection to be read from streams.

\param input				The stream to read from.
\param pyxTileCollection	The tile collection to write to the stream.

\return The stream after the operation.
*/
std::basic_istream< char>& operator >>(	std::basic_istream< char>& input, 
							PYXTileCollection& pyxTileCollection	)
{
	pyxTileCollection.deserialize(input);
	return input;
}

/*!
Copies a representation of this geometry into a tile collection.

\param	pTileCollection	The destination tile collection (must not be null).
*/
void PYXTileCollection::copyTo(PYXTileCollection* pTileCollection) const
{
	assert(pTileCollection != 0);

	*pTileCollection = *this;
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution, aggregating, if necessary.

\param	pTileCollection		The destination tile collection (must not be null).
\param	nTargetResolution	The target resolution.
*/
void PYXTileCollection::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	if(nTargetResolution == getCellResolution())
	{
		copyTo(pTileCollection);
	}
	else
	{
		// aggregate, if necessary
		std::set<PYXIcosIndex> setIndices;

		pTileCollection->clear();

		for (PYXPointer<PYXTileCollectionIterator> spIt = this->getTileIterator();
			!spIt->end(); spIt->next())
		{
			PYXIcosIndex index = spIt->getTile()->getRootIndex();
			if (index.getResolution() > nTargetResolution)
			{
 				index.setResolution(nTargetResolution);
			}
			setIndices.insert(index);
		}

		std::set<PYXIcosIndex>::const_iterator it = setIndices.begin();
		for (; it != setIndices.end(); ++it)
		{
			pTileCollection->addTile(*it, nTargetResolution);
		}
	}
}

PYXBoundingCircle PYXTileCollection::getBoundingCircle() const
{
	PYXBoundingCircle circle;

	for (PYXPointer<PYXTileCollectionIterator> spIt = this->getTileIterator();!spIt->end(); spIt->next())
	{
		circle += spIt->getTile()->getBoundingCircle();
	}

	return circle;
}

//! The equality operator.
bool operator ==(const PYXTileCollection& lhs, const PYXTileCollection& rhs)
{
	return *(lhs.m_spTileSet) == *(rhs.m_spTileSet);
}

// PYXTileCollection::Iterator

/*!
Construct the iterator.

\param	tileCollection	The tile collection to iterate over. Altering
						this collection during iteration will cause
						undefined behavior in the iterator.
*/
PYXTileCollection::Iterator::Iterator(const PYXTileCollection& tileCollection) :
	m_tileSet(*(tileCollection.m_spTileSet)), // Never null
	m_apIterator(new PYXTileSet::Iterator(m_tileSet))
{
}

/*!
Move to the next tile in the collection.
*/
void PYXTileCollection::Iterator::next()
{
	assert(0 != m_apIterator.get());
	m_apIterator->next();
}

/*!
See if we have covered all the geometries.

\return	true if all geometries have been covered, otherwise false.
*/
bool PYXTileCollection::Iterator::end() const
{
	assert(0 != m_apIterator.get());
	return (m_apIterator->end());
}

/*!
Start the iteration at the beginning of the collection.
*/
void PYXTileCollection::Iterator::reset()
{
	m_apIterator.reset(new PYXTileSet::Iterator(m_tileSet));
}

//! Return the current item.  If past end, return the last item.
const PYXIcosIndex& PYXTileCollection::Iterator::operator *() const
{
	assert(0 != m_apIterator.get());
	return **m_apIterator;
}

// PYXTileCollectionIterator

/*!
Create a new instance of a PYXTileCollectionIterator. The iterator is
returned in a managed pointer. The iterator is not built to handle
changes in the originating tile collection. If the originating tile 
collection is changed the behavior is undefined.

\param	spTileCollection	The tile collection to iterate over. Altering
							this collection during iteration will cause
							undefined behavior in the iterator.
*/
PYXTileCollectionIterator::PYXTileCollectionIterator(
	const PYXTileCollection & tileCollection	)
	:
	m_collection(tileCollection),
	m_resolution(tileCollection.getCellResolution()),
	m_itCurrent(tileCollection)
{
}

/*!
Start the iteration at the beginning of the collection.
*/
void PYXTileCollectionIterator::reset()
{
	m_itCurrent.reset();
}

/*!
Move to the next tile in the collection.
*/
void PYXTileCollectionIterator::next()
{
	if (!end())
	{
		++m_itCurrent;
	}
}

/*!
See if we have covered all the geometries.

\return	true if all geometries have been covered, otherwise false.
*/
bool PYXTileCollectionIterator::end() const
{
	return (m_itCurrent.end());
}

/*!
Get a pointer to a new tile at the current position in the collection.

\return	The current tile in the iteration.
*/
PYXPointer<PYXGeometry> PYXTileCollectionIterator::getGeometry() const
{
	return getTile();
}

/*!
Create a new tile object that represents the current position in the iteration.

\return	The current tile.
*/
PYXPointer<PYXTile> PYXTileCollectionIterator::getTile() const
{
	if (end())
	{
		return PYXPointer<PYXTile>();
	}
	return PYXTile::create(*m_itCurrent, m_resolution);
}





///////////////////////////////////////////////////////////
// TileCollectionInnerTileIntersectionIterator 
///////////////////////////////////////////////////////////

class TileCollectionInnerTileIntersectionIterator : public PYXInnerTileIntersectionIterator
{
private:
	PYXTileSet::Iterator m_iterator;
	PYXInnerTile m_root;
	
	//m_partialChild is used to skip tiles that are deeper then the requested resolution
	PYXIcosIndex m_partialChild;

	std::vector<PYXInnerTile> m_innerTiles;
	unsigned int m_innerTileIndex;
	PYXInnerTileIntersection m_intersection;
	bool m_ended;

public:
	static PYXPointer<TileCollectionInnerTileIntersectionIterator> create(const PYXTileCollection & collection,const PYXInnerTile & innerTile)
	{
		return PYXNEW(TileCollectionInnerTileIntersectionIterator,collection,innerTile);
	}
	TileCollectionInnerTileIntersectionIterator(const PYXTileCollection & collection,const PYXInnerTile & innerTile) : m_root(innerTile), m_iterator(PYXTileSet::Iterator(*collection.m_spTileSet)), m_intersection(knIntersectionComplete)
	{
		m_ended = m_iterator.end();

		if (!end())
		{
			findFirstTile();
		}
	}

private:
	void createInnerTiles()
	{
		m_innerTileIndex = 0;
		if ((*m_iterator).getResolution() <= m_root.getCellResolution())
		{
			m_partialChild.reset();
			if ((*m_iterator).getResolution() >= m_root.asTile().getRootIndex().getResolution()) 
			{
				//m_iterator is descendant of m_root - iterate over it inner tiles
				m_innerTiles = PYXInnerTile::createInnerTiles(PYXTile(*m_iterator,m_root.getCellResolution()));
			}
			else 
			{
				//m_root is descendant of m_iterator - return m_root as the inner tile
				m_innerTiles = PYXInnerTile::createInnerTiles(m_root.asTile());
			}
		} 
		else 
		{
			m_partialChild = *m_iterator;
			m_partialChild.setResolution(m_root.getCellResolution());
			m_innerTiles = PYXInnerTile::createInnerTiles(PYXTile(m_partialChild,m_root.getCellResolution()));
		}
	}

	void findFirstTile()
	{
		while (!m_iterator.end() && !m_root.intersects(PYXCell(*m_iterator)))
		{
			m_iterator.next();
		}

		m_ended = m_iterator.end();

		if (!m_ended)
		{
			createInnerTiles();
		}
	}

public:
	virtual void next()
	{
		if (m_ended)
		{
			return;
		}

		m_innerTileIndex++;

		if (m_innerTileIndex < m_innerTiles.size())
		{
			return;
		}
		else 
		{
			m_innerTiles.clear();
			m_innerTileIndex=0;
		}


		if ((*m_iterator).isAncestorOf(m_root.getRootIndex()))
		{
			m_ended = true;
			return;
		}

		m_iterator.next();

		//skip tiles that are children of our partialChild 
		if (!m_partialChild.isNull())
		{
			while (!m_iterator.end() && m_partialChild.isAncestorOf(*m_iterator)) 
			{
				m_iterator.next();
			}
		}
		
		//continue until we finish all the tiles or got out of the m_root
		m_ended = m_iterator.end() || !m_root.intersects(PYXCell(*m_iterator));

		if (!m_ended)
		{
			createInnerTiles();
		}
	}

	virtual bool end() const
	{
		return m_ended;
	}

	virtual const PYXInnerTile & getTile() const
	{
		assert(m_root.intersects(m_innerTiles[m_innerTileIndex]));
		return m_innerTiles[m_innerTileIndex];
	}

	virtual const PYXInnerTileIntersection & getIntersection() const
	{
		return m_intersection;
	}
};

PYXPointer<PYXInnerTileIntersectionIterator> PYXTileCollection::getInnerTileIterator(const PYXInnerTile & tile) const
{
	return TileCollectionInnerTileIntersectionIterator::create(*this,tile);
}

void PYXTileCollection::limitCellsCountTo(int maxCellsCount)
{
	int originalResolution = getCellResolution();
	int resolution = std::min(5,originalResolution);
	PYXPointer<PYXTileCollection> originalTile = PYXTileCollection::create(*this);
	
	setCellResolution(resolution);

	while (getGeometryCount() < maxCellsCount && resolution < originalResolution)
	{
		resolution++;
		originalTile->copyTo(this,resolution);
	}

	while (getGeometryCount() > maxCellsCount)
	{
		resolution--;
		originalTile->copyTo(this,resolution);
	}
}

long PYXTileCollection::getCellCount() const
{
	PYXTileSet::Iterator iter(*m_spTileSet);
	long cellCount = 0;
	auto resolution = getCellResolution();
	while(!iter.end())
	{
		cellCount += PYXIcosMath::getCellCount(*iter, resolution);
		iter.next();
	}
	return cellCount;
}
	 
double PYXTileCollection::getAreaOnReferenceShpere() const
{
	PYXTileSet::Iterator iter(*m_spTileSet);
	long hexagonCount = 0;
	long pentagonCount = 0;
	while(!iter.end())
	{
		const PYXIcosIndex & index = *iter;
		if (index.isPentagon()) 
		{
			pentagonCount += PYXIcosMath::getCellCount(index,getCellResolution());
		}
		else 
		{
			hexagonCount += PYXIcosMath::getCellCount(index,getCellResolution());
		}
		iter.next();
	}

	PYXIcosIndex hexagonCell("M-0");
	PYXIcosIndex pentagonCell("1-0");

	hexagonCell.setResolution(getCellResolution());
	pentagonCell.setResolution(getCellResolution());

	return hexagonCount * SnyderProjection::getInstance()->calcCellAreaOnReferenceSphere(hexagonCell) +
		pentagonCount * SnyderProjection::getInstance()->calcCellAreaOnReferenceSphere(pentagonCell);
}