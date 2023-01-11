/******************************************************************************
CompressIndex.h

begin		: 2009-10-27
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#pragma once

// pyxlib includes
// #include "pyxlib.h"
// #include "pyxis/derm/index.h"

// standard includes
#include <string>
#include <vector>
#include <string>
#include "map_indexes.h"
#include "assert.h"
using namespace std ;


//! Pyxis indexes
class COriginalIndexes : public vector<string>
{
public:
	static string encode(const string current);
	void verifyIndexes(CompressedPyxisIndex indexes);
	void verifyIndexes(CIndexEntryTable indexes);
};
