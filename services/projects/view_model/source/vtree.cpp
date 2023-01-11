/******************************************************************************
vtree.cpp

begin		: 2008-04-25
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "vtree.h"

// view model includes
#include "addr_utils.h"
#include "pyxtree_utils.h"

// pyxlib includes
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/geometry/hit_test_utils.h"
#include "pyxis/utility/coord_3d.h"

// boost includes

// standard includes
#include <algorithm>
#include <set>

////////////////////////////////////////////////////////////////////////////////

//! Compare fragment nodes by begin ID.
struct FragmentCompareByBeginID
{
	bool operator() (const Fragment& lhs, const Fragment& rhs)
	{
		return lhs.m_nBegin < rhs.m_nBegin;
	}
};

//! Compare fragment nodes by end ID.
struct FragmentCompareByEndID
{
	bool operator() (const Fragment& lhs, const Fragment& rhs)
	{
		return lhs.m_nEnd < rhs.m_nEnd;
	}
};

//! Compare feature nodes by ID.
struct FeatureNodeCompareByID
{
	bool operator() (const FeatureNode& lhs, const FeatureNode& rhs)
	{
		return lhs.m_nFid < rhs.m_nFid;
	}
};

////////////////////////////////////////////////////////////////////////////////

Fragment& FeatureNode::insertPoint(Fragment::PointIDType nPid)
{
	// Get approximate location of relevant fragment.
	std::vector<Fragment>::iterator it = lowerBoundPoint(nPid);

	// TODO it's quite likely this can be simplified or otherwise cleaned up
	// to avoid extra work (e.g. insert then erase) and be clearer code.

	if (it != m_vecFragment.end() && it->m_nBegin == nPid && it->m_nEnd == nPid)
	{
		// Extend fragment to include point.
		++it->m_nEnd;
	}
	else if (it == m_vecFragment.end() || (nPid < it->m_nBegin || it->m_nEnd <= nPid))
	{
		// Insert new fragment for point.
		it = m_vecFragment.insert(it, Fragment(nPid, nPid + 1));
	}

	if ((it+1) != m_vecFragment.end()
		&& it->m_nEnd == (it+1)->m_nBegin && (it+1)->m_nBegin == (it+1)->m_nEnd)
	{
		// Next fragment is single line segment after this point so it can be
		// omitted.
		it = m_vecFragment.erase(it+1) - 1;
	}

	return *it;
}

Fragment& FeatureNode::insertSegment(Fragment::PointIDType nPid)
{
	// Get approximate location of relevant fragment.
	std::vector<Fragment>::iterator it = lowerBoundPoint(nPid);

	bool bInPrevFragment = it != m_vecFragment.begin() && nPid <= (it - 1)->m_nEnd;
	bool bInThisFragment = it != m_vecFragment.end() && it->m_nBegin <= nPid;

	if (!(bInPrevFragment || bInThisFragment))
	{
		it = m_vecFragment.insert(it, Fragment(nPid, nPid));
	}
	else if (bInPrevFragment)
	{
		--it;
	}

	return *it;
}

/*!
Returns an iterator to the fragment containing the point, or the next fragment, or end.
<pre>
[0,2)      lowerBound(0) returns begin+0
[0,2)      lowerBound(1) returns begin+0
[0,2)      lowerBound(2) returns begin+1
[0,2)[3,3) lowerBound(2) returns begin+1
[0,2)[3,3) lowerBound(3) returns begin+1
[0,2)[3,3) lowerBound(4) returns begin+2
</pre>
*/
std::vector<Fragment>::iterator FeatureNode::lowerBoundPoint(Fragment::PointIDType nPid)
{
	// Search by begin ID.
	Fragment dummy(nPid, nPid);
	std::vector<Fragment>::iterator it =
		std::lower_bound(m_vecFragment.begin(), m_vecFragment.end(), dummy, FragmentCompareByBeginID());
	// But also check previous fragment's end ID.
	if (it != m_vecFragment.begin() && nPid < (it - 1)->m_nEnd)
	{
		--it;
	}
	return it;
}

////////////////////////////////////////////////////////////////////////////////

FeatureNode& VTreeNode::insertFeatureNode(FeatureNode::FeatureIDType nFid)
{
	FeatureNode dummy(nFid);
	std::vector<FeatureNode>::iterator it =
		std::lower_bound(m_vecFeatureNode.begin(), m_vecFeatureNode.end(), dummy, FeatureNodeCompareByID());
	if (it == m_vecFeatureNode.end() || it->m_nFid != nFid)
	{
		// Create feature node.
		it = m_vecFeatureNode.insert(it, dummy);
		// TODO with a bunch of optimized swap routines, it may be more efficient to write a
		// custom loop doing the swaps to move nodes
	}
	return *it;
}

FeatureNode* VTreeNode::queryFeatureNode(FeatureNode::FeatureIDType nFid)
{
	FeatureNode dummy(nFid);
	std::vector<FeatureNode>::iterator it =
		std::lower_bound(m_vecFeatureNode.begin(), m_vecFeatureNode.end(), dummy, FeatureNodeCompareByID());
	return (it != m_vecFeatureNode.end() && it->m_nFid == nFid) ? &*it : 0;
}

////////////////////////////////////////////////////////////////////////////////

/*!
This is basically a copy of pyxtreeInsert which ensures that feature IDs are
added to the tree nodes at intermediate resolutions (except for the first two).
*/
FeatureNode& vtreeInsertFeatureNode(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const Addr& addr)
{
	VTreeNG* pt = &vtree;
	FeatureNode* pFeatureNode;

	int n = 0;
	while (n <= addr.maxn)
	{
		int nDigit = addr[n];

		if (nDigit == 0xf)
		{
			break;
		}

		if (!pt)
		{
			break;
		}

		pt = &(*pt)[nDigit];

		if (2 <= n)
		{
			pFeatureNode = &pt->data().insertFeatureNode(nFid);
		}

		++n;
	}

	return *pFeatureNode;
}

/*!
\return A pair containing a pointer to the feature node and the index of the fragment.
*/
std::pair<FeatureNode*, int> vtreeInsertPoint(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, Fragment::PointIDType nPid, const PYXIcosIndex& index)
{
	// Get node for feature. This will create the node if none exists, in which case it will be empty.
	FeatureNode& featureNode = vtreeInsertFeatureNode(vtree, nFid, indexToAddr(index));
	assert(featureNode.m_nFid == nFid);

	// Insert point into feature. This will ensure that a fragment containing the point exists.
	Fragment& fragment = featureNode.insertPoint(nPid);

	return std::make_pair(&featureNode, &fragment - &featureNode.m_vecFragment[0]);
}

/*!
\return A pair containing a pointer to the feature node and the index of the fragment.
*/
std::pair<FeatureNode*, int> vtreeInsertSegment(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, Fragment::PointIDType nPid, const PYXIcosIndex& index)
{
	// Get node for feature. This will create the node if none exists, in which case it will be empty.
	FeatureNode& featureNode = vtreeInsertFeatureNode(vtree, nFid, indexToAddr(index));
	assert(featureNode.m_nFid == nFid);

	// Insert point into feature. This will ensure that a fragment containing the point exists.
	Fragment& fragment = featureNode.insertSegment(nPid);

	return std::make_pair(&featureNode, &fragment - &featureNode.m_vecFragment[0]);
}

FeatureNode* vtreeQueryFeatureNode(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index)
{
	assert(PYXIcosIndex::knMinSubRes <= index.getResolution());
	const PYXIndex& subindex = index.getSubIndex();
	const int nDigits = subindex.getDigitCount();

	// Descend the first levels of the tree.
	int nRoot = encodeIndexRoot(index);
	VTreeNG* p = vtree.getChild(nRoot>>4);
	if (p)
	{
		p = p->getChild(nRoot&0x7);
	}

	// Descend the subsequent levels of the tree. Abort if we do not find an entry for the feature.
	FeatureNode* pFeatureNode = reinterpret_cast<FeatureNode*>(p); // let p proxy for pFeatureNode for first test
	for (int nPos = 0; pFeatureNode && nPos != nDigits; ++nPos)
	{
		int nDigit = subindex.getDigit(nPos);
		p = p->getChild(nDigit);
		pFeatureNode = p ? (**p).queryFeatureNode(nFid) : 0;
	}

	return pFeatureNode;
}

/*!
Returns whether a fragment list for a feature for a cell requires further refinement.
*/
bool requiresRefinement(const std::vector<Fragment>& vecFragment)
{
	if (2 < vecFragment.size())
	{
		// Too many fragments, must refine.
		return true;
	}

	if (2 == vecFragment.size())
	{
		// We allow two adjacent fragments of line segments.
		return vecFragment[0].m_nBegin != vecFragment[0].m_nEnd
			|| vecFragment[1].m_nBegin != vecFragment[1].m_nEnd
			|| vecFragment[0].m_nBegin != vecFragment[1].m_nBegin - 1;
	}

	// We allow a fragment to contain a single point.
	return !vecFragment.empty() && 1 < (vecFragment[0].m_nEnd - vecFragment[0].m_nBegin);
}

//! Merges fragments from second list into the first. Does not simplify.
void mergeInFragments(std::vector<Fragment>& vecFragment, const std::vector<Fragment>& vecFragmentIncoming)
{
	std::vector<Fragment> vecTemp;
	std::set_union(vecFragment.begin(), vecFragment.end(),
		vecFragmentIncoming.begin(), vecFragmentIncoming.end(),
		std::back_inserter(vecTemp));
	vecFragment.swap(vecTemp);
}

void mergeInFragments(const VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index, std::vector<Fragment>& vecFragment)
{
	// TODO const_cast here for now
	FeatureNode* pFeatureNode = vtreeQueryFeatureNode(const_cast<VTreeNG&>(vtree), nFid, index);
	if (pFeatureNode)
	{
		mergeInFragments(vecFragment, pFeatureNode->m_vecFragment);
	}
}

//! Merges fragments from second list into the first. Does not simplify.
void mergeInFinalFragments(std::vector<Fragment>& vecFragment, const std::vector<Fragment>& vecFragmentIncoming)
{
	if (!requiresRefinement(vecFragmentIncoming))
	{
		std::vector<Fragment> vecTemp;
		std::set_union(vecFragment.begin(), vecFragment.end(),
			vecFragmentIncoming.begin(), vecFragmentIncoming.end(),
			std::back_inserter(vecTemp));
		vecFragment.swap(vecTemp);
	}
}

void mergeInFinalFragments(const VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index, std::vector<Fragment>& vecFragment)
{
	// TODO const_cast here for now
	FeatureNode* pFeatureNode = vtreeQueryFeatureNode(const_cast<VTreeNG&>(vtree), nFid, index);
	if (pFeatureNode)
	{
		mergeInFinalFragments(vecFragment, pFeatureNode->m_vecFragment);
	}
}

//! Merges fragments from second list into the first. Does not simplify.
void mergeInNonFinalFragments(std::vector<Fragment>& vecFragment, const std::vector<Fragment>& vecFragmentIncoming)
{
	if (requiresRefinement(vecFragmentIncoming))
	{
		std::vector<Fragment> vecTemp;
		std::set_union(vecFragment.begin(), vecFragment.end(),
			vecFragmentIncoming.begin(), vecFragmentIncoming.end(),
			std::back_inserter(vecTemp));
		vecFragment.swap(vecTemp);
	}
}

void mergeInNonFinalFragments(const VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index, std::vector<Fragment>& vecFragment)
{
	// TODO const_cast here for now
	FeatureNode* pFeatureNode = vtreeQueryFeatureNode(const_cast<VTreeNG&>(vtree), nFid, index);
	if (pFeatureNode)
	{
		mergeInNonFinalFragments(vecFragment, pFeatureNode->m_vecFragment);
	}
}

bool isFeatureNodeFinal(const VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index)
{
	FeatureNode* pFeatureNode = vtreeQueryFeatureNode(const_cast<VTreeNG&>(vtree), nFid, index);
	return pFeatureNode ? !requiresRefinement(pFeatureNode->m_vecFragment) : true;
}

//! Returns true if a feature node is fully covered by final cells.
bool isFeatureNodeFullyCovered(const VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index)
{
	// This code uses spatial containment and slices to determine when a cell
	// is fully covered by cells that require no refinement.
	std::vector<PYXIcosIndex> vecCovering;
	std::vector<short> vecSlice;
	PYXIcosMath::getAllCoveringCells(index, vecCovering, vecSlice);
	int nSlice = 0;
	for (int n = 0; n != static_cast<int>(vecCovering.size()); ++n)
	{
		if (isFeatureNodeFinal(vtree, nFid, vecCovering[n]))
		{
			nSlice |= vecSlice[n];
			if (nSlice == 0x0fff)
			{
				return true;
			}
		}
	}
	return false;
}

//! Returns the slice code for cells which fully cover this cell.
int getFeatureNodeFinalSlices(const VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index)
{
	// This code uses spatial containment and slices to determine when a cell
	// is fully covered by cells that require no refinement.
	std::vector<PYXIcosIndex> vecCovering;
	std::vector<short> vecSlice;
	PYXIcosMath::getAllCoveringCells(index, vecCovering, vecSlice);
	int nSlice = 0;
	for (int n = 0; n != static_cast<int>(vecCovering.size()); ++n)
	{
		if (isFeatureNodeFinal(vtree, nFid, vecCovering[n]))
		{
			nSlice |= vecSlice[n];
		}
	}
	return nSlice;
}

/*!
The purpose of this function is to change a set of fragments (described by
point ranges with implicit line segments) into a set of point ranges which
fully covers those fragments (e.g. for creating sub-curves for hit testing).

One issue is that the expansion of the range should not go beyond the number
of points (just as it should not go below zero). This is a bit tricky as it
requires knowing the number of points. It's not a problem if we adjust it
later, but if we start supporting multi-curves we shouldn't be merging
ranges across curve parts.
*/
void simplifyFragmentsToPointRanges(std::vector<Fragment>& vecFragment)
{
	// First make a pass to expand all fragments to point ranges.
	for (int n = 0; n != vecFragment.size(); ++n)
	{
		if (vecFragment[n].m_nBegin)
		{
			--vecFragment[n].m_nBegin;
		}
		if (vecFragment[n].m_nEnd)
		{
			// Note this may go beyond the end
			++vecFragment[n].m_nEnd;
		}
	}

	// Next merge them if they overlap.
	for (int n = 1; n < static_cast<int>(vecFragment.size()); ++n)
	{
		if (vecFragment[n].m_nBegin < vecFragment[n-1].m_nEnd)
		{
			vecFragment[n-1].m_nEnd = vecFragment[n].m_nEnd;
			vecFragment.erase(vecFragment.begin() + n--); // note alteration of loop variable
		}
	}
}

/*!
The purpose of this function is to remove line segments that are already
implicit due to neighbouring point ranges.
*/
void simplifyFragments(std::vector<Fragment>& vecFragment)
{
	for (int n = 0; n != vecFragment.size(); ++n)
	{
		if (vecFragment[n].m_nBegin == vecFragment[n].m_nEnd)
		{
			// This fragment is a line segment. See if it can be
			// omitted because we already have one of the points.
			if (n != 0 && vecFragment[n-1].m_nEnd == vecFragment[n].m_nBegin
					&& vecFragment[n-1].m_nBegin != vecFragment[n-1].m_nEnd
				|| n+1 != vecFragment.size() && vecFragment[n].m_nEnd == vecFragment[n+1].m_nBegin
					&& vecFragment[n+1].m_nBegin != vecFragment[n+1].m_nEnd)
			{
				vecFragment.erase(vecFragment.begin() + n--); // note alteration of loop variable
			}
		}
	}
}

/*!
This function queries a vector tree for the fragments of a feature's geometry
relevant to a cell.

The algorithm is conceptually simple. Use the entry for the cell (if it
exists), but also any entry of any covering cell (if it is fully refined).
This means inspecting every entry on the path from root to child (i.e. all
ancestors), but remember there are also non-ancestors that cover the cell.
The easiest way to ensure they are all found is to use the other two
covering cells of any vertex child (non-zero digit) in the tree path. This
yields false positives but no false negatives.

For example, suppose you want to query 3-005030. You must check 3-0, 3-00,
3-005, 3-0050, 3-00503, and 3-005030 of course. But 3-005 is a vertex
child and also covered by 3-04 and 3-05, so check those cells for final
entries too. Likewise, 3-00503 is a vertex child which requires checking
its other covering cells 3-004 and 3-0401.

Another example, consider 3-005020. As a vertex child, 3-00502 is covered
by 3-0004 and 3-0005, so we check those. And we might also check 3-04 and
3-05 as before (because 3-005 is a vertex child), but in this case it's not
necessary because 3-005020 is fully covered by its ancestor 3-00. Without
rich spatial containment functions, we don't know this ahead of time, so
we play it safe and live with the false positives.

\param vtree	The vector tree.
\param nFid		The feature ID.
\param index	The cell.
\return The fragments (of point ranges) necessary to reconstruct the feature's
	geometry for the specified cell.
*/
std::vector<Fragment> vtreeQueryFragments(const VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXIcosIndex& index)
{
	std::vector<Fragment> vecFragment;

	// Keep track of which slices are covered by final cells.
	int nSlice = 0;

	if (PYXIcosIndex::knMinSubRes < index.getResolution())
	{
		// Get all covering cells.
		std::vector<PYXIcosIndex> vecCovering;
		std::vector<short> vecSlice;
		PYXIcosMath::getAllCoveringCells(index, vecCovering, vecSlice);

		for (int n = 0; n != static_cast<int>(vecCovering.size()); ++n)
		{
			if ((nSlice | vecSlice[n]) != nSlice) // cell covers uncovered slices?
			{
				// Check the tree for a feature node, and see if it is final.
				FeatureNode* pFeatureNode = vtreeQueryFeatureNode(const_cast<VTreeNG&>(vtree), nFid, vecCovering[n]); // TODO const_cast here for now
				if (pFeatureNode && !requiresRefinement(pFeatureNode->m_vecFragment))
				{
					// Update covered slices and merge in fragments.
					nSlice |= vecSlice[n];
					mergeInFragments(vecFragment, pFeatureNode->m_vecFragment);
				}
			}
		}
	}

	// If cell isn't fully covered, use its fragments if they exist,
	// regardless of whether they are final or not.
	if (nSlice != 0x0fff)
	{
		FeatureNode* pFeatureNode = vtreeQueryFeatureNode(const_cast<VTreeNG&>(vtree), nFid, index); // TODO const_cast here for now
		if (pFeatureNode)
		{
			mergeInFragments(vecFragment, pFeatureNode->m_vecFragment);
		}
	}

	// Simplify the fragments.
	simplifyFragments(vecFragment);
	//simplifyFragmentsToPointRanges(vecFragment);

	return vecFragment;
}

void vtreeInsertFeatureCurve(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, const PYXCurve& curve)
{
	const int nPoints = static_cast<int>(curve.getNodes().size());

	// Fill vecPoint with the curve's nodes in XYZ coordinates.
	std::vector<PYXCoord3DDouble> vecPoint(nPoints);
	for (int n = 0; n != nPoints; ++n)
	{
		SnyderProjection::getInstance()->pyxisToXYZ(curve.getNodes()[n], &vecPoint[n]);
	}

	// Delegate work.
	vtreeInsertFeatureCurve(vtree, nFid, nPoints, &vecPoint[0]);
}

/*!
This function takes a curve, dissects it into little itty bitty pieces, and
shoves it mercilessly into hexagonal cells of various sizes. Kinda cruel but
all in the name of science.

The algorithm is conceptually simple. First, we bin the curve into the lowest
res. We do this by binning each point, using Snyder projection. This produces
a certain answer, but we can't be sure that any line between two points in
the same cell is fully contained within the cell. So we only assume this to
be true if both points are well within the same cell.

If either point is not well within the cell, or they are within different
cells, we must test the line segment between them. Again, our test is
uncertain so we live with false positives to avoid false negatives.

So if a cell's entry is [5,6), we are saying the cell definitely contains
point 5, possibly including the curve some ways back to point 4, and forward
to point 6, but not containing points 4 or 6.

An entry for [5,7) means the cell contains a fragment from before point 5 to
after point 6, and the cell definitely contains both those points, and the
entire line segment between them, without going outside the cell.

An entry for [5,5)[6,6) means the cell may contain fragments leading to
points 5 and 6, but certainly does not include points 5 or 6.

Note that an entry for [5,6)[6,6) is redundant, as [5,6) already denotes the
fact that there is some fragment towards point 6 present over the cell.

Now, having produced entries for cells, some will contain more fragments, or
more complex fragments. We will call an entry "final" if it contains either a
single point like [5,6), or two adjacent line segments like [5,5)[6,6). This
is convenient because in practical terms, when the time comes to perform
geospatial operations, both will require the curve from point 4 to point 6 to
actually test against.

Entries that are not final will require refinement. For each, its 6 or 7
covered cells are generated. When we process the next resolution, we take
those cells in turn, and refine the fragments from the previous resolution's
non-final cells. We take care to merge these fragments before processing them
at this higher resolution; this way we avoid the problem of having split the
curve at a lower resolution, having that split forced on us at a higher
resolution unnecessarily.

And so portions of the curve that are simple (few points for an area) will
contain entries in the lower resolution of the tree, while portions of the
curve that are more complex (more points in a smaller area) will be refined to
higher resolutions as necessary. But we do not refine areas and curve
fragments that have already been refined (to avoid redundancy in the vector
tree). This is why, when we later query the vector tree, we'll have to
additionally search for refined cells at lower resolutions that cover the cell
of interest.

\param vtree		The vector tree.
\param nFid			The feature ID.
\param nPoints		The number of points.
\param pPointData	The point XYZ data.
*/
void vtreeInsertFeatureCurve(VTreeNG& vtree, FeatureNode::FeatureIDType nFid, int nPoints, const PYXCoord3DDouble* pPointData)
{
	// Fill vecNormal with a normal for each line segment between points. We'll
	// be using these normals to test line segments.
	std::vector<PYXCoord3DDouble> vecNormal(nPoints - 1);
	for (int n = 0; n != nPoints - 1; ++n)
	{
		vecNormal[n] = pPointData[n].cross(pPointData[n + 1]);
		vecNormal[n].normalize();
	}

	// Start at this resolution.
	int nRes = PYXIcosIndex::knMinSubRes;

	// At any given time we will be refining a contiguous fragment from this
	// list. We start with one fragment which contains the entire curve.
	std::vector<Fragment> vecFragment(1, Fragment(0, nPoints));

	// setRefineCurrent is the set of cells we are currently refining.
	// setRefineNext is where, after processing a cell, we will note if it
	// requires further refinement. Both of these sets contain cells at the
	// same resolution. setRefinePrev contains cells at the previous
	// resolution that required refinement. That is, it contains the
	// contents of setRefineNext for the previous resolution.
	std::set<PYXIcosIndex> setRefinePrev;
	std::set<PYXIcosIndex> setRefineCurrent;
	std::set<PYXIcosIndex> setRefineNext;

	// This will contain the cells which are used to test line segments in the
	// fragment that is being processed. We start with the whole world at the
	// first res, then test against cells individually as we refine.
	std::vector<PYXIcosIndex> vecCellsToTest;
	for (PYXIcosIterator it(nRes); !it.end(); it.next())
	{
		vecCellsToTest.push_back(it.getIndex());
	}

	while (!vecFragment.empty())
	{
		// We need to transform the fragment to process into a series of points
		// (and implicit line segments) to process. This basically involves
		// expanding the range, and noting whether we are supposed to process
		// the first and last points.
		int nPidBegin = vecFragment.back().m_nBegin;
		int nPidEnd = vecFragment.back().m_nEnd;
		vecFragment.pop_back();
		bool bProcessFirstPoint = true;
		if (nPidBegin != 0)
		{
			--nPidBegin;
			bProcessFirstPoint = false;
		}
		bool bProcessLastPoint = true;
		if (nPidEnd != nPoints)
		{
			++nPidEnd;
			bProcessLastPoint = false;
		}

		// Point A is the first point on the line segment.
		PYXIcosIndex indexA;
		SnyderProjection::getInstance()->xyzToPYXIS(pPointData[nPidBegin], &indexA, nRes);
		bool bSureA = HitTestUtils::intersectPointCellCertain(pPointData[nPidBegin], indexA);
		std::pair<FeatureNode*, int> current;
		if (bProcessFirstPoint)
		{
			current = vtreeInsertPoint(vtree, nFid, nPidBegin, indexA);
			if (1 < current.first->m_vecFragment.size())
			{
				setRefineNext.insert(indexA);
			}
		}
		else
		{
			current.first = 0; // effectively disable this
		}

		// Process fragment, a point (and implicit line segment) at a time.
		for (int nPid = nPidBegin + 1; nPid < nPidEnd; ++nPid)
		{
			// Point B is the last point on the line segment.
			PYXIcosIndex indexB;
			SnyderProjection::getInstance()->xyzToPYXIS(pPointData[nPid], &indexB, nRes);
			bool bSureB = HitTestUtils::intersectPointCellCertain(pPointData[nPid], indexB);
			bool bLast = (nPid == nPidEnd - 1);
			if (!bProcessLastPoint && bLast)
			{
				current.first = 0; // effectively disable this
			}

			if (current.first && (bSureA && bSureB) && indexA == indexB) // TODO decide AND or OR
			{
				// If both A and B are well within the same cell, we can assume the line segment
				// is also fully contained within the same cell.

				// Expand the current fragment to include this point (if necessary).
				if (current.first->m_vecFragment[current.second].m_nEnd <= (unsigned int)nPid)
				{
					++current.first->m_vecFragment[current.second].m_nEnd;
					setRefineNext.insert(indexB); // must refine
				}
			}
			else
			{
				// If A and B are not both well within the same cell, we can't assume the line
				// segment is fully contained within a single cell.

				// Test line segment against all cells.
				for (std::vector<PYXIcosIndex>::iterator it = vecCellsToTest.begin();
					it != vecCellsToTest.end(); ++it)
				{
					int nResult = HitTestUtils::intersectSegmentCellUncertain(	pPointData[nPid - 1],
																				pPointData[nPid],
																				vecNormal[nPid - 1],
																				*it	);
					if (nResult != HitTestUtils::knMiss)
					{
						// Insert new line segment.
						std::pair<FeatureNode*, int> temp = vtreeInsertSegment(vtree, nFid, nPid, *it);
						if (requiresRefinement(temp.first->m_vecFragment))
						{
							setRefineNext.insert(*it);
						}
					}
				}

				// Insert new point.
				if (bProcessLastPoint || !bLast)
				{
					current = vtreeInsertPoint(vtree, nFid, nPid, indexB);
					if (requiresRefinement(current.first->m_vecFragment))
					{
						setRefineNext.insert(indexB);
					}
				}
			}

			// Update to the next line segment.
			indexA = indexB;
			bSureA = bSureB;
		}

		if (vecFragment.empty())
		{
			// No more fragments to process. Get more from next cell to refine.

			if (setRefineCurrent.empty())
			{
				// No more cells to refine in this res. Check next res.

				if (setRefineNext.empty())
				{
					// No more cells to refine at all.
					break;
				}

				//TRACE_INFO("Changing res " << nRes << " to " << nRes + 1);
				//vtreeTrace(vtree);

				// Go to next res.
				if (38 < ++nRes)
				{
					// We can't do this forever.
					TRACE_INFO("FID " << nFid << " still requires " << setRefineNext.size()
						<< " cells to refine but we're not going to continue past res " << (nRes - 1));
					break;
				}

				// Generate set of all cells covered by set of cells to refine next.
				setRefineCurrent.clear();
				std::set<PYXIcosIndex>::iterator itInsert = setRefineCurrent.begin();
				for (std::set<PYXIcosIndex>::iterator it = setRefineNext.begin();
					it != setRefineNext.end(); ++it)
				{
					PYXIcosIndex covered[7];
					int nCovered = PYXIcosMath::getCoveredCells(*it, covered);

					for (int n = 0; n != nCovered; ++n)
					{
						itInsert = setRefineCurrent.insert(itInsert, covered[n]);
					}
				}

				// Cull cells that are fully covered.
				for (std::set<PYXIcosIndex>::iterator it = setRefineCurrent.begin();
					it != setRefineCurrent.end();)
				{
					if (isFeatureNodeFullyCovered(vtree, nFid, *it))
					{
						//TRACE_INFO("culling " << it->toString() << " since it is fully covered");
						it = setRefineCurrent.erase(it);
					}
					else
					{
						++it;
					}
				}

				// Swap in next resolution of cells to refine.
				setRefinePrev.swap(setRefineNext); // next becomes prev
				setRefineNext.clear(); // next is cleared
			}

			if (!setRefineCurrent.empty())
			{

				// Set up new cell to refine by calculating its covered cells and
				// getting its list of fragments.
				PYXIcosIndex index = *setRefineCurrent.begin();
				//TRACE_INFO("Changing to index " << index.toString());
				//vtreeTrace(vtree);
				setRefineCurrent.erase(setRefineCurrent.begin());

				// Test against just this cell now.
				vecCellsToTest.assign(1, index);

				// Collate fragment list.
				int nSlice = getFeatureNodeFinalSlices(vtree, nFid, index);
				PYXIcosIndex covering[3];
				int nCovering = PYXIcosMath::getCoveringCells(index, covering);
				for (int n = 0; n != nCovering; ++n)
				{
					if (setRefinePrev.find(covering[n]) != setRefinePrev.end()) // must happen for at least one
					{
						// Only refine fragments which are not final.
						mergeInFragments(vtree, nFid, covering[n], vecFragment);
					}
				}
			}

			// Simplify fragment list.
			simplifyFragments(vecFragment);
		}
	}
}

//! Output a vtree node to a stream.
inline
std::ostream& pyxtreeNodeToStream(std::ostream& out, const VTreeNG& vtree)
{
	const VTreeNode& vtreeNode = *vtree;
	int fcount = static_cast<int>(vtreeNode.m_vecFeatureNode.size());
	for (int fn = 0; fn != fcount; ++fn)
	{
		const FeatureNode& featureNode = vtreeNode.m_vecFeatureNode[fn];
		out << " #" << featureNode.m_nFid << ' ';
		int scount = static_cast<int>(featureNode.m_vecFragment.size());
		for (int sn = 0; sn != scount; ++sn)
		{
			const Fragment& fragment = featureNode.m_vecFragment[sn];
			out << '[' << fragment.m_nBegin << ',' << fragment.m_nEnd << ')';
		}
		if (!vtree.getFlagCount() && requiresRefinement(featureNode.m_vecFragment))
		{
			out << "<<< ";
		}
	}
	return out;
}

void vtreeTrace(const VTreeNG& vtree)
{
	TRACE_INFO('\n' << pyxtreeToString(vtree));
}

////////////////////////////////////////////////////////////////////////////////

namespace
{

struct VTreeTester
{
	static void test();
};

Tester<VTreeTester> gTester;

void VTreeTester::test()
{
	int nSeed = 52; // a particular seed

	VTreeNG vtree;
	PYXCurve curve;
	srand(nSeed);
	curve.randomize(PYXIcosIndex("F-0"), PYXIcosIndex("3-0"), 38, 3);

	// Create vtree.
	vtreeInsertFeatureCurve(vtree, nSeed, curve);
	//vtreeTrace(vtree);

	// This test can be affected by both the printing of trees and
	// of course any changes that affect the creation of the tree.
	std::string strActual = pyxtreeToString(vtree);
	std::string strExpected =
		"0 \n"
		"+-1 \n"
		"| \\-3 \n"
		"|   +-0  #52 [4,9)\n"
		"|   | +-0  #52 [4,4)[5,6)[6,9)\n"
		"|   | | +-0  #52 [7,9)\n"
		"|   | | | +-0  #52 [7,9)\n"
		"|   | | | | +-0  #52 [7,7)[8,9)\n"
		"|   | | | | | +-0  #52 [8,9)\n"
		"|   | | | | | \\-5  #52 [7,7)[8,8)\n"
		"|   | | | | +-5  #52 [7,7)\n"
		"|   | | | | \\-6  #52 [7,8)\n"
		"|   | | | +-4  #52 [5,6)\n"
		"|   | | | \\-5  #52 [6,7)\n"
		"|   | | +-4  #52 [5,5)[6,6)\n"
		"|   | | \\-5  #52 [4,5)[5,6)[6,7)\n"
		"|   | |   \\-0  #52 [4,5)[6,6)[7,7)\n"
		"|   | |     +-0  #52 \n"
		"|   | |     | \\-3  #52 [4,5)\n"
		"|   | |     +-2  #52 [6,6)[7,7)\n"
		"|   | |     +-3  #52 [4,5)[6,6)\n"
		"|   | |     | \\-0  #52 [5,5)[6,6)\n"
		"|   | |     \\-4  #52 [4,4)[5,5)\n"
		"|   | +-4  #52 [4,5)[6,6)\n"
		"|   | | \\-0  #52 [4,4)\n"
		"|   | |   \\-1  #52 [4,4)[5,5)[6,6)\n"
		"|   | |     \\-0  #52 [4,4)\n"
		"|   | |       \\-6  #52 [4,4)[5,5)\n"
		"|   | \\-5  #52 [4,4)[5,5)[6,6)<<< \n"
		"|   +-4  #52 [3,4)\n"
		"|   \\-5  #52 [3,3)[4,4)\n"
		"\\-4 \n"
		"  \\-1 \n"
		"    \\-0  #52 [0,3)\n"
		"      +-0  #52 [0,2)\n"
		"      | +-0  #52 [0,2)\n"
		"      | | +-0  #52 [0,1)\n"
		"      | | +-4  #52 [1,1)[2,2)\n"
		"      | | \\-5  #52 [1,2)\n"
		"      | \\-5  #52 [2,2)\n"
		"      +-4  #52 [2,3)\n"
		"      \\-5  #52 [3,3)\n";
	TEST_ASSERT(strActual == strExpected);

	// Query vtree.
	// These tests are of course affected by the creation of the tree.
	std::vector<Fragment> vecFragment;
	vecFragment = vtreeQueryFragments(vtree, nSeed + 1, PYXIcosIndex("3-0")); // feature not present
	TEST_ASSERT(vecFragment.size() == 0);
	vecFragment = vtreeQueryFragments(vtree, nSeed, PYXIcosIndex("M-020003")); // cell not present
	TEST_ASSERT(vecFragment.size() == 0);
	vecFragment = vtreeQueryFragments(vtree, nSeed, PYXIcosIndex("3-000000"));
	TEST_ASSERT(vecFragment.size() == 1 && vecFragment[0] == Fragment(8, 9));
	vecFragment = vtreeQueryFragments(vtree, nSeed, PYXIcosIndex("3-000005")); // also includes 3-00005 3-00006
	TEST_ASSERT(vecFragment.size() == 1 && vecFragment[0] == Fragment(7, 8));
	vecFragment = vtreeQueryFragments(vtree, nSeed, PYXIcosIndex("3-004"));
	TEST_ASSERT(vecFragment.size() == 2 && vecFragment[0] == Fragment(5, 5) && vecFragment[1] == Fragment(6, 6));
	vecFragment = vtreeQueryFragments(vtree, nSeed, PYXIcosIndex("F-05")); // also includes 3-4
	TEST_ASSERT(vecFragment.size() == 1 && vecFragment[0] == Fragment(3, 4));
	vecFragment = vtreeQueryFragments(vtree, nSeed, PYXIcosIndex("3-040106")); // also includes 3-00504 3-04010
	TEST_ASSERT(vecFragment.size() == 2 && vecFragment[0] == Fragment(4, 4) && vecFragment[1] == Fragment(5, 5));
	vecFragment = vtreeQueryFragments(vtree, nSeed, PYXIcosIndex("3-005003")); // also includes 3-00504
	TEST_ASSERT(vecFragment.size() == 1 && vecFragment[0] == Fragment(4, 5));
	vecFragment = vtreeQueryFragments(vtree, nSeed, PYXIcosIndex("3-005030")); // also includes 3-0004
	TEST_ASSERT(vecFragment.size() == 1 && vecFragment[0] == Fragment(5, 6));
}

}
