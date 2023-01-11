/******************************************************************************
polygon.cpp

begin		: 2006-01-09
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/polygon.h"

// pyxlib includes
#include "pyxis/geometry/exceptions.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cfloat>

namespace
{

//! Epsilon used for floating-point comparisons.
const double kfEps = DBL_EPSILON;

//! Expansion factor for cell radius (to account for distortion due to Snyder projection).
const double kfExp = 1.13;

//! Cell radius, per resolution, major (has vertex children) then minor (no vertex children).
double arrayCR[40/*PYXMath::knMaxAbsResolution*/][2];

//! Vector of resolution 2 indices with which to begin traversal.
std::vector<PYXIcosIndex> vecRes2;

/*!
Convert a PYXIS index to a 3D point on the reference sphere.

\param index	The index to convert.
\return			The converted point.
*/
inline
PYXCoord3DDouble convertIndexTo3D(const PYXIcosIndex& index)
{
	CoordLatLon ll;
	SnyderProjection::getInstance()->pyxisToNative(index, &ll);
	return SphereMath::llxyz(ll);
}

/*!
Second form of convertIndexTo3D which returns the converted point
by setting the value into a passed parameter.

Convert a PYXIS index to a 3D point on the reference sphere.

\param index	The index to convert.
\param result	The converted point.
*/

inline
void convertIndexTo3D(const PYXIcosIndex& index, PYXCoord3DDouble* result)
{
	CoordLatLon ll;
	SnyderProjection::getInstance()->pyxisToNative(index, &ll);
	SphereMath::llxyz(ll, result);
}
}

//! Tester class
Tester<PYXPolygon> gTester;

//! Test method
void PYXPolygon::test()
{
	// Test results against which to compare.
	// 2-20 and 2-60 are included due to a "shadow" effect with the crossing test.
	// A-0 is broken up because the traversal is conservative in its checks.
	std::vector<PYXIcosIndex> vec;
	vec.push_back(PYXIcosIndex("2-20"));
	vec.push_back(PYXIcosIndex("2-60"));
	vec.push_back(PYXIcosIndex("A-00"));
	vec.push_back(PYXIcosIndex("A-01"));
	vec.push_back(PYXIcosIndex("A-02"));
	vec.push_back(PYXIcosIndex("A-03"));
	vec.push_back(PYXIcosIndex("A-04"));
	vec.push_back(PYXIcosIndex("A-05"));
	vec.push_back(PYXIcosIndex("A-06"));

	// Spherical polygon around hex A.
	PYXPolygon poly;
	poly.addVertex(PYXIcosIndex("1-30"));
	poly.addVertex(PYXIcosIndex("3-20"));
	poly.addVertex(PYXIcosIndex("3-30"));
	poly.addVertex(PYXIcosIndex("2-60"));
	poly.addVertex(PYXIcosIndex("2-20"));
	poly.addVertex(PYXIcosIndex("1-20"));
	poly.closeRing();

	// TO DO: Figure out why the loop below isn't being entered.
	int n = 0;
	for (PYXPointer<PYXIterator> spIt(poly.getIterator()); !spIt->end(); spIt->next(), ++n)
	{
		const PYXIcosIndex& cellIndex = spIt->getIndex();
		TEST_ASSERT(cellIndex == vec[n]);

		PYXCell cell(cellIndex);
		TEST_ASSERT(poly.intersects(cell));
		try
		{
			PYXPointer<PYXGeometry> spIntersection = poly.intersection(cell);

			TEST_ASSERT(cellIndex == dynamic_cast<PYXCell&>(*spIntersection).getIndex());
		}
		catch (...)
		{
			TEST_ASSERT(false && "Incorrect type.");
		}
	}

	// Test union with a tile collection.
	{
		PYXTileCollection tc1;
		tc1.addTile(PYXIcosIndex("C-04"), 3);
		tc1.addTile(PYXIcosIndex("C-03"), 3);
		{
			PYXTileCollection tc2;
			poly.copyTo(&tc2);
			TEST_ASSERT(3 == tc2.getGeometryCount());
		}
		{
			PYXPointer<PYXGeometry> spResult = tc1.disjunction(poly);
			TEST_ASSERT(0 != spResult);
			try
			{
				PYXTileCollection& tcResult = dynamic_cast<PYXTileCollection&>(*spResult);

				TEST_ASSERT(tcResult.getGeometryCount() == 5);
				TEST_ASSERT(tcResult.intersects(tc1));
				TEST_ASSERT(tcResult.intersects(poly));
			}
			catch (...)
			{
				TEST_ASSERT(0 && "Should have been a tile collection.");
			}
		}
		{
			PYXPointer<PYXGeometry> spResult = poly.disjunction(tc1);
			TEST_ASSERT(0 != spResult);
			try
			{
				PYXTileCollection& tcResult = dynamic_cast<PYXTileCollection&>(*spResult);

				TEST_ASSERT(tcResult.getGeometryCount() == 5);
				TEST_ASSERT(tcResult.intersects(tc1));
				TEST_ASSERT(tcResult.intersects(poly));
			}
			catch (...)
			{
				TEST_ASSERT(0 && "Should have been a tile collection.");
			}
		}
	}

	// TO DO: add verified test case for interior rings.
}

/*!
Module set-up.
*/
void PYXPolygon::initStaticData()
{
	// Can't use PYXMath::knMaxAbsResolution in array declaration so verify size here.
	assert(sizeof(arrayCR)/sizeof(arrayCR[0]) == PYXMath::knMaxAbsResolution);

	// Cell radius for each resolution (including epsilon).
	for (int nRes = PYXIcosIndex::knMinSubRes; nRes != PYXMath::knMaxAbsResolution; ++nRes)
	{
		arrayCR[nRes][0] = kfExp * PYXMath::calcCircumRadius(nRes - 1) * Icosahedron::kfCentralAngle + kfEps;
		arrayCR[nRes][1] = kfExp * PYXMath::calcCircumRadius(nRes) * Icosahedron::kfCentralAngle + kfEps;
	}

	// Initialize vecRes2.
	{
		PYXIcosIterator it(2);
		for (; !it.end(); it.next())
		{
			vecRes2.push_back(it.getIndex());
		}
	}
}

/*!
Module tear-down.
*/
void PYXPolygon::freeStaticData()
{
}

/*!
Equality operator.

\param	rhs	The polygon to compare with this one.

\return	true if the two polygons are equal, otherwise false.
*/
bool PYXPolygon::operator ==(const PYXPolygon& rhs) const
{
	if (m_vecVertex.size() != rhs.m_vecVertex.size())
	{
		return false;
	}

	IndexVector::const_iterator itLeft = m_vecVertex.begin();
	IndexVector::const_iterator itRight = rhs.m_vecVertex.begin();
	for (; itLeft != m_vecVertex.end(); ++itLeft, ++itRight)
	{
		if (*itLeft != *itRight)
		{
			return false;
		}
	}

	return true;
}

/*!
Less than operator. The polygon with fewer nodes sorts first, otherwise the
nodes are compared in order until one node is less that the other.

\param	rhs	The polygon to compare with this one.

\return	true If lhs < rhs
*/
bool PYXPolygon::operator <(const PYXPolygon& rhs) const
{
	if (m_vecVertex.size() != rhs.m_vecVertex.size())
	{
		return (m_vecVertex.size() < rhs.m_vecVertex.size());
	}

	IndexVector::const_iterator itLeft = m_vecVertex.begin();
	IndexVector::const_iterator itRight = rhs.m_vecVertex.begin();
	for (; itLeft != m_vecVertex.end(); ++itLeft, ++itRight)
	{
		if (*itLeft != *itRight)
		{
			return (*itLeft < *itRight);
		}
	}

	return false;
}

/*!
Get the intersection of this geometry and the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return	The intersection.
*/
PYXPointer<PYXGeometry> PYXPolygon::intersection(	const PYXGeometry& geometry,
													bool bCommutative	) const
{
	const PYXTileCollection* const pTileCollection = 
		dynamic_cast<const PYXTileCollection*>(&geometry);
	if (pTileCollection)
	{
		return intersection(*pTileCollection);
	}

	const PYXTile* const pTile = dynamic_cast<const PYXTile*>(&geometry);
	if (pTile)
	{
		return intersection(*pTile);
	}

	const PYXCell* const pCell = dynamic_cast<const PYXCell*>(&geometry);
	if (pCell)
	{
		return intersection(*pCell);
	}

	if (!m_spGeometry)
	{
		createGeometry();
	}
	return m_spGeometry->intersection(geometry);
}

PYXPointer<PYXGeometry> PYXPolygon::intersection(const PYXTileCollection& collection) const
{
	std::vector<PYXIcosIndex> vec;
	for (PYXPointer<PYXTileCollectionIterator> spIt = collection.getTileIterator();
		!spIt->end(); spIt->next())
	{
		vec.push_back(spIt->getTile()->getRootIndex());
	}
	return intersectionImpl(vec, collection.getCellResolution());
}

PYXPointer<PYXGeometry> PYXPolygon::intersection(const PYXTile& tile) const
{
	std::vector<PYXIcosIndex> vec(1, tile.getRootIndex());
	return intersectionImpl(vec, tile.getCellResolution());
}

PYXPointer<PYXGeometry> PYXPolygon::intersection(const PYXCell& cell) const
{
	PYXPointer<PYXGeometry> spGeom;

	createIntermediateDataStructures();

	if (classifyCellToPolygon(cell.getIndex()) == knOutside)
	{
		spGeom = PYXEmptyGeometry::create();
	}
	else
	{
		spGeom = PYXCell::create(cell);
	}

	createIntermediateDataStructures();

	return spGeom;
}

PYXPointer<PYXGeometry> PYXPolygon::boundaryIntersection(const PYXTile& tile) const
{
	std::vector<PYXIcosIndex> vec(1, tile.getRootIndex());
	return boundaryIntersectionImpl(vec, tile.getCellResolution());
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXPolygon::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	// Note: If input geometry is a cell, we could just use
	// classifyCellToPolygon() or classifyPointToPolygon() directly.
	if (!m_spGeometry)
	{
		createGeometry();
	}
	return m_spGeometry->intersects(geometry);
}

/*!
Get the cell resolution.

\return	The cell resolution.
*/
int PYXPolygon::getCellResolution() const
{
	int nResolution = 0;

	// get the resolution of the first node
	if (!m_vecVertex.empty())
	{
		nResolution = m_vecVertex[0].getResolution();
	}

	return nResolution;
}

/*!
Set the PYXIS resolution of cells in the geometry. This method changes the
resolution of each of the nodes in the line.

\param	nCellResolution	The cell resolution.
*/
void PYXPolygon::setCellResolution(int nCellResolution)
{
	assert(0 <= nCellResolution && nCellResolution < PYXMath::knMaxAbsResolution);

	int nIndex = 0;

	IndexVector::iterator it = m_vecVertex.begin();
	for (; it != m_vecVertex.end(); ++it)
	{
		if (nIndex == 0)
		{
			m_vecVertex[nIndex++].setResolution(nCellResolution);
		}
		else
		{
			it->setResolution(nCellResolution);

			// don't permit duplicate nodes
			if (*it != m_vecVertex[nIndex - 1])
			{
				m_vecVertex[nIndex++] = *it;
			}
		}
	}

	m_vecVertex.resize(nIndex);
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXPolygon::getIterator() const
{
	if (!m_spGeometry)
	{
		createGeometry();
	}
	return m_spGeometry->getIterator();
}

/*!
Create a copy of the geometry.

\return	A copy of the geometry.
*/
//! Create a copy of the geometry.
PYXPointer<PYXGeometry> PYXPolygon::clone() const
{
	return create(*this);
}

/*!
Clears the geometry so it is empty.
*/
void PYXPolygon::clear()
{
	// Free geometry.
	m_spGeometry = 0;
	// Clear vertices.
	m_vecVertex.clear();
	// Clear rings.
	m_vecRing.clear();
	// Clear exterior point.
	m_w = PYXCoord3DDouble(0, 0, 0);
}

/*!
Add a vertex to the polygon.

\param	index	The vertex.
*/
void PYXPolygon::addVertex(const PYXIcosIndex& index)
{
	if (index.isNull())
	{
		PYXTHROW(PYXGeometryException, "Null index.");
	}

	assert(0 <= index.getResolution() && index.getResolution() < PYXMath::knMaxAbsResolution);

	// don't store consecutive repeated indices
	if (!m_vecVertex.empty() && index == m_vecVertex.back())
	{
		return;
	}

	m_vecVertex.push_back(index);
}

/*!
Closes the current ring.
*/
void PYXPolygon::closeRing() const
{
	// Beginning of this ring is end of last ring.
	int nRingBegin = m_vecRing.empty() ? 0 : m_vecRing.back();

	int nRingEnd = static_cast<int>(m_vecVertex.size());

	// Don't repeat first vertex as last.
	if (1 < (nRingEnd - nRingBegin)
		&& m_vecVertex[nRingBegin] == m_vecVertex.back())
	{
		m_vecVertex.pop_back();
		--nRingEnd;
	}

	m_vecRing.push_back(nRingEnd);
}

/*!
Sets the exterior point.

\param index	The index of a point exterior to the spherical polygon.
*/
void PYXPolygon::setExteriorPoint(const PYXIcosIndex& index)
{
	 convertIndexTo3D(index, &m_w);
}

/*!
Copies a representation of this geometry into a tile collection.

\param pTileCollection	The tile collection.
*/
inline void PYXPolygon::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, getCellResolution());
}

/*!
Copies a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The tile collection.
\param	nTargetResolution	The target resolution.
*/
inline void PYXPolygon::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	assert(pTileCollection != 0);

	pTileCollection->clear();

	if (!m_spGeometry)
	{
		createGeometry();
	}

	m_spGeometry->copyTo(pTileCollection, nTargetResolution);
}

/*!
Classify a cell with regard to this spherical polygon.

\param index	The index of the cell to classify.
\return knOutside if the cell and all its children are outside;
        knInside if the cell and all its children are inside;
        knBoundary if the cell or its children possibly straddle the boundary.
*/
PYXPolygon::eClass PYXPolygon::classifyCellToPolygon(const PYXIcosIndex& index) const
{
	// Ray from t (test point) to w (exterior point).
	PYXCoord3DDouble t = convertIndexTo3D(index);
	PYXCoord3DDouble rayNormal = t.cross(m_w);
	rayNormal.normalize();

	// Cell radius for distance comparisons.
	const double kfCR = arrayCR[index.getResolution()][index.hasVertexChildren() ? 0 : 1];

	// Count the number of line segments crossed by the ray.
	int nCross = 0;

	for (int nLine = 0; nLine != m_vecLine.size(); ++nLine)
	{
		// Line from u to v.
		Line& line = m_vecLine[nLine];
		const PYXCoord3DDouble& u = m_vecPoint[line.m_nU];
		const PYXCoord3DDouble& v = m_vecPoint[line.m_nV];

		// Distance to line (project to line normal).
		const double kfLineDt = line.m_normal.dot(t);

		// Check for point/line straddling.
		if (-kfCR <= kfLineDt && kfLineDt <= kfCR)
		{
			// Test point is close to line.

			// Distance to u.
			const double kfDirDt = line.m_dir.dot(t);

			if (-kfCR <= kfDirDt && kfDirDt <= line.m_fDirDv + kfCR)
			{
				// Test point is close to line segment.

				// Regardless of number of crossings, the cell is near the boundary.
				return knBoundary;
			}
		}

		// Distance to line (project to line normal).
		const double kfLineDw = line.m_fLineDw;

		// Check for ray/line crossing.
		if (kfLineDt < -kfEps && kfEps < kfLineDw)
		{
			// Ray crosses line forwards (t back to w front).
			// Note: If a vertex is within epsilon of the ray, it is
			// considered to be infinitessimally in front of the ray.
			if (rayNormal.dot(v) < -kfEps && -kfEps <= rayNormal.dot(u))
			{
				// Line crosses ray backwards (u front to v back).
				// 
				//        u
				//
				//        ^........ray normal
				//        |
				//        |    ....line normal
				// t -----+----> w
				//        |
				//        |
				//
				//        v
				// 
				++nCross;
			}
		}
		else if (kfLineDw < -kfEps && kfEps < kfLineDt)
		{
			// Ray crosses line backwards (t front to w back).
			// Note: If a vertex is within epsilon of the ray, it is
			// considered to be infinitessimally in front of the ray.
			if (rayNormal.dot(u) < -kfEps && -kfEps <= rayNormal.dot(v))
			{
				// Line crosses ray forwards (u back to v front).
				// 
				//        u
				//
				//        |
				//        |    ....line normal
				// w -----+----> t
				//        |
				//        |
				//        v........ray normal
				//
				//        v
				// 
				++nCross;
			}
		}
	}

	// Point is in polygon if number of crossings is odd.
	return nCross % 2 != 0 ? knInside : knOutside;
}

/*!
Classify a point with regard to this spherical polygon.

\param index	The index of the point to classify.
\return knOutside if the point is outside;
        knInside if the point is inside.
*/
PYXPolygon::eClass PYXPolygon::classifyPointToPolygon(const PYXIcosIndex& index) const
{
	// Ray from t (test point) to w (exterior point).
	PYXCoord3DDouble t = convertIndexTo3D(index);
	PYXCoord3DDouble rayNormal = t.cross(m_w);
	rayNormal.normalize(); // not comparing distances, but just to be safe

	// Count the number of line segments crossed by the ray.
	int nCross = 0;

	for (int nLine = 0; nLine != m_vecLine.size(); ++nLine)
	{
		// Line from u to v.
		Line& line = m_vecLine[nLine];
		const PYXCoord3DDouble& u = m_vecPoint[line.m_nU];
		const PYXCoord3DDouble& v = m_vecPoint[line.m_nV];

		// Distances to line (project to line normal).
		const double kfLineDt = line.m_normal.dot(t);
		const double kfLineDw = line.m_fLineDw;

		// Check for ray/line crossing.
		if (kfLineDt < -kfEps && kfEps < kfLineDw)
		{
			// Ray crosses line forwards (t back to w front).
			// Note: If a vertex is within epsilon of the ray, it is
			// considered to be infinitessimally in front of the ray.
			if (rayNormal.dot(v) < -kfEps && -kfEps <= rayNormal.dot(u))
			{
				// Line crosses ray backwards (u front to v back).
				// 
				//        u
				//
				//        ^........ray normal
				//        |
				//        |    ....line normal
				// t -----+----> w
				//        |
				//        |
				//
				//        v
				// 
				++nCross;
			}
		}
		else if (kfLineDw < -kfEps && kfEps < kfLineDt)
		{
			// Ray crosses line backwards (t front to w back).
			// Note: If a vertex is within epsilon of the ray, it is
			// considered to be infinitessimally in front of the ray.
			if (rayNormal.dot(u) < -kfEps && -kfEps <= rayNormal.dot(v))
			{
				// Line crosses ray forwards (u back to v front).
				// 
				//        u
				//
				//        |
				//        |    ....line normal
				// w -----+----> t
				//        |
				//        |
				//        v........ray normal
				//
				//        v
				// 
				++nCross;
			}
		}
	}

	// Point is in polygon if number of crossings is odd.
	return nCross % 2 != 0 ? knInside : knOutside;
}

/*!
Classify a cell with regard to this spherical polygon.

\param index	The index of the cell to classify.
\return knOutside if the cell and all its children are outside;
        knInside if the cell and all its children are inside;
        knBoundary if the cell or its children possibly straddle the boundary.
*/
PYXPolygon::eClass PYXPolygon::classifyCellToPolygonBoundary(const PYXIcosIndex& index) const
{
	// Ray from t (test point) to w (exterior point).
	PYXCoord3DDouble t = convertIndexTo3D(index);
	PYXCoord3DDouble rayNormal = t.cross(m_w);
	rayNormal.normalize();

	// Cell radius for distance comparisons.
	const double kfCR = arrayCR[index.getResolution()][1];

	// Count the number of line segments crossed by the ray.
	int nCross = 0;

	bool bBoundary = false;

	for (int nLine = 0; nLine != m_vecLine.size(); ++nLine)
	{
		// Line from u to v.
		Line& line = m_vecLine[nLine];
		const PYXCoord3DDouble& u = m_vecPoint[line.m_nU];
		const PYXCoord3DDouble& v = m_vecPoint[line.m_nV];

		// Distance to line (project to line normal).
		const double kfLineDt = line.m_normal.dot(t);

		// Check for point/line straddling.
		if (-kfCR <= kfLineDt && kfLineDt <= kfCR)
		{
			// Test point is close to line.

			// Distance to u.
			const double kfDirDt = line.m_dir.dot(t);

			if (-kfCR <= kfDirDt && kfDirDt <= line.m_fDirDv + kfCR)
			{
				// Test point is close to line segment.

				// Regardless of number of crossings, the cell is near the boundary.
				bBoundary = true;
			}
		}

		// Distance to line (project to line normal).
		const double kfLineDw = line.m_fLineDw;

		// Check for ray/line crossing.
		if (kfLineDt < -kfEps && kfEps < kfLineDw)
		{
			// Ray crosses line forwards (t back to w front).
			// Note: If a vertex is within epsilon of the ray, it is
			// considered to be infinitessimally in front of the ray.
			if (rayNormal.dot(v) < -kfEps && -kfEps <= rayNormal.dot(u))
			{
				// Line crosses ray backwards (u front to v back).
				// 
				//        u
				//
				//        ^........ray normal
				//        |
				//        |    ....line normal
				// t -----+----> w
				//        |
				//        |
				//
				//        v
				// 
				++nCross;
			}
		}
		else if (kfLineDw < -kfEps && kfEps < kfLineDt)
		{
			// Ray crosses line backwards (t front to w back).
			// Note: If a vertex is within epsilon of the ray, it is
			// considered to be infinitessimally in front of the ray.
			if (rayNormal.dot(u) < -kfEps && -kfEps <= rayNormal.dot(v))
			{
				// Line crosses ray forwards (u back to v front).
				// 
				//        u
				//
				//        |
				//        |    ....line normal
				// w -----+----> t
				//        |
				//        |
				//        v........ray normal
				//
				//        v
				// 
				++nCross;
			}
		}
	}

	// Point is in polygon if number of crossings is odd.
	return nCross % 2 != 0 ? knBoundary : knOutside;
}

/*!
Pre-processes the linear rings into intermediate data structures.
*/
void PYXPolygon::createIntermediateDataStructures() const
{
	// Simple test to see if we don't need to do this.
	if (!m_vecLine.empty())
	{
		return;
	}

	// Convert each vertex to a 3D point on the reference sphere.
	m_vecPoint.reserve(m_vecVertex.size());
	PYXCoord3DDouble result;
	for (int n = 0; n != m_vecVertex.size(); ++n)
	{
		convertIndexTo3D(m_vecVertex[n], &result);
		m_vecPoint.push_back(result);
	}

	if (m_w == PYXCoord3DDouble(0, 0, 0) && 2 <= m_vecPoint.size())
	{
		// Assume normal of first line segment is exterior to the polygon.
		m_w = m_vecPoint[0].cross(m_vecPoint[1]);
		m_w.normalize();
	}

	// Create a line (halfspace) for each vertex.
	m_vecLine.reserve(m_vecVertex.size());

	// Ring line segments.
	{
		int nRingBegin = 0;
		int nRingEnd;
		for (int nRing = 0; nRing != m_vecRing.size(); ++nRing, nRingBegin = nRingEnd)
		{
			nRingEnd = m_vecRing[nRing];

			if ((nRingEnd - nRingBegin) < 3)
			{
				// Too few vertices in ring, so abort this ring.
				continue;
			}

			for (int n = nRingBegin; n != nRingEnd; ++n)
			{
				// Line from this vertex (u) to the next vertex (v) along a ring.
				int nU = n;
				int nV = (n + 1) == nRingEnd ? nRingBegin : (n + 1);

#if 0
				// Note: If interior rings were ordered backwards, we could simply
				// swap u and v here to properly get an outward-facing normal.
				if (nRing != 0)
				{
					std::swap(nU, nV);
				}
#endif

				// Line from u to v.
				const PYXCoord3DDouble& u = m_vecPoint[nU];
				const PYXCoord3DDouble& v = m_vecPoint[nV];
				assert(!(u == v) && "Can't handle consecutive identical points.");

				// Get outward-facing normal to line.
				PYXCoord3DDouble normal = u.cross(v);
				normal.normalize();

				// Get vector in direction of line.
				PYXCoord3DDouble dir = normal.cross(u);
				dir.normalize();

				// Distance to u (project to dir).
				const double kfDirDv = dir.dot(v);

				// Distance to line (project to line normal).
				const double kfLineDw = normal.dot(m_w);

				// Store information for line segment.
				m_vecLine.push_back(Line(nU, nV, kfDirDv, kfLineDw, normal, dir));
			}
		}
	}
}

/*!
Cleans up the intermediate data structures.
*/
void PYXPolygon::destroyIntermediateDataStructures() const
{
	m_vecPoint.swap(std::vector<PYXCoord3DDouble>());
	m_vecLine.swap(std::vector<PYXPolygon::Line>());
}

/*!
Traverse the intermediate data structures using 3D math.
*/
PYXPointer<PYXTileCollection> PYXPolygon::intersectionImpl(std::vector<PYXIcosIndex>& vec, int nTargetResolution) const
{
	assert(!m_vecVertex.empty());

	PYXPointer<PYXTileCollection> spTC(PYXTileCollection::create());

	if ((m_vecRing.empty() ? m_vecVertex.size() : m_vecRing[0]) < 3)
	{
		// Too few vertices in exterior ring, so abort this polygon.
		return spTC;
	}

	createIntermediateDataStructures();

	// Traversal.
	while (!vec.empty())
	{
		PYXIcosIndex& index = vec.back();
		if (index.getResolution() != nTargetResolution)
		{
			// Classify cells at non-data resolution.
			eClass nClass = classifyCellToPolygon(index);
			if (nClass == knOutside)
			{
				vec.pop_back();
			}
			else if (nClass == knInside)
			{
				spTC->addTile(index, nTargetResolution);
				vec.pop_back();
			}
			else
			{
				PYXChildIterator it(index);
				vec.pop_back();
				for (; !it.end(); it.next())
				{
					vec.push_back(it.getIndex());
				}
			}
		}
		else
		{
			// Classify points at data resolution.
			if (classifyPointToPolygon(index) == knInside)
			{
				spTC->addTile(index, nTargetResolution);
			}
			vec.pop_back();
		}
	}

	destroyIntermediateDataStructures();

	return spTC;
}

/*!
Traverse the intermediate data structures using 3D math.
*/
PYXPointer<PYXTileCollection> PYXPolygon::boundaryIntersectionImpl(std::vector<PYXIcosIndex>& vec, int nTargetResolution) const
{
	assert(!m_vecVertex.empty());

	PYXPointer<PYXTileCollection> spTC(PYXTileCollection::create());

	if ((m_vecRing.empty() ? m_vecVertex.size() : m_vecRing[0]) < 3)
	{
		// Too few vertices in exterior ring, so abort this polygon.
		return spTC;
	}

	createIntermediateDataStructures();

	// Traversal.
	while (!vec.empty())
	{
		PYXIcosIndex& index = vec.back();
		if (index.getResolution() != nTargetResolution)
		{
			// Classify cells at non-data resolution.
			eClass nClass = classifyCellToPolygon(index);
			if (nClass == knBoundary)
			{
				PYXChildIterator it(index);
				vec.pop_back();
				for (; !it.end(); it.next())
				{
					vec.push_back(it.getIndex());
				}
			}
			else
			{
				vec.pop_back();
			}
		}
		else
		{
			// TODO 2008-02-05 mlepage should be cell (not tile or point)
			// as boundary but need to reconcile with interior and exterior

			// Classify points at data resolution.
			if (classifyCellToPolygonBoundary(index) == knBoundary)
			{
				spTC->addTile(index, nTargetResolution);
			}
			vec.pop_back();
		}
	}

	destroyIntermediateDataStructures();

	return spTC;
}

/*!
Create the geometry for this spherical polygon.

Uses 3D math to traverse and construct a tile collection from the intermediate
data structures.
*/
void PYXPolygon::createGeometry() const
{
	m_spGeometry = intersectionImpl(
		std::vector<PYXIcosIndex>(vecRes2),
		m_vecVertex.front().getResolution());
}
