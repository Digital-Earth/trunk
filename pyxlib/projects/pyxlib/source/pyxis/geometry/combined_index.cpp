/******************************************************************************
combined_index.cpp

begin		: 2009-11-16
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "combined_index.h"

#include <iostream>
#include <iomanip>
#include "limits.h"
#include "pyxis/utility/tester.h"
#include "pyxis/derm/snyder_projection.h"

using namespace std;

// Forward
static string encode(const string current);

//! Tester class
Tester<CombinedPyxisIndex> gTester;

//! Test method
void CombinedPyxisIndex::test()
{
	// set the limit higher for proper testing
	// 100 is not unreasonable 

	for (int i=0; i<100; i++)
	{
		// Test combined index
		CoordLatLon coordStart;
		coordStart.randomize();

		CombinedPyxisIndex combinedPyxisIndex(coordStart,
			PYXIcosIndex::knMinSubRes,
			PYXMath::knMaxAbsResolution
			);
		for (int nResolution = PYXIcosIndex::knMinSubRes;
			nResolution < PYXMath::knMaxAbsResolution; ++nResolution)
		{
			PYXIcosIndex index;
			SnyderProjection::getInstance()->nativeToPYXIS(coordStart, &index, nResolution);
			std::string strCombined =
				combinedPyxisIndex.extractPyxisIndex(nResolution);
			TEST_ASSERT(encode(index.toString()) == strCombined);
		}
	}

	CoordLatLon coordStart, coordBack;
	for (int i=0; i<10; i++)
	{
		coordStart.randomize();
		while (coordStart.isNorthPole() || coordStart.isSouthPole())
		{
			coordStart.randomize();
		}
		for (int nResolution = PYXIcosIndex::knMinSubRes;
			nResolution != PYXMath::knMaxAbsResolution; ++nResolution)
		{
			PYXIcosIndex index; 
			SnyderProjection::getInstance()->nativeToPYXIS(coordStart, &index, nResolution);
			SnyderProjection::getInstance()->pyxisToNative(index, &coordBack);
			double circumRadius = index.isHexagon() ? 
				CoordLatLon::hexagonCircumradius(nResolution) :
				CoordLatLon::pentagonCircumradius(nResolution);
			double distance = coordBack - coordStart;
			double ratio = distance/circumRadius;
			TEST_ASSERT(ratio < 1.2);
		}
	}
}

//! These are the originalIndex indexes that will be combined to create a compressed index.
class CombinedOriginalIndexes  
{

	std::vector<std::string> originalIndex;

public:

	CombinedOriginalIndexes(const PYXCurve& curve);

	void gatherKeys(CombinedIndexEntryTable& tableIndexes) const;

	void verifyIndexes(const CombinedPyxisIndex& indexes) const;

	void verifyIndexes(const CombinedIndexEntryTable& indexes) const;

};


#pragma region Index entries used to build the compress index

//! One entry per Pyxis index resolution level
class CombinedIndexEntry
{
public:

	//! the portion of an index
	string indexEntry;

	//! position in origin one
	int left; 

	//! position in origin one
	int right; 

	/*!
	Create a CombinedIndexEntry
	\param left position in origin one
	\param right position in origin one 
	\param str a portion of the index
	*/
	//! Note: right - left + 1 == length(str)
	CombinedIndexEntry(int left, int right, std::string str)
		: left(left), right(right), indexEntry(str)
	{
		assert(right - left + 1 == indexEntry.length());
	}
};

std::ostream& operator<<(std::ostream& out, const CombinedIndexEntry& ix);

//! Temporary table of indexes being compressed
class CombinedIndexEntryTable
{
public:
	vector<CombinedIndexEntry> entries;

	std::string getCompressedIndex() const;

	void displayIndexes(std::string description = "") const;

	void removeIdenticalPrefixes();

	void markIdenticalPrefixes();

	std::string resolvePyxisIndex(int resolution) const;
};

/*!
Gather the table entries into a string.
\return (std::string)
*/
string CombinedIndexEntryTable::getCompressedIndex() const
{
	string stringIndexes;
	for (vector<CombinedIndexEntry>::const_iterator currentIndex = entries.begin();
		currentIndex != entries.end(); currentIndex++)
	{
		stringIndexes += currentIndex->indexEntry;
		stringIndexes += char(currentIndex->right - 40);
	}
	return stringIndexes;
}

/*!
TESTING: display the table entries. 
\param description
\return (void)
*/
void CombinedIndexEntryTable::displayIndexes(const string description) const
{
#ifdef VERBOSE
	if (description.length() > 0)
	{
		cout << "\n" << description << "\n";
	}
	for (vector<CombinedIndexEntry>::const_iterator currentIndex = entries.begin();
		currentIndex != entries.end(); currentIndex++)
	{
		cout << '[' << setw(2) << currentIndex - entries.begin() << "] " << *currentIndex << "\n";
	}
#endif
}

/*!
Identical prefixes are merged.
Prefix maybe Identical to the previous index or the one before the previous one.
\return (void)
*/
void CombinedIndexEntryTable::markIdenticalPrefixes()
{
	if (entries.size() > 2)
	{
		for (vector<CombinedIndexEntry>::iterator currentIndex = entries.begin(), previousIndex, afterNextIndex;
			(afterNextIndex = currentIndex + 2) != entries.end();
			previousIndex = currentIndex++)
		{
			if (currentIndex->left == afterNextIndex->left && 
				currentIndex->indexEntry == afterNextIndex->indexEntry.substr(0, currentIndex->indexEntry.length()))
			{
				if (currentIndex == entries.begin() || previousIndex->indexEntry != "=")
				{
					if (currentIndex->left + 1 == currentIndex->right) 
					{
						currentIndex->indexEntry = "=";
					}
				}
				else
				{
					if (previousIndex->left == currentIndex->left &&
						previousIndex->right + 1 == currentIndex->right)
					{
						currentIndex->indexEntry = "=";
					}
				}
			}
		}
	}
}

/*!
Identical prefixes are merged. 
The right position reflexes the merge. 
\return (void)
*/
void CombinedIndexEntryTable::removeIdenticalPrefixes()
{
	bool sign = false;
	for (vector<CombinedIndexEntry>::iterator currentIndex = entries.begin();
		currentIndex != entries.end(); currentIndex++)
	{
		while (currentIndex->indexEntry == "=")
		{
			sign = true;
			currentIndex = entries.erase(currentIndex);
		}
		if (sign)
		{
			currentIndex->right = -currentIndex->right;
			sign = false;
		}
	}
}

/*!
For testing 
\param resolution of the the requested index.
\return (std::string) the Pyxis index
*/
string CombinedIndexEntryTable::resolvePyxisIndex(int resolution) const
{
	string index;
	unsigned int len = resolution + 1;
	for (vector<CombinedIndexEntry>::const_iterator currentIndex = entries.begin(); 
		index.length() < len && currentIndex != entries.end(); 
		currentIndex++)
	{
		int right = currentIndex->right;
		string s = currentIndex->indexEntry;
		int left = abs(right) - s.length() + 1;
		if (right < 0)
		{
			right = -right;
			if (len <= (unsigned)right)
			{
				s = (currentIndex + (right - len) % 2)->indexEntry;
			}
			else
			{
				s = (currentIndex + 1)->indexEntry;
			}
		}
		index = index.substr(0, left - 1) + s;
	}
	return index.substr(0, len);
}

/*!
TESTING: Stream a CombinedIndexEntry
\param out
\param ix
\return (std::ostream&)
*/
std::ostream& operator<<(std::ostream& out, const CombinedIndexEntry& ix)
{
	out << setw(3) <<  ix.left << setw(4) << ix.right << ' ' << ix.indexEntry ;
	return out;
}

#pragma endregion Index entries used to build the compress index

/*!
Extract the string representation of a Pyxis index for the given resolution
\param resolution of the the requested index.
\return (std::string) the Pyxis index
*/
string CombinedPyxisIndex::extractPyxisIndex(const int resolution) const
{
	string index;
	string s;
	unsigned int len = resolution;
	for (unsigned int i = 0;
		index.length() < len && i < compressedIndex.length();
		i++)
	{
		int c = (int)compressedIndex[i];
		if (c < 0)
		{
			int right = c + 40;
			int left = abs(right) - s.length() + 1;
			if (right < 0)
			{
				right = -right;
				if (len > (unsigned)right || (right - len) % 2 == 1)
				{
					int j = i;
					while (compressedIndex[j + 1] > 0)
					{
						j++;
					}
					s = compressedIndex.substr(i + 1, j - i);
				}					
			}
			index = index.substr(0, left - 1) + s;
			s = "";
		}
		else
		{
			s += (char)c;
		}
	}
	return index.substr(0, len);
}

/*!
The maximum PyxisIndex available 
\return (int) the maxLevel
*/
int CombinedPyxisIndex::getMaximumIndexLevel() const
{
	string index;
	string s;
	for (unsigned int i = 0; i < compressedIndex.length() ; i++)
	{
		int c = (int)compressedIndex[i];
		if (c < 0)
		{
			int right = c + 40;
			int left = abs(right) - s.length() + 1;
			if (right < 0)
			{
				right = -right;
				int j = i;
				while (compressedIndex[j + 1] > 0)
				{
					j++;
				}
				s = compressedIndex.substr(i + 1, j - i);
			}
			index = index.substr(0, left - 1) + s;
			s = "";
		}
		else
		{
			s += (char)c;
		}
	}
	return index.length() - 1;
}

/*!
Prepare an extended index to be display
\return (std::string)
*/
string CombinedPyxisIndex::toString() const
{
	string s;
	char len[4];
	for (unsigned int i = 0; i < compressedIndex.length(); i++)
	{
		int c = (int) compressedIndex[i];
		if (c < 0)
		{
			_itoa_s(c+40, len, 10);
			s += "[" + string(len) + "]";
		}
		else
		{
			s += (char)c;
		}
	}
	return s;
}

CombinedPyxisIndex::CombinedPyxisIndex(const CoordLatLon& ll,
									   int lowResolution,
									   int highResolution)
{
	assert(lowResolution < highResolution);
	PYXCurve curve;
	for (int nResolution = lowResolution; nResolution < highResolution; ++nResolution)
	{
		PYXIcosIndex index;
		SnyderProjection::getInstance()->nativeToPYXIS(ll, &index, nResolution);
		curve.addNode(index);
	}
	CombinedOriginalIndexes combinedIndex(curve);
	CombinedIndexEntryTable combinedIndexEntryTable;
	combinedIndex.gatherKeys(combinedIndexEntryTable);
	combinedIndexEntryTable.markIdenticalPrefixes();
	combinedIndexEntryTable.removeIdenticalPrefixes();
	compressedIndex = combinedIndexEntryTable.getCompressedIndex();
}

/*!
TESTING  display a compress index
\param out
\param ix
\return (std::ostream&)
*/
std::ostream& operator<<(std::ostream& out, const CombinedPyxisIndex& ix)
{
	out << ix.toString();
	return out;
}

/*!
TESTING: check that a retrieve index is the same as the originalIndex  
\param indexes
\return (void)
*/
void CombinedOriginalIndexes::verifyIndexes(const CombinedPyxisIndex& indexes) const
{
	for (unsigned int i = 0; i < originalIndex.size(); i++)
	{
		string ri = indexes.extractPyxisIndex(i + 2);
		assert(originalIndex[i] == ri);
	}
	cout << "Maximum index: " << indexes.getMaximumIndexLevel() << '\n';
}

/*!
TESTING: check that a retrieve index is the same as the originalIndex
\param indexes
\return (void)
*/
void CombinedOriginalIndexes::verifyIndexes(const CombinedIndexEntryTable& indexes) const
{
	for (unsigned int i = 0; i < originalIndex.size(); i++)
	{
		string ri = indexes.resolvePyxisIndex(i + 1);
		string encoded = originalIndex[i];
		assert(encoded == ri);
	}
}


/*!

\param tableIndexes receive the 
\return (void)
*/
void CombinedOriginalIndexes::gatherKeys(CombinedIndexEntryTable& tableIndexes) const
{
	string prevEncodedIx;

	for (vector<std::string>::const_iterator currentIndex = originalIndex.begin();
		currentIndex != originalIndex.end(); currentIndex++)
	{
		string currEncodedIx = *currentIndex;
		if (currentIndex == originalIndex.begin()) // place first index as received
		{
			tableIndexes.entries.push_back(
				CombinedIndexEntry(1, currEncodedIx.length(), currEncodedIx));
		}
		else if (prevEncodedIx == currEncodedIx.substr(0, prevEncodedIx.length()))
		{  
			CombinedIndexEntry &last = tableIndexes.entries.back();
			string ss = currEncodedIx.substr(prevEncodedIx.length());
			last.indexEntry += currEncodedIx.substr(prevEncodedIx.length());
			last.right = last.left + last.indexEntry.length() - 1;
		}
		else
		{
			int left = 0;
			for (unsigned int i = 0; i < prevEncodedIx.length(); i++)
			{
				left = i + 1;
				if (prevEncodedIx[i] != currEncodedIx[i])
				{
					break;
				}
			}
			CombinedIndexEntry newIndex(left,
				currEncodedIx.length(), currEncodedIx.substr(left - 1));
			tableIndexes.entries.push_back(newIndex);
		}
		// skip missing keys
		if (currEncodedIx.length() > 0) 
		{
			prevEncodedIx = currEncodedIx;
		}
	}
}

/*!
Encode a display key (Use for testing)

\param current
\return (std::string)
*/
static string encode(const string current)
{
	string newkey;
	if (current.length() != 0)
	{
		int sep = current.find("-");
		assert(sep != string::npos);
		if (sep == 1)
		{
			newkey = current[0];
		}
		else
		{
			newkey = '9' + atoi(current.substr(0, 2).c_str()) - 9;
		}
		newkey += current.substr(sep + 1);
	}
	return newkey;
}

/*!

\param current
\param newkey
\return (void)
*/
static void encode(const PYXIcosIndex &current, string &newkey)
{
	int primaryResolution = current.getPrimaryResolution();
	const PYXIndex& index = current.getSubIndex();
	if (current.isFace())
		newkey = static_cast<char>(primaryResolution);
	else
		newkey = '9' + primaryResolution - 9;
	newkey.append(index.toString());
}

CombinedOriginalIndexes::CombinedOriginalIndexes(const PYXCurve& curve)
{
	const std::vector<PYXIcosIndex>& nodes = curve.getNodes();
#ifndef NDEBUG
	int resolution = PYXIcosIndex::knMinSubRes;
#endif
	for (vector<PYXIcosIndex>::const_iterator currentIndex = nodes.begin();
		currentIndex != nodes.end(); currentIndex++)
	{
#ifndef NDEBUG
		assert(currentIndex->getResolution() == resolution);
		++resolution;
#endif
		string newkey;
		encode(*currentIndex, newkey);
		originalIndex.push_back(newkey);
	}
}
