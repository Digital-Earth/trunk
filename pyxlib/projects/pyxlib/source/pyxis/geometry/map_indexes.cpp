/******************************************************************************
map_indexes.cpp

begin		: 2009-11-16
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "map_indexes.h"

#include <iostream>
#include <iomanip>
#include "limits.h"

using namespace std;

MapIndexes::MapIndexes( int resolution ) : resolution(resolution)
{
	assert(resolution > 0 && resolution <= 39); // 39 should be max resolution
}

tMapPyxisIndex& MapIndexes::operator[]( const string &pyxisIndexString )
{
	ToMapIndex(pyxisIndexString);
	assert(MAP_FACE(pyxisIndexString[0]) < sizeof(indexes) / sizeof(indexes[0]));
	return indexes[MAP_FACE(pyxisIndexString[0])][string(faceless)];
}

void MapIndexes::Add(string const &pyxisIndexString, void *pi)
{
	ToMapIndex(pyxisIndexString);
	assert(MAP_FACE(pyxisIndexString[0]) < sizeof(indexes) / sizeof(indexes[0]));
	tMapIndexes &mi = indexes[MAP_FACE(pyxisIndexString[0])];
	mi[string(faceless)].push_back(pi);
}

void MapIndexes::ToMapIndex( string const &pyxisIndexString )
{
	int offset = 3;
	int nextPosition = 0;
	for (string::const_iterator  Ichr = pyxisIndexString.begin()+1; Ichr != pyxisIndexString.end(); Ichr++)
	{
		unsigned char bits = TOBIT(*Ichr);
		if (offset <= 3)
			faceless[nextPosition] = bits << (CHAR_BIT - offset);
		else if (offset <= CHAR_BIT)
			faceless[nextPosition] |=  bits << (CHAR_BIT - offset);
		else
		{
			faceless[nextPosition] |= bits >> (offset - CHAR_BIT);
			faceless[++nextPosition] = bits << (CHAR_BIT - (offset -= CHAR_BIT));
		}
		offset += 3;
	}
	if (faceless[nextPosition] == 0)
		faceless[nextPosition] = 1;
	faceless[++nextPosition] = 0;
}

