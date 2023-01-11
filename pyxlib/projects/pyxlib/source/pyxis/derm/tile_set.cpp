/******************************************************************************
tile_set.cpp

begin		: 2006-08-07
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/derm/tile_set.h"

#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/bit_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"

#include <cassert>
#include <fstream>

#ifdef PERFORM_TREE_VALIDATION
#define VALIDATE_NODE validate()
#else
#define VALIDATE_NODE 
#endif


// PYXTileSet

static const unsigned int knRootByteCount = 4;
static const unsigned int knRootCount = knRootByteCount * BitUtils::knUsedBitCountPerByte;

//! The unit test class
Tester<PYXTileSet> gTester();

//! Test method
void PYXTileSet::test()
{
	// Temporary files.
	std::string strInPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
	std::string strOutPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));

	// Test the tile set using non-aggregated insertion.
	// The aggregated variety is tested in PYXTileCollection.
	// TO DO: Move the aggregation tests into here from PYXTileCollection and adapt to the class.
	bool bAggregated = false;

	/* Create a set with the following indices:
		3-2
		3-30
		4-2
		M-02
		M-0302
		M-0303004
		M-050
		M-060002
		M-060010
		M-0600503
		M-0600504
	*/
	PYXTileSet set;
	TEST_ASSERT(set.empty());
	TEST_ASSERT(set.count() == 0);
	{
		set.insert(PYXIcosIndex("3-2"), bAggregated);
		TEST_ASSERT(!set.empty());
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("3-20"))); // Descendant
		set.insert(PYXIcosIndex("3-20"), bAggregated); // Descendant
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("3-20"))); // Descendant
		set.insert(PYXIcosIndex("3-30"), bAggregated);
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(!set.contains(PYXIcosIndex("3-3"))); // Ancestor
		TEST_ASSERT(set.contains(PYXIcosIndex("3-30")));
		set.insert(PYXIcosIndex("3-30"), bAggregated); // Same
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(!set.contains(PYXIcosIndex("3-3"))); // Ancestor
		TEST_ASSERT(set.contains(PYXIcosIndex("3-30")));
		set.insert(PYXIcosIndex("4-2"), bAggregated);
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("3-30")));
		TEST_ASSERT(set.contains(PYXIcosIndex("4-2")));
		set.insert(PYXIcosIndex("M-02"), bAggregated);
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("3-30")));
		TEST_ASSERT(set.contains(PYXIcosIndex("4-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-02")));
		set.insert(PYXIcosIndex("M-0302"), bAggregated);
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("3-30")));
		TEST_ASSERT(set.contains(PYXIcosIndex("4-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-02")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-0302")));
		set.insert(PYXIcosIndex("M-03030040506"), bAggregated);
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("3-30")));
		TEST_ASSERT(set.contains(PYXIcosIndex("4-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-02")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-0302")));
		TEST_ASSERT(!set.contains(PYXIcosIndex("M-0303004"))); // Ancestor
		TEST_ASSERT(set.contains(PYXIcosIndex("M-03030040506")));
		set.insert(PYXIcosIndex("M-0303004"), bAggregated); // Ancestor
		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("3-30")));
		TEST_ASSERT(set.contains(PYXIcosIndex("4-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-02")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-0302")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-0303004")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-03030040506"))); // Descendant

		PYXTileSet anotherSet;
		anotherSet.insert(PYXIcosIndex("M-050"), bAggregated);
		anotherSet.insert(PYXIcosIndex("M-060002"), bAggregated);
		anotherSet.insert(PYXIcosIndex("M-060010"), bAggregated);
		anotherSet.insert(PYXIcosIndex("M-0600503"), bAggregated);
		anotherSet.insert(PYXIcosIndex("M-0600504"), bAggregated);
		set.insert(anotherSet, bAggregated);

		TEST_ASSERT(set.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("3-30")));
		TEST_ASSERT(set.contains(PYXIcosIndex("4-2")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-02")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-0302")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-0303004")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-050")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-060002")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-060010")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-0600503")));
		TEST_ASSERT(set.contains(PYXIcosIndex("M-0600504")));

		TEST_ASSERT(set.count() == 11);

		// Iterate.
		PYXTileSet::Iterator iTiles(set);
		TEST_ASSERT(*iTiles == PYXIcosIndex("3-2"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("3-30"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("4-2"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("M-02"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("M-0302"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("M-0303004"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("M-050"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("M-060002"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("M-060010"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("M-0600503"));
		iTiles.next();
		TEST_ASSERT(*iTiles == PYXIcosIndex("M-0600504"));
		iTiles.next();
		TEST_ASSERT(iTiles.end());

		// Serialize.
		std::basic_ofstream<char> out(strInPath.c_str(), std::ios_base::binary);
		set.serialize(out);
	}

	{
		// Initialize from the file.
		std::basic_ifstream<char> testIn(strInPath.c_str(), std::ios_base::binary);
		PYXTileSet loaded(testIn);
		testIn.close();

		// Verify contents.
		TEST_ASSERT(loaded == set);
		TEST_ASSERT(loaded.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("3-30")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("4-2")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-02")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-0302")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-0303004")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-050")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-060002")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-060010")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-0600503")));
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-0600504")));

		// Make sure that descendants are contained.
		TEST_ASSERT(loaded.contains(PYXIcosIndex("M-06000203")));

		// Make sure that ancestors aren't contained.
		TEST_ASSERT(!loaded.contains(PYXIcosIndex("M-05")));
		TEST_ASSERT(!loaded.contains(PYXIcosIndex("M-060")));
		TEST_ASSERT(!loaded.contains(PYXIcosIndex("M-0600")));

		// Test equality with original.
		TEST_ASSERT(loaded == set);

		// Output to another file.
		{
			std::basic_ofstream<char> testOut(strOutPath.c_str(), std::ios_base::binary);
			loaded.serialize(testOut);
		}
		
		// Set resolution to a higher value and verify that they're the same.
		PYXTileSet copy(loaded);
		copy.setResolution(8);
		TEST_ASSERT(copy.count() == loaded.count());

		// Set resolution to a lower value and verify truncation.
		copy.setResolution(5);
		TEST_ASSERT(copy.count() == 8);
		for (PYXTileSet::Iterator iTiles(copy); !iTiles.end(); iTiles.next())
		{
			TEST_ASSERT((*iTiles).getResolution() <= 5);
		}
		TEST_ASSERT(copy.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("3-30")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("4-2")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("M-02")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("M-0302")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("M-0303")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("M-050")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("M-0600")));

		// Set resolution to a lower value and verify truncation.
		copy.setResolution(2);
		TEST_ASSERT(copy.count() == 4);
		for (PYXTileSet::Iterator iTiles(copy); !iTiles.end(); iTiles.next())
		{
			TEST_ASSERT((*iTiles).getResolution() == 2);
		}
		TEST_ASSERT(copy.contains(PYXIcosIndex("3-2")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("3-3")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("4-2")));
		TEST_ASSERT(copy.contains(PYXIcosIndex("M-0")));
	}

	// Verify contents of output file.
	{
		std::basic_ifstream<char> in(strOutPath.c_str(), std::ios_base::binary);

		// The indentation below reflects the tree structure.
		const unsigned char aExpected[] = {
			// Roots
			12, 0, 0, 1,

			// 3
			12,
				128,
				129,

			// 4
			132,

			// M
			1, 108,
				128,
				1, 12,
					128,
					1, 1, 144,
				129,
				1, 1, 35,
					132,
					129,
					1, 152,

			// Resolution
			0
		};

		char byte = 0;
		const size_t nCount = sizeof(aExpected) / sizeof(*aExpected);
		for (size_t nIndex = 0; nIndex < nCount; ++nIndex)
		{
			in.get(byte);
			TEST_ASSERT(aExpected[nIndex] == (byte));
		}
		TEST_ASSERT(!in.get(byte));
	}

	// Test child removal.
	{
		PYXTileSet ts;
		ts.insert(PYXIcosIndex("3-0200"), true);
		ts.insert(PYXIcosIndex("3-0400"), true);
		ts.insert(PYXIcosIndex("3-02"), true);
		TEST_ASSERT(ts.contains(PYXIcosIndex("3-02")));
		TEST_ASSERT(ts.contains(PYXIcosIndex("3-0400")));
		TEST_ASSERT(!ts.contains(PYXIcosIndex("3-04")));
	}
}

PYXTileSet::PYXTileSet(unsigned char nResolution) :
	m_tree(), m_nResolution(nResolution)
{
}

//! Construct by deserializing from stream.
PYXTileSet::PYXTileSet(std::basic_istream<char>& in) :
	m_tree(in), m_nResolution(0)
{
	if (!in.eof())
	{
		char c;
		in.get(c);
		m_nResolution =  (c);
	}
}

PYXTileSet::~PYXTileSet()
{
}

//! Serialize to stream.
void PYXTileSet::serialize(std::basic_ostream<char>& out) const
{
	m_tree.serialize(out);
	out.put(m_nResolution);
}

bool PYXTileSet::intersects(const PYXIcosIndex& index) const
{
	return m_tree.intersects(index);
}


bool PYXTileSet::contains(const PYXIcosIndex& index) const
{
	return m_tree.contains(index);
}

//! Expands the tile set as necessary to insert the index.
void PYXTileSet::insert(const PYXIcosIndex& index, bool bAggregate)
{
	// Return false if the resolution is higher than that of the set.
	if (m_nResolution > 0)
	{
		const unsigned int nIndexResolution = index.getResolution();
		if (nIndexResolution > m_nResolution)
		{
			// Decimate the index and insert.
			PYXIcosIndex copyIndex(index);
			copyIndex.setResolution(m_nResolution);
			m_tree.insert(copyIndex, bAggregate);
			return;
		}
	}
	m_tree.insert(index, bAggregate);
}

//! Expand the tile set as necessary to insert each index in the set.
void PYXTileSet::insert(const PYXTileSet& set, bool bAggregate)
{
	// Insert each index in the set.
	for (Iterator iTiles(set); !iTiles.end(); ++iTiles)
	{
		insert(*iTiles, bAggregate);
	}
}

//! Sets the resolution of the tile set to the new resolution.
void PYXTileSet::setResolution(const int nResolution)
{
	// If this isn't true, this code will need to be changed
	// to accommodate the larger resolution.
	assert(PYXMath::knMaxAbsResolution <= std::numeric_limits<unsigned char>::max());

	if (nResolution > PYXMath::knMaxAbsResolution)
	{
		PYXTHROW(PYXIndexException, "Invalid resolution: '" << nResolution << "'.");
	}

	const unsigned char nCharResolution = static_cast<unsigned char>(nResolution);
	if (0 == m_nResolution || nCharResolution < m_nResolution) // Optimization
	{
		m_tree.setResolution(nCharResolution);
	}
	m_nResolution = nCharResolution;
}

int PYXTileSet::resolution() const
{
	return m_nResolution;
}

//! Clear the set.
void PYXTileSet::clear()
{
	m_tree.clear();
}

//! Returns true if empty.
bool PYXTileSet::empty() const
{
	return m_tree.empty();
}

//! Returns the number of tiles.
size_t PYXTileSet::count() const
{
	size_t nCount = 0;
	assert(0 != this);
	for (Iterator iTiles(*this); !iTiles.end(); ++iTiles)
	{
		++nCount;
	}
	return nCount;
}

//! The equality operator.
bool operator ==(const PYXTileSet& lhs, const PYXTileSet& rhs)
{
	// If the resolutions don't match, no deal.
	// Don't count a resolution of 0.
	int nLhsResolution = lhs.resolution();
	int nRhsResolution = rhs.resolution();
	if (0 != nLhsResolution && 0 != nRhsResolution && nLhsResolution != nRhsResolution)
	{
		return false;
	}

	// Verify that the root indices are the same.
	// At the same time, tally the actual resolution.
	unsigned int nResolution = 0;
	PYXTileSet::Iterator iRhs(rhs);
	for (PYXTileSet::Iterator iLhs(lhs); !iLhs.end(); ++iLhs, ++iRhs)
	{
		if (iRhs.end())
		{
			return false;
		}
		const PYXIcosIndex& indexLhs = *iLhs;
		if (indexLhs != *iRhs)
		{
			return false;
		}
		const unsigned int nCurrentResolution = indexLhs.getResolution();
		if (nCurrentResolution > nResolution)
		{
			nResolution = nCurrentResolution;
		}
	}
	if (!iRhs.end())
	{
		return false;
	}

	// Verify that if only one of the resolutions is 0, 
	// the actual resolution matches the non-zero resolution.
	if (0 == nLhsResolution)
	{
		nLhsResolution = nResolution;
	}
	if (0 == nRhsResolution)
	{
		nRhsResolution = nResolution;
	}
	return nLhsResolution == nRhsResolution;
}

// PYXTileSet::Iterator

//! Construct an iterator.
PYXTileSet::Iterator::Iterator(const PYXTileSet& set) : m_iTree(set.m_tree)
{
}

//! Destroy the instance.
PYXTileSet::Iterator::~Iterator()
{
}

//! Move to the next item.
void PYXTileSet::Iterator::next()
{
	m_iTree.next();
}

/*!
See if we have covered all the items.

\return	true if all items have been covered, otherwise false.
*/
bool PYXTileSet::Iterator::end() const
{
	return m_iTree.end();
}

//! Return the current item.  If past end, return the last item.
const PYXIcosIndex& PYXTileSet::Iterator::operator *() const
{
	return *m_iTree;
}

// PYXTileSet::Tree

// Convert the primary resolution to the root index.
// Converts eg. "A" from ascii to 13.
unsigned int PYXTileSet::Tree::getRootIndex(const int nPrimaryResolution)
{
	if (nPrimaryResolution <= 12)
	{
		assert(nPrimaryResolution > 0);
		return nPrimaryResolution - 1;
	}
	return (nPrimaryResolution - 'A' + 12);
}

// Convert the root index to the primary resolution.
unsigned int PYXTileSet::Tree::getPrimaryResolution(const unsigned int nRootIndex)
{
	if (nRootIndex < 12)
	{
		return nRootIndex + 1;
	}
	return (nRootIndex - 12 + 'A');
}

PYXTileSet::Tree::Tree() :
	m_vRoots(knRootCount)
{
}

//! Construct a copy.
PYXTileSet::Tree::Tree(const Tree& source) :
	m_vRoots(knRootCount)
{
	assert(0 != this);
	*this = source;
}

//! Construct by deserializing from stream.
PYXTileSet::Tree::Tree(std::basic_istream<char>& in) :
	m_vRoots(knRootCount)
{
	// Read byte array for root index.
	unsigned char bytes[knRootByteCount] = {0}; // Initialize them all to 0.
	for (unsigned int nByteIndex = 0; nByteIndex < knRootByteCount; ++nByteIndex)
	{
		char c;
		if (!in.get(c))
		{
			break;
		}
		bytes[nByteIndex] =  (c);
	}

	// Construct roots accordingly.
	for (unsigned int nByteIndex = 0; nByteIndex < knRootByteCount; ++nByteIndex)
	{
		unsigned int nByte = bytes[nByteIndex];
		for (unsigned int nIndex = 0; nByte != 0 && nIndex < BitUtils::knUsedBitCountPerByte; ++nIndex, nByte >>= 1)
		{
			if (0 != (nByte & 1))
			{
				m_vRoots[nIndex + (BitUtils::knUsedBitCountPerByte * nByteIndex)] = new Node(in);
			}
		}
	}
}

PYXTileSet::Tree::~Tree()
{
	clear();
}

//! Assign a copy.
PYXTileSet::Tree& PYXTileSet::Tree::operator =(const Tree& source)
{
	if (this != &source)
	{
		// Copy each root from source.
		assert(m_vRoots.size() == source.m_vRoots.size());
		for (size_t nIndex = m_vRoots.size(); nIndex > 0; )
		{
			--nIndex;
			delete m_vRoots[nIndex];
			m_vRoots[nIndex] = (0 == source.m_vRoots[nIndex])? 0: new Node(*(source.m_vRoots[nIndex]));
		}
	}
	assert(0 != this);
	return *this;
}

//! Serialize to stream.
void PYXTileSet::Tree::serialize(std::basic_ostream<char>& out) const
{
	std::vector<Node *> vRoots;
	vRoots.reserve(knRootCount);

	// Write the byte, and get a list of root nodes to serialize.
	for (unsigned int nByteIndex = 0; nByteIndex < knRootByteCount; ++nByteIndex)
	{
		unsigned char byte = 0;
		unsigned int mask = 1;
		for (unsigned int nIndex = 0; nIndex < BitUtils::knUsedBitCountPerByte; ++nIndex, mask <<= 1)
		{
			Node * pNode = m_vRoots[nIndex + (BitUtils::knUsedBitCountPerByte * nByteIndex)];
			if (0 != pNode)
			{
				vRoots.push_back(pNode);
				byte |= mask;
			}
		}
		out.put(byte);
	}

	// Serialize each root.
	for (std::vector<Node *>::const_iterator iRoots = vRoots.begin();
		iRoots != vRoots.end();
		++iRoots)
	{
		(*iRoots)->serialize(out);
	}
}


bool PYXTileSet::Tree::intersects(const PYXIcosIndex& index) const
{
	// Get the primary resolution, and convert it to the root index.
	unsigned int nRoot = getRootIndex(index.getPrimaryResolution());

	// If the root is null, return false.
	if (0 == m_vRoots[nRoot])
	{
		return false;
	}

	// Ask the root if it contains the sub index.
	return m_vRoots[nRoot]->intersects(index.getSubIndex());
}


bool PYXTileSet::Tree::contains(const PYXIcosIndex& index) const
{
	// Get the primary resolution, and convert it to the root index.
	unsigned int nRoot = getRootIndex(index.getPrimaryResolution());

	// If the root is null, return false.
	if (0 == m_vRoots[nRoot])
	{
		return false;
	}

	// Ask the root if it contains the sub index.
	return m_vRoots[nRoot]->contains(index.getSubIndex());
}

//! Expands the tree as necessary to insert the index.
void PYXTileSet::Tree::insert(const PYXIcosIndex& index, bool bAggregate)
{
	// Get the primary resolution, and convert it to the root index.
	unsigned int nRoot = getRootIndex(index.getPrimaryResolution());

	// If the root is null, create it.
	if (0 == m_vRoots[nRoot])
	{
		m_vRoots[nRoot] = new Node(index, bAggregate);
		return;
	}

	// Tell the root to insert the sub index.
	m_vRoots[nRoot]->insert(index, bAggregate);
}

//! Set the resolution.
void PYXTileSet::Tree::setResolution(const unsigned char nResolution)
{
	switch (nResolution)
	{
	case 0:
		// We treat a tree resolution of 0 as "any resolution".
		break;
	case 1:
		// Remove everything but empty roots.
		for (int nRootIndex = 0; nRootIndex < knRootCount; ++nRootIndex)
		{
			Node* const pNode = m_vRoots[nRootIndex];
			if (0 != pNode)
			{
				pNode->setResolution(0);
			}
		}
		break;
	default:
		// Decimate all indexes to 'nResolution'.
		for (std::vector<Node *>::iterator iRoots = m_vRoots.begin();
			iRoots != m_vRoots.end();
			++iRoots)
		{
			Node* const pNode = *iRoots;
			if (0 != pNode)
			{
				// Every index with subindex of length 1 is resolution 2.
				pNode->setResolution(nResolution - 1);
			}
		}
	}
}

//! Clear the set.
void PYXTileSet::Tree::clear()
{
	for (std::vector<Node *>::iterator iRoots = m_vRoots.begin();
		iRoots != m_vRoots.end();
		++iRoots)
	{
		delete *iRoots;
		*iRoots = 0;
	}
}

//! Returns true if empty.
bool PYXTileSet::Tree::empty() const
{
	for (std::vector<Node *>::const_iterator iRoots = m_vRoots.begin();
		iRoots != m_vRoots.end();
		++iRoots)
	{
		if (0 != *iRoots)
		{
			return false;
		}
	}
	return true;
}

// PYXTileSet::Tree::Iterator

/*!
	Advance to next non-empty root node iterator.
	Does not advance past the first index.
*/
void PYXTileSet::Tree::Iterator::nextNonEmptyRootNodeIterator()
{
	m_spiNode.reset();

	for (const size_t nRootCount = m_tree.m_vRoots.size(); m_nRootIndex < nRootCount; ++m_nRootIndex)
	{
		Node * const pNode = m_tree.m_vRoots[m_nRootIndex];
		if (0 != pNode)
		{
			// Update index to be the correct root index (with no descendants).
			assert(m_nRootIndex < knRootCount);
			const int nPrimaryResolution = getPrimaryResolution(m_nRootIndex);
			m_index.setPrimaryResolution(nPrimaryResolution);
			m_index.setResolution(1);

			// Create node iterator.
			m_spiNode.reset(new Node::Iterator(*pNode));

			// That's it.
			break;
		}
	}
}

//! Construct an iterator.
PYXTileSet::Tree::Iterator::Iterator(const Tree& tree) :
	m_tree(tree),
	m_nRootIndex(0),
	m_index(),
	m_spiNode(0)
{
	nextNonEmptyRootNodeIterator();
	if (!end())
	{
		assert(0 != m_spiNode.get());
		m_spiNode->next(m_index.getSubIndex());
	}
}

//! Construct a copy.
PYXTileSet::Tree::Iterator::Iterator(const Iterator& source) :
	m_tree(source.m_tree),
	m_nRootIndex(source.m_nRootIndex),
	m_spiNode((0 == source.m_spiNode.get()) ? 0 : new Node::Iterator(*(source.m_spiNode))),
	m_index(source.m_index)
{
}

//! Destroy the instance.
PYXTileSet::Tree::Iterator::~Iterator()
{
}

//! Move to the next item.
void PYXTileSet::Tree::Iterator::next()
{
	if (end())
	{
		return;
	}

	assert(0 != m_spiNode.get());
	if (m_spiNode->end())
	{
		// Advance root.
		++m_nRootIndex;
		nextNonEmptyRootNodeIterator();
		if (end())
		{
			return;
		}
	}

	// Advance node.
	assert(0 != m_spiNode.get());
	m_spiNode->next(m_index.getSubIndex());
}

/*!
See if we have covered all the items.

\return	true if all items have been covered, otherwise false.
*/
bool PYXTileSet::Tree::Iterator::end() const
{
	return (0 == m_spiNode.get());
}

//! Return the current item.  If past end, return the last item.
const PYXIcosIndex& PYXTileSet::Tree::Iterator::operator *() const
{
	return m_index;
}

// PYXTileSet::Tree::Node

bool PYXTileSet::Tree::Node::containsDigit(const unsigned int nSubstringByte, const unsigned int nDigit)
{
	return BitUtils::isOffsetBitSet(nSubstringByte, nDigit);
}

PYXTileSet::Tree::Node * PYXTileSet::Tree::Node::childNode(const unsigned int nDigit) const
{
	if (m_vChildNodes.empty() || m_vSubstring.empty())
	{
		return 0;
	}
	const unsigned int nByte = m_vSubstring.back();
	if (!BitUtils::isOffsetBitSet(nByte, nDigit))
	{
		return 0;
	}
	const unsigned int nLowerSetBitCount = BitUtils::getLowerSetBitCount(nByte, nDigit);
	Node * const pChild = m_vChildNodes[nLowerSetBitCount];
	assert(0 != pChild);
	return pChild;
}

void PYXTileSet::Tree::Node::copyChildNodes(const Node& source)
{
	removeChildNodes();
	m_vChildNodes.resize(source.m_vChildNodes.size());
	assert(m_vChildNodes.size() == source.m_vChildNodes.size());
	for (size_t nIndex = m_vChildNodes.size(); nIndex > 0; )
	{
		--nIndex;
		Node * const pNode = source.m_vChildNodes[nIndex];
		assert(0 != pNode);
		m_vChildNodes[nIndex] = new Node(*pNode);
	}
}

void PYXTileSet::Tree::Node::clearChildNode(unsigned int nDigit)
{
	if (m_vChildNodes.empty())
	{
		return;
	}
	assert(!m_vSubstring.empty());

	// Get the child index.
	const unsigned int nChildIndex = BitUtils::getLowerSetBitCount(m_vSubstring.back(), nDigit);
	assert(nChildIndex < m_vChildNodes.size());

	// Delete the child and replace with a new, empty one.
	const std::vector<Node *>::iterator iChildNodes = m_vChildNodes.begin() + nChildIndex;
	delete *iChildNodes;
	if (1 == m_vChildNodes.size())
	{
		m_vChildNodes.clear();
	}
	else
	{
		*iChildNodes = new Node();
	}
}

void PYXTileSet::Tree::Node::removeChildNode(unsigned int nDigit)
{
	const size_t nByteCount = m_vSubstring.size();
	if (0 == nByteCount)
	{
		assert(0 == m_vChildNodes.size());
		return;
	}

	// Get the child index.
	const unsigned int nChildIndex = BitUtils::getLowerSetBitCount(m_vSubstring.back(), nDigit);
	assert(nChildIndex < m_vChildNodes.size());

	// Remove it from the list and delete it.
	const std::vector<Node *>::iterator iChildNodes = m_vChildNodes.begin() + nChildIndex;
	delete *iChildNodes;
	m_vChildNodes.erase(iChildNodes);

	// Remove the bit from the last byte.
	m_vSubstring.back() = BitUtils::clearOffsetBit(m_vSubstring.back(), nDigit);
}

// This function simply removes all child nodes.
void PYXTileSet::Tree::Node::removeChildNodes()
{
	for (std::vector<Node *>::iterator iChildNodes = m_vChildNodes.begin();
		iChildNodes != m_vChildNodes.end();
		++iChildNodes)
	{
		delete *iChildNodes;
	}
	m_vChildNodes.clear();
}

bool PYXTileSet::Tree::Node::intersects(const PYXIndex& index, unsigned int nDigitOffset) const
{
	const size_t nByteCount = m_vSubstring.size();
	if (nByteCount == 0)
	{
		return true;
	}

	const unsigned int nDigitCount = index.getDigitCount();
	unsigned int nDigit;
	for (unsigned int nByteIndex = 0; ; ++nDigitOffset)
	{
		// If there are no more digits in the index, we are contained by this index, return true
		if (nDigitOffset >= nDigitCount)
		{
			assert(nDigitOffset == nDigitCount);
			return true;
		}
		nDigit = index.getDigit(nDigitOffset);
		if (!containsDigit(m_vSubstring[nByteIndex], nDigit))
		{
			// It's not in here.
			return false;
		}
		if (++nByteIndex >= nByteCount)
		{
			break;
		}
	}

	assert(nDigitOffset < nDigitCount);
	const Node * const pChild = childNode(nDigit);
	if (0 == pChild)
	{
		// It is either this, or a descendant.
		return true;
	}
	return pChild->intersects(index, ++nDigitOffset);
}

bool PYXTileSet::Tree::Node::contains(const PYXIndex& index, unsigned int nDigitOffset) const
{
	const size_t nByteCount = m_vSubstring.size();
	if (nByteCount == 0)
	{
		return true;
	}

	const unsigned int nDigitCount = index.getDigitCount();
	unsigned int nDigit;
	for (unsigned int nByteIndex = 0; ; ++nDigitOffset)
	{
		// If there are no more digits in the index, return false.
		if (nDigitOffset >= nDigitCount)
		{
			assert(nDigitOffset == nDigitCount);
			return false;
		}
		nDigit = index.getDigit(nDigitOffset);
		if (!containsDigit(m_vSubstring[nByteIndex], nDigit))
		{
			// It's not in here.
			return false;
		}
		if (++nByteIndex >= nByteCount)
		{
			break;
		}
	}

	assert(nDigitOffset < nDigitCount);
	const Node * const pChild = childNode(nDigit);
	if (0 == pChild)
	{
		// It is either this, or a descendant.
		return true;
	}
	return pChild->contains(index, ++nDigitOffset);
}

void PYXTileSet::Tree::Node::insert(const PYXIcosIndex& icosIndex, unsigned int nDigitOffset, bool bAggregate)
{
	insertWithoutLocalAggregation(icosIndex, nDigitOffset, bAggregate);

	VALIDATE_NODE;

	if (bAggregate)
	{
		aggregate(icosIndex, nDigitOffset);

		VALIDATE_NODE;
	}
}

void PYXTileSet::Tree::Node::insertWithoutLocalAggregation(const PYXIcosIndex& icosIndex, unsigned int nDigitOffset, bool bAggregateChild)
{
	const PYXIndex& index = icosIndex.getSubIndex();
	const unsigned int nDigitCount = index.getDigitCount();

	unsigned int nDigit = 7;

	// Deal with the substring.
	if (!m_vSubstring.empty())
	{
		for (unsigned int nByteIndex = 0; ; ++nDigitOffset)
		{
			// If there's nothing left in the index...
			if (nDigitOffset >= nDigitCount)
			{
				// We're adding an ancestor.
				m_vSubstring.resize(nByteIndex); // Wipe out the remaining substring.

				//can clear the nodes
				removeChildNodes();
				break;
			}

			// Get the next digit, and check for a match with the current byte in the substring.
			nDigit = index.getDigit(nDigitOffset);
			assert(nByteIndex < m_vSubstring.size());
			if (!containsDigit(m_vSubstring[nByteIndex], nDigit))
			{
				// Set the branch bit.
				m_vSubstring[nByteIndex] = BitUtils::setOffsetBit(m_vSubstring[nByteIndex], nDigit);

				// Get the set bit count, which will be useful below.
				unsigned int nSetBitCount = BitUtils::getSetBitCount(m_vSubstring[nByteIndex]);

				// If there are no more bytes in the substring...
				if (nByteIndex + 1 >= m_vSubstring.size())
				{
					// If there are no actual child nodes...
					if (m_vChildNodes.empty())
					{
						// If there are more digits in the index, append the rest of them to the substring.
						if (++nDigitOffset < nDigitCount)
						{
							// Create empty nodes for all existing children (none of which have nodes).
							m_vChildNodes.reserve(m_vChildNodes.size() + nSetBitCount + 1);
							do
							{
								m_vChildNodes.push_back(new Node());
							}
							while (--nSetBitCount > 0);

							// Just append the index digits to the child's substring, as bit offsets.
							Node * const pChildNode = childNode(nDigit);
							assert(0 != pChildNode);
							std::vector<char>& vChildBytes = pChildNode->m_vSubstring;
							do
							{
								vChildBytes.push_back(BitUtils::setOffsetBit(0, index.getDigit(nDigitOffset)));
							}
							while (++nDigitOffset < nDigitCount);
						}

						// We're finished.

						VALIDATE_NODE;

						return;
					}
				}
				else // There are more bytes in the substring.
				{
					// Create a new node and move the remainder of this one into it.
					Node * const pChildNode = new Node();
					{
						// Copy substring.
						pChildNode->m_vSubstring.assign(m_vSubstring.begin() + nByteIndex + 1, m_vSubstring.end());

						// Copy child nodes.
						pChildNode->m_vChildNodes = m_vChildNodes;

						// Trim these from 'node'.
						m_vSubstring.resize(nByteIndex + 1);
						m_vChildNodes.clear();
					}
					assert(m_vChildNodes.empty()); // The children were cleared out by the previous line.
					assert(2 == nSetBitCount); // 2 == the existing bit, and the new one that is creating the branch.

					// Make the new node actual child 0.
					m_vChildNodes.reserve(2);
					m_vChildNodes.push_back(pChildNode);
				}
				assert(!m_vChildNodes.empty());

				// Create the child from the remainder of the index.
				assert(nByteIndex + 1 == m_vSubstring.size()); // This is the last byte in the substring.
				const unsigned int nLowerSetBitCount = BitUtils::getLowerSetBitCount(m_vSubstring.back(), nDigit);
				assert(nDigitOffset < nDigitCount);
				m_vChildNodes.insert(m_vChildNodes.begin() + nLowerSetBitCount, new Node(icosIndex, bAggregateChild, nDigitOffset + 1));

				// We're finished.

				VALIDATE_NODE;

				return;
			}

			// Increment to the next byte in the substring.
			// If we are now past the end, we're finished with the substring.
			if (++nByteIndex >= m_vSubstring.size())
			{
				break;
			}
		}
	}

	// At this point, we've gone through all the bytes in the substring and they are contained therein.
	if (nDigitOffset + 1 < nDigitCount) // There are more digits in the index.
	{
		const unsigned int nDigit = index.getDigit(nDigitOffset);
		Node * const pChildNode = childNode(nDigit);
		if (0 != pChildNode)
		{
			// Tell the child to insert it.
			pChildNode->insert(icosIndex, nDigitOffset + 1, bAggregateChild);
		}
	}
	else // There are no more digits in the index.
	{
		if (7 != nDigit) // There was a subscript (otherwise, there are no children).
		{
			// Clear the child node.
			clearChildNode(nDigit);
		}
	}

	VALIDATE_NODE;
}

// If all children are present and empty, aggregate into parent.
void PYXTileSet::Tree::Node::aggregate(const PYXIcosIndex& icosIndex, unsigned int nDigitOffset)
{
	// Iterate through children, and if all are dead ends, remove child nodes.
	if (!m_vChildNodes.empty())
	{
		bool bChildrenEmpty = true;
		for (std::vector<Node *>::const_iterator iChildNodes = m_vChildNodes.begin();
			iChildNodes != m_vChildNodes.end();
			++iChildNodes)
		{
			const Node * pChild = *iChildNodes;
			assert(0 != pChild);
			if (!pChild->empty())
			{
				bChildrenEmpty = false;
				break;
			}
		}
		if (bChildrenEmpty)
		{
			removeChildNodes();
		}
	}

	// If all children are dead ends...
	if (m_vChildNodes.empty())
	{
		// If there is something to aggregate:
		if (!m_vSubstring.empty())
		{
			assert(std::numeric_limits<unsigned int>::max() - nDigitOffset >= m_vSubstring.size());
			unsigned int nSubIndexDigitCount = nDigitOffset + m_vSubstring.size();
			do
			{
				// For the icos index corresponding to the parents + the substring (not including back byte),
				// get the maximum number of children.
				assert(nSubIndexDigitCount > 0);
				unsigned int nMaximumChildCount = icosIndex.getMaximumChildCount(--nSubIndexDigitCount);

				// If we cannot aggregate, return.
				if (childNodeCount() < nMaximumChildCount)
				{
					return;
				}

				// Remove the back byte of the substring.
				m_vSubstring.pop_back();
			}
			while (!m_vSubstring.empty());
		}
	}
}

//! Construct an empty node.
PYXTileSet::Tree::Node::Node()
{
}

//! Construct a copy.
PYXTileSet::Tree::Node::Node(const Node& source) :
	ObjectMemoryUsageCounter<Node>(source),
	m_vSubstring(source.m_vSubstring),
	m_vChildNodes(source.m_vChildNodes.size())
{
	copyChildNodes(source);
}

PYXTileSet::Tree::Node::Node(std::basic_istream<char>& in)
{
	// Iterate through until we find a terminator or a multi-bit (creating branches accordingly).
	if (!in.good())
	{
		return;
	}

	for (unsigned char byte = 0; ; )
	{
		char c;
		in.get(c);
		byte =  (c);
		if (!in.good())
		{
			break;
		}

		const bool wasSignBitSet = BitUtils::isSignBitSet(byte);
		byte = BitUtils::clearSignBit(byte);
		if (0 != byte)
		{
			m_vSubstring.push_back(byte);
		}
		if (wasSignBitSet)
		{
			break;
		}

		unsigned int nSetBitCount = BitUtils::getSetBitCount(byte);
		if (nSetBitCount == 0)
		{
			PYXTHROW(PYXException, "Error reading node.  No bits are set.");
		}

		if (nSetBitCount > 1)
		{
			// Populate child array with branches.
			m_vChildNodes.reserve(nSetBitCount);
			do
			{
				m_vChildNodes.push_back(new Node(in));
			}
			while (--nSetBitCount > 0);
			break;
		}
	}
}

//! Copy the bytes from 'index'.
PYXTileSet::Tree::Node::Node(const PYXIcosIndex& icosIndex, bool bAggregate, unsigned int nDigitOffset)
{
	const PYXIndex& index = icosIndex.getSubIndex();
	const unsigned int nOriginalDigitOffset = nDigitOffset;

	const size_t nDigitCount = index.getDigitCount();
	if (nDigitOffset >= nDigitCount)
	{
		return;
	}

	m_vSubstring.reserve(nDigitCount - nDigitOffset);
	do
	{
		// Push the position byte for the digit.
		const unsigned char byte = BitUtils::setOffsetBit(0, index.getDigit(nDigitOffset));
		m_vSubstring.push_back(byte);
	}
	while (++nDigitOffset < nDigitCount);

	if (bAggregate)
	{
		aggregate(icosIndex, nOriginalDigitOffset);
		VALIDATE_NODE;
	}
}

PYXTileSet::Tree::Node::~Node()
{
	removeChildNodes();
}

//! Assign a copy.
PYXTileSet::Tree::Node& PYXTileSet::Tree::Node::operator =(const Node& source)
{
	if (this != &source)
	{
		// Copy the substring.
		m_vSubstring = source.m_vSubstring;

		// Copy the child nodes.
		copyChildNodes(source);
	}
	assert(0 != this);
	return *this;
}

//! Serialize to stream.
void PYXTileSet::Tree::Node::serialize(std::basic_ostream<char>& out) const
{
	const size_t nByteCount = m_vSubstring.size();
	const size_t nChildCount = m_vChildNodes.size();

	// Write all the bytes in the substring except the last one.
	for (unsigned int nIndex = 0; nIndex + 1 < nByteCount; ++nIndex)
	{
		assert(nIndex < m_vSubstring.size());
		out.put(m_vSubstring[nIndex]);
	}

	// Write the last one, adding a terminator as necessary.
	{
		unsigned char byte = 0;
		if (nByteCount > 0)
		{
			assert(nByteCount - 1 < m_vSubstring.size());
			byte |= m_vSubstring[nByteCount - 1];
		}
		if (nChildCount == 0)
		{
			byte |= BitUtils::knSignBitMask;
		}
		if (0 == byte)
		{
			PYXTHROW( PYXException, "Error serializing node.  If nChildCount > 0, nByteCount must be > 0 to hold bit mapping byte.");
		}
		out.put(byte);
	}

	// Write children.
	for (unsigned int nIndex = 0; nIndex < nChildCount; ++nIndex)
	{
		// Write each child.
		assert(0 != m_vChildNodes[nIndex]);
		m_vChildNodes[nIndex]->serialize(out);
	}
}

//! Set the resolution, relative to the start of the node.
void PYXTileSet::Tree::Node::setResolution(unsigned char nResolution)
{
	const size_t nSubstringSize = m_vSubstring.size();

	if (nSubstringSize <= nResolution)
	{
		nResolution -= static_cast<unsigned char>(nSubstringSize);

		// For each actual child, set resolution to nResolution.
		const size_t nChildCount = m_vChildNodes.size();
		for (unsigned int nIndex = 0; nIndex < nChildCount; ++nIndex)
		{
			assert(0 != m_vChildNodes[nIndex]);
			m_vChildNodes[nIndex]->setResolution(nResolution);
		}

		return;
	}

	// Drop all the children.
	removeChildNodes();

	// Truncate substring.
	m_vSubstring.resize(nResolution);
}

bool PYXTileSet::Tree::Node::intersects(const PYXIndex& index) const
{
	return intersects(index, 0);
}

bool PYXTileSet::Tree::Node::contains(const PYXIndex& index) const
{
	return contains(index, 0);
}

void PYXTileSet::Tree::Node::insert(const PYXIcosIndex& icosIndex, bool bAggregate)
{
	insert(icosIndex, 0, bAggregate);
}

unsigned int PYXTileSet::Tree::Node::childNodeCount() const
{
	return m_vSubstring.empty() ? 0 : BitUtils::getSetBitCount(m_vSubstring.back());
}

//! Return true if the node is empty (no substring).
bool PYXTileSet::Tree::Node::empty() const
{
	return m_vSubstring.empty();
}

#ifdef PERFORM_TREE_VALIDATION
void PYXTileSet::Tree::Node::validate() const
{
	if (m_vSubstring.size()>0)
	{
		size_t bitCount = BitUtils::getSetBitCount(m_vSubstring.back());
		if (bitCount == 1) assert(m_vChildNodes.size()==0);
		else assert(bitCount == m_vChildNodes.size() || m_vChildNodes.empty());
	}
	else 
	{
		assert(m_vChildNodes.empty());
	}
}
#endif

// PYXTileSet::Tree::Node::Iterator

//! Construct an iterator.
PYXTileSet::Tree::Node::Iterator::Iterator(Node& node) :
	m_node(node),
	m_nVisitedChildCount(0),
	m_spChildNodeIterator(),
	m_nResolution(0)
{
}

//! Construct a copy.
PYXTileSet::Tree::Node::Iterator::Iterator(const Iterator& source) :
	m_node(source.m_node),
	m_nVisitedChildCount(source.m_nVisitedChildCount),
	m_spChildNodeIterator((0 == source.m_spChildNodeIterator.get()) ? 0 : new Node::Iterator(*(source.m_spChildNodeIterator))),
	m_nResolution(source.m_nResolution)
{
}

//! Destroy the instance.
PYXTileSet::Tree::Node::Iterator::~Iterator()
{
}

//! Iterate past the next index and update 'index' in the process.
void PYXTileSet::Tree::Node::Iterator::next(PYXIndex& index)
{
	const unsigned int nChildNodeCount = m_node.childNodeCount();
	if (m_nVisitedChildCount > nChildNodeCount)
	{
		return;
	}

	// Deal with the substring.
	if (0 == m_nVisitedChildCount)
	{
		assert(0 == m_spChildNodeIterator.get());

		// Copy the substring bytes (excluding the last) into index.
		for (std::vector<char>::const_iterator iSubstring = m_node.m_vSubstring.begin();
			iSubstring != m_node.m_vSubstring.end();
			++iSubstring)
		{
			// Get offset of highest substring bit and set it in index.
			const unsigned int nSubstringByte = *iSubstring;
			assert(0 < BitUtils::getSetBitCount(nSubstringByte));
			const unsigned int nDigit = BitUtils::getSetBitOffset(nSubstringByte, 0);
			index.appendDigit(nDigit);
		}

		// Store the current index resolution.
		m_nResolution = index.getResolution();

		// We have now visited the substring.
		// If there are no child nodes, we're done.
		if (++m_nVisitedChildCount > nChildNodeCount)
		{
			return;
		}
	}
	assert(0 < m_nVisitedChildCount);

	// Go to the next child?
	if (0 == m_spChildNodeIterator.get() || m_spChildNodeIterator->end())
	{
		// Restore index to the proper resolution (prior to last byte).
		index.setResolution(m_nResolution - 1);

		// Convert the substring byte to an index digit, and set it in the index.
		assert(!m_node.m_vSubstring.empty());
		const unsigned int nDigit = BitUtils::getSetBitOffset(m_node.m_vSubstring.back(), m_nVisitedChildCount - 1);
		index.appendDigit(nDigit);

		// If there are children without actual nodes, increment the visited child count and return.
		if (m_node.m_vChildNodes.empty())
		{
			assert(0 == m_spChildNodeIterator.get());
			++m_nVisitedChildCount;
			return;
		}

		// There are actual child nodes, so create next child iterator.
		Node * const pChildNode = m_node.m_vChildNodes[m_nVisitedChildCount - 1];
		assert(0 != pChildNode);
		m_spChildNodeIterator.reset(new Iterator(*pChildNode));
	}
	assert(0 != m_spChildNodeIterator.get());

	// Go to next (or first).
	m_spChildNodeIterator->next(index);

	// If we've finished a child, update the visited child count.
	if (m_spChildNodeIterator->end())
	{
		++m_nVisitedChildCount;
	}
}

/*!
See if we have covered all the items.

\return	true if all items have been covered, otherwise false.
*/
bool PYXTileSet::Tree::Node::Iterator::end() const
{
	return (m_nVisitedChildCount > m_node.childNodeCount());
}
