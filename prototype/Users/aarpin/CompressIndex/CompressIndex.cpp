// CompressIndex.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "CompressIndex.h"

#include "assert.h"
#pragma warning(disable:4786)
#include <iostream>
#include <fstream>
#include <iomanip>
#include "string.h"
#include <algorithm>
#include "map_indexes.h"

using namespace std;
//#define VERBOSE true

#define MAXLEVEL 38
int TRUNCATE_TO = MAXLEVEL;
int LOWEST = 1;
int HIGHEST = 5; // 100000

/*!
Encode a display key
\param current
\return (std::string)
*/
string COriginalIndexes::encode(const string current)
{
	string newkey;
	if (current.length() != 0)
	{
		int sep = current.find("-");
		assert(sep != string.npos);
		if (sep == 1)
			newkey = current[0];
		else
			newkey = '9' + atoi(current.substr(0,2).c_str()) - 10;
		newkey+=current.substr(sep + 1);
	}
	return newkey;
}

/*!
TESTING check that a retrieve index is the same as the original  
\param indexes
\return (void)
*/
void COriginalIndexes::verifyIndexes(CompressedPyxisIndex indexes)
{
	for (unsigned int i = 0; i < this->size() ; i++)
	{
		string ri = indexes.extractPyxisIndex(i+1);
		assert(encode((*this)[i]) == ri);
	}
	cout << "Maximum index: " << indexes.maximumIndexLevel() << '\n';
}

/*!
TESTING check that a retrieve index is the same as the original
\param indexes
\return (void)
*/
void COriginalIndexes::verifyIndexes(CIndexEntryTable indexes)
{
	for (unsigned int i = 0; i < size() ; i++)
	{
		string ri = indexes.ResolvePyxisIndex(i+1);
		string encoded = encode((*this)[i]);
		assert(encoded == ri);
	}
}

/*!
use for testing
\return (void)
*/
#define VERBOSE
typedef vector <CompressedPyxisIndex *> tTableOfExtendedPyxisIndexes;
typedef vector<PyxisIndex *> tTableOfPyxisIndexes;
void test(CMapIndexes &mapIndexes, tTableOfPyxisIndexes &tableOfPyxisIndexes)
{
	tTableOfExtendedPyxisIndexes tablelOfXPyxisIndexes;
	string current;
	filebuf fb;
	fb.open("log.log",ios::in);
	assert(fb.is_open());
	istream fl(&fb);
	vector<string> inLogLog;
	copy (istream_iterator<string>(fl), istream_iterator<string>(), back_inserter(inLogLog));
	fb.close();

	vector<int> distribution;
	COriginalIndexes originalIndexes;
	originalIndexes.resize(TRUNCATE_TO);

	for (vector<string>::iterator inLogLogIndex = inLogLog.begin()+(LOWEST-1) * MAXLEVEL;
		inLogLogIndex - inLogLog.begin() < HIGHEST * MAXLEVEL 
		&& inLogLogIndex != inLogLog.end(); inLogLogIndex += MAXLEVEL)
	{
		CIndexSet indexSet; // Pyxis index string
		int number = 1 + (inLogLogIndex - inLogLog.begin()) / MAXLEVEL;
		if (number % 100 == 0) 
			cerr << number<< ' ';
		assert(inLogLogIndex + MAXLEVEL <= inLogLog.end());
		copy(inLogLogIndex, inLogLogIndex + TRUNCATE_TO, originalIndexes.begin());

		cout << '#' << number << '\n';
		for (COriginalIndexes::iterator currentIndex = originalIndexes.begin(); currentIndex != originalIndexes.end(); currentIndex++)
		{
			indexSet.push_back(COriginalIndexes::encode(*currentIndex));
#ifdef VERBOSE
			cout << setw(2) << (currentIndex - originalIndexes.begin()) + 1 << " " << *currentIndex  << "\n";
#endif
		}
		CIndexEntryTable tableIndexes = indexSet.GatherKeys();
		tableIndexes.DisplayIndexes("Gathered");
		tableIndexes.MarkIdenticalPrefix();
		tableIndexes.DisplayIndexes( "Identical Prefix");
		tableIndexes.RemoveIdenticalPrefix();
		tableIndexes.DisplayIndexes("Identical Prefix Merged");
		originalIndexes.verifyIndexes(tableIndexes);
		indexSet.verifyIndexes(tableIndexes);
		tablelOfXPyxisIndexes.push_back(new CompressedPyxisIndex(tableIndexes.ToString()));
#ifdef VERBOSE
		cout << tablelOfXPyxisIndexes.back()->length() << '\n';
		cout << tablelOfXPyxisIndexes.back() << "\n";
#endif		
		originalIndexes.verifyIndexes(*tablelOfXPyxisIndexes.back());
		indexSet.verifyIndexes(*tablelOfXPyxisIndexes.back());
		distribution.resize(max(distribution.size(), tablelOfXPyxisIndexes.back()->length()+1));
		distribution[tablelOfXPyxisIndexes.back()->length()] += 1;
	}

	// show stop leading zeros from showing
	cout << "Distribution:\n";
	for (unsigned int i = 0, total = 0, number = 0; i < distribution.size(); i++)
	{
		if (number || distribution[i])
		{
			cout << i << " " << distribution[i] << "\n";
			number += distribution[i];
			total = total + i * distribution[i];
		}
		if (i+1 == distribution.size())
			cout << "Average: " << total / number <<
			" for " << number << " samples, TRUNCATE_TO: " << TRUNCATE_TO << "\n";
	}

	for (vector<CompressedPyxisIndex *>::iterator it=tablelOfXPyxisIndexes.begin(); it != tablelOfXPyxisIndexes.end(); it++)
	{
		string s=(*it)->extractPyxisIndex(mapIndexes.Level());
		PyxisIndex *pi = new PyxisIndex(s);
		tableOfPyxisIndexes.push_back(pi);
		mapIndexes.Add(s, pi);
	}
}

int main(int argc, char* argv[])
{
	int maxLevel=TRUNCATE_TO;
	for(int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-t") == 0)
			TRUNCATE_TO = atoi(argv[++i]);
		else if (strcmp(argv[i], "-l") == 0)
			LOWEST = atoi(argv[++i]);
		else if (strcmp(argv[i], "-h") == 0)
			HIGHEST = atoi(argv[++i]);
		else if (strcmp(argv[i], "-m") == 0)
			maxLevel =  atoi(argv[++i]);
		else
			assert(false); // Invalid argument
	}
	assert(TRUNCATE_TO >= 1 && TRUNCATE_TO <= MAXLEVEL);
	assert(LOWEST > 0);
	assert(LOWEST <= HIGHEST);
	CMapIndexes mapIndexes(min(maxLevel, TRUNCATE_TO));
	// this table is only for testing should use features
	tTableOfPyxisIndexes tabelOfPyxisIndexes;
	test(mapIndexes, tabelOfPyxisIndexes);
	tTableOfPyxisIndexes  copy_tabelOfPyxisIndexes = tabelOfPyxisIndexes;
	
	random_shuffle(copy_tabelOfPyxisIndexes.begin(), copy_tabelOfPyxisIndexes.end());
	for (tTableOfPyxisIndexes::iterator cti = copy_tabelOfPyxisIndexes.begin(); cti != copy_tabelOfPyxisIndexes.end(); cti++)
	{
		string index = string(**cti);
		tMapPyxisIndex mapPyxisIndex = mapIndexes[index];
		cout << index << ':'<< mapPyxisIndex.size() << '\n';
		int found = 0;
		PyxisIndex *miP, *ctiP;
		ctiP = *cti;
		for (tMapPyxisIndex::iterator mi = mapPyxisIndex.begin(); mi != mapPyxisIndex.end(); mi++)
		{
			miP = *mi;
			if (miP == ctiP)
				found++;
		}
		cout << "Found: " << found << " out of " << mapPyxisIndex.size() << "\n";
		assert(found==1);
	}
	return 0;
}