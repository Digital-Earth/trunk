/******************************************************************************
map_indexes.h

begin		: 2009-10-27
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#pragma once

// pyxlib includes
//#include "pyxlib.h"
//#include "pyxis/derm/index.h"

// standard includes
#include <map>
#include <vector>
#include <string>
#include "assert.h"
#include <iostream>
using namespace std ;
class CIndexEntryTable;
class CompressedPyxisIndex;

//! Pyxis indexes
class CIndexSet : public vector<string>
{
public:
	CIndexEntryTable GatherKeys();
	void verifyIndexes(CompressedPyxisIndex indexes);
	void verifyIndexes(CIndexEntryTable indexes);
};

//!TESTING: only
class PyxisIndex : public string
{
public:
	/*!
	TESTING: only
	\param s
	\return ()
	*/
	PyxisIndex(string s) : string(s) {}
};

class CompressedPyxisIndex;

//! Pieces of Pyxis index being compressed with there location
class CIndexEntry :  public string
{
public:
	//! position in origin one
	int left; 
	//! position in origin one
	int right; 

	/*!
	Create a CIndexEntry
	\param left position in origin one
	\param right position in origin one 
	\param str a portion of the index
	*/

	//! Note: right - left + 1 == length(str)
	CIndexEntry(int left, int right, string str) : left(left), right(right), string(str)
	{
		assert(right - left + 1 == length());
	}

	/*!
	assign a character from the index
	\param c
	\return (CIndexEntry&)
	*/
	CIndexEntry& operator= (char *c)
	{
		string::operator=(c);
		return *this;
	}

	/*!
	assign a string from the index
	\param s
	\return (CIndexEntry&)
	*/
	CIndexEntry& operator= (string &s)
	{
		string::operator=(s);
		return *this;
	}
};

std::ostream& operator<< (std::ostream& out, const CIndexEntry& ix);


//! Temporary table of indexes being compressed
class CIndexEntryTable : public vector<CIndexEntry>
{
public:
	string ToString();
	void DisplayIndexes(string description = "");
	void RemoveIdenticalPrefix();
	void MarkIdenticalPrefix();
	string ResolvePyxisIndex(int level);
};


//!Compressed Pyxis indexes
class CompressedPyxisIndex 
{
	string compressedIndex;
public:
	/*!
	Construct a compress index
	\param s
	\return ()
	*/
	CompressedPyxisIndex(string s) : compressedIndex(s) { }
	/*!
	The length of the compressed index string
	\return (unsigned int)
	*/
	unsigned int length() {return compressedIndex.length(); }
	string toString() const;
	int maximumIndexLevel();
	string extractPyxisIndex(const int level);
};


std::ostream& operator<< (std::ostream& out, const CompressedPyxisIndex&ix);
std::istream& operator>> (std::istream& input, CompressedPyxisIndex& ix);

typedef vector<PyxisIndex *> tMapPyxisIndex;
typedef map<string, tMapPyxisIndex> tMapIndexes;

#define TOBIT(c) c - '0' + 1
#define MAP_FACE(f) (f >= '0' && f <'9'+3) ? f-'0' : f -'A'  + '9' + 3 + 1

//! An index map of indexes at a specific level
class CMapIndexes 
{
	char faceless[38/3+2];
	int level;
	tMapIndexes indexes[MAP_FACE('T')+1];
	void ToMapIndex(string const &pyxisIndexString);
public:
	/*!
	Construct a map index at a specific level.
	No checks are made on the level it is provided as a service to the caller.
	\param level
	\return ()
	*/
	CMapIndexes(int level);
	/*!
	Get the level 
	\return (int)
	*/
	int Level() {return level; }
	/*!
	Add an index to the map.
	\param pyxisIndexString
	\param pi
	\return (void)
	*/
	void CMapIndexes::Add(const string &pyxisIndexString, PyxisIndex *pi);
	/*!
	Retrieve a set of Pyxis indexes at a specific index.
	\param pyxisIndexString
	\return (tMapPyxisIndex&)
	*/
	tMapPyxisIndex& operator[]( const string &pyxisIndexString);
};

