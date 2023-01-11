/******************************************************************************
xy_intersection_test.cpp

begin		: 2005-06-22
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/sampling/xy_intersection_test.h"

// pyxlib includes
#include "pyxis/geometry/tile.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/hexagon.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/vertex_iterator.h"

// standard includes
#include <string>

// Resolution difference to perform complete coverage test.
const int knResDiffCompleteTest = 2;

// The depth of tile that we will use to test the if an index is contained 
// (including all children) to this depth.
const int knTestTileDepth = 10;

// Table of PYXIS indices to walk out to edge of tile's tessellation.
// Used in methods hasHexAllVerticesInArea and hasHexSingleVertexInArea.
// The length of these strings should match the tile factor for performance reasons.
// The length of these string MUST match knTestTileDepth.
static std::string arrayOfStrings[] =
{
	"0101010101",
	"0202020202",
	"0303030303",
	"0404040404",
	"0505050505",
	"0606060606"
};

/*!
Constructor initializes member variables.
*/
PYXXYIntersectionTest::PYXXYIntersectionTest() :
	m_pConverter(0),
	m_bounds()
{
#ifdef _DEBUG
	//! metrics which can be checked in a debug run.
	m_nTestsDone = 0;
	m_nResultsYes = 0;
	m_nResultsNo = 0;
	m_nResultsYesComplete = 0;
	m_nResultsMaybe = 0;
#endif
}

/*!
Set the converter to be used.

\param	pConverter	The converter (ownership retained by caller)
*/
void PYXXYIntersectionTest::setConverter(
										const ICoordConverter* pConverter)
{
	if (0 != pConverter)
	{
		m_pConverter = pConverter;	
	}
	else
	{
		assert(false);
	}
}

/*!
Set the bounds of the area to test against.

\param	bounds	The bounds of the coverage.
*/
void PYXXYIntersectionTest::setBounds(const PYXRect2DDouble& bounds)
{
	m_bounds = bounds;
}

/*!
This test determines if the given index intersects with the area
of a coverage.

There are two tests which need to be done in order to determine if a given 
index falls in a given area.  First check to see if any of the vertices of 
the hexagon fall in the given area, second, convert the area's corners into 
indices and see if any are children of the hexagon.

The response of the test depends on the type of the hexagon.  If it is a 
centroid child, then there are two possibilities.  A) either there was no 
intersection detected and the answer is maybe, or B) intersection was 
detected and the response is yes.  

If the hexagon is a vertex child, then there are also two possibilites.
A) Either a intersection was detected, and the answer is maybe, or B) no
intersection was detected and the answer is no.

If the resolution of the index is at least two less than the target
resolution, then it is worth checking for complete coverage of the hexagon
by the area.  Otherwise the test is pointless.

\param	index			The index to test.
\param	pbAbort			true to stop the test immediately, initialized to
						false on entry (out).

\return	The test result.
*/
PYXTest::eTestResult PYXXYIntersectionTest::doTestIndex(
												const PYXIcosIndex& index,
												bool* pbAbort	)
{
	bool bIntersection = false;
	bool bAllVertices = false;

#ifdef _DEBUG
	++m_nTestsDone;
#endif

	// this test never needs to abort
	*pbAbort = false;

	// get the cell resolution.
	int nResolution = index.getResolution();

	// we should never have to get this high to determine intersection
	assert ((getDataResolution() >= nResolution) && "Invalid resolution.");

	/*
	Are any of the hexagon's vertices in the area?  Determine if it is 
	advantageous to check for all vertices.  If the index's resolution
	is less by two or more then test for complete coverage.  Otherwise test 
	until a single vertex is found in the area.
	*/
	
	if (getTargetResolution() - knResDiffCompleteTest > index.getResolution())
	{
		// check if all vertices are in the area.
		eVertexResult knResult = hasHexAllVerticesInArea(index);
		switch(knResult)
		{
			case knNoVertices:
			{
				// no vertices in the area - no intersection
				bIntersection = false;
				break;
			}

			case knSomeVertices:
			{
				// at least one vertex in the area - intersection
				bIntersection = true;
				break;
			}

			case knAllVertices:
			{
				// all vertices in the area - intersection
				bIntersection = true;

				// set flag for to perform the complete coverage test
				bAllVertices = true;
				break;	
			}
		}
	}
	else
	{
		// just see if it has at least one vertex in the area
		bIntersection = hasHexSingleVertexInArea(index);
	}


	/* 
	If the hexagon has no vertices in the area then check to see if the
	area has a corner in the hexagon.  Another way to say this is see if the 
	cell is in fact one of the corners of the coverage.
	*/
	if (!bIntersection)
	{
		if (isCellAreaCorner(index))
		{
			bIntersection = true;
		}
	}

	PYXTest::eTestResult nRetVal = knNo;

	/*
	If the test is at the data resolution, then the test is absolute. If one
	of the tests passed, then  intersection is certain. If neither of the two
	tests passed, then no intersection exists.
	*/
	if (getDataResolution() == nResolution)
	{
		if (bIntersection)
		{
			nRetVal = knYes;
		}
		else
		{
			nRetVal = knNo;	
		}
	}
	// otherwise further testing is required.
	else
	{
	
		// Check to see if the index is a centroid child or vertex child. 
		if (index.hasVertexChildren())
		{
			/*
			The index is a centroid child.
			If one of the tests passed then the index definitely has
			intersection.
			*/
			if (bIntersection)
			{	
				nRetVal = PYXTest::knYes;

				if (bAllVertices)
				{
					/*
					The centroid child is definitely covered, but the vertex
					children will require further testing.
					*/
					nRetVal = PYXTest::knYesComplete;
				}
			}
			/*
			If both tests failed, then there may still be intersection. The
			children of the centroid must be examined.
			*/
			else
			{
				nRetVal = PYXTest::knMaybe;
			}
		}
		else
		{
			/*
			The index is a vertex and one of the tests passes, All that can
			be said is maybe there is intersection.  The child of the vertex
			must first be checked to see if we actually have intersection.
			*/
			if (bIntersection)
			{	
				/*
				If all of the vertices of the child are in the area,
				then it can be said that there is complete coverage.
				*/
				if (bAllVertices)
				{
					nRetVal = PYXTest::knYesComplete;
				}
				else
				{
					nRetVal = PYXTest::knMaybe;
				}
			}

			/*
			If neither of the two tests passed then there is definitely no
			intersection.
			*/
			else
			{
				nRetVal = PYXTest::knNo;
			}
		}
	}

#ifdef _DEBUG
	if (nRetVal == PYXTest::knYes)
	{
		++m_nResultsYes;
	}
	if (nRetVal == PYXTest::knNo)
	{
		++m_nResultsNo;
	}
	if (nRetVal == PYXTest::knYesComplete)
	{
		++m_nResultsYesComplete;
	}
	if (nRetVal == PYXTest::knMaybe)
	{
		++m_nResultsMaybe;
	}
#endif

	return nRetVal;
}

/*!
Method converts the vertices of the hexagon into native coordinates, 
and checks to see if they are all contained by the coverage area.
It also checks the hexagon's edges, to account for localized distortion.

\param	index The index to test.

\return	true if all vertices reside inside the area, false otherwise.
*/
PYXXYIntersectionTest::eVertexResult 
PYXXYIntersectionTest::hasHexAllVerticesInArea(
										const PYXIcosIndex& index	) const
{
	bool bHasAllVertexInArea = true;
	bool bHasVertexInArea = false;
		
	// walk around the hexagon's vertices
	PYXVertexIterator itVert(index);
	for (; !itVert.end(); itVert.next())
	{
		const PYXIcosIndex& childIndex = itVert.getIndex();

		// get the PYXIS index in native coordinates
		PYXCoord2DDouble native;
		m_pConverter->pyxisToNative(childIndex, &native);	
	
		if (m_bounds.inside(native))
		{
			bHasVertexInArea = true;
		}
		else
		{
			bHasAllVertexInArea = false;
		}
	}
	
	if (!bHasVertexInArea && !index.hasVertexChildren())
	{
		// walk around the hexagon's edges
		// (to account for localized distortion)
		int nDepth = knTestTileDepth;
		if (getDataResolution() < index.getResolution() + nDepth)
		{
			// limit depth to data resolution
			nDepth = getDataResolution() - index.getResolution();
		}

		if (0 < nDepth)
		{
			if (static_cast<int>(arrayOfStrings[0].size()) <
				knTestTileDepth)
			{
				// sanity check
				assert(false && "Intersection test logic error.");
			}

			for (int n = 0; n != Hexagon::knNumSides; ++n)
			{
				// walk out to edge of tile's tessellation
				std::string strIndex = index.toString();
				strIndex.append(arrayOfStrings[n], 0, nDepth);
				PYXIcosIndex childIndex = strIndex;

				if (!PYXIcosMath::isValidIndex(childIndex))
				{
					continue;
				}

				// get the PYXIS index in native coordinates
				PYXCoord2DDouble native;
				m_pConverter->pyxisToNative(childIndex, &native);	

				if (m_bounds.inside(native))
				{
					bHasVertexInArea = true;
				}
				else
				{
					bHasAllVertexInArea = false;
				}
			}
		}
	}

	if (!bHasVertexInArea)
	{
		return knNoVertices;
	}
	else
	{
		if (bHasAllVertexInArea)
		{
			return knAllVertices;
		}
		else
		{
			return knSomeVertices;
		}
	}

}

/*!
Checks to see if the PYXIS cell has at least one vertex in the coverage area.
It also checks the hexagon's edges, to account for localized distortion.

\param	index	The index to test.

\return	true if at least one vertex resides inside the area, false otherwise.
*/
bool PYXXYIntersectionTest::hasHexSingleVertexInArea(
										const PYXIcosIndex& index	) const
{
	bool bHasVertexInArea = false;

	// walk around the hexagon's vertices
	PYXVertexIterator itVert(index);
	for (; !itVert.end(); itVert.next())
	{
		const PYXIcosIndex& childIndex = itVert.getIndex();

		// get the PYXIS index in native coordinates
		PYXCoord2DDouble native;
		m_pConverter->pyxisToNative(childIndex, &native);	
		if (m_bounds.inside(native))
		{
			// one was found - the check may finish
			bHasVertexInArea = true;
			break;
		}
	}

	if (!bHasVertexInArea && !index.hasVertexChildren())
	{
		// walk around the hexagon's edges
		// (to account for localized distortion)
		int nDepth = knTestTileDepth;
		if (getDataResolution() < index.getResolution() + nDepth)
		{
			// limit depth to data resolution
			nDepth = getDataResolution() - index.getResolution();
		}

		if (0 < nDepth)
		{
			if (	static_cast<int>(arrayOfStrings[0].size()) <
					knTestTileDepth)
			{
				// sanity check
				assert(false && "Intersection test logic error.");
			}

			int n = 0;
			for (; n != Hexagon::knNumSides; ++n)
			{
				// walk out to edge of tile's tessellation
				std::string strIndex = index.toString();
				strIndex.append(arrayOfStrings[n], 0, nDepth);
				PYXIcosIndex childIndex = strIndex;

				if (!PYXIcosMath::isValidIndex(childIndex))
				{
					continue;
				}

				// get the PYXIS index in native coordinates
				PYXCoord2DDouble native;
				m_pConverter->pyxisToNative(childIndex, &native);	

				if (m_bounds.inside(native))
				{
					bHasVertexInArea = true;
					break;
				}
			}
		}
	}

	return bHasVertexInArea;
}



/*!
Check if the cell is one of the areas corners.  Each corner of the area is
converted to a PYXIS Cell and its index is compared to the index passed in.

\param	index	The index to test.

\return	true if the cell is one of the areas corners.
*/
bool PYXXYIntersectionTest::isCellAreaCorner(
										const PYXIcosIndex& index) const
{
	PYXCoord2DDouble pt;
	PYXIcosIndex corner;

	// get the resolution of the cell passed in.
	int nResolution = index.getResolution();

	// ensure resolution is not less than minumum resolution
	if (PYXIcosIndex::knMinSubRes > nResolution)
	{
		nResolution = PYXIcosIndex::knMinSubRes;
	}

	pt.setX(m_bounds.xMin());
	pt.setY(m_bounds.yMin());
	m_pConverter->nativeToPYXIS(pt, &corner, nResolution);
	if (index == corner)
	{
		return true;
	}

	pt.setX(m_bounds.xMin());
	pt.setY(m_bounds.yMax());
	m_pConverter->nativeToPYXIS(pt, &corner, nResolution);
	if (index == corner)
	{
		return true;
	}

	pt.setX(m_bounds.xMax());
	pt.setY(m_bounds.yMin());
	m_pConverter->nativeToPYXIS(pt, &corner, nResolution);
	if (index == corner)
	{
		return true;
	}

	pt.setX(m_bounds.xMax());
	pt.setY(m_bounds.yMax());
	m_pConverter->nativeToPYXIS(pt, &corner, nResolution);
	if (index == corner)
	{
		return true;
	}

	return false;
}

/*!
Walk the vertexes of the hexagon and check to see if all of the children are
covered by the coverage area.

\param	index The index to test.

\return	true if all the children are covered by the area of interest.
*/
bool PYXXYIntersectionTest::allChildrenCovered(
											const PYXIcosIndex& index) const
{
	bool bAllChildrenCovered = true;

	// TODO: Optimize this method.  Needs an edge iterator.

	// use brute fore method, check every cell
	eVertexResult nResult = knNoVertices;
	PYXVertexIterator itVert(index);
	for (; !itVert.end(); itVert.next())
	{
		const PYXIcosIndex& childIndex = itVert.getIndex();
		nResult = hasHexAllVerticesInArea(childIndex);
		if (knAllVertices != nResult)
		{
			bAllChildrenCovered = false;
			break;
		}
	}

	return bAllChildrenCovered;
}
