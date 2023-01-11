/******************************************************************************

begin		: 2009-10-27
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


#include "map_indexes.h"
#include <iostream>
#include <iomanip>
#include "limits.h"

using namespace std ;

#pragma region Index entries used to build the compress index
/*!
Gather the table entries into a string.

\return (std::string)
*/
string CIndexEntryTable::ToString()
{
	string stringIndexes;
	for (CIndexEntryTable::iterator currentIndex = begin(); currentIndex != end(); currentIndex++)
	{
		stringIndexes += *currentIndex;
		stringIndexes += char(currentIndex->right - 40);
	}
	return stringIndexes;
}

/*!
TESTING: display the table entries. 

\param description
\return (void)
*/
void CIndexEntryTable::DisplayIndexes(const string description )
{
#ifdef VERBOSE
	if (description.length() > 0)
		cout << "\n" << description << "\n";
	for (vector<CIndexEntry>::iterator currentIndex = begin(); currentIndex != end(); currentIndex++)
		cout << '[' << setw(2) << currentIndex-begin() << "] " <<  *currentIndex << "\n";
#endif
}

/*!
Identical prefixes are merged.
Prefix maybe Identical to the previous index or the one before the previous one.

\return (void)
*/
void CIndexEntryTable::MarkIdenticalPrefix()
{
	if (size() > 2)
	{
		for (vector<CIndexEntry>::iterator currentIndex = begin(), previousIndex, afterNextIndex; 
			(afterNextIndex = currentIndex + 2) != end() ; previousIndex = currentIndex++)
			if (currentIndex->left == afterNextIndex->left && 
				*currentIndex == afterNextIndex->substr(0, currentIndex->length()))
				if  (currentIndex == begin() || *previousIndex != "=" )
				{
					if (currentIndex->left + 1 == currentIndex->right  ) 
						*currentIndex = "=";
				}
				else 
					if (previousIndex->left == currentIndex->left && previousIndex->right + 1 == currentIndex->right )
						*currentIndex = "=";
	}
}

/*!
Identical prefix are merged. 
The right position reflexes the merge. 

\return (void)
*/
void CIndexEntryTable::RemoveIdenticalPrefix()
{
	bool sign = false;
	for (vector<CIndexEntry>::iterator currentIndex = begin(); currentIndex != end(); currentIndex++)
	{
		while (*currentIndex == "=")
		{
			sign = true;
			currentIndex = erase(currentIndex);
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

\param level of the the requested index.
\return (std::string) the Pyxis index
*/
string CIndexEntryTable::ResolvePyxisIndex(int level)
{
	string index;
	unsigned int len = level + 1;
	for (vector<CIndexEntry>::iterator currentIndex = begin(); 
		index.length() < len && currentIndex != end(); 
		currentIndex++)
	{
		int right = currentIndex->right;
		string s = *currentIndex;
		int left = abs(right) - s.length() + 1;
		if (right < 0)
		{
			right = -right;
			if (len <= (unsigned)right)
				s = *(currentIndex + (right - len) % 2);
			else
				s = *(currentIndex + 1);
		}
		index = index.substr(0, left - 1) + s;
	}
	return index.substr(0, len);
}


/*!
TESTING: Stream a CIndexEntry

\param out
\param ix
\return (std::ostream&)
*/
std::ostream& operator<< (std::ostream& out, const CIndexEntry& ix)
{
	out << setw(3) <<  ix.left << setw(4) << ix.right << ' ' << (string)ix ;
	return out;
}

#pragma endregion Index entries used to build the compress index
/*!
Extract the string representation of a Pyxis index for the given level
\param level of the the requested index.
\return (std::string) the Pyxis index
*/
string CompressedPyxisIndex::extractPyxisIndex(const int level)
{
	string index;
	string s;
	unsigned int len = level + 1;
	for (unsigned int i = 0; index.length() < len && i < compressedIndex.length() ; i++)
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
					while (compressedIndex[j+1] > 0)
						j++;
					s=compressedIndex.substr(i+1, j-i);
				}					
			}
			index = index.substr(0, left - 1) + s;
			s="";
		}
		else
			s += (char)c ; 
	}
	return index.substr(0, len);
}

/*!
The maximum PyxisIndex available 
\return (int) the maxLevel
*/
int CompressedPyxisIndex::maximumIndexLevel()
{
	string index;
	string s;
	for (unsigned int i = 0; i < compressedIndex.length() ; i++)
	{
		int c = (int) compressedIndex[i];
		if (c < 0)
		{
			int right = c + 40;
			int left = abs(right) - s.length() + 1;
			if (right < 0)
			{
				right = -right;
				int j = i;
				while (compressedIndex[j+1] > 0)
					j++;
				s=compressedIndex.substr(i+1, j-i);
			}
			index = index.substr(0, left - 1) + s;
			s="";
		}
		else
			s += (char)c ; 
	}
	return index.length() - 1;
}

/*!
Prepare an extended index to be display
\return (std::string)
*/
string CompressedPyxisIndex::toString() const
{
	string s;
	char len[4];
	for (unsigned int i=0; i < compressedIndex.length(); i++)
	{
		int c = (int) compressedIndex[i];
		if (c < 0)
		{
			_itoa_s(c+40, len, 10);
			s +=  "[" + string(len) + "]";
		}
		else
			s += (char)c ; 
	}
	return s;
}

/*!
TESTING  display a compress index
\param out
\param ix
\return (std::ostream&)
*/
std::ostream& operator<< (std::ostream& out, const CompressedPyxisIndex& ix)
{
	out << ix.toString() ;
	return out;
}

/*!
TESTING: check that a retrieve index is the same as the original  
\param indexes
\return (void)
*/
void CIndexSet::verifyIndexes(CompressedPyxisIndex indexes)
{
	for (unsigned int i = 0; i < this->size() ; i++)
	{
		string ri = indexes.extractPyxisIndex(i+1);
		assert((*this)[i] == ri);
	}
	cout << "Maximum index: " << indexes.maximumIndexLevel() << '\n';
}

/*!
TESTING: check that a retrieve index is the same as the original
\param indexes
\return (void)
*/
void CIndexSet::verifyIndexes(CIndexEntryTable indexes)
{
	for (unsigned int i = 0; i < size() ; i++)
	{
		string ri = indexes.ResolvePyxisIndex(i+1);
		string encoded = (*this)[i];
		assert(encoded == ri);
	}
}

/*!
Given a set of stream index 
\return (CIndexEntryTable)
*/
CIndexEntryTable CIndexSet::GatherKeys()
{
	CIndexEntryTable tableIndexes;
	string prevEncodedIx;

	for (CIndexSet::iterator currentIndex = begin(); currentIndex != end(); currentIndex++)
	{
		string currEncodedIx = *currentIndex;
		if (currentIndex == begin()) // place first index as received
			tableIndexes.push_back(CIndexEntry(1, currEncodedIx.length(), currEncodedIx));
		else if (prevEncodedIx == currEncodedIx.substr(0, prevEncodedIx.length()))
		{  
			CIndexEntry &last = tableIndexes.back();
			string ss = currEncodedIx.substr(prevEncodedIx.length());
			last += currEncodedIx.substr(prevEncodedIx.length());
			last.right = last.left + last.length()-1;
		}
		else
		{
			int left = 0;
			for (unsigned int i = 0; i < prevEncodedIx.length() ; i++)
			{
				left = i + 1;
				if (prevEncodedIx[i] != currEncodedIx[i])
					break;
			}
			CIndexEntry newIndex(left, currEncodedIx.length(), currEncodedIx.substr(left-1));
			tableIndexes.push_back(newIndex);
		}
		// skip missing keys
		if (currEncodedIx.length() > 0) 
			prevEncodedIx = currEncodedIx;
	}
	return tableIndexes;
}


CMapIndexes::CMapIndexes( int level ) : level(level)
{
	assert(level > 0 && level <= 38); // 38 should be max resolution
}

 tMapPyxisIndex& CMapIndexes::operator[]( const string &pyxisIndexString )
{
	ToMapIndex(pyxisIndexString);
	assert(MAP_FACE(pyxisIndexString[0]) < sizeof(indexes) / sizeof(indexes[0]));
	return indexes[MAP_FACE(pyxisIndexString[0])][string(faceless)];
}

void CMapIndexes::Add(string const &pyxisIndexString, PyxisIndex *pi)
{
	ToMapIndex(pyxisIndexString);
	assert(MAP_FACE(pyxisIndexString[0]) < sizeof(indexes) / sizeof(indexes[0]));
	tMapIndexes &mi = indexes[MAP_FACE(pyxisIndexString[0])];
	mi[string(faceless)].push_back(pi);
}

void CMapIndexes::ToMapIndex( string const &pyxisIndexString )
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