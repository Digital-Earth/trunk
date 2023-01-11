/******************************************************************************
combined_index.h

begin		: 2009-11-16
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#pragma once

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/curve.h"

// standard includes
#include <cassert>
#include <string>
#include <vector>

class CombinedIndexEntryTable;
class CombinedPyxisIndex;


//! Compressed Pyxis indexes
class CombinedPyxisIndex 
{
	std::string compressedIndex;

public:

	//! Test method
	static void test();

	/*!
	Construct a compress index using lat, long
	for all the resolutions excluding the high resolution
	\param ll Lat Long
	\param lowResolution
	\param highResolution
	*/
	CombinedPyxisIndex(const CoordLatLon& ll,
		int lowResolution,
		int highResolution);

	/*!
	The length of the compressed index string
	\return (unsigned int)
	*/
	unsigned int getLength() const { return compressedIndex.length(); }

	std::string toString() const;

	int getMaximumIndexLevel() const;

	std::string extractPyxisIndex(const int resolution) const;
};

std::ostream& operator<<(std::ostream& out, const CombinedPyxisIndex& ix);
std::istream& operator>>(std::istream& input, CombinedPyxisIndex& ix);
