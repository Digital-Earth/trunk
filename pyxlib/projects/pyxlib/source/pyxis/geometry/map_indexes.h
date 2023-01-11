/******************************************************************************
map_indexes.h

begin		: 2009-11-16
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#pragma once

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/curve.h"

// standard includes
#include <map>
#include <vector>
#include <string>

typedef std::vector<void *> tMapPyxisIndex;
typedef std::map<std::string, tMapPyxisIndex> tMapIndexes;

#define TOBIT(c) c - '0' + 1
#define MAP_FACE(f) (f >= PYXIcosIndex::kcFaceFirstChar && f <= PYXIcosIndex::kcFaceLastChar ? \
	f - PYXIcosIndex::kcFaceFirstChar + PYXIcosIndex::knLastVertex : f - '1')

//! An map of indexes at a specific resolution
class MapIndexes 
{
	//! compress index without the face value 0-6 are encoded as octal 1-7
	char faceless[knMaxDigits/3+2];
	unsigned int resolution;
	tMapIndexes indexes[MAP_FACE(PYXIcosIndex::kcFaceLastChar)+1];
	void ToMapIndex(std::string const &pyxisIndexString);
public:
	/*!
	Construct a map index at a specific resolution.
	No checks are made on the resolution it is provided as a service to the caller.
	\param resolution
	\return ()
	*/
	MapIndexes(int resolution);
	/*!
	Get the resolution 
	\return (int)
	*/
	int Level() {return resolution; }
	/*!
	Add an index to the map.
	\param pyxisIndexString
	\param pi
	\return (void)
	*/
	void MapIndexes::Add(const std::string &pyxisIndexString, void *pi);
	/*!
	Retrieve a set of Pyxis indexes at a specific index.
	Side effect: Set the faceless field 
	\param pyxisIndexString
	\return (tMapPyxisIndex&)
	*/
	tMapPyxisIndex& operator[]( const std::string &pyxisIndexString);

#ifndef NDEBUG
	void ShowMapInfo() 
	{
		for (int i = 0; i < MAP_FACE(PYXIcosIndex::kcFaceLastChar)+1; i++)
		{
			std::cerr << "123456789:;<ABCDEFGHIJKLMNOPQRST????"[i];
			std::cerr << '=' << i << ':' << indexes[i].size() << '\n';
		}
	}

	const std::string GetFaceless()
	{
		#define TODIGIT(v, sh) ((v - '0') << sh)
		std::string binary, result;
		for (unsigned int i = 0; i<strlen(faceless); i++)
		{
			for (int j = CHAR_BIT; --j >= 0; )
				binary = binary + ((faceless[i] >> j) & 1 ? '1' : '0');
		}
		for (unsigned int rx = 0; rx + 3 < binary.length() && result.length() < resolution; rx += 3)
		{
			result += '0' - 1 + (TODIGIT(binary[rx], 2) |  TODIGIT(binary[rx+1], 1) | TODIGIT(binary[rx+2], 0));
		}
		#undef TODIGIT
		return result;
	}
#endif
};
