/******************************************************************************
index_math.cpp

begin		: 2004-01-29
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/index_math.h"

// pyxlib includes
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/edge_iterator.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/hexagon.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/icosahedron.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/pentagon.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/utility/coord_polar.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cctype>
#include <cmath>

//! PYXIcosIndex for "01"
static const PYXIcosIndex kIndex01("01");

//! PYXIcosIndex for "A"
static const PYXIcosIndex kIndexA("A");

// The gap direction for each vertex
static const PYXMath::eHexDirection knGapDirection[13] =
{
	// first value is a place holder with the 1 based offset
	PYXMath::knDirectionZero,
	PYXMath::knDirectionOne,
	PYXMath::knDirectionOne,
	PYXMath::knDirectionOne,
	PYXMath::knDirectionOne,
	PYXMath::knDirectionOne,
	PYXMath::knDirectionOne,
	PYXMath::knDirectionFour,
	PYXMath::knDirectionFour,
	PYXMath::knDirectionFour,
	PYXMath::knDirectionFour,
	PYXMath::knDirectionFour,
	PYXMath::knDirectionFour
};

// Connectivity of the resolution 0 pentagons.
/*
This connectivity chart describes the resolution 0 pentagon that is the result
of moving from another resolution 0 pentagon in a given direction. The first
index into the array is the vertex number, the second index is the direction
number. The third index specifies the type of information being requested:

	0	The 1 based destination of the resolution 0 pentagon.
	1	The counter-clockwise rotation required to correct the movement.

A value of -1 indicates the direction is invalid.
*/
static const int kpnRes0Connect[12][6][2] =
{
	// Vertex 1
	{{-1,-1},{2,3},{3,2},{4,1},{5,0},{6,5}},
	// Vertex 2
	{{-1,-1},{1,3},{6,0},{11,0},{7,0},{3,0}},
	// Vertex 3
	{{-1,-1},{1,4},{2,0},{7,0},{8,0},{4,0}},
	// Vertex 4
	{{-1,-1},{1,5},{3,0},{8,0},{9,0},{5,0}},
	// Vertex 5
	{{-1,-1},{1,0},{4,0},{9,0},{10,0},{6,0}},
	// Vertex 6
	{{-1,-1},{1,1},{5,0},{10,0},{11,0},{2,0}},
	// Vertex 7
	{{3,0},{2,0},{11,0},{-1,-1},{12,1},{8,0}},
	// Vertex 8
	{{4,0},{3,0},{7,0},{-1,-1},{12,0},{9,0}},
	// Vertex 9
	{{5,0},{4,0},{8,0},{-1,-1},{12,5},{10,0}},
	// Vertex 10
	{{6,0},{5,0},{9,0},{-1,-1},{12,4},{11,0}},
	// Vertex 11
	{{2,0},{6,0},{10,0},{-1,-1},{12,3},{7,0}},
	// Vertex 12
	{{9,1},{8,0},{7,5},{-1,-1},{11,3},{10,2}}
};

// Connectivity of resolution 1 hexagons on vertices.
/*
This connectivity chart describes the resolution 1 hexagon that is the result
of moving from a resolution 1 vertex pentagon in a given direction. The first
index into the array is the vertex number, the second index is the direction
number. The third index determines the type of information being requested:

	0	The destination resolution 1 hexagon face. 
	1	The counter-clockwise rotation required to correct the movement.
	 
A value of -1 indicates the direction is not valid.
*/
static const int kpnRes1VertConnect[12][6][2] =
{
	// Vertex 1
	{{-1,-1},{'A',2},{'B',1},{'C',0},{'D',5},{'E',4}},
	// Vertex 2
	{{-1,-1},{'E',0},{'J',0},{'O',0},{'F',0},{'A',0}},
	// Vertex 3
	{{-1,-1},{'A',0},{'F',0},{'K',0},{'G',0},{'B',0}},
	// Vertex 4
	{{-1,-1},{'B',0},{'G',0},{'L',0},{'H',0},{'C',0}},
	// Vertex 5
	{{-1,-1},{'C',0},{'H',0},{'M',0},{'I',0},{'D',0}},
	// Vertex 6
	{{-1,-1},{'D',0},{'I',0},{'N',0},{'J',0},{'E',0}},
	// Vertex 7
	{{'F',0},{'O',0},{'T',0},{-1,-1},{'P',0},{'K',0}},
	// Vertex 8
	{{'G',0},{'K',0},{'P',0},{-1,-1},{'Q',0},{'L',0}},
	// Vertex 9
	{{'H',0},{'L',0},{'Q',0},{-1,-1},{'R',0},{'M',0}},
	// Vertex 10
	{{'I',0},{'M',0},{'R',0},{-1,-1},{'S',0},{'N',0}},
	// Vertex 11
	{{'J',0},{'N',0},{'S',0},{-1,-1},{'T',0},{'O',0}},
	// Vertex 12
	{{'Q',0},{'P',5},{'T',4},{-1,-1},{'S',2},{'R',1}}
};

// Connectivity of resolution 1 hexagons on faces
/*
This connectivity chart provides the resolution 1 hexagon that is the result of
moving from a resolution 1 face hexagon in a given direction. The first index
into the array is the face value (A=0, B=1...), the second index is the
direction number. The third index determines the type of information being
requested:

	0	The destination resolution 1 hexagon face or pentagon vertex
	1	The counter-clockwise rotation required to correct the movement. 
*/
static const int kpnRes1FaceConnect[20][6][2] =
{
	// Face A
	{{1,4},{'E',1},{2,0},{'F',0},{3,0},{'B',5}},
	// Face B
	{{1,5},{'A',1},{3,0},{'G',0},{4,0},{'C',5}},
	// Face C
	{{1,0},{'B',1},{4,0},{'H',0},{5,0},{'D',5}},
	// Face D
	{{1,1},{'C',1},{5,0},{'I',0},{6,0},{'E',5}},
	// Face E
	{{1,2},{'D',1},{6,0},{'J',0},{2,0},{'A',5}},
	// Face F
	{{'A',0},{2,0},{'O',0},{7,0},{'K',0},{3,0}},
	// Face G
	{{'B',0},{3,0},{'K',0},{8,0},{'L',0},{4,0}},
	// Face H
	{{'C',0},{4,0},{'L',0},{9,0},{'M',0},{5,0}},
	// Face I
	{{'D',0},{5,0},{'M',0},{10,0},{'N',0},{6,0}},
	// Face J
	{{'E',0},{6,0},{'N',0},{11,0},{'O',0},{2,0}},
	// Face K
	{{3,0},{'F',0},{7,0},{'P',0},{8,0},{'G',0}},
	// Face L
	{{4,0},{'G',0},{8,0},{'Q',0},{9,0},{'H',0}},
	// Face M
	{{5,0},{'H',0},{9,0},{'R',0},{10,0},{'I',0}},
	// Face N
	{{6,0},{'I',0},{10,0},{'S',0},{11,0},{'J',0}},
	// Face O
	{{2,0},{'J',0},{11,0},{'T',0},{7,0},{'F',0}},
	// Face P
	{{'K',0},{7,0},{'T',5},{12,1},{'Q',1},{8,0}},
	// Face Q
	{{'L',0},{8,0},{'P',5},{12,0},{'R',1},{9,0}},
	// Face R
	{{'M',0},{9,0},{'Q',5},{12,5},{'S',1},{10,0}},
	// Face S
	{{'N',0},{10,0},{'R',5},{12,4},{'T',1},{11,0}},
	// Face T
	{{'O',0},{11,0},{'S',5},{12,2},{'P',1},{7,0}}
};

// Connectivity of faces beyond resolution 1.
/*
This table provides the vertex and direction that results when stepping from a
face onto a vertex. The first index into the array is the face being
referenced. The second index is the direction of movement from that face. The
third index determines the type of information being requested:

	0	The vertex resulting from the the move.
	1	The first sub index direction.
	2	The counter-clockwise rotation required to correct the movement.
*/
static const int kpnRes2FaceConnect[20][6][3] =
{
	// Face A
	{{1,5,4},{1,4,4},{2,1,1},{2,6,0},{3,3,0},{3,2,0}},
	// Face B
	{{1,5,5},{1,4,5},{3,1,1},{3,6,0},{4,3,0},{4,2,0}},
	// Face C
	{{1,5,0},{1,4,0},{4,1,1},{4,6,0},{5,3,0},{5,2,0}},
	// Face D
	{{1,5,1},{1,4,1},{5,1,1},{5,6,0},{6,3,0},{6,2,0}},
	// Face E
	{{1,5,3},{1,4,2},{6,1,1},{6,6,0},{2,3,0},{2,2,0}},
	// Face F
	{{3,3,0},{2,6,0},{2,5,0},{7,2,0},{7,1,0},{3,4,0}},
	// Face G
	{{4,3,0},{3,6,0},{3,5,0},{8,2,0},{8,1,0},{4,4,0}},
	// Face H
	{{5,3,0},{4,6,0},{4,5,0},{9,2,0},{9,1,0},{5,4,0}},
	// Face I
	{{6,3,0},{5,6,0},{5,5,0},{10,2,0},{10,1,0},{6,4,0}},
	// Face J
	{{2,3,0},{6,6,0},{6,5,0},{11,2,0},{11,1,0},{2,4,0}},
	// Face K
	{{3,5,0},{3,4,0},{7,1,0},{7,6,0},{8,3,0},{8,2,0}},
	// Face L
	{{4,5,0},{4,4,0},{8,1,0},{8,6,0},{9,3,0},{9,2,0}},
	// Face M
	{{5,5,0},{5,4,0},{9,1,0},{9,6,0},{10,3,0},{10,2,0}},
	// Face N
	{{6,5,0},{6,4,0},{10,1,0},{10,6,0},{11,3,0},{11,2,0}},
	// Face O
	{{2,5,0},{2,4,0},{11,1,0},{11,6,0},{7,3,0},{7,2,0}},
	// Face P
	{{8,3,0},{7,6,0},{7,5,0},{12,2,1},{12,1,1},{8,4,1}},
	// Face Q
	{{9,3,0},{8,6,0},{8,5,0},{12,2,0},{12,1,0},{9,4,1}},
	// Face R
	{{10,3,0},{9,6,0},{9,5,0},{12,2,5},{12,1,5},{10,4,1}},
	// Face S
	{{11,3,0},{10,6,0},{10,5,0},{12,2,4},{12,1,4},{11,4,1}},
	// Face T
	{{7,3,0},{11,6,0},{11,5,0},{12,2,3},{12,1,2},{7,4,1}}
};

/*
This array indicates which vertex owns each of the faces.  The index into the
array is the Face indicator - 'A' and the value held at that position in the 
array is the vertex that owns it.  This array can be used as an iterator to 
avoid repetition while moving through vertices or used during drawing 
algorithms for the same reason.
*/
static const int kpnFaceOwningVertex[20] = 
{
	3,4,5,6,2,3,3,4,5,6,7,8,9,10,11,7,8,9,10,11
};

//! Number of vertex cells in each resolution
PYXIcosMath::ResolutionCellsVector PYXIcosMath::m_vecVertexResolutionCells;

//! Number of face cells in each resolution
PYXIcosMath::ResolutionCellsVector PYXIcosMath::m_vecFaceResolutionCells;

//! Offset for each resolution in a vertex
PYXIcosMath::ResolutionOffsetsVector PYXIcosMath::m_vecVertexResolutionOffsets;

//! Offset for each resolution in a face
PYXIcosMath::ResolutionOffsetsVector PYXIcosMath::m_vecFaceResolutionOffsets;

//! Tester class
Tester<PYXIcosMath> gTester;

//! Test method
void PYXIcosMath::test()
{
	// test the zoomIntoNeighbourhood and zoomOut methods on conventional indices
	PYXIcosIndex index1;
	PYXIcosIndex index2;
	PYXIcosIndex index3;
	PYXIcosIndex index4;
	PYXIcosIndex index5;

	try
	{
		index1 = "1-403";
		zoomIntoNeighbourhood(&index1);

		index2 = "1-4030";
		TEST_ASSERT(index1 == index2);
	}
	catch (PYXException&)
	{
		TEST_ASSERT(false);
	}

	try
	{
		index1 = "1-4030";
		zoomOut(&index1);

		index2 = "1-403";
		TEST_ASSERT(index1 == index2);
	}
	catch (PYXException&)
	{
		TEST_ASSERT(false);
	}

	// test the zoomIntoNeighbourhood and zoomOut methods on 1st and 2nd resolution
	try
	{
		index1 = "1";
		zoomIntoNeighbourhood(&index1);

		index2 = "1-0";
		TEST_ASSERT(index1 == index2);
	}
	catch (PYXException&)
	{
		TEST_ASSERT(false);
	}

	try
	{
		index1 = "01-0";
		zoomOut(&index1);

		index2 = "1";
		TEST_ASSERT(index1 == index2);
	}
	catch (PYXException&)
	{
		TEST_ASSERT(false);
	}

	try
	{
		index1 = "A";
		zoomIntoNeighbourhood(&index1);

		index2 = "A-0";
		TEST_ASSERT(index1 == index2);
	}
	catch (PYXException&)
	{
		TEST_ASSERT(false);
	}

	try
	{
		index1 = "A-0";
		zoomOut(&index1);

		index2 = "A";
		TEST_ASSERT(index1 == index2);
	}
	catch (PYXException&)
	{
		TEST_ASSERT(false);
	}

	// test calc descendant index
	PYXIcosIndex parent = "1-2010304";
	PYXIcosIndex child = "2-201030405";

	try
	{
		calcDescendantIndex(parent, child);

		// expecting exception since not a descendant
		TEST_ASSERT(false);
	}
	catch (PYXException&)
	{
		// ignore
	}

	child = "1-20103040506";
	PYXIndex relativeIndex = calcDescendantIndex(parent, child);
	TEST_ASSERT(PYXIndex("0506") == relativeIndex);

	// test getCoveringCells
	{
		PYXIcosIndex indices[3];
		int nCount;
		nCount = getCoveringCells(PYXIcosIndex("A-0"), indices);
		TEST_ASSERT(nCount == 1 && indices[0] == PYXIcosIndex("A"));
		nCount = getCoveringCells(PYXIcosIndex("1-0"), indices);
		TEST_ASSERT(nCount == 1 && indices[0] == PYXIcosIndex("1"));
		nCount = getCoveringCells(PYXIcosIndex("1-2"), indices);
		TEST_ASSERT(nCount == 3 && indices[0] == PYXIcosIndex("1") && indices[1] == PYXIcosIndex("A") && indices[2] == PYXIcosIndex("E"));
		nCount = getCoveringCells(PYXIcosIndex("A-00"), indices);
		TEST_ASSERT(nCount == 1 && indices[0] == PYXIcosIndex("A-0"));
		nCount = getCoveringCells(PYXIcosIndex("1-60"), indices);
		TEST_ASSERT(nCount == 1 && indices[0] == PYXIcosIndex("1-6"));
		nCount = getCoveringCells(PYXIcosIndex("A-01"), indices);
		TEST_ASSERT(nCount == 3 && indices[0] == PYXIcosIndex("A-0") && indices[1] == PYXIcosIndex("1-3") && indices[2] == PYXIcosIndex("1-2"));
		nCount = getCoveringCells(PYXIcosIndex("1-06"), indices);
		TEST_ASSERT(nCount == 3 && indices[0] == PYXIcosIndex("1-0") && indices[1] == PYXIcosIndex("1-6") && indices[2] == PYXIcosIndex("1-2"));
	}

	// test getCoveredCells
	{
		PYXIcosIndex indices[7];
		int nCount;
		nCount = getCoveredCells(PYXIcosIndex("1"), indices);
		TEST_ASSERT(nCount == 6 && indices[0] == PYXIcosIndex("1-0") && indices[1] == PYXIcosIndex("1-2") && indices[2] == PYXIcosIndex("1-3")
			 && indices[3] == PYXIcosIndex("1-4") && indices[4] == PYXIcosIndex("1-5") && indices[5] == PYXIcosIndex("1-6"));
		nCount = getCoveredCells(PYXIcosIndex("A"), indices);
		TEST_ASSERT(nCount == 7 && indices[0] == PYXIcosIndex("A-0") && indices[1] == PYXIcosIndex("1-3") && indices[2] == PYXIcosIndex("1-2")
			 && indices[3] == PYXIcosIndex("2-2") && indices[4] == PYXIcosIndex("2-6") && indices[5] == PYXIcosIndex("3-3") && indices[6] == PYXIcosIndex("3-2"));
		nCount = getCoveredCells(PYXIcosIndex("1-0"), indices);
		TEST_ASSERT(nCount == 6 && indices[0] == PYXIcosIndex("1-00") && indices[1] == PYXIcosIndex("1-02") && indices[2] == PYXIcosIndex("1-03")
			 && indices[3] == PYXIcosIndex("1-04") && indices[4] == PYXIcosIndex("1-05") && indices[5] == PYXIcosIndex("1-06"));
		nCount = getCoveredCells(PYXIcosIndex("1-2"), indices);
		TEST_ASSERT(nCount == 7 && indices[0] == PYXIcosIndex("1-20") && indices[1] == PYXIcosIndex("E-06") && indices[2] == PYXIcosIndex("A-02")
			 && indices[3] == PYXIcosIndex("A-01") && indices[4] == PYXIcosIndex("1-02") && indices[5] == PYXIcosIndex("1-06") && indices[6] == PYXIcosIndex("E-01"));
		nCount = getCoveredCells(PYXIcosIndex("A-0"), indices);
		TEST_ASSERT(nCount == 7 && indices[0] == PYXIcosIndex("A-00") && indices[1] == PYXIcosIndex("A-01") && indices[2] == PYXIcosIndex("A-02")
			 && indices[3] == PYXIcosIndex("A-03") && indices[4] == PYXIcosIndex("A-04") && indices[5] == PYXIcosIndex("A-05") && indices[6] == PYXIcosIndex("A-06"));
	}

	// test getAllCoveringCells
	{
		std::vector<PYXIcosIndex> vecCovering;
		std::vector<short> vecSlice;

		getAllCoveringCells(PYXIcosIndex("A-05"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 3 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x001e
			&& vecCovering[1] == PYXIcosIndex("3-3") && vecSlice[1] == 0x01e0
			&& vecCovering[2] == PYXIcosIndex("3-2") && vecSlice[2] == 0x0e01);
		getAllCoveringCells(PYXIcosIndex("A-005"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 4 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("A-00") && vecSlice[1] == 0x001e
			&& vecCovering[2] == PYXIcosIndex("A-04") && vecSlice[2] == 0x01e0
			&& vecCovering[3] == PYXIcosIndex("A-05") && vecSlice[3] == 0x0e01);
		getAllCoveringCells(PYXIcosIndex("A-050"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 4 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x003c
			&& vecCovering[1] == PYXIcosIndex("3-3") && vecSlice[1] == 0x03c0
			&& vecCovering[2] == PYXIcosIndex("3-2") && vecSlice[2] == 0x0c03
			&& vecCovering[3] == PYXIcosIndex("A-05") && vecSlice[3] == 0x0fff);
		getAllCoveringCells(PYXIcosIndex("A-0050"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 5 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("A-00") && vecSlice[1] == 0x000f
			&& vecCovering[2] == PYXIcosIndex("A-04") && vecSlice[2] == 0x00f0
			&& vecCovering[3] == PYXIcosIndex("A-05") && vecSlice[3] == 0x0f00
			&& vecCovering[4] == PYXIcosIndex("A-005") && vecSlice[4] == 0x0fff);

		getAllCoveringCells(PYXIcosIndex("2-02"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 3 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("2-0") && vecSlice[0] == 0x0780
			&& vecCovering[1] == PYXIcosIndex("2-2") && vecSlice[1] == 0x0807
			&& vecCovering[2] == PYXIcosIndex("2-3") && vecSlice[2] == 0x0078);
		getAllCoveringCells(PYXIcosIndex("2-002"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 4 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("2-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("2-00") && vecSlice[1] == 0x0780
			&& vecCovering[2] == PYXIcosIndex("2-06") && vecSlice[2] == 0x0807
			&& vecCovering[3] == PYXIcosIndex("2-02") && vecSlice[3] == 0x0078);
		getAllCoveringCells(PYXIcosIndex("2-020"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 4 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("2-0") && vecSlice[0] == 0x0f00
			&& vecCovering[1] == PYXIcosIndex("2-2") && vecSlice[1] == 0x000f
			&& vecCovering[2] == PYXIcosIndex("2-3") && vecSlice[2] == 0x00f0
			&& vecCovering[3] == PYXIcosIndex("2-02") && vecSlice[3] == 0x0fff);
		getAllCoveringCells(PYXIcosIndex("2-0020"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 5 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("2-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("2-00") && vecSlice[1] == 0x03c0
			&& vecCovering[2] == PYXIcosIndex("2-06") && vecSlice[2] == 0x0c03
			&& vecCovering[3] == PYXIcosIndex("2-02") && vecSlice[3] == 0x003c
			&& vecCovering[4] == PYXIcosIndex("2-002") && vecSlice[4] == 0x0fff);

		getAllCoveringCells(PYXIcosIndex("7-05"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 3 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("7-0") && vecSlice[0] == 0x001e
			&& vecCovering[1] == PYXIcosIndex("7-5") && vecSlice[1] == 0x01e0
			&& vecCovering[2] == PYXIcosIndex("7-6") && vecSlice[2] == 0x0e01);
		getAllCoveringCells(PYXIcosIndex("7-005"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 4 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("7-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("7-00") && vecSlice[1] == 0x001e
			&& vecCovering[2] == PYXIcosIndex("7-03") && vecSlice[2] == 0x01e0
			&& vecCovering[3] == PYXIcosIndex("7-05") && vecSlice[3] == 0x0e01);
		getAllCoveringCells(PYXIcosIndex("7-050"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 4 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("7-0") && vecSlice[0] == 0x003c
			&& vecCovering[1] == PYXIcosIndex("7-5") && vecSlice[1] == 0x03c0
			&& vecCovering[2] == PYXIcosIndex("7-6") && vecSlice[2] == 0x0c03
			&& vecCovering[3] == PYXIcosIndex("7-05") && vecSlice[3] == 0x0fff);
		getAllCoveringCells(PYXIcosIndex("7-0050"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 5 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("7-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("7-00") && vecSlice[1] == 0x000f
			&& vecCovering[2] == PYXIcosIndex("7-03") && vecSlice[2] == 0x00f0
			&& vecCovering[3] == PYXIcosIndex("7-05") && vecSlice[3] == 0x0f00
			&& vecCovering[4] == PYXIcosIndex("7-005") && vecSlice[4] == 0x0fff);

		getAllCoveringCells(PYXIcosIndex("A-0505"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 6 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("3-3") && vecSlice[0] == 0x01f8
			&& vecCovering[1] == PYXIcosIndex("3-2") && vecSlice[1] == 0x0e07
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0fff
			&& vecCovering[3] == PYXIcosIndex("A-050") && vecSlice[3] == 0x001e
			&& vecCovering[4] == PYXIcosIndex("3-301") && vecSlice[4] == 0x01e0
			&& vecCovering[5] == PYXIcosIndex("3-204") && vecSlice[5] == 0x0e01);
		getAllCoveringCells(PYXIcosIndex("A-0501"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 6 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x007e
			&& vecCovering[1] == PYXIcosIndex("3-2") && vecSlice[1] == 0x0f81
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0fff
			&& vecCovering[3] == PYXIcosIndex("A-050") && vecSlice[3] == 0x01e0
			&& vecCovering[4] == PYXIcosIndex("3-203") && vecSlice[4] == 0x0e01
			&& vecCovering[5] == PYXIcosIndex("A-006") && vecSlice[5] == 0x001e);
		getAllCoveringCells(PYXIcosIndex("A-0503"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 6 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x081f
			&& vecCovering[1] == PYXIcosIndex("3-3") && vecSlice[1] == 0x07e0
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0fff
			&& vecCovering[3] == PYXIcosIndex("A-050") && vecSlice[3] == 0x0e01
			&& vecCovering[4] == PYXIcosIndex("A-005") && vecSlice[4] == 0x001e
			&& vecCovering[5] == PYXIcosIndex("3-302") && vecSlice[5] == 0x01e0);

		getAllCoveringCells(PYXIcosIndex("A-00505"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 7 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("A-04") && vecSlice[1] == 0x01f8
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0e07
			&& vecCovering[3] == PYXIcosIndex("A-005") && vecSlice[3] == 0x0fff
			&& vecCovering[4] == PYXIcosIndex("A-0050") && vecSlice[4] == 0x001e
			&& vecCovering[5] == PYXIcosIndex("A-0406") && vecSlice[5] == 0x01e0
			&& vecCovering[6] == PYXIcosIndex("A-0503") && vecSlice[6] == 0x0e01);
		getAllCoveringCells(PYXIcosIndex("A-00501"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 7 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("A-00") && vecSlice[1] == 0x007e
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0f81
			&& vecCovering[3] == PYXIcosIndex("A-005") && vecSlice[3] == 0x0fff
			&& vecCovering[4] == PYXIcosIndex("A-0050") && vecSlice[4] == 0x01e0
			&& vecCovering[5] == PYXIcosIndex("A-0502") && vecSlice[5] == 0x0e01
			&& vecCovering[6] == PYXIcosIndex("A-0005") && vecSlice[6] == 0x001e);
		getAllCoveringCells(PYXIcosIndex("A-00503"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 7 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("A-00") && vecSlice[1] == 0x081f
			&& vecCovering[2] == PYXIcosIndex("A-04") && vecSlice[2] == 0x07e0
			&& vecCovering[3] == PYXIcosIndex("A-005") && vecSlice[3] == 0x0fff
			&& vecCovering[4] == PYXIcosIndex("A-0050") && vecSlice[4] == 0x0e01
			&& vecCovering[5] == PYXIcosIndex("A-0004") && vecSlice[5] == 0x001e
			&& vecCovering[6] == PYXIcosIndex("A-0401") && vecSlice[6] == 0x01e0);

		getAllCoveringCells(PYXIcosIndex("A-05050"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 7 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("3-3") && vecSlice[0] == 0x03f0
			&& vecCovering[1] == PYXIcosIndex("3-2") && vecSlice[1] == 0x0c0f
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0fff
			&& vecCovering[3] == PYXIcosIndex("A-050") && vecSlice[3] == 0x003c
			&& vecCovering[4] == PYXIcosIndex("3-301") && vecSlice[4] == 0x03c0
			&& vecCovering[5] == PYXIcosIndex("3-204") && vecSlice[5] == 0x0c03
			&& vecCovering[6] == PYXIcosIndex("A-0505") && vecSlice[6] == 0x0fff);
		getAllCoveringCells(PYXIcosIndex("A-05010"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 7 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x00fc
			&& vecCovering[1] == PYXIcosIndex("3-2") && vecSlice[1] == 0x0f03
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0fff
			&& vecCovering[3] == PYXIcosIndex("A-050") && vecSlice[3] == 0x03c0
			&& vecCovering[4] == PYXIcosIndex("3-203") && vecSlice[4] == 0x0c03
			&& vecCovering[5] == PYXIcosIndex("A-006") && vecSlice[5] == 0x003c
			&& vecCovering[6] == PYXIcosIndex("A-0501") && vecSlice[6] == 0x0fff);
		getAllCoveringCells(PYXIcosIndex("A-05030"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 7 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x003f
			&& vecCovering[1] == PYXIcosIndex("3-3") && vecSlice[1] == 0x0fc0
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0fff
			&& vecCovering[3] == PYXIcosIndex("A-050") && vecSlice[3] == 0x0c03
			&& vecCovering[4] == PYXIcosIndex("A-005") && vecSlice[4] == 0x003c
			&& vecCovering[5] == PYXIcosIndex("3-302") && vecSlice[5] == 0x03c0
			&& vecCovering[6] == PYXIcosIndex("A-0503") && vecSlice[6] == 0x0fff);

		getAllCoveringCells(PYXIcosIndex("A-005050"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 8 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("A-04") && vecSlice[1] == 0x00fc
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0f03
			&& vecCovering[3] == PYXIcosIndex("A-005") && vecSlice[3] == 0x0fff
			&& vecCovering[4] == PYXIcosIndex("A-0050") && vecSlice[4] == 0x000f
			&& vecCovering[5] == PYXIcosIndex("A-0406") && vecSlice[5] == 0x00f0
			&& vecCovering[6] == PYXIcosIndex("A-0503") && vecSlice[6] == 0x0f00
			&& vecCovering[7] == PYXIcosIndex("A-00505") && vecSlice[7] == 0x0fff);
		getAllCoveringCells(PYXIcosIndex("A-005010"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 8 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("A-00") && vecSlice[1] == 0x003f
			&& vecCovering[2] == PYXIcosIndex("A-05") && vecSlice[2] == 0x0fc0
			&& vecCovering[3] == PYXIcosIndex("A-005") && vecSlice[3] == 0x0fff
			&& vecCovering[4] == PYXIcosIndex("A-0050") && vecSlice[4] == 0x00f0
			&& vecCovering[5] == PYXIcosIndex("A-0502") && vecSlice[5] == 0x0f00
			&& vecCovering[6] == PYXIcosIndex("A-0005") && vecSlice[6] == 0x000f
			&& vecCovering[7] == PYXIcosIndex("A-00501") && vecSlice[7] == 0x0fff);
		getAllCoveringCells(PYXIcosIndex("A-005030"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 8 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("A-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("A-00") && vecSlice[1] == 0x0c0f
			&& vecCovering[2] == PYXIcosIndex("A-04") && vecSlice[2] == 0x03f0
			&& vecCovering[3] == PYXIcosIndex("A-005") && vecSlice[3] == 0x0fff
			&& vecCovering[4] == PYXIcosIndex("A-0050") && vecSlice[4] == 0x0f00
			&& vecCovering[5] == PYXIcosIndex("A-0004") && vecSlice[5] == 0x000f
			&& vecCovering[6] == PYXIcosIndex("A-0401") && vecSlice[6] == 0x00f0
			&& vecCovering[7] == PYXIcosIndex("A-00503") && vecSlice[7] == 0x0fff);

		getAllCoveringCells(PYXIcosIndex("3-005030"), vecCovering, vecSlice);
		TEST_ASSERT(vecCovering.size() == 8 && vecCovering.size() == vecSlice.size()
			&& vecCovering[0] == PYXIcosIndex("3-0") && vecSlice[0] == 0x0fff
			&& vecCovering[1] == PYXIcosIndex("3-00") && vecSlice[1] == 0x0c0f
			&& vecCovering[2] == PYXIcosIndex("3-04") && vecSlice[2] == 0x03f0
			&& vecCovering[3] == PYXIcosIndex("3-005") && vecSlice[3] == 0x0fff
			&& vecCovering[4] == PYXIcosIndex("3-0050") && vecSlice[4] == 0x0f00
			&& vecCovering[5] == PYXIcosIndex("3-0004") && vecSlice[5] == 0x000f
			&& vecCovering[6] == PYXIcosIndex("3-0401") && vecSlice[6] == 0x00f0
			&& vecCovering[7] == PYXIcosIndex("3-00503") && vecSlice[7] == 0x0fff);
	}

	// test faceDirectionFromVertex and faceFromVertex methods
	PYXMath::eHexDirection nDirection;
	PYXIcosMath::faceDirectionFromVertex(1, 'A', &nDirection);
	TEST_ASSERT(nDirection == PYXMath::knDirectionTwo);

	TEST_ASSERT(PYXIcosMath::faceFromVertex(	1, 
												PYXMath::knDirectionTwo	) == 'A');

	PYXIcosMath::faceDirectionFromVertex(12, 'S', &nDirection);
	TEST_ASSERT(nDirection == PYXMath::knDirectionFive);

	TEST_ASSERT(PYXIcosMath::faceFromVertex(	12, 
												PYXMath::knDirectionFive	) == 'S');

	PYXIcosMath::faceDirectionFromVertex(8, 'L', &nDirection);
	TEST_ASSERT(nDirection == PYXMath::knDirectionSix);

	TEST_ASSERT(PYXIcosMath::faceFromVertex(	8, 
												PYXMath::knDirectionSix	) == 'L');

	TEST_ASSERT(!PYXIcosMath::faceDirectionFromVertex(1, 'T', &nDirection));

	// test the faceOwner method
	int nCounter = 0;
	int nVertex;
	for (nCounter = 0; nCounter < Icosahedron::knNumFaces; nCounter++)
	{
		nVertex = 
			PYXIcosMath::faceOwner(nCounter + PYXIcosIndex::kcFaceFirstChar);
		TEST_ASSERT(nVertex >= PYXIcosIndex::knFirstVertex &&
					nVertex <= PYXIcosIndex::knLastVertex);
	}

	// test the isValidDirection method
	index1 = "1-0";
	for (nCounter = 1; nCounter <= Hexagon::knNumSides; nCounter++)
	{
		PYXMath::eHexDirection nDirection = static_cast<PYXMath::eHexDirection>(nCounter);

		if (PYXMath::knDirectionOne == nDirection)
		{
			TEST_ASSERT(!PYXIcosMath::isValidDirection(index1, nDirection));
		}
		else
		{
			TEST_ASSERT(PYXIcosMath::isValidDirection(index1, nDirection));
		}
	}

	index1 = "1-2";

	for (nCounter = 1; nCounter <= Hexagon::knNumSides; nCounter++)
	{
		PYXMath::eHexDirection nDirection = static_cast<PYXMath::eHexDirection>(nCounter);
		TEST_ASSERT(PYXIcosMath::isValidDirection(index1, nDirection));
	}

	// test resolution0Move method
	index1.setPrimaryResolution(1);
	TEST_ASSERT(PYXIcosMath::resolution0Move(&index1, PYXMath::knDirectionFour));
	TEST_ASSERT(index1.getPrimaryResolution() == 4);

	// test a resolution0Move in an invalid direction
	TEST_ASSERT(!PYXIcosMath::resolution0Move(&index1, PYXMath::knDirectionOne));

	// test resolution1Move method
	index1 = "1";
	TEST_ASSERT(PYXIcosMath::resolution1Move(&index1, PYXMath::knDirectionSix));
	index2 = "E";
	TEST_ASSERT(index1 == index2);

	TEST_ASSERT(PYXIcosMath::resolution1Move(&index1, PYXMath::knDirectionThree));
	index2 = "6";
	TEST_ASSERT(index1 == index2);

	/*
	Using the 'move' method the following additional methods will also be
	tested:
		overflowCorrect
		gapCorrect
	*/

	// step across a gap
	index1 = "3-00201";
	index2 = "3-06030";
	index3 = move(index1, PYXMath::knDirectionSix);
	TEST_ASSERT(index3 == index2);

	index3 = move(index2, PYXMath::knDirectionTwo);
	TEST_ASSERT(index3 == index1);

	index1 = "1-206";
	index2 = "1-602";
	index3 = move(index1, PYXMath::knDirectionSix);
	TEST_ASSERT(index3 == index2);

	index3 = move(index2, PYXMath::knDirectionTwo);
	TEST_ASSERT(index3 == index1);
	
	// step from vertex to vertex
	index1 = "3-40404";
	index2 = "7-10101";
	index3 = move(index1, PYXMath::knDirectionFour);
	TEST_ASSERT(index3 == index2);

	index3 = move(index2, PYXMath::knDirectionOne);
	TEST_ASSERT(index3 == index1);

	// step from face to face
	index1 = "K-020202";
	index2 = "F-050505";
	index3 = move(index1, PYXMath::knDirectionTwo);
	TEST_ASSERT(index3 == index2);

	index3 = move(index2, PYXMath::knDirectionFive);
	TEST_ASSERT(index3 == index1);
	
	// step from vertex to face
	index1 = "3-4020";
	index2 = "F-0606";
	index3 = move(index1, PYXMath::knDirectionTwo);
	TEST_ASSERT(index3 == index2);

	index3 = move(index2, PYXMath::knDirectionFive);
	TEST_ASSERT(index3 == index1);

	// step on and off of PYXIS poles in both classes
	index1 = "1-60501";
	index2 = "D-06010";
	index3 = move(index1, PYXMath::knDirectionSix);
	TEST_ASSERT(index3 == index2);

	index3 = move(index2, PYXMath::knDirectionTwo);
	TEST_ASSERT(index3 == index1);

	index1 = "1-605006";
	index2 = "D-060102";
	index3 = move(index1, PYXMath::knDirectionSix);
	TEST_ASSERT(index3 == index2);

	index3 = move(index2, PYXMath::knDirectionTwo);
	TEST_ASSERT(index3 == index1);

	// step across a gap with overflow
	index1 = "03-201";
	index2 = "B-030";
	index3 = "B-003";
	index4 = "B-020";

	index5 = move(index1, PYXMath::knDirectionSix);
	TEST_ASSERT(index5 == index2);
	index5 = move(index2, PYXMath::knDirectionTwo);
	TEST_ASSERT(index5 == index1);

	index5 = move(index1, PYXMath::knDirectionOne);
	TEST_ASSERT(index5 == index3);
	index5 = move(index3, PYXMath::knDirectionThree);
	TEST_ASSERT(index5 == index1);

	index5 = move(index1, PYXMath::knDirectionTwo);
	TEST_ASSERT(index5 == index4);
	index5 = move(index4, PYXMath::knDirectionFour);
	TEST_ASSERT(index5 == index1);

	index1 = "3-202";
	index2 = "1-303";

	index5 = move(index4, PYXMath::knDirectionThree);
	TEST_ASSERT(index1 == index5);
	index5 = move(index1, PYXMath::knDirectionOne);
	TEST_ASSERT(index5 == index4);

	index5 = move(index4, PYXMath::knDirectionTwo);
	TEST_ASSERT(index2 == index5);
	index5 = move(index2, PYXMath::knDirectionFour);
	TEST_ASSERT(index5 == index4);

	// test getNumCells() and related methods
	TEST_ASSERT(12 == getCellCount(0));
	TEST_ASSERT(32 == getCellCount(1));
	TEST_ASSERT(92 == getCellCount(2));
	TEST_ASSERT(272 == getCellCount(3));
	TEST_ASSERT(5314412 == getCellCount(12));

	// test centroid child on vertex
	index1 = "01";
	TEST_ASSERT(1 == getCellCount(index1, 1));
	TEST_ASSERT(6 == getCellCount(index1, 2));
	TEST_ASSERT(11 == getCellCount(index1, 3));

	// test vertex child on vertex
	index1 = "01-2";
	TEST_ASSERT(1 == getCellCount(index1, 2));
	TEST_ASSERT(1 == getCellCount(index1, 3));
	TEST_ASSERT(7 == getCellCount(index1, 4));

	// test vertex child on face
	index1 = "A";
	TEST_ASSERT(1 == getCellCount(index1, 1));
	TEST_ASSERT(1 == getCellCount(index1, 2));
	TEST_ASSERT(7 == getCellCount(index1, 3));

	// test centroid child on face
	index1 = "A-0";
	TEST_ASSERT(1 == getCellCount(index1, 2));
	TEST_ASSERT(7 == getCellCount(index1, 3));
	TEST_ASSERT(13 == getCellCount(index1, 4));

	// test calcCellPosition() and related methods
	int knTestResolution = 2;
	unsigned int nOffset = 0;
	PYXIcosIterator it(knTestResolution);
	for (; !it.end(); it.next())
	{
		TEST_ASSERT(calcCellPosition(it.getIndex()) == nOffset);
		nOffset++;
	}

	{
		// test rotateDirections

		// pentagon
		index1 = "1";
		TEST_ASSERT(PYXMath::knDirectionTwo == rotateDirection(index1, PYXMath::knDirectionOne, 1));
		TEST_ASSERT(PYXMath::knDirectionSix == rotateDirection(index1, PYXMath::knDirectionOne, -1));
		TEST_ASSERT(PYXMath::knDirectionThree == rotateDirection(index1, PYXMath::knDirectionTwo, 1));
		TEST_ASSERT(PYXMath::knDirectionSix == rotateDirection(index1, PYXMath::knDirectionTwo, -1));
		TEST_ASSERT(PYXMath::knDirectionFive == rotateDirection(index1, PYXMath::knDirectionFour, 1));
		TEST_ASSERT(PYXMath::knDirectionThree == rotateDirection(index1, PYXMath::knDirectionFour, -1));

		// hexagon
		index1 = "A";
		TEST_ASSERT(PYXMath::knDirectionTwo == rotateDirection(index1, PYXMath::knDirectionOne, 1));
		TEST_ASSERT(PYXMath::knDirectionSix == rotateDirection(index1, PYXMath::knDirectionOne, -1));
		TEST_ASSERT(PYXMath::knDirectionThree == rotateDirection(index1, PYXMath::knDirectionTwo, 1));
		TEST_ASSERT(PYXMath::knDirectionOne == rotateDirection(index1, PYXMath::knDirectionTwo, -1));
		TEST_ASSERT(PYXMath::knDirectionFive == rotateDirection(index1, PYXMath::knDirectionFour, 1));
		TEST_ASSERT(PYXMath::knDirectionThree == rotateDirection(index1, PYXMath::knDirectionFour, -1));
	}

	{
		// Testing calcAncestorIndex
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A")) == PYXIcosIndex("A"));
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A-01")) == PYXIcosIndex("A-01"));
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A-0102")) == PYXIcosIndex("A-0102"));
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A-010203")) == PYXIcosIndex("A-010203"));
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A-01020300")) == PYXIcosIndex("A-010203"));
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A-01020304")) == PYXIcosIndex("A-010203"));

		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A-010204")) == PYXIcosIndex("A-01020"));
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A-010304")) == PYXIcosIndex("A-010"));
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("A-020304")) == PYXIcosIndex("A-0"));

		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("1-020304"), PYXIcosIndex("1-20304")) == PYXIcosIndex("1"));
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("B-010203")) == PYXIcosIndex());
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex("12-010203")) == PYXIcosIndex());

		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex("A-010203"), PYXIcosIndex()) == PYXIcosIndex());
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex(), PYXIcosIndex("A-010203")) == PYXIcosIndex());
		TEST_ASSERT(calcAncestorIndex(PYXIcosIndex(), PYXIcosIndex()) == PYXIcosIndex());
	}

	{
		// Testing getNeighbourDirection
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("1-0"), PYXIcosIndex("2-0")) == PYXMath::knDirectionZero);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("1-0"), PYXIcosIndex("1-2")) == PYXMath::knDirectionTwo);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("1-0"), PYXIcosIndex("1-3")) == PYXMath::knDirectionThree);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("1-0"), PYXIcosIndex("1-4")) == PYXMath::knDirectionFour);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("1-0"), PYXIcosIndex("1-5")) == PYXMath::knDirectionFive);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("1-0"), PYXIcosIndex("1-6")) == PYXMath::knDirectionSix);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("A-00"), PYXIcosIndex("A-01")) == PYXMath::knDirectionOne);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("A-00"), PYXIcosIndex("A-02")) == PYXMath::knDirectionTwo);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("A-00"), PYXIcosIndex("A-03")) == PYXMath::knDirectionThree);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("A-00"), PYXIcosIndex("A-04")) == PYXMath::knDirectionFour);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("A-00"), PYXIcosIndex("A-05")) == PYXMath::knDirectionFive);
		TEST_ASSERT(PYXIcosMath::getNeighbourDirection(PYXIcosIndex("A-00"), PYXIcosIndex("A-06")) == PYXMath::knDirectionSix);
	}

	{
		// Testing calcIndexFromOffset
		for (int nTest = 0; nTest != 10; ++nTest)
		{
			PYXIcosIndex root;
			root.randomize(2 + rand() % 10);
			const int nRes = root.getResolution() + rand() % 10;
			PYXExhaustiveIterator it(root, nRes);
			for (int n = 0; !it.end(); it.next(), ++n)
			{
				TEST_ASSERT (calcIndexFromOffset(root, nRes, n) == it.getIndex());
			}
		}
		TEST_ASSERT_EXCEPTION( calcIndexFromOffset(PYXIcosIndex("M-0"), 10, getCellCount(PYXIcosIndex("M-0"), 10)), PYXMathException);
	}

	{
		{
			// Cross tessellation.
			PYXIcosIndex index("A-01");
			int nRotate = 0;
			move(&index, PYXMath::knDirectionOne, &nRotate);
			TEST_ASSERT(nRotate == 4);
		}
		{
			// Cross tessellation.
			PYXIcosIndex index("A-02");
			int nRotate = 0;
			move(&index, PYXMath::knDirectionTwo, &nRotate);
			TEST_ASSERT(nRotate == 1);
		}
		{
			// Cross tessellation.
			PYXIcosIndex index("A-06");
			int nRotate = 0;
			move(&index, PYXMath::knDirectionSix, &nRotate);
			TEST_ASSERT(nRotate == 5);
		}
		{
			// Cross gap.
			PYXIcosIndex index("1-6");
			int nRotate = 0;
			move(&index, PYXMath::knDirectionTwo, &nRotate);
			TEST_ASSERT(nRotate == 1);
		}
		{
			// Cross gap.
			PYXIcosIndex index("1-2");
			int nRotate = 0;
			move(&index, PYXMath::knDirectionSix, &nRotate);
			TEST_ASSERT(nRotate == 5);
		}
	}
}

/*!
Move from a resolution 0 vertex to an adjacent resolution 0 vertex in a given
direction. This method does not correct for stepping into a pentagon gap.

\param pIndex			The index that we are moving (in/out)
\param nHexDirection	The direction to move on resolution 0
\param pnRotate			The rotation performed (out) (optional)

\return	true if a valid move was made, otherwise false
*/ 
bool PYXIcosMath::resolution0Move(	PYXIcosIndex* pIndex, 
									PYXMath::eHexDirection nHexDirection,
									int* pnRotate	)
{
	assert(0 != pIndex);

	int nTableVal = -1;
	int nRotate = 0;

	if (nHexDirection != PYXMath::knDirectionZero)
	{
		// verify a vertex was passed
		if (pIndex->isVertex())
		{
			int nVertex = pIndex->m_nPrimaryResolution - PYXIcosIndex::knFirstVertex;

			// assign the table value
			nTableVal = kpnRes0Connect[nVertex][nHexDirection - 1][0];

			if (nTableVal != -1)
			{
				nRotate = kpnRes0Connect[nVertex][nHexDirection - 1][1];
				pIndex->m_nPrimaryResolution = nTableVal;
			}

			// rotate the index according to the table val
			if (nRotate > 0)
			{
				rotateIndex(pIndex, nRotate);
			}
		}
		else
		{
			PYXTHROW(	PYXMathException,
						"Invalid vertex: '" << pIndex->getPrimaryResolution() << "'."	);
		}
	}

	PYXMath::rotateDelta(pnRotate, nRotate);

	return (nTableVal != -1);
}

/*!
Move from a resolution 1 vertex or face to an adjacent resolution 1 vertex or
face in a given direction.  This method does not correct for stepping into a
pentagon gap.

\param pIndex			The index that we are moving (in/out)
\param nHexDirection	The direction to move on resolution 1
\param pnRotate			The rotation performed (out) (optional)

\return	true if a valid move was made otherwise false
*/
bool PYXIcosMath::resolution1Move(	PYXIcosIndex* pIndex,
									PYXMath::eHexDirection nHexDirection,
									int* pnRotate	)
{
	assert(0 != pIndex);

	int nTableVal = -1;
	int nRotate = 0;

	// verify the extents of the values
	if (nHexDirection != PYXMath::knDirectionZero)
	{

		if (pIndex->isVertex())
		{
			// on a vertex
			int nVertex = pIndex->m_nPrimaryResolution - PYXIcosIndex::knFirstVertex;
			nTableVal = kpnRes1VertConnect[nVertex][nHexDirection - 1][0];
			nRotate = kpnRes1VertConnect[nVertex][nHexDirection - 1][1];
		}
		else
		{
			// on a face
			int nFace = pIndex->m_nPrimaryResolution - PYXIcosIndex::kcFaceFirstChar;
			nTableVal = kpnRes1FaceConnect[nFace][nHexDirection - 1][0];
			nRotate = kpnRes1FaceConnect[nFace][nHexDirection - 1][1];
		}

		// process the table value
		if (PYXIcosIndex::isValidPrimary(nTableVal))
		{
			pIndex->setPrimaryResolution(nTableVal);

			// rotate the subindex if necessary
			if (nRotate > 0)
			{
				rotateIndex(pIndex, nRotate);
			}
		}
		else
		{
			// recursively rotate
			int nDirection = nHexDirection - 1;
			if (0 >= nDirection)
			{
				nDirection = Hexagon::knNumSides;
			}

			resolution1Move(	pIndex,
								static_cast<PYXMath::eHexDirection>(nDirection),
								pnRotate	);

			nRotate = -1;
			rotateIndex(pIndex, nRotate);
			nTableVal = 1;
		}
	}

	PYXMath::rotateDelta(pnRotate, nRotate);

	return (-1 != nTableVal);
}

/*!
Move from a resolution 2 face to an adjacent resolution 2 face in a given
direction. This method does not correct for stepping into a pentagon gap.

\param pIndex			The index that we are moving (in/out)
\param nHexDirection	The direction to move on resolution 2
\param pnRotate			The rotation performed (out) (optional)
*/
void PYXIcosMath::resolution2Move(	PYXIcosIndex* pIndex,
									PYXMath::eHexDirection nHexDirection,
									int* pnRotate	)
{
	assert(0 != pIndex);
	assert(pIndex->isFace());

	int nRotate = 0;

	// verify the extents of the values
	if (nHexDirection != PYXMath::knDirectionZero)
	{
		int nFace =	pIndex->getPrimaryResolution() - PYXIcosIndex::kcFaceFirstChar;
		
		pIndex->setPrimaryResolution(kpnRes2FaceConnect[nFace][nHexDirection - 1][0]);

		pIndex->m_pyxIndex.setDigit(0, kpnRes2FaceConnect[nFace][nHexDirection - 1][1]);
		
		nRotate = kpnRes2FaceConnect[nFace][nHexDirection - 1][2];

		// rotate the subindex if necessary
		if (nRotate > 0)
		{
			rotateIndex(pIndex, nRotate);
		}
	}

	PYXMath::rotateDelta(pnRotate, nRotate);
}

/*!
Move at any resolution. Index must not be null. Will not move into an invalid
direction from a pentagon.

\param pIndex			The index that we are moving (in/out)
\param nHexDirection	The direction to move
\param pnRotate			The rotation performed (out) (optional)

\return	True if a valid move was made otherwise false
*/
bool PYXIcosMath::move(	PYXIcosIndex* pIndex,
						PYXMath::eHexDirection nHexDirection,
						int* pnRotate	)
{
	assert(0 != pIndex);
	assert(!pIndex->isNull());

	int nResolution = pIndex->getResolution();

	// deal with all indices that have sub resolutions
	if (PYXIcosIndex::knMinSubRes <= nResolution)
	{
		if (isValidDirection(*pIndex, nHexDirection))
		{
			PYXIcosIndex start = *pIndex;

			// call add on the subindex.
			PYXMath::move(start.m_pyxIndex, nHexDirection, &(pIndex->m_pyxIndex));

			// perform special handling for stepping off of tesselation
			overflowCorrect(pIndex, nResolution, pnRotate);

			// correct the address if we stepped into a gap
			gapCorrect(start, pIndex, pnRotate);

			return true;
		}
	}
	else if (nResolution == PYXIcosIndex::knResolution1)
	{
		return resolution1Move(pIndex, nHexDirection, pnRotate);
	}
	else if (nResolution == PYXIcosIndex::knResolution0)
	{
		// On the first resolution (12 pentagons), simply do lookup
		return resolution0Move(pIndex, nHexDirection, pnRotate);
	}

	return false;
}

/*!
Zoom in to the next higher resolution. The new cell is either an origin child
or a vertex child of the specified cell. If the direction is set to
knDirectionZero (default) the new index will be the origin child. If the 
direction is set to a value other than knDirectionZero the new index will be
the corresponding vertex child. If the function fails to find a child cell the
passed index will remain unchanged.

\param pIndex			The cell index (in/out)
\param nHexDirection	The direction to zoom down in

\return true if the new index is valid otherwise false
*/
bool PYXIcosMath::zoomIntoChildren(PYXIcosIndex* pIndex, PYXMath::eHexDirection nHexDirection)
{
	assert(0 != pIndex);

	if (pIndex->isNull())
	{
		PYXTHROW(PYXMathException, "Invalid index.");
	}

	int const nResolution = pIndex->getResolution();

	// verify the direction and resolution
	if (nResolution < PYXMath::knMaxAbsResolution && isValidDirection(*pIndex, nHexDirection))
	{
		// check for the most common case first
		if (PYXIcosIndex::knMinSubRes <= nResolution)
		{
			// just zoom in on the sub index
			return PYXMath::zoomIn(&(pIndex->getSubIndex()), nHexDirection);
		}

		if (PYXMath::knDirectionZero == nHexDirection)
		{
			pIndex->setResolution(nResolution + 1);

			return true;
		}

		if (pIndex->hasVertexChildren())
		{
			pIndex->setResolution(nResolution + 1);
			*pIndex = move(*pIndex, nHexDirection);

			return true;
		}
	}

	return false;
}

/*!
Zoom in to the next higher resolution. If the direction is set to
knDirectionZero (default) the new index will be the centroid child. If the 
direction is set to a value other than knDirectionZero, the new index will be
the corresponding neighbour of the centroid child.

\param pIndex			The cell index (in/out)
\param nHexDirection	The direction to zoom down in

\return true if the new index is valid otherwise false
*/
bool PYXIcosMath::zoomIntoNeighbourhood(PYXIcosIndex* pIndex, PYXMath::eHexDirection nHexDirection)
{
	assert(0 != pIndex);

	bool bValidIndex = false;

	if (pIndex->isNull() )
	{
		PYXTHROW(PYXMathException, "Invalid index.");
	}
	else
	{
		int nResolution = pIndex->getResolution();

		// verify the direction and resolution
		if (nResolution < PYXMath::knMaxAbsResolution)
		{
			if (isValidDirection(*pIndex, nHexDirection))
			{
				bValidIndex = true;

				// check for the most common case first
				if (nResolution >= PYXIcosIndex::knMinSubRes)
				{
					// just zoom in on the sub index
					if (!PYXMath::zoomIn(&(pIndex->getSubIndex()), nHexDirection))
					{
						PYXMath::zoomIn(&(pIndex->getSubIndex()));
						*pIndex = move(*pIndex, nHexDirection);
					}
				}
				else if (nResolution == PYXIcosIndex::knResolution1)
				{
					// resolution 1 requires the sub index to be initialized
					pIndex->setResolution(nResolution + 1);
					if (nHexDirection != PYXMath::knDirectionZero)
					{
						*pIndex = move(*pIndex, nHexDirection);
					}					
				}
				else 
				{
					// we are only left with res 0
					assert(nResolution == PYXIcosIndex::knResolution0);

					// we zoom into the resolution 1 vertex
					if (nHexDirection == PYXMath::knDirectionZero)
					{
						// simple zoom in so we are done
						pIndex->setResolution(nResolution + 1);
					}
					else
					{
						// we are zooming in onto a face
						int nFaceVal = pIndex->getPrimaryResolution();
						pIndex->setResolution(nResolution + 1);
						*pIndex = move(*pIndex, nHexDirection);

						// check to see if the starting vertex owns this face
						bValidIndex =
							(nFaceVal == PYXIcosMath::faceOwner(pIndex->getPrimaryResolution()));
					}				
				}
			}
		}
	}

	return bValidIndex;
}

/*!
Zoom out to the next lower resolution. The new cell either encloses the current
cell or the current cell is located at one of its vertices. The location of the
current cell relative to the new cell is returned.  The location of the new 
cell is ambiguous if new cell resides in resolution 0 or 1.

\param	pIndex		The cell index (in/out)
\param	pnDirection	The direction of the new index relative to the old index (out)

\return	true if the zoom out operation was successful otherwise false.
*/
bool PYXIcosMath::zoomOut(	PYXIcosIndex* pIndex,
							PYXMath::eHexDirection* pnDirection	)
{
	assert(0 != pIndex);

	// check for conditions where we can't zoom out
	if (!pIndex->isNull() && !pIndex->m_pyxIndex.isNull())
	{
		try
		{
			return PYXMath::zoomOut(&(pIndex->m_pyxIndex), pnDirection);
		}
		catch (PYXException& e)
		{
			PYXRETHROW(e, PYXMathException, "Unable to zoom out on index '" << *pIndex << "'.");
		}		
	}

	return false;
}

/*!
Rotate the index a specified number of times in the counter-clockwise
direction. If nRotation is negative then rotate in the clockwise direction.
This method does not correct for stepping into the gap of a pentagon
tessellation.

\param pIndex		The PYXIcosIndex to be rotated (in/out)
\param nRotation	The number of rotations to perform
*/
void PYXIcosMath::rotateIndex(	PYXIcosIndex* pIndex,
								int nRotation	)
{
	PYXMath::eRotateDir nDirection = PYXMath::knCCW;

	if (nRotation < 0)
	{
		nDirection = PYXMath::knCW;
		nRotation = -nRotation;
	}

	// perform a standard rotation of the sub index
	PYXMath::rotateIndex(&(pIndex->m_pyxIndex), nRotation, nDirection);
}

/*!
Rotate the specified index a specified number of times in the counter-clockwise
direction. If nRotation is negative then rotate in the clockwise direction.  
This method does not correct for stepping into the gap of a pentagon
tessellation.

\param pIndex		The PYXIcosIndex to be rotated (in/out)
\param nDigits		The number of digits on the right side to rotate.
\param nRotation	The number of rotations to perform
*/
void PYXIcosMath::rotateTail(	PYXIcosIndex* pIndex,
								int nDigits,
								int nRotation	)
{
	if (	nDigits > pIndex->m_pyxIndex.getResolution() ||
			nDigits < 1	)
	{
		PYXTHROW(	PYXMathException,
					"Invalid digit count of '" << nDigits <<
					"' for rotation of index '" << *pIndex << "'."	);
	}


	PYXIndex front = pIndex->m_pyxIndex.split(	
									pIndex->m_pyxIndex.getDigitCount() - 
									nDigits	); 
	
	// rotate the tail
	PYXMath::eRotateDir nDirection = PYXMath::knCCW;
	if (nRotation < 0)
	{
		nDirection = PYXMath::knCW;
		nRotation = -nRotation;
	}

	PYXMath::rotateIndex(&pIndex->m_pyxIndex, nRotation, nDirection);

	// reassemble the tail
	front.append(pIndex->m_pyxIndex);
	pIndex->m_pyxIndex = front;
}

/*!
Calculate the common ancestor index of the two indices.

\param	index1	The first index.
\param	index2	The second index.

\return	The common ancestor index of the two indices, or the null index if
		there is no common ancestor.
*/
PYXIcosIndex PYXIcosMath::calcAncestorIndex(	const PYXIcosIndex& index1,
												const PYXIcosIndex& index2	)
{
	PYXIcosIndex indexAncestor;

	if (index1.m_nPrimaryResolution == index2.m_nPrimaryResolution)
	{
		indexAncestor.m_nPrimaryResolution = index1.m_nPrimaryResolution;
		indexAncestor.m_pyxIndex =
			PYXMath::calcAncestorIndex(index1.m_pyxIndex, index2.m_pyxIndex);
	}

	return indexAncestor;
}

/*!
Calculate the relative descendant index from a parent to child.

\param	parent	The parent index.
\param	child	The child index.

\return	The relative descendant index of the child to the parent.
*/
PYXIndex PYXIcosMath::calcDescendantIndex(	const PYXIcosIndex& parent,
											const PYXIcosIndex& child	)
{
	PYXIndex relativeIndex;

	if (parent.isNull())
	{
		PYXTHROW(	PYXMathException,
					"Invalid root index: '" << parent << "'."	);
	}
	else
	{
		// make sure child is a descendant of parent
		if (parent.m_nPrimaryResolution != child.m_nPrimaryResolution)
		{
			PYXTHROW(PYXMathException, "'" << child << "' is not a child of '" << parent << "'.");
		}

		try
		{
			relativeIndex =
				PYXMath::calcDescendantIndex(	parent.getSubIndex(),
												child.getSubIndex()	);	
		}
		catch (PYXException& e)
		{
			PYXRETHROW(e, PYXMathException, "'" << child << "' is not a child of '" << parent << "'.");
		}
	}

	return relativeIndex;
}

/*!
Rotate a direction by the specified number of 30 degree rotations.

\param	index		Index upon which the rotation is based
\param	nDirection	The direction to rotate.
\param	nRotate		The number of rotations to perform (positive for counter-
					clockwise rotations and negative for clockwise rotations).

\return The resulting value after the rotation.
*/
PYXMath::eHexDirection PYXIcosMath::rotateDirection(	const PYXIcosIndex& index,
														PYXMath::eHexDirection nDirection,
														int nRotate	)
{
	PYXMath::eHexDirection nNewDirection = PYXMath::knDirectionZero;

	// pentagons have some invalid directions
	if (index.isPentagon())
	{
		nNewDirection = nDirection;

		// zero values do not rotate
		if (PYXMath::knDirectionZero != nDirection)
		{
			// rotate CCW skipping over invalid directions
			while (0 < nRotate)
			{
				if (PYXMath::knDirectionSix == nNewDirection)
				{
					nNewDirection = PYXMath::knDirectionOne;
				}
				else
				{
					nNewDirection = static_cast<PYXMath::eHexDirection>(nNewDirection + 1);
				}
				
				if (isValidDirection(index, nNewDirection))
				{
					--nRotate;
				}
			}

			// rotate CW skipping over invalid directions
			while (0 > nRotate)
			{
				if (PYXMath::knDirectionOne == nNewDirection)
				{
					nNewDirection = PYXMath::knDirectionSix;
				}
				else
				{
					nNewDirection = static_cast<PYXMath::eHexDirection>(nNewDirection - 1);
				}

				if (isValidDirection(index, nNewDirection))
				{
					++nRotate;
				}
			}
		}
	}
	else
	{
		// defer to rotate method in PYXMath
		nNewDirection = PYXMath::rotateDirection(nDirection, nRotate);
	}

	return nNewDirection;
}

/*!
This method creates a new index by moving one index over in the 
specified direction.

\param start			The index that will be moved from
\param nHexDirection	The direction to move in (as defined in PYXMath)

\return The resulting index after the move, Null if invalid direction
*/
PYXIcosIndex PYXIcosMath::move(	const PYXIcosIndex& start,
								PYXMath::eHexDirection nHexDirection	)
{
	PYXIcosIndex result;

	if (!start.isNull())
	{
		int nResolution = start.getResolution();

		// deal with all indices that have sub resolutions
		if (nResolution >= PYXIcosIndex::knMinSubRes)
		{
			// verify we are not a vertex moving into the void
			if (isValidDirection(start, nHexDirection))
			{
				result.setPrimaryResolution(start.getPrimaryResolution());

				// call add on the subindex.
				PYXMath::move(start.m_pyxIndex, nHexDirection, &(result.m_pyxIndex));

				// perform special handling for stepping off of tesselation
				overflowCorrect(&result, nResolution);

				// correct the address if we stepped into a gap
				gapCorrect(start, &result);
			}
		}
		else if (nResolution == PYXIcosIndex::knResolution1)
		{
			result = start;
			if (!resolution1Move(&result, nHexDirection))
			{
				result.reset();
			}
		}
		else if (nResolution == PYXIcosIndex::knResolution0)
		{
			// On the first resolution (12 pentagons), simply do lookup
			result = start;
			if (!resolution0Move(&result, nHexDirection))
			{
				result.reset();
			}
		}
	}

	return result;
}

/*!
Check if the sub index falls within a gap on a vertex tessellation. Also verify
that any faces start with an origin child index.

\param pyxIndex	The index being tested

\return true if the index is valid otherwise false
*/
bool PYXIcosMath::isValidIndex(const PYXIcosIndex& pyxIndex)
{
	if (!pyxIndex.m_pyxIndex.isNull())
	{
		if (pyxIndex.isVertex())
		{
			PYXMath::eHexDirection nSegment = PYXMath::hexSector(pyxIndex.m_pyxIndex);
			return isValidDirection(pyxIndex.m_nPrimaryResolution, nSegment);
		}
		else
		{
			return pyxIndex.m_pyxIndex.getDigit(0) == 0;
		}
	}

	return true;
}

/*!
Add two PYXIcosIndices. The two indices must share a common origin (first two
resolutions must be equal) and the resolution at which the addition is to take
place must be greater than or equal to the PYXIcosIndex::knMinSubRes. If either
index  is less than the stated resolution the sub-index will be prepended with
'0' values until the stated resolution is reached. The method handles
conditions such as stepping off of a tesselation as well as stepping into the
gap of a vertex.

\param first		The augend.  
\param second		The addend.  
\param nResolution	The resolution at which to perform the addition.

\return The resulting PYXIcosIndex
*/

PYXIcosIndex PYXIcosMath::add(	PYXIcosIndex first,
								PYXIcosIndex second,
								int nResolution	)
{
	PYXIcosIndex finalIndex;

	if (!first.isNull() && !second.isNull())
	{
		if (nResolution >= PYXIcosIndex::knMinSubRes)
		{
			// verify the tesselation parent is the same for both indices
			if (first.m_nPrimaryResolution == second.m_nPrimaryResolution)
			{
				finalIndex.setPrimaryResolution(first.getPrimaryResolution());

				// verify the resolution is correct
				if (first.getResolution() != nResolution)
				{
					if (first.getResolution() > nResolution)
					{
						PYXTHROW(	PYXMathException,
									"Unable to add index '" << first.toString() <<
									"' on resolution '" << nResolution << "'."	);
					}
					else
					{
						if (nResolution > first.getResolution())
						{
							first.m_pyxIndex.adjustResolutionLeft(nResolution);
						}
					}
				}

				if (second.getResolution() != nResolution)
				{
					if (second.getResolution() > nResolution)
					{
						PYXTHROW(	PYXMathException,
									"Unable to add index '" << second.toString() <<
									"' on resolution '" << nResolution << "'."	);
					}
					else
					{
						if (nResolution > second.getResolution())
						{
							second.m_pyxIndex.adjustResolutionLeft(nResolution);
						}
					}
				}

				PYXMath::add(	first.m_pyxIndex,
								second.m_pyxIndex,
								nResolution,
								&(finalIndex.m_pyxIndex),
								true	);

				// perform special handling for stepping off of tesselation
				overflowCorrect(&finalIndex, nResolution);

				// correct the address if we stepped into a gap
				gapCorrect(first, &finalIndex);
			}
		}
		else
		{
			PYXTHROW(	PYXMathException,
						"Invalid resolution: '" << nResolution << "'."	);
		}
	}
	return finalIndex;
}

/*!
Determine the parent index of the passed index. The passed index must be at
least knMinSubRes in resolution to return a valid index.

\param pyxIndex	The index for which the parent index is being requested

\return The parent index value
*/
PYXIcosIndex PYXIcosMath::getParent(const PYXIcosIndex& pyxIndex)
{
	PYXIcosIndex parentIndex = pyxIndex;

	if (pyxIndex.getResolution() > PYXIcosIndex::knMinSubRes)
	{
		parentIndex.m_pyxIndex = PYXMath::getParent(pyxIndex.m_pyxIndex);
	}
	else if (pyxIndex.getResolution() == PYXIcosIndex::knMinSubRes)
	{
		parentIndex.setResolution(PYXIcosIndex::knResolution1);
	}
	else
	{
		PYXTHROW(	PYXMathException,
					"Invalid resolution: '" << parentIndex.getResolution()	<< "'."	);
	}

	return parentIndex;
}

/*!
The order of the cells returned is: first the parent cell, then the cell in the
same direction as the child, then the other covering cell.
\param index		The index of the cell whose covering cells are requested.
\param pIndices		The covering cells (output parameter, must be sufficiently large).
\return The number of covering cells (1 or 3).
*/
int PYXIcosMath::getCoveringCells(const PYXIcosIndex& index, PYXIcosIndex* pIndices)
{
	int nCount = 1;

	assert(PYXIcosIndex::knMinSubRes <= index.getResolution());
	assert(pIndices);

	pIndices[0] = PYXIcosMath::getParent(index);
	PYXMath::eHexDirection nDir;
	PYXIcosMath::directionFromParent(index, &nDir);

	if (nDir != PYXMath::knDirectionZero)
	{
		nCount = 3;
		pIndices[1] = PYXIcosMath::move(pIndices[0], nDir);
		pIndices[2] = PYXIcosMath::move(pIndices[0], PYXIcosMath::rotateDirection(pIndices[0], nDir,
			PYXMath::getHexClass(index.getResolution()) == PYXMath::knClassI ? -1 : 1));
	}

	return nCount;
}

/*!
The order of the cells returned is: first the origin child, then the cells as
returned by a vertex iterator.
\param index		The index of the cell whose covered cells are requested.
\param pIndices		The covered cells (output parameter, must be sufficiently large).
\return The number of covering cells (6 or 7).
*/
int PYXIcosMath::getCoveredCells(const PYXIcosIndex& index, PYXIcosIndex* pIndices)
{
	int nCount = 1;

	assert(PYXIcosIndex::knMinSubRes - 1 <= index.getResolution());
	assert(pIndices);

	*pIndices = index;
	pIndices->incrementResolution();

	for (PYXVertexIterator it(index); !it.end(); it.next())
	{
		*(++pIndices) = it.getIndex();
		++nCount;
	}

	return nCount;
}

inline
short rotl(short s, int n)
{
	return ((s << n) & 0x0fff) | (s >> (12 - n));
}

inline short rotr(short s, int n)
{
	return ((s << (12 - n)) & 0x0fff) | (s >> n);
}

// negative is CCW, positive is CW
inline short rot(short s, int n)
{
	return 0 < n ? rotl(s, n) : rotr(s, -n);
}

/*!
\param index		The index of the cell whose covering cells are requested.
\param vecCovering	The covering cells.
\param vecSlice		The covering slice codes.
*/
void PYXIcosMath::getAllCoveringCells(const PYXIcosIndex& index, std::vector<PYXIcosIndex>& vecCovering, std::vector<short>& vecSlice)
{
	assert(PYXIcosIndex::knMinSubRes < index.getResolution());
	vecCovering.clear();
	vecSlice.clear();

	// Mmm... slices.
	const short nSliceWhole = 0x0fff; // lower 12 bits set
	const short nSliceHalf = 0x003f; // lower 6 bits set
	const short nSliceThird = 0x000f; // lower 4 bits set

	const PYXIndex& subindex = index.getSubIndex();

	PYXIcosIndex parent = index;
	parent.setResolution(2);

	const int nDigitCount = subindex.getDigitCount();
	for (int nPos = 1; nPos < nDigitCount; ++nPos)
	{
		int nDigit = subindex.getDigit(nPos);
		if (nDigit)
		{
			// Digit is non-zero so we need to look at subsequent digits to
			// determine whether parent or neighbours cover cell.

			// Class of digit cell (0=I, 1=II).
			int nClass = nPos % 2;

			// Subsequent digits will determine which case applies.
			int nCase = 0; // 0=vert, 1=edge, 2=face

			// Edge moves will only occur in dir nDigit, nDigit rot2, and
			// nDigit rot4, in the same class.
			int nDigitRot2 = PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), 2);
			int nDigitRot4 = PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), 4);

			// If we move onto an edge, we'll store the edge dirs here.
			int nEdgeDir, nEdgeDirNeg;

			// If we move onto a face, we'll store its direction from parent
			// here.
			int nFaceDir = 0;

			int nPosD = nPos + 1;
			int nDigitD;
			int nClassD = nClass; // class of descendant cell
			for (; nPosD != nDigitCount; ++nPosD)
			{
				nDigitD = subindex.getDigit(nPosD);
				nClassD = !nClassD;
				if (nDigitD)
				{
					if (nCase == 0) // on vert?
					{
						if (nClass == nClassD) // same class?
						{
							if (nDigitD == nDigit || nDigitD == nDigitRot2 || nDigitD == nDigitRot4) // edge dir?
							{
								nCase = 1; // move onto edge
								nEdgeDir = nDigitD;
								nEdgeDirNeg = PYXMath::negateDir(static_cast<PYXMath::eHexDirection>(nDigitD));
							}
							else
							{
								nCase = 2; // move onto face
								if (nDigitD == PYXMath::negateDir(static_cast<PYXMath::eHexDirection>(nDigitRot2)))
								{
									nFaceDir = PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), nClass - 1);
								}
								else if (nDigitD == PYXMath::negateDir(static_cast<PYXMath::eHexDirection>(nDigitRot4)))
								{
									nFaceDir = PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), nClass);
								}
								break;
							}
							// else remain on vert
						}
						else // alternate class
						{
							nCase = 2; // move onto face
							int nDigitAdj = nDigit + nClass;
							if (nDigitD == nDigitAdj
								|| nDigitD == PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigitAdj), 1))
							{
								nFaceDir = PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), nClass);
							}
							else if (nDigitD == PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigitAdj), -1)
								|| nDigitD == PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigitAdj), -2))
							{
								nFaceDir = PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), nClass - 1);
							}
							break;
						}
					}
					else // on edge
					{
						if (nClass == nClassD) // same class?
						{
							if (nDigitD != nEdgeDir && nDigitD != nEdgeDirNeg) // non-edge dir?
							{
								nCase = 2; // move onto face
								bool bCCW = nDigitD == PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nEdgeDir), 1)
									|| nDigitD == PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nEdgeDir), 2);
								if (nEdgeDir == nDigitRot2)
								{
									nFaceDir = bCCW ? 0 : PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), nClass);
								}
								else if (nEdgeDir == nDigitRot4)
								{
									nFaceDir = bCCW ? PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), nClass - 1) : 0;
								}
								else
								{
									nFaceDir = PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit),
										bCCW ? nClass : nClass - 1);
								}
								break;
							}
							// else remain on edge
						}
						else
						{
							nCase = 2; // move onto face
							int nEdgeDirAdj = nEdgeDir + nClass;
							bool bCCW = nDigitD == nEdgeDirAdj
								|| nDigitD == PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nEdgeDirAdj), 1)
								|| nDigitD == PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nEdgeDirAdj), 2);
							if (nEdgeDir == nDigitRot2)
							{
								nFaceDir = bCCW ? 0 : PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), nClass);
							}
							else if (nEdgeDir == nDigitRot4)
							{
								nFaceDir = bCCW ? PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit), nClass - 1) : 0;
							}
							else
							{
								nFaceDir = PYXMath::rotateDirection(static_cast<PYXMath::eHexDirection>(nDigit),
									bCCW ? nClass : nClass - 1);
							}
							break;
						}
					}
				}
			}

			if (nCase == 0) // vert
			{
				// First the parent slices.
				vecCovering.push_back(parent);
				int nDigitNeg = PYXMath::negateDir(static_cast<PYXMath::eHexDirection>(nDigit));
				// In this case s denotes the slices on the third covered by the parent.
				short s = rot(nSliceThird, (nDigitNeg - 2) * 2 + 1 + (nClass - nClassD));
				vecSlice.push_back(s);
				// Then the CCW slices.
				vecCovering.push_back(PYXIcosMath::move(parent,
					PYXIcosMath::rotateDirection(parent, static_cast<PYXMath::eHexDirection>(nDigit), nClass - 1)));
				vecSlice.push_back(rotl(s, 4));
				// Then the CW slices.
				vecCovering.push_back(PYXIcosMath::move(parent,
					PYXIcosMath::rotateDirection(parent, static_cast<PYXMath::eHexDirection>(nDigit), nClass)));
				vecSlice.push_back(rotr(s, 4));
			}
			else if (nCase == 1) // edge
			{
				// We're either on the edge in dir=digit, or rot2, or rot4.
				int nDigitNeg = PYXMath::negateDir(static_cast<PYXMath::eHexDirection>(nDigit));
				// In this case s denotes the slices on the half CCW from the dir=digit line.
				short s = rot(nSliceHalf, nDigit * 2 - 1 + (nClass - nClassD));
				if (nEdgeDir != nDigit)
				{
					vecCovering.push_back(parent);
					vecSlice.push_back(rot(s, (nEdgeDir == nDigitRot2) ? 4 : 2));
				}
				if (nEdgeDir != nDigitRot2)
				{
					vecCovering.push_back(PYXIcosMath::move(parent,
						PYXIcosMath::rotateDirection(parent, static_cast<PYXMath::eHexDirection>(nDigit), nClass - 1)));
					vecSlice.push_back(rot(s, (nEdgeDir == nDigit) ? 6 : -4));
				}
				if (nEdgeDir != nDigitRot4)
				{
					vecCovering.push_back(PYXIcosMath::move(parent,
						PYXIcosMath::rotateDirection(parent, static_cast<PYXMath::eHexDirection>(nDigit), nClass)));
					vecSlice.push_back(rot(s, (nEdgeDir == nDigit) ? 0 : -2));
				}
			}
			else // face
			{
				// Face cell covers cell fully.
				vecCovering.push_back(PYXIcosMath::move(parent, static_cast<PYXMath::eHexDirection>(nFaceDir)));
				vecSlice.push_back(nSliceWhole);
			}
		}
		else
		{
			// Digit is zero so parent covers cell fully.
			vecCovering.push_back(parent);
			vecSlice.push_back(nSliceWhole);
		}

		parent.getSubIndex().appendDigit(nDigit);
	}
}

/*!
Get the gap direction for a given index

\param index		The index to find the gap direction
\param pnDirection	The direction of the gap for this index (out)

\return true if the index has a gap direction, otherwise false.
*/
bool PYXIcosMath::getCellGap(	const PYXIcosIndex& index,
								PYXMath::eHexDirection* pnDirection	)
{
	assert((pnDirection != 0) && "Invalid argument.");

	// only return a direction if the index represents a pentagon cell
	if (index.isPentagon())
	{
		*pnDirection = knGapDirection[index.getPrimaryResolution()];
		return true;
	}
	return false;
}



/*!
Determine if a specified direction is valid for the given index. Pentagon cells
have one direction that is invalid.

\param pyxIndex		 The index being examined
\param nHexDirection The direction of movement to test

\return true if the direction is valid, otherwise false.
*/
bool PYXIcosMath::isValidDirection(	const PYXIcosIndex& pyxIndex,
	 								PYXMath::eHexDirection nHexDirection)
{
	if (pyxIndex.isNull())
	{
		return false;
	}
	else
	{
		// we only lose a direction if it is a pentagon
		if ((PYXMath::knDirectionZero != nHexDirection) && pyxIndex.isPentagon())
		{
			if (kpnRes0Connect[pyxIndex.m_nPrimaryResolution - 1][nHexDirection - 1][0] == -1)
			{
				return false;
			}
		}
	}

	return true;
}

/*!
Determine if a specified direction is valid for the given vertex. Vertices have
one direction that is invalid.

\param nVertex		 The vertex number.
\param nHexDirection The direction of movement to test

\return true if the direction is valid, otherwise false.
*/
bool PYXIcosMath::isValidDirection(	int nVertex,
	 								PYXMath::eHexDirection nHexDirection)
{

	/*
	NO EXCEPTION ON ERROR
	assert rather than exception due to high speed requirements.
	*/
	assert(PYXIcosIndex::knFirstVertex <= nVertex);
	assert(PYXIcosIndex::knLastVertex >= nVertex);

	bool bValidDirection = true;

	// we only lose a direction if it is a pentagon
	if (PYXMath::knDirectionZero != nHexDirection)
	{
		if (kpnRes0Connect[nVertex - 1][nHexDirection - 1][0] == -1)
		{
			bValidDirection = false;
		}
	}

	return bValidDirection;
}

/*!
Determine the face that results from a move from a specified vertex in a given
direction.  

\param nVertex		 The vertex.
\param nHexDirection The direction.

\return	The face or -1 if no face found.
*/  
char PYXIcosMath::faceFromVertex(int nVertex, int nHexDirection)
{
	char nReturnChar = -1;

	// verify a valid vertex was provided
	if (nVertex >= PYXIcosIndex::knFirstVertex && 
		nVertex <= PYXIcosIndex::knLastVertex)
	{
		// verify a valid direction was provided
		if (nHexDirection > PYXMath::knDirectionZero && 
			nHexDirection <= PYXMath::knDirectionSix)
		{
			nReturnChar = kpnRes1VertConnect[nVertex - 1][nHexDirection - 1][0];
		}
		else
		{
			// an invalid direction was provided
			PYXTHROW(	PYXMathException,
						"Invalid direction: '" << nHexDirection << "'."	);
		}
	}
	else
	{
		// an invalid vertex was provided
		PYXTHROW(	PYXMathException,
					"Invalid vertex: '" << nVertex << "'."	);
	}

	return nReturnChar;
}

inline char PYXIcosMath::faceFromVertexUnsafe(int nVertex, int nHexDirection)
{
	return kpnRes1VertConnect[nVertex - 1][nHexDirection - 1][0];
}

/*!
Determine the direction onto a face from a given vertex.

\param	nVertex The 1 based value of of the vertex
\param	nFace	 The face we want the direction to.
\param	pnDirection	The direction from the first to second index (out)

\return The relative direction of the face from the vertex.  knUndefined if the
		vertex and the face are not siblings.  Direction is only accurate on 
		resolution 1.

\sa PYXMath::eHexDirection
*/
bool PYXIcosMath::faceDirectionFromVertex(	int nVertex,
											char nFace,
											PYXMath::eHexDirection* pnDirection	)
{
	/*
	NO EXCEPTION ON ERROR
	assert rather than exception due to high speed requirements.
	*/
	assert (	(nVertex >= PYXIcosIndex::knFirstVertex) &&
				(nVertex <= PYXIcosIndex::knLastVertex) &&
				(nFace >= PYXIcosIndex::kcFaceFirstChar) &&
				(nFace <= PYXIcosIndex::kcFaceLastChar) && "Invalid argument."	);

	assert((pnDirection != 0) && "Invalid argument.");

	// decrement the index value since it is 1 based
	nVertex -= PYXIcosIndex::knFirstVertex;
	int nCounter;

	for (nCounter = 0; nCounter < Hexagon::knNumSides; nCounter++)
	{			
		if (kpnRes1VertConnect[nVertex][nCounter][0] == nFace)
		{
			*pnDirection = static_cast<PYXMath::eHexDirection>(nCounter + 1);
			return true;
		}
	}
	return false;
}

/*! 
Determine the vertex that owns a specified face.

\param nFace	The face.

\return The 0 based owning vertex of this face.
*/
int PYXIcosMath::faceOwner(char nFace)
{
	// verify the face values
	if (	(nFace >= PYXIcosIndex::kcFaceFirstChar) &&
			(nFace <= PYXIcosIndex::kcFaceLastChar)	)
	{
		return (kpnFaceOwningVertex[nFace - PYXIcosIndex::kcFaceFirstChar]);
	}

	PYXTHROW(	PYXMathException,
				"Invalid face: '" << nFace << "'."	);
}

/*!
Check an index has stepped off of the tesselation and correct the index to
reflect the new face or vertex it moves to.

\param pIndex			The index (in/out)
\param nResolution		The resolution on which the preceeding operation occurred.
\param pnRotate			The rotation performed (out) (optional)
*/
void PYXIcosMath::overflowCorrect(	PYXIcosIndex* pIndex,
									int nResolution,
									int* pnRotate	)
{
	if (	(nResolution <= PYXMath::knMaxAbsResolution) && 
			(nResolution >= PYXIcosIndex::knMinSubRes) && 
			(!pIndex->isNull())	)
	{
		// extract basic information
		int nOverflowRes = pIndex->getResolution();
		PYXMath::eHexClass nHexClass = PYXMath::getHexClass(nResolution);
		int nOverflow = 0;
		PYXIndex overflow;

		switch (nOverflowRes - nResolution)
		{
		case 0:
			// equal resolution can only step off on a face
			if (pIndex->isFace())
			{
				// check to see if the first digit in subindex has a value
				nOverflow = pIndex->m_pyxIndex.getDigit(0);
				if (nOverflow != PYXMath::knDirectionZero)
				{
					resolution2Move(	pIndex,
										static_cast<PYXMath::eHexDirection>(nOverflow),
										pnRotate	);
				}
			}
			break;

		case 1:
			{
				// The resolution has increased by 1 in this addition
				nOverflow = pIndex->m_pyxIndex.stripLeft();

				// move according to the overflow direction
				resolution1Move(	pIndex, 
									static_cast<PYXMath::eHexDirection>(nOverflow),
									pnRotate	);
				break;
			}

		case 2:
			{
				// Can only be here on a vertex
				assert(pIndex->isVertex());
			
				// strip off the overflow characters 
				nOverflow = pIndex->m_pyxIndex.stripLeft();
				pIndex->m_pyxIndex.stripLeft();
				
				// move according to the overflow direction
				resolution0Move(	pIndex, 
									static_cast<PYXMath::eHexDirection>(nOverflow),
									pnRotate	);
				break;
			}

		default:
			// this is an overflow error condition
			PYXTHROW(	PYXMathException,
						"Overflow error on index: '" << *pIndex << "'."	);
			break;
		}
	}
}

/*! 
If the resulting index falls in the gap this method performs any necessary
rotation to correct the result. The method is only valid when starting and
ending on the same vertex or face.

\sa PYXMath::eRotateDir
\sa PYXMath::rotateIndex

\param startIndex		The index from which we are stepping off.
\param pResultIndex		The index to be corrected (in/out)
\param pnRotate			The rotation performed (out) (optional)

\return true if a gap correction occurred, otherwise false

*/
bool PYXIcosMath::gapCorrect(	const PYXIcosIndex& startIndex,
								PYXIcosIndex* pResultIndex,
								int* pnRotate	)
{
	assert(0 != pResultIndex);

	bool bRotated = false;

	if (!pResultIndex->isNull() && 
		pResultIndex->getResolution() >= PYXIcosIndex::knMinSubRes)
	{
		if (pResultIndex->isVertex())
		{
			PYXMath::eHexDirection nSector = PYXMath::hexSector(pResultIndex->m_pyxIndex);

			// check to see if we are in a gap for this vertex
			if (!isValidDirection(pResultIndex->m_nPrimaryResolution, nSector))
			{
				// we are in a gap so determine rotation direction
				PYXMath::eRotateDir nRotateDir = PYXMath::knCCW;
	
				/*
				NO EXCEPTION ON ERROR
				assert rather than exception due to high speed requirements.
				*/
				assert(	startIndex.getPrimaryResolution() == 
						pResultIndex->getPrimaryResolution());
		
				// we started and ended movement on the same vertex
				int nStartOffset = PYXMath::hexSector(startIndex.m_pyxIndex);
				nStartOffset++;
				if (nStartOffset > Hexagon::knNumSides)
				{
					nStartOffset = PYXMath::knDirectionOne;
				}

				// set the rotation direction
				if (nStartOffset > nSector)
				{
					nRotateDir = PYXMath::knCW;
				}
				
				// rotate the index in appropriate direction
				PYXMath::rotateIndex(&(pResultIndex->m_pyxIndex), 1, nRotateDir);
				PYXMath::rotateDelta(pnRotate, nRotateDir == PYXMath::knCCW ? 1 : -1);
				bRotated = true;
			}
		}
	}

	return bRotated;
}

/*!
Determine how much counter cloockwise rotation is required from a pole in order
to align with the face it is associated with.  All rotation is calculated 
according to the alignment of resolution 1.

\param	nPole			The numeric value representing the PYXIS pole
\param	nHexDirection	The direction of movement from the pole

\return The counter clockwise rotation required to align. -1 if nPole is invalid
*/
int PYXIcosMath::getPolarRotation(	unsigned int nPole,
									PYXMath::eHexDirection nHexDirection)
{
	/*
	NO EXCEPTION ON ERROR
	assert rather than exception due to high speed requirements.
	*/
	assert (	nPole == PYXIcosIndex::knVertexPole1 || 
				nPole == PYXIcosIndex::knVertexPole2	);

	return kpnRes1VertConnect[nPole - 1][nHexDirection - 1][1];

	return -1;
}

/*!
Get the vertex indices for a given face. Faces are oriented so they point up
(towards direction 1) or down (towards direction 4). The vertices are returned
in counter-clockwise order starting with the vertex that points either up or
down.

\param	cFace		The face.
\param	pnVertex1	Pointer to 1-based index for vertex 1 (out)
\param	pnVertex2	Pointer to 1-based index for vertex 2 (out)
\param	pnVertex3	Pointer to 1-based index for vertex 3 (out)
*/
void PYXIcosMath::getFaceVertices(	char cFace,
									int* pnVertex1,
									int* pnVertex2,
									int* pnVertex3	)
{
	// convert face to an index
	int nFaceIndex = cFace - PYXIcosIndex::kcFaceFirstChar;
	assert(0 <= nFaceIndex);
	assert(Icosahedron::knNumFaces > nFaceIndex);

	if (PYXIcosIndex::kcFaceFirstChar > kpnRes1FaceConnect[nFaceIndex][0][0])
	{
		/*
		Direction 1 is a vertex, so the triangle points up. Get the vertices at
		directions 1, 3 and 5.
		*/
		if (0 != pnVertex1)
		{
			*pnVertex1 = kpnRes1FaceConnect[nFaceIndex][0][0];
		}

		if (0 != pnVertex2)
		{
			*pnVertex2 = kpnRes1FaceConnect[nFaceIndex][2][0];
		}

		if (0 != pnVertex3)
		{
			*pnVertex3 = kpnRes1FaceConnect[nFaceIndex][4][0];
		}
	}
	else
	{
		/*
		Direction 1 is a face, so the triangle points down. Get the vertices at
		directions 4, 6 and 2.
		*/
		if (0 != pnVertex1)
		{
			*pnVertex1 = kpnRes1FaceConnect[nFaceIndex][3][0];
		}

		if (0 != pnVertex2)
		{
			*pnVertex2 = kpnRes1FaceConnect[nFaceIndex][5][0];
		}

		if (0 != pnVertex3)
		{
			*pnVertex3 = kpnRes1FaceConnect[nFaceIndex][1][0];
		}
	}
}

/*!
Get the angle of direction one measured counter-clockwise from the base of the
face.

\param	cFace	The face.

\return	The angle in radians.
*/
double PYXIcosMath::getDir1Angle(char cFace)
{
	// convert face to an index
	int nFaceIndex = cFace - PYXIcosIndex::kcFaceFirstChar;
	assert(0 <= nFaceIndex);
	assert(Icosahedron::knNumFaces > nFaceIndex);

	double fAngle;

	if (PYXIcosIndex::kcFaceFirstChar > kpnRes1FaceConnect[nFaceIndex][0][0])
	{
		// direction 1 is a vertex, so the triangle points up.
		fAngle = MathUtils::kf90Rad;
	}
	else
	{
		// direction 1 is a face so the triangle points down
		fAngle = -MathUtils::kf90Rad;
	}

	return fAngle;	
}

/*!
Return an indication if the face is oriented with the point facing upwards or
downwards

\param	cFace	The face being queried

\return true if the face is pointing upwards, otherwise false
*/
bool PYXIcosMath::getFaceOrientation(char cFace)
{
	assert(	cFace >= PYXIcosIndex::kcFaceFirstChar &&
			cFace <= PYXIcosIndex::kcFaceLastChar	);

	int nFaceIndex = cFace - PYXIcosIndex::kcFaceFirstChar;
	if (PYXIcosIndex::kcFaceFirstChar > kpnRes1FaceConnect[nFaceIndex][0][0])
	{
		// direction 1 is a vertex, so the triangle points up.
		return true;
	}
	
	return false;
}

/*! 
This method determines the direction represented by a particular angle.  The
angle (in radians) is the counter clockwise rotation from the 1 direction.

\param fAngle	The angle of the polar coordinate from tessellation centre.

\return The direction.
*/  
PYXMath::eHexDirection PYXIcosMath::getDirectionFromPolar(double fAngle)
{
	PYXMath::eHexDirection nDirection = PYXMath::knDirectionZero;

	if (fAngle < 0.0)
	{
		fAngle = MathUtils::kf360Rad + fAngle;
	}

	if (fAngle < MathUtils::kf60Rad)
	{
		nDirection = PYXMath::knDirectionOne;
	}
	else if (fAngle < MathUtils::kf120Rad)
	{
		nDirection = PYXMath::knDirectionTwo;
	}
	else if (fAngle < MathUtils::kf180Rad)
	{
		nDirection = PYXMath::knDirectionThree;
	}
	else if (fAngle < MathUtils::kf240Rad)
	{
		nDirection = PYXMath::knDirectionFour;
	}
	else if (fAngle < MathUtils::kf300Rad)
	{
		nDirection = PYXMath::knDirectionFive;
	}
	else if (fAngle < MathUtils::kf360Rad)
	{
		nDirection = PYXMath::knDirectionSix;
	}
	else
	{
		assert(false);
	}

	return nDirection;
}

/*!
Convert polar coordinates to a PYXIndex. The angle must be measured counter-
clockwise relative to the base of the face and the radius is specified in 
inter-cell units at resolution zero, which also corresponds to the length of a
side of the icosahedron.  The face that is passed in is not necessarily the face
that will be the ancestor of the index that is returned.

\param	pt			The polar coordinates.
\param	nResolution	The target resolution.
\param	cFace		The face that is the origin of the polar coordinate
\param	pIndex		The PYXIS index (out).

\return	The index at the appropriate resolution.
*/
void PYXIcosMath::polarToIndex(	const PYXCoordPolar& pt,
								int nResolution,
								char cFace,
								PYXIcosIndex* pIndex	)
{
	assert(	(cFace >= PYXIcosIndex::kcFaceFirstChar) &&
			(cFace <= PYXIcosIndex::kcFaceLastChar)	);

	assert(PYXIcosIndex::knMinSubRes <= nResolution);

	if (0 != pIndex)
	{
		// make the angle relative to direction 1
		PYXCoordPolar tempPoint;
		tempPoint.setAngle(pt.angle() - PYXIcosMath::getDir1Angle(cFace));
		tempPoint.setRadius(pt.radius());

		// get the relative position of the index from the face origin
		int nRelResolution = nResolution - PYXIcosIndex::knMinSubRes;
		PYXIndex relIndex;
		PYXMath::polarToIndex(tempPoint, nRelResolution, relIndex, true);

		if (relIndex.getResolution() == nRelResolution)
		{
			// the resolution size has not increased
			pIndex->setPrimaryResolution(cFace);
			pIndex->m_pyxIndex = relIndex;

			int nPos;
			int nDirection = relIndex.mostSignificant(&nPos);

			if (nPos == nRelResolution)
			{
				pIndex->m_pyxIndex.setDigit(0, 0);
				resolution2Move(	pIndex, 
									static_cast<PYXMath::eHexDirection>(nDirection)	);
			}	
		}
		else
		{
			// strip most sig digit to find vertex owner of index
			unsigned int nDirection = relIndex.stripLeft();

			assert(	(0 < nDirection) && 
					(Hexagon::knNumSides >= static_cast<int>(nDirection))	);
			assert(relIndex.getResolution() == nRelResolution);

			int nTableVal = kpnRes1FaceConnect[cFace - PYXIcosIndex::kcFaceFirstChar][nDirection - 1][0];

			/*
			In the case where a polar angle has pushed into a different face from
			the value that was passed, the method is recursively called with the
			corrected face value.
			*/
			int nRotate = 0;
			pIndex->m_pyxIndex = relIndex;
			if (nTableVal >= PYXIcosIndex::kcFaceFirstChar)
			{
				pIndex->setPrimaryResolution(nTableVal);
				nRotate = kpnRes1FaceConnect[cFace - PYXIcosIndex::kcFaceFirstChar][nDirection - 1][1];
			}
			else
			{
				pIndex->setPrimaryResolution(nTableVal);

				// find the relative direction of the face from the vertex and rotate
				if (pIndex->isPolar())
				{
					nRotate = kpnRes1FaceConnect[cFace - PYXIcosIndex::kcFaceFirstChar][nDirection - 1][1];
				}
			}

			if (nRotate != 0)
			{
				PYXIcosMath::rotateIndex(pIndex, nRotate);
			}
		}	

		// if the result is in the gap of the vertex then rotate
		if (!PYXIcosMath::isValidIndex(*pIndex))
		{
			int nRotate = 1;
			if (!pIndex->isPolar() && fabs(pt.angle()) < MathUtils::kf90Rad)
			{
				// If not a polar index, decide which way to rotate based on owning face.
				nRotate = -1;
			}
			PYXIcosMath::rotateIndex(pIndex, nRotate);
		}
	}
}

/*!
Convert a PYXIcosIndex to a polar coordinate. The angle is measured counter-
clockwise relative to the base of the face on which the index resides and
the radius is specified in inter-cell units at resolution zero which also 
corresponds to the length of a side of the icosahedron.  The minimum resolution
at which a polar coordinate can be found is resolution 1 (first resolution 
with faces);

\param	index	The PYXIcosIndex being converted to a polar
\param	pPolar	The polar coordinate the best represents the index (out)
\param	pcFace	The face the polar coordinate is referenced to (out)
*/

void PYXIcosMath::indexToPolar(	const PYXIcosIndex& index,
								PYXCoordPolar* pPolar,
								char* pcFace	)
{
	assert(index.getResolution() >= PYXIcosIndex::knResolution1);

	if (0 != pPolar)
	{
		char cFace = 0;

		PYXMath::indexToPolar(index.getSubIndex(), pPolar);

		if (index.isFace())
		{
			cFace = index.getPrimaryResolution();
			assert('0' != cFace);

			// measure angle from base of triangle
			pPolar->setAngle(	pPolar->angle() + 
								PYXIcosMath::getDir1Angle(cFace));
		}
		else
		{	
			// calculate the direction from the angle
			PYXMath::eHexDirection nDirection = 
							getDirectionFromPolar(pPolar->angle() + MathUtils::kf30Rad);

			// retrieve the face in the designated direction
			cFace = faceFromVertex(index.getPrimaryResolution(), nDirection);

			// rotate by 1 if invalid direction
			if (-1 == cFace) 
			{
				nDirection = static_cast<PYXMath::eHexDirection>(
										nDirection - PYXMath::knDirectionOne);
				if (PYXMath::knDirectionZero >= nDirection)
				{
					nDirection = PYXMath::knDirectionSix;
				}

				cFace = PYXIcosMath::faceFromVertex(	index.getPrimaryResolution(), 
														nDirection	);

				// rotate the polar across the gap between faces
				pPolar->setAngle(pPolar->angle() - MathUtils::kf60Rad);
			}

			// find the direction of movement off of the face
			PYXIcosMath::faceDirectionFromVertex(	index.getPrimaryResolution(), 
													cFace,
													&nDirection);

			// rotate the polar to align with the standard face
			if (index.isPolar())
			{
				int nRotate = getPolarRotation(	index.getPrimaryResolution(),
												static_cast<PYXMath::eHexDirection>(nDirection)	);
				pPolar->setAngle(	pPolar->angle() + 
									(MathUtils::kf60Rad * static_cast<double>(nRotate))	);

				if (index.getPrimaryResolution() == PYXIcosIndex::knVertexPole1)
				{
					nDirection = PYXMath::knDirectionOne;
				}
				else
				{
					nDirection = PYXMath::knDirectionFour;
				}
				
			}

			// find the polar that connects the face to the specified vertex
			PYXCoordPolar vertexPolar;
			vertexPolar.setRadius(PYXMath::calcCircumRadius(0));
			switch (nDirection)
			{
				case PYXMath::knDirectionOne:
				case PYXMath::knDirectionFour:
					vertexPolar.setAngle(MathUtils::kf90Rad);
					break;

				case PYXMath::knDirectionTwo:
				case PYXMath::knDirectionFive:
					vertexPolar.setAngle(-MathUtils::kf30Rad);
					break;

				case PYXMath::knDirectionThree:
				case PYXMath::knDirectionSix:
					vertexPolar.setAngle(-MathUtils::kf150Rad);
					break;

				default:
					// unknown direction
					assert(false);
					break;
			}

			// rotate the angle to align with the face orientation
			if (PYXIcosMath::getFaceOrientation(cFace))
			{
				pPolar->setAngle(pPolar->angle() + MathUtils::kf90Rad);
			}
			else
			{
				pPolar->setAngle(pPolar->angle() - MathUtils::kf90Rad);
			}
			
			// add the two polars together
			double fDx;
			double fDy;
			double fTotalX;
			double fTotalY;

			// vertex components
			fDx = cos(vertexPolar.angle()) * vertexPolar.radius();
			fDy = sin(vertexPolar.angle()) * vertexPolar.radius();

			// origin components
			fTotalX = fDx + cos(pPolar->angle()) * pPolar->radius();
			fTotalY = fDy + sin(pPolar->angle()) * pPolar->radius();

			pPolar->setRadius(sqrt((fTotalX * fTotalX) + (fTotalY * fTotalY)));
			pPolar->setAngle(atan2(fTotalY,fTotalX));
		}

		if (0 != pcFace)
		{
			*pcFace = cFace;
		}
	}
}

/*!
This method is called at application startup to initialize the following data
structures:
- A vector of vertex cell counts for each resolution.
- A vector of vertex cell offsets for each resolution.
- A vector of face cell counts for each resolution.
- A vector of face cell offsets for each resolution.

The cell offsets are the sum of cell counts for all prior resolutions. Note
that the cell at resolution 0 is assumed to be a vertex child (i.e. it has
one child, the centroid child).
*/
void PYXIcosMath::initStaticData()
{
	initVertexVectors();
	initFaceVectors();
	UnitSphere::initialize();
}

/*!
Initialize cell count and resolution offset vectors for vertices.
*/
void PYXIcosMath::initVertexVectors()
{
	// initialize resolution offset and resolution cells
	unsigned int nResolutionOffset = 0;
	int nResolutionCells = 0;

	// initialize number of centroid and vertex children in resolution 0
	int nCentroidChildren = 0;
	int nVertexChildren = 1;

	// initialize resolution cells
	m_vecVertexResolutionCells.resize(PYXMath::knMaxRelResolution + 1);
	ResolutionCellsVector::iterator itCells = m_vecVertexResolutionCells.begin();

	// initialize resolution offsets
	m_vecVertexResolutionOffsets.resize(PYXMath::knMaxRelResolution + 1);
	ResolutionOffsetsVector::iterator itOffsets = m_vecVertexResolutionOffsets.begin();

	for (; itOffsets != m_vecVertexResolutionOffsets.end(); ++itOffsets, ++itCells)
	{
		// add number of cells in previous resolution to total
		nResolutionOffset += static_cast<unsigned int>(nResolutionCells);
		*itOffsets = nResolutionOffset;

		// calculate the number of cells for this resolution
		nResolutionCells = nCentroidChildren + nVertexChildren;
		*itCells = nResolutionCells;

		/*
		Calculate the number of children for the next layer. Each centroid
		child spawns a centroid child and 6 vertex children except for the
		centre pentagon which spawns 5 vertex children. Each vertex child
		spawns a centroid child.
		*/
		int nNewCentroidChildren = nCentroidChildren + nVertexChildren;
		if (0 < nCentroidChildren)
		{
			nVertexChildren = (nCentroidChildren - 1) * Hexagon::knNumSides 
								+ Pentagon::knNumSides;
		}
		else
		{
			nVertexChildren = 0;
		}
		nCentroidChildren = nNewCentroidChildren;
	}
}

/*!
Initialize cell count and resolution offset vectors for faces.
*/
void PYXIcosMath::initFaceVectors()
{
	// initialize resolution offset and resolution cells
	unsigned int nResolutionOffset = 0;
	int nResolutionCells = 0;

	// initialize number of centroid and vertex children in resolution 0
	int nCentroidChildren = 0;
	int nVertexChildren = 1;

	// initialize resolution cells
	m_vecFaceResolutionCells.resize(PYXMath::knMaxRelResolution + 1);
	ResolutionCellsVector::iterator itCells = m_vecFaceResolutionCells.begin();

	// initialize resolution offsets
	m_vecFaceResolutionOffsets.resize(PYXMath::knMaxRelResolution + 1);
	ResolutionOffsetsVector::iterator itOffsets = m_vecFaceResolutionOffsets.begin();

	for (; itOffsets != m_vecFaceResolutionOffsets.end(); ++itOffsets, ++itCells)
	{
		// add number of cells in previous resolution to total
		nResolutionOffset += static_cast<unsigned int>(nResolutionCells);
		*itOffsets = nResolutionOffset;

		// calculate the number of cells for this resolution
		nResolutionCells = nCentroidChildren + nVertexChildren;
		*itCells = nResolutionCells;

		/*
		Calculate the number of children for the next layer. Each centroid
		child spawns a centroid child and 6 vertex children. Each vertex child
		spawns a centroid child.
		*/
		int nNewCentroidChildren = nCentroidChildren + nVertexChildren;
		nVertexChildren = nCentroidChildren * Hexagon::knNumSides;
		nCentroidChildren = nNewCentroidChildren;
	}
}

/*!
Get the number of cells for a given absolute resolution. Resolution 0 is the
icosahedron vertices. Resolution 1 is the vertices and faces and so on. This
method becomes invalid once the count exceeds the value of an integer.

\param	nResolution	The resolution (0 based)

\return	The number of cells for the resolution.
*/
int PYXIcosMath::getCellCount(int nResolution)
{
	// make sure we are within our offset and cell vectors
	if ((0 > nResolution) || (PYXMath::knMaxRelResolution < nResolution))
	{
		PYXTHROW(	PYXMathException,
					"Invalid resolution: '" << nResolution << "'."	);
	}

	int nNumCells = 0;

	// handle special cases
	if (0 == nResolution)
	{
		nNumCells = Icosahedron::knNumVertices;
	}
	else if (1 == nResolution)
	{
		nNumCells = Icosahedron::knNumVertices + Icosahedron::knNumFaces;
	}
	else
	{
		nNumCells =	Icosahedron::knNumVertices *
					getCellCount(kIndex01, nResolution);

		nNumCells +=	Icosahedron::knNumFaces *
						getCellCount(kIndexA, nResolution);
	}
	
	return nNumCells;	
}

/*!
Get the number of cells for a given resolution that are children of a given
root index. This method becomes invalid when the number of cells exceeds the
count that can be expressed by an integer.

\param	root		The root index from which the resolution is being measured.
\param	nResolution	The absolute resolution (0 - based)

\return	The number of cells for the resolution.
*/
int PYXIcosMath::getCellCount(const PYXIcosIndex& root, int nResolution)
{
	assert(!root.isNull());
	assert(1 <= root.getResolution());

	// handle roots that are centroid children
	if (root.isVertex() && root.hasVertexChildren())
	{
		nResolution++;
	}

	// handle faces that are not vertex children
	if (root.isFace() && root.hasVertexChildren())
	{
		nResolution++;
	}

	// calculate resolution relative to the root index
	nResolution -= root.getResolution();

	// make sure we are within our offset and cell vectors
	if ((0 > nResolution) || (PYXMath::knMaxRelResolution < (nResolution + 1)))
	{
		PYXTHROW(	PYXMathException,
					"Invalid resolution of '" << nResolution << 
					"' for getCellCount."	);
	}

	int nNumCells = 0;
	if (root.isPentagon())
	{
		nNumCells = m_vecVertexResolutionCells[nResolution];
	}
	else	// root.isFace()
	{
		nNumCells = m_vecFaceResolutionCells[nResolution];
	}

	return nNumCells;
}

/*!
Get the offset for a given resolution.

\param	root		The root index from which we are measuring.
\param	nResolution	The absolute resolution.

\return	The number of cells in all previous resolutions.
*/
unsigned int PYXIcosMath::getOffset(const PYXIcosIndex& root, int nResolution)
{
	assert(!root.isNull());

	int nRelativeResolution = nResolution - root.getResolution();

	// make sure we are within our offset and cell vectors
	if (	(0 > nRelativeResolution) ||
			(PYXMath::knMaxRelResolution <= nRelativeResolution)	)
	{
		PYXTHROW(	PYXMathException,
					"Invalid resolution: '" << nRelativeResolution << "'."	);
	}

	/*
	Note: the resolutionOffsets vectors assume the root is a vertex child. If
	the root is a centroid child, then we need to move ahead one position in
	the vector and subtract a cell from the result.
	*/
	unsigned int nOffset = 0;
	if (root.hasVertexChildren())
	{
		++nRelativeResolution;
		nOffset = -1;
	}

	if (root.isPentagon())
	{
		nOffset += m_vecVertexResolutionOffsets[nRelativeResolution];
	}
	else
	{
		nOffset += m_vecFaceResolutionOffsets[nRelativeResolution];
	}
	
	return nOffset;	
}

/*!
Calculate the position of a given cell within its resolution. This is the
position in PYXIS ordering. 

\param	index	The index for which the position which is being calculated.

\return	The position of the cell within its resolution.
*/
unsigned int PYXIcosMath::calcCellPosition(const PYXIcosIndex& index)
{
	assert(!index.isNull());

	// make sure we are within our offset and cell vectors
	int nResolution = index.getResolution();
	if ((0 > nResolution) || (PYXMath::knMaxRelResolution < nResolution))
	{
		PYXTHROW(	PYXMathException,
					"Invalid resolution: '" << nResolution << "'."	);
	}

	PYXIcosIndex root;
	root.setPrimaryResolution(	index.getPrimaryResolution()	);

	unsigned int nOffset = 0;
	if (index.isVertex())
	{
		// offset for previous vertices
		nOffset =	(index.getPrimaryResolution() - PYXIcosIndex::knFirstVertex) * 
					getCellCount(root, nResolution);

		// offset in current vertex
		nOffset += calcCellPosition(root, index);
	}
	else	// index.isFace()
	{
		// offset for vertices
		nOffset =	Icosahedron::knNumVertices *
					getCellCount(kIndex01, nResolution);

		// offset for previous faces
		nOffset +=	(index.getPrimaryResolution() - PYXIcosIndex::kcFaceFirstChar) * 
					getCellCount(root, nResolution);

		// offset in current face
		nOffset += calcCellPosition(root, index);
	}

	return nOffset;	
}
	
/*!
Calculate the position of a given cell within its resolution relative to a root
index. This is the position in standard PYXIS ordering.

\sa PYXIterator

This method accesses private members of PYXIndex for performance reasons.

\param	root	The root index.
\param	index	The index for which the position is being calculated.

\return	The position of the cell within its resolution. If the index being
		examined is not a child of the root index an exception is thrown.
*/
unsigned int PYXIcosMath::calcCellPosition(	const PYXIcosIndex& root,
											const PYXIcosIndex& index	)
{
	assert(!root.isNull());
	assert(!index.isNull());
	
	if(index == root)
	{
		return 0;
	}

	// get the descendant index relative to the root index
	PYXIndex relativeIndex = calcDescendantIndex(root, index);

	/*
	Each non-zero digit in the index contributes to its position. Use the
	vector of cell counts for each resolution to calculate the position
	of this cell within its resolution.
	*/
	unsigned int nPosition = 0;

	char* pcDigit = relativeIndex.m_pcDigits;
	char* pcDigitEnd = pcDigit + relativeIndex.m_nDigitCount;
	ResolutionCellsVector::const_iterator vecFaceResolutionCells_begin = m_vecFaceResolutionCells.begin();
	ResolutionCellsVector::const_iterator itFaceCells =
		vecFaceResolutionCells_begin + (relativeIndex.m_nDigitCount - 1);

	bool bItFaceCellsInvalid = false;

	// special handling for missing cells on vertices
	PYXMath::eHexDirection nGapDirection;
	if (getCellGap(root, &nGapDirection))
	{
		ResolutionCellsVector::const_iterator itVertexCells =
			m_vecVertexResolutionCells.begin() + relativeIndex.m_nDigitCount;

		for (; pcDigit < pcDigitEnd; ++pcDigit)
		{
			unsigned int nDigit = *pcDigit - '0';

			// handle the gap
			if (nGapDirection < static_cast<int>(nDigit))
			{
				nDigit--;
			}

			if (0 != nDigit)
			{
				nPosition += *itVertexCells;
				assert( !bItFaceCellsInvalid);
				nPosition += (nDigit - 1) * (*itFaceCells);

				// gap no longer matters after the first non-zero digit
				++pcDigit;
			
				if (!bItFaceCellsInvalid && (itFaceCells == vecFaceResolutionCells_begin))
				{	
					bItFaceCellsInvalid = true;
				}
				else
				{
                    --itFaceCells;
				}
				
				break;
			}

			if (!bItFaceCellsInvalid && (itFaceCells == vecFaceResolutionCells_begin))
			{	
				bItFaceCellsInvalid = true;
			}
			else
			{
                --itFaceCells;
			}
			--itVertexCells;
		}
	}

	ResolutionCellsVector::const_iterator itFaceCellsPlus1 = itFaceCells;
	++itFaceCellsPlus1;
	for (; pcDigit < pcDigitEnd; ++pcDigit)
	{
		unsigned int nDigit = *pcDigit - '0';

		if (0 != nDigit)
		{
			assert( !bItFaceCellsInvalid);
			nPosition += *(itFaceCellsPlus1);
			nPosition += (nDigit - 1) * (*itFaceCells);
		}

		
		if (!bItFaceCellsInvalid && (itFaceCells == vecFaceResolutionCells_begin ))
		{	
			bItFaceCellsInvalid = true;
		}
		else
		{
            --itFaceCells;
			--itFaceCellsPlus1;
		}
	}

	return nPosition;
}

/*!
Calculate the offset position of an edge index.  The position will be the 
offset from the starting position of a full edge iterator.

\param root		The root index of the tesselation
\param index	The index of the edge cell for which to find the offset pos

\return The offset of the specified cell into a standard edge iterator or -1 
		if the index is not a valid edge index.

*/
int PYXIcosMath::calcEdgePosition(	const PYXIcosIndex& root,
									const PYXIcosIndex& index	)
{
	// TO DO: just move this into edge iterator directly

	// create a new edge iterator
	PYXEdgeIterator itEdge(root, index.getResolution());

	// set the position
	if (itEdge.setIteratorIndex(index))
	{
		return itEdge.calcCurrentOffset();	
	}
	else
	{
		assert (false);
		return -1;
	}
}

/*!
Calculate an index from a root at a resolution given an offset which
is the position in which PYXIterator would place the cell.

\sa PYXIterator

\param	root	The root index.
\param	nRes	The resolution at which the offset is being applied.
\param	nOffset	The offset to apply.

\return	The index at the offset. If there are insufficient children
		an exception is thrown.
*/
PYXIcosIndex PYXIcosMath::calcIndexFromOffset(	const PYXIcosIndex& root,
												int nRes,
												unsigned int nOffset	)
{
	assert(!root.isNull() && root.getResolution() <= nRes);

	if (static_cast<unsigned int>(PYXIcosMath::getCellCount(root, nRes)) <= nOffset)
	{
		PYXTHROW(	PYXMathException,
					"Insufficient children in '" << root.toString() << " " << nRes
						<< "' for offset " << nOffset << "."	);
	}

	int nLoop = nRes - root.getResolution();
	PYXIcosIndex index = root;

	while (nLoop--)
	{
		PYXChildIterator it(index);
		unsigned int nCellCount = PYXIcosMath::getCellCount(it.getIndex(), nRes);
		while (nCellCount <= nOffset)
		{
			it.next();
			nOffset -= nCellCount;
			nCellCount = PYXIcosMath::getCellCount(it.getIndex(), nRes);
		}
		index = it.getIndex();
	}

	return index;
}

/*!
Determine if the specified indices are siblings on the grid.

\param	index1		The first index
\param	index2		The second index
\param	pnDirection	The direction from the first to second index (out)
			
\return	true if the indices are on the same resolution and are adjacent to
		one another otherwise false
*/
bool PYXIcosMath::areSiblings(	const PYXIcosIndex& index1,
								const PYXIcosIndex& index2,
								PYXMath::eHexDirection* pnDirection	)
{
	if (!index1.isNull() && !index2.isNull())
	{
		PYXIcosIndex tempIndex;

		for (int nDirection = 1; nDirection <= Hexagon::knNumSides; ++nDirection)
		{
			// look in each of the directions from this index
			tempIndex = PYXIcosMath::move(	index1,
											static_cast<PYXMath::eHexDirection>(nDirection)	);

			if (tempIndex == index2)
			{
				// return the result
				if (0 != pnDirection)
				{
					*pnDirection = static_cast<PYXMath::eHexDirection>(nDirection);
				}

				return true;
			}
		}
	}

	return false;
}


/*!
Find the relative direction of the specified index from its parent.

\param index		The index.
\param pnDirection	The direction. (least significant index digit) (out)

\return true if the direction is valid or false if it is ambiguous
*/ 
bool PYXIcosMath::directionFromParent(	const PYXIcosIndex& index,
										PYXMath::eHexDirection* pnDirection)
{
	bool bValid = false;
	PYXMath::eHexDirection nDirection = PYXMath::knDirectionZero;

	if (!index.isNull())
	{
		if (index.m_pyxIndex.isNull())
		{
			/*
			The faces do not have an actual owner since they are shared
			by several top level verticies.  For ease of looping through
			the overall PYXIS icosahedron an arbitrary ownership was 
			developed.  This value will be returned to the caller.
			*/
			if (index.isFace())
			{
				int nVertParent = faceOwner(index.getPrimaryResolution());
				faceDirectionFromVertex(	nVertParent,
											index.getPrimaryResolution(),
											&nDirection	);
			}
			else
			{
				// if it is a vertex then it is a centroid child
				nDirection = PYXMath::knDirectionZero;
			}
		}
		else
		{
			// get the least significant digit in the index
			nDirection = static_cast<PYXMath::eHexDirection>(
				index.m_pyxIndex.getDigit(index.m_pyxIndex.getResolution()));
			bValid = true;
		}
	}

	if (pnDirection != 0)
	{
		*pnDirection = nDirection;
	}
	
	return bValid;
}

/*! 
The data will be considered contained if the resolution above the data 
resolution has all of its vertices as well as its centroid centred over 
hexagons that belong to the same parent.  This value will toggle as the 
tile depth is increased or decreased for a given root value.

\param	rootIndex	The root index to be tested.
\param	nDataDepth	The relative data resolution beneath the root.

return true if the data is contained otherwise false.
*/
bool PYXIcosMath::isDataContained(	const PYXIcosIndex& rootIndex, 
									int	nDataDepth	)
{
	// verify a valid PYXIS index
	if (!rootIndex.isNull())
	{
		// determine if the data depth is an even or odd number
		bool bContained = ((nDataDepth % 2) == 0);

		// Toggle the containment according to the starting type
		if (rootIndex.hasVertexChildren())
		{
			bContained = !bContained;
		}
			
		return bContained;
	}
	else
	{
		PYXTHROW(	PYXMathException,
					"Invalid root index: '" << rootIndex << "'."	);
	}
}

/*! 
Determine the valid neighbour indices for the passed index starting with the 
neighbour in knDirectionOne and moving in a counter clockwise direction.  If
the passed index is a pentagon the 5 valid neighbours will be saved and a null 
index will be placed in the gap position as a place holder.

\param pyxIndex			The index to find the neighbours of
\param pVecNeighbours	A pointer to a vector of PYXIcosIndex values (out).
*/
void PYXIcosMath::getNeighbours(
							const PYXIcosIndex& pyxIndex,	
							std::vector<PYXIcosIndex>* pVecNeighbours)
{
	if (pyxIndex.isNull())
	{
		PYXTHROW(PYXIndexException, "Null index.");
	}

	assert((pVecNeighbours != 0) && "Invalid argument.");
	pVecNeighbours->resize(Hexagon::knNumSides);
	std::vector<PYXIcosIndex>::iterator it = pVecNeighbours->begin();

	// sequentially find the neighbours in a counter clockwise direction
	int nDirection = PYXMath::knDirectionOne;
	while (nDirection <= PYXMath::knDirectionSix)
	{
		*it = PYXIcosMath::move(
						pyxIndex, 
						static_cast<PYXMath::eHexDirection> (nDirection)	);
		++it;
		++nDirection;
	}
}

/*! 
Determine the direction from an index to a neighbouring index. Performing a
move from the first index in the returned direction will yield the second
index (if it is a neighbour).

Note that due to tesselation direction changes, the inverse operation's results
are unpredictable in general. For example, moving from the second to first
index could yield the same direction as moving from the first to second index.

\param indexFrom	The index from which to find the direction.
\param indexTo		The neighbouring index.
\return The direction from the first index to the second index, or direction
	zero if the indices are not neighbours.
*/
PYXMath::eHexDirection PYXIcosMath::getNeighbourDirection(
							const PYXIcosIndex& indexFrom,	
							const PYXIcosIndex& indexTo	)
{
	for (PYXMath::eHexDirection nDir = PYXMath::knDirectionOne;
		nDir <= PYXMath::knDirectionSix;
		nDir = static_cast<PYXMath::eHexDirection>(nDir + 1))
	{
		if (PYXIcosMath::move(indexFrom, nDir) == indexTo)
		{
			return nDir;
		}
	}

	return PYXMath::knDirectionZero;
}

/*!
Is the resolution valid.

\param nResolution The resolution to check.

\return true if valid, false otherwise.
*/
bool PYXIcosMath::isValidResolution(int nResolution)
{
	if (nResolution < PYXIcosIndex::knMinSubRes) 
	{
		return false;
	}

	return (nResolution < PYXMath::knMaxAbsResolution);
}





//! the cache itself.
std::vector<double> PYXIcosMath::UnitSphere::s_radius;



//! Calculate the circumradius for a given resolution (0 based)
double PYXIcosMath::UnitSphere::calcCellCircumRadius(int nResolution)
{
	return s_radius[nResolution];
}

//! Calculate the circumradius for a given cell
double PYXIcosMath::UnitSphere::calcCellCircumRadius(const PYXIcosIndex & index)
{
	return s_radius[index.getResolution()];
}

//! Calculate the circumradius for a inifinte tile with a root index
double PYXIcosMath::UnitSphere::calcTileCircumRadius(const PYXIcosIndex & index)
{
	if (index.hasVertexChildren())
	{
		//add a tile factor - cell radius of the parent resolution
		return s_radius[index.getResolution()-1];
	} 
	else 
	{
		return s_radius[index.getResolution()];
	}
}

void PYXIcosMath::UnitSphere::initialize()
{
	//! Epsilon used for floating-point comparisons.
	const double kfEps = DBL_EPSILON;

	//! Expansion factor for cell radius (to account for distortion due to Snyder projection).
	const double kfExp = 1.13;

	s_radius.resize(PYXMath::knMaxAbsResolution);

	// Cell radius for each resolution (including epsilon).
	// (we start from knMinSubRes-1 to be able to calculate calcTileCircumRadius for knMinSubRes resolution.)
	for (int nRes = PYXIcosIndex::knMinSubRes-2; nRes < PYXMath::knMaxAbsResolution; ++nRes)
	{
		s_radius[nRes] = kfExp * PYXMath::calcCircumRadius(nRes) * Icosahedron::kfCentralAngle + kfEps;
	}
}

void PYXIcosMath::UnitSphere::uninitialize()
{
}