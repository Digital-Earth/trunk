/******************************************************************************
cache_map.cpp

begin		: 2011-02-08
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/cache_map.h"

#include "pyxis/utility/tester.h"

class CacheMapTester
{
public:
	static void test()
	{
		std::map<int,int> map;

		for(int i=1;i<=10;i++)
		{
			map[i] = i+1;
		}

		CacheMap<int,int> cacheMap(map);

		TEST_ASSERT(cacheMap.size() == map.size());
		TEST_ASSERT(cacheMap.size() == cacheMap.maxSize());

		//Test we have all items in the map
		for(std::map<int,int>::iterator it = map.begin();it!=map.end();++it)
		{
			TEST_ASSERT(cacheMap[it->first] == it->second);
		}

		TEST_ASSERT(cacheMap.exists(1));
		TEST_ASSERT(!cacheMap.exists(11));

		TEST_ASSERT(*cacheMap.getLeastUsedKey()==1);
		TEST_ASSERT(*cacheMap.getLastUsedKey()==10);

		//Add new item - it should remove the least used key from the map
		cacheMap[11] = 12;

		TEST_ASSERT(!cacheMap.exists(1));
		TEST_ASSERT(cacheMap.exists(11));

		int key = *cacheMap.getLeastUsedKey();
		TEST_ASSERT(cacheMap[key] == map[key]);
		TEST_ASSERT(*cacheMap.getLeastUsedKey() != key);

		//add new item - remove the new least used key
		cacheMap[12] = 13;

		TEST_ASSERT(cacheMap.exists(key));
	}
};


Tester<CacheMapTester> gTester;