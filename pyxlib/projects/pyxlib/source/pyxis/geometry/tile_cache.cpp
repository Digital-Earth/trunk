/******************************************************************************
tile_cache.cpp

begin		: 2005-11-17
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "tile_cache.h"

// local includes 
#include "pyxis/utility/tester.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

//! Tester class
Tester< PYXTileCache<TestTile> > gCacheTester;

/*!
Test the functionality of the generic tile cache using a TestTile as the 
implementation for the cache.  NOTE: This method does not get called by the 
testing framework it is called explicitly by the TestTile class test method.
*/
void PYXTileCache<TestTile>::test()
{
	// Different tiles for testing
	PYXTile tile1(PYXIcosIndex("A-0"), 10);
	PYXTile tile2(PYXIcosIndex("B-0"), 10);
	PYXTile tile3(PYXIcosIndex("C-0"), 10);
	PYXTile tile4(PYXIcosIndex("D-0"), 10);
	PYXTile tile5(PYXIcosIndex("E-0"), 10);
	PYXTile tile6(PYXIcosIndex("F-0"), 10);

	// create a tile cache for testing
	PYXPointer<PYXTileCache<TestTile> > spTileCache = PYXTileCache<TestTile>::create();
	
	// add more tiles than the cache can handle
	int nTileCount = static_cast<int>(spTileCache->getMaxTileCount() * 1.25);
	for (nTileCount; nTileCount > 0; --nTileCount)
	{
		// if the cache is full it will free required resources
		PYXPointer<TestTile> spDisplayTile = TestTile::create(tile1);
		spTileCache->add(spDisplayTile);
		TEST_ASSERT(spTileCache->size() <= spTileCache->getMaxTileCount());
	}

	// force the cache to free tiles to change the size
	TEST_ASSERT(spTileCache->setMaxTileCount(PYXTileCache<TestTile>::knMinSize));

	// remove all tiles from the cache
	spTileCache->clearAllTiles();
	TEST_ASSERT(0 == spTileCache->size());

	/* 
	Create test display tiles at different times
	The ownership of these tiles is transferred to the tile cache when they
	are added.  The tile pointers are only being kept for tile comparison and
	will not be explicitly deleted.
	*/
	PYXPointer<TestTile> spTestTile = TestTile::create(tile5);
	spTileCache->add(spTestTile);
	spTestTile = TestTile::create(tile1);
	spTileCache->add(spTestTile);
	spTestTile.reset();
	
	// sleep for a second to segregate older tiles
	boost::xtime time;
	boost::xtime_get(&time, boost::TIME_UTC_);
	time.sec += 1;
	boost::thread::sleep(time);	

	{
		PYXPointer<TestTile> spTestTile1A = TestTile::create(tile1);
		spTileCache->add(spTestTile1A);
		PYXPointer<TestTile> spTestTile2 = TestTile::create(tile2);
		spTileCache->add(spTestTile2);
		PYXPointer<TestTile> spTestTile3 = TestTile::create(tile3);
		spTileCache->add(spTestTile3);
		PYXPointer<TestTile> spTestTile4 = TestTile::create(tile4);
		spTileCache->add(spTestTile4);
		PYXPointer<TestTile> spTestTile1B = TestTile::create(tile1);
		spTileCache->add(spTestTile1B);
		TEST_ASSERT(spTileCache->size() == 7);

		// query for a specific tile
		//TestTile::TestTilePtrList tileList;
		TestTile::TestTilePtrList tileList;
		TEST_ASSERT(tileList.size() == 0);

		// clear the oldest tiles (pTestTileR1 and pTestTileR2)
		TEST_ASSERT(spTileCache->clearTiles(2));
		TEST_ASSERT(spTileCache->size() == 5);
		TEST_ASSERT(0 == spTileCache->getTiles(tile5, &tileList)); 
		TEST_ASSERT(2 == spTileCache->getTiles(tile1, &tileList));
		TEST_ASSERT(tileList.size() == 2);
		tileList.clear();

		// get tile matching geometry for tile2
		TEST_ASSERT(1 == spTileCache->getTiles(tile2, &tileList));
		TEST_ASSERT(tileList.front() == spTestTile2);
		tileList.clear();

		// attempt to retrieve a tile that is not in the list
		TEST_ASSERT(0 == spTileCache->getTiles(tile5, &tileList));

		// retrieve and verify a multiple match
		TEST_ASSERT(2 == spTileCache->getTiles(tile1, &tileList));
		
		TEST_ASSERT(	tileList.front() == spTestTile1B ||
						tileList.front() == spTestTile1A	);
		TEST_ASSERT(	tileList.back() == spTestTile1B ||
						tileList.back() == spTestTile1A	);
		tileList.clear();
		
		// set one of the tiles to be expired
		spTestTile1A->getCacheStatus()->setState(CacheStatus::knExpired);
		TEST_ASSERT(1 == spTileCache->getTiles(tile1, &tileList));
		TEST_ASSERT(spTileCache->size() == 4);
		TEST_ASSERT(tileList.front() == spTestTile1B);
		tileList.clear();
	}

	// clear more tiles than are available in the cache
	TEST_ASSERT(!spTileCache->clearTiles(10));
	TEST_ASSERT(spTileCache->size() == 0);
}
