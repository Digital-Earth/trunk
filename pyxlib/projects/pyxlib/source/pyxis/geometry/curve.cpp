/******************************************************************************
curve.cpp

begin		: 2005-01-12
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/curve.h"

// Define new curve method to use recursive great circle arc rendering for line.  Otherwise use original method.
//#define NEW_CURVE_METHOD

// pyxlib includes
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/exceptions.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/great_circle_arc.h"
#include "pyxis/utility/tester.h"
#include "pyxis/geometry/combined_index.h"

#ifdef NEW_CURVE_METHOD
#	include "pyxis/utility/sphere_math.h"
#endif

// standard includes
#include <algorithm>

#ifdef NEW_CURVE_METHOD

void recursiveGCArc(std::vector<PYXIcosIndex>& vecIndex,
					const PYXIcosIndex& Apyx,
					const PYXCoord3DDouble& Axyz,
					const PYXIcosIndex& Bpyx,
					const PYXCoord3DDouble& Bxyz)
{
	// If cells are the same - we are done.  Exit.
	if (Apyx == Bpyx)
	{
		return;
	}

	// If cells are neighbours - we are done.  Exit.
	std::vector<PYXIcosIndex> vec;
	PYXIcosMath::getNeighbours(Apyx, &vec);

	if (std::find(vec.begin(), vec.end(), Bpyx) != vec.end())
	{
		return;
	}

	PYXCoord3DDouble Cxyz;
	Cxyz.setX(Axyz.x() + Bxyz.x());
	Cxyz.setY(Axyz.y() + Bxyz.y());
	Cxyz.setZ(Axyz.z() + Bxyz.z());
	Cxyz.normalize();

	CoordLatLon Cll = SphereMath::xyzll(Cxyz);

	int nResolution = Apyx.getResolution();

	PYXIcosIndex Cpyx;

	SnyderProjection::getInstance()->nativeToPYXIS(Cll, &Cpyx, nResolution);

	vecIndex.push_back(Cpyx);

	recursiveGCArc(vecIndex, Apyx, Axyz, Cpyx, Cxyz);
	recursiveGCArc(vecIndex, Cpyx, Cxyz, Bpyx, Bxyz);
}

void GCArc(std::vector<PYXIcosIndex>& vecIndex, const PYXIcosIndex& Apyx, const PYXIcosIndex& Bpyx)
{
	CoordLatLon All;
	SnyderProjection::getInstance()->pyxisToNative(Apyx, &All);
	PYXCoord3DDouble Axyz = SphereMath::llxyz(All);

	Axyz.normalize();

	CoordLatLon Bll;
	SnyderProjection::getInstance()->pyxisToNative(Bpyx, &Bll);
	PYXCoord3DDouble Bxyz = SphereMath::llxyz(Bll);

	Bxyz.normalize();

	if (Apyx == Bpyx)
	{
		vecIndex.push_back(Apyx);
	}
	else
	{
		vecIndex.push_back(Apyx);
		vecIndex.push_back(Bpyx);

		recursiveGCArc(vecIndex, Apyx, Axyz, Bpyx, Bxyz);
	}
}

#endif

//! Tester class
Tester<PYXCurve> gTester;

//! Deserialization Constructor.
PYXCurve::PYXCurve(std::basic_istream< char>& in)
{
	deserialize(in);
}

//! Test method
void PYXCurve::test()
{
	{
		// Test: empty curve.
		PYXCurve curve;
		PYXPointer<PYXIterator> spIt(curve.getIterator());
		TEST_ASSERT(spIt->end());
	}

	{
		// Explicitly test this pentagon.
		PYXIcosIndex indexStart("1-0");
		CoordLatLon coordStart;
		SnyderProjection::getInstance()->pyxisToNative(indexStart, &coordStart);

		// Test: pentagon point.
		testPoint(coordStart);
		testLine(coordStart, coordStart, true);
	}

#if this_is_a_slow_section
	{
		// Explicitly test this line across a pentagon.
		PYXIcosIndex indexStart("1-2");
		CoordLatLon coordStart;
		SnyderProjection::getInstance()->pyxisToNative(indexStart, &coordStart);

		PYXIcosIndex indexDest("1-4");
		CoordLatLon coordDest;
		SnyderProjection::getInstance()->pyxisToNative(indexDest, &coordDest);

		// Test: curve across pentagon.
		testLine(coordStart, coordDest, true);
	}
#endif

#if 0 // TODO mlepage disabled this on 2008-03-28 until the tests can be fixed
	{
		// Explicitly test this line across a gap.
		PYXIcosIndex indexStart("12-030601");
		CoordLatLon coordStart;
		SnyderProjection::getInstance()->pyxisToNative(indexStart, &coordStart);

		PYXIcosIndex indexDest("12-050201");
		CoordLatLon coordDest;
		SnyderProjection::getInstance()->pyxisToNative(indexDest, &coordDest);

		// Test: curve across gap.
		testLine(coordStart, coordDest, true);
	}

	// Random testing.
	bool bDetailed = true;
	for (int nStart = 0; nStart != 11; ++nStart)
	{
		CoordLatLon coordStart;
		coordStart.randomize();

		// Test: single point.
		testPoint(coordStart);
		// Test: line with one point.
		testLine(coordStart, coordStart, bDetailed);

		for (int nDest = 0; nDest != 1; ++nDest)
		{
			{
				// Test: short line with one segment.
				CoordLatLon coordDest(coordStart);
				coordDest.perturb(1);
				testLine(coordStart, coordDest, bDetailed);
			}

			{
				// Test: long line with one segment.
				CoordLatLon coordDest(coordStart);
				coordDest.perturb(15);
				testLine(coordStart, coordDest, bDetailed);
			}
		}
		// No need to perform detailed test on all cases
		bDetailed = false;
	}
#endif
}

//! Test method
bool PYXCurve::testIsNeighbour(const PYXIcosIndex& index1, const PYXIcosIndex& index2)
{
	std::vector<PYXIcosIndex> vecIndex;
	PYXIcosMath::getNeighbours(index1, &vecIndex);
	return std::find(vecIndex.begin(), vecIndex.end(), index2) != vecIndex.end();
}

//! Test method
void PYXCurve::testLine(const CoordLatLon& coordStart, const CoordLatLon& coordDest, bool bDetailed)
{
	for (	int nResolution = 7 /*PYXIcosIndex::knMinSubRes*/;
			nResolution != 11 /*PYXMath::knMaxAbsResolution*/;
			++nResolution	)
	{
		PYXCurve curve;

		PYXIcosIndex indexStart;
		SnyderProjection::getInstance()->nativeToPYXIS(	coordStart, 
														&indexStart, 
														nResolution	);
		curve.addNode(indexStart);

		PYXIcosIndex indexDest;
		SnyderProjection::getInstance()->nativeToPYXIS(	coordDest, 
														&indexDest, 
														nResolution	);
		curve.addNode(indexDest);

		PYXPointer<PYXIterator> spIt(curve.getIterator());
		TEST_ASSERT(!spIt->end());

		PYXIcosIndex indexCurrent = spIt->getIndex();
		TEST_ASSERT(indexCurrent == indexStart);
		PYXIcosIndex indexPrev = indexCurrent;
		spIt->next();

		while (!spIt->end())
		{
			indexCurrent = spIt->getIndex();
			TEST_ASSERT(testIsNeighbour(indexPrev, indexCurrent));
			if (bDetailed)
			{
				PYXCell cell(indexCurrent);
				TEST_ASSERT(curve.intersects(cell));
				PYXPointer<PYXGeometry> spIntersection = curve.intersection(cell);
				try
				{
					const PYXCurve& curveIntersection = dynamic_cast<const PYXCurve&>(*spIntersection);
					PYXPointer<PYXIterator> spIt = curveIntersection.getIterator();
					TEST_ASSERT(!spIt->end());
					if (!spIt->end())
					{
						TEST_ASSERT(spIt->getIndex() == indexCurrent);
						spIt->next();
						TEST_ASSERT(spIt->end());
					}
				}
				catch (...)
				{
					TEST_ASSERT(false && "Incorrect type.");
				}
			}
			indexPrev = indexCurrent;
			spIt->next();
		}
		TEST_ASSERT(indexCurrent == indexDest);
	}
}

//! Test method
PYXCurve PYXCurve::testPoint(const CoordLatLon& coord)
{
	PYXCurve result;
	for (int nResolution = PYXIcosIndex::knMinSubRes; nResolution != PYXMath::knMaxAbsResolution; ++nResolution)
	{

		PYXIcosIndex index;
		SnyderProjection::getInstance()->nativeToPYXIS(	coord, 
														&index, 
														nResolution	);
		PYXCurve curve;
		curve.addNode(index);
		result.addNode(index);

		PYXPointer<PYXIterator> spIt(curve.getIterator());
		TEST_ASSERT(!spIt->end());
		const PYXIcosIndex& indexCurrent = spIt->getIndex();
		TEST_ASSERT(indexCurrent == index);
		spIt->next();
		TEST_ASSERT(spIt->end());
	}
	return result;
}

/*!
Equality operator.

\param	rhs	The curve to compare with this one.

\return	true if the two curves are equal, otherwise false.
*/
bool PYXCurve::operator ==(const PYXCurve& rhs) const
{
	if (m_vecNode.size() != rhs.m_vecNode.size())
	{
		return false;
	}

	IndexVector::const_iterator itLeft = m_vecNode.begin();
	IndexVector::const_iterator itRight = rhs.m_vecNode.begin();
	for (; itLeft != m_vecNode.end(); ++itLeft, ++itRight)
	{
		if (*itLeft != *itRight)
		{
			return false;
		}
	}

	return true;
}

/*!
Less than operator. The curve with fewer nodes sorts first, otherwise the nodes
are compared in order until one node is less that the other.

\param	rhs	The curve to compare with this one.

\return	true If lhs < rhs
*/
bool PYXCurve::operator <(const PYXCurve& rhs) const
{
	if (m_vecNode.size() != rhs.m_vecNode.size())
	{
		return (m_vecNode.size() < rhs.m_vecNode.size());
	}

	IndexVector::const_iterator itLeft = m_vecNode.begin();
	IndexVector::const_iterator itRight = rhs.m_vecNode.begin();
	for (; itLeft != m_vecNode.end(); ++itLeft, ++itRight)
	{
		if (*itLeft != *itRight)
		{
			return (*itLeft < *itRight);
		}
	}

	return false;
}

/*!
Create a copy of the geometry.

\return	A copy of the geometry.
*/
PYXPointer<PYXGeometry> PYXCurve::clone() const
{
	return create(*this);
}

/*!
Special function for intersection tests against cells.
\param vecPoint		Container of points, one to start each line segment, plus a final point to end the last line segment.
\param vecNormal	Container of normals, one for each line segment.
\param bExpTile		Whether to expand to next lower res to account for tiles with vertex children.
\return true if intersects, false otherwise.
*/
bool intersectsTest(const PYXIcosIndex& index,
	const std::vector<PYXCoord3DDouble>& vecPoint,
	const std::vector<PYXCoord3DDouble>& vecNormal,
	bool bExpTile = false)
{
	// TODO we could probably speed this up by using a vector for the line segment (A to B),
	// a vector for the normal, and a double telling us how far B is from A.

	// Convert the index to an XYZ coordinate.
	PYXCoord3DDouble coord;
	SnyderProjection::getInstance()->pyxisToXYZ(index, &coord);

	// Epsilon used for floating-point comparisons.
	const double kfEps = DBL_EPSILON;
	// Expansion factor for cell radius (to account for distortion due to Snyder projection).
	const double kfExp = 1.13;
	// Cell radius for distance comparisons.
	const double kfCR =
		kfExp * PYXMath::calcCircumRadius(index.getResolution() - ((bExpTile && index.hasVertexChildren()) ? 1 : 0)) * Icosahedron::kfCentralAngle + kfEps;

	// Check against each line segment.
	const int nNormalCount = static_cast<int>(vecNormal.size());
	for (int n = 0; n != nNormalCount; ++n)
	{
		// First project onto the line segment's normal. By comparing the
		// distance, we will know if we intersect with the line segment's
		// great circle.
		const double fDist = vecNormal[n].dot(coord);
		if (-kfCR <= fDist && fDist <= kfCR)
		{
			// We are on the great circle, but we might be beyond the line
			// segment's end points. We will know by comparing against
			// normals which point from the end points (A and B) toward each
			// other. Note that this simple test will introduce false positives
			// due to "squaring" the thick line caps; we could do a slower
			// more accurate test afterwards using Euclidean distance.
			PYXCoord3DDouble NxA = vecNormal[n].cross(vecPoint[n]);
			PYXCoord3DDouble BxN = vecPoint[n + 1].cross(vecNormal[n]);
			if (-kfCR <= NxA.dot(coord) && -kfCR <= BxN.dot(coord))
			{
				return true;
			}
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
PYXPointer<PYXGeometry> PYXCurve::intersection(	const PYXGeometry& geometry,
												bool bCommutative	) const
{
	const int nPoints = static_cast<int>(m_vecNode.size());
	if (nPoints == 0)
	{
		return PYXEmptyGeometry::create();
	}
	else if (nPoints == 1)
	{
		return geometry.intersection(PYXCell(m_vecNode.front()));
	}

	// Fill vecTest with the geometry's cells against which to test.
	std::vector<PYXIcosIndex> vecTest;
	if (const PYXTile* p = dynamic_cast<const PYXTile*>(&geometry))
	{
		vecTest.push_back(p->getRootIndex());
	}
	else if (const PYXEmptyGeometry* p = dynamic_cast<const PYXEmptyGeometry*>(&geometry))
	{
		return PYXEmptyGeometry::create();
	}
	else if (const PYXGlobalGeometry* p = dynamic_cast<const PYXGlobalGeometry*>(&geometry))
	{
		for (PYXIcosIterator it(2); !it.end(); it.next())
		{
			vecTest.push_back(it.getIndex());
		}
	}
	else
	{
		// Default implementation uses every cell.
		for (PYXPointer<PYXIterator> spIt = geometry.getIterator();
			!spIt->end(); spIt->next())
		{
			vecTest.push_back(spIt->getIndex());
		}
	}

	// Fill vecPoint with the curve's nodes in XYZ coordinates.
	std::vector<PYXCoord3DDouble> vecPoint(nPoints);
	for (int n = 0; n != nPoints; ++n)
	{
		SnyderProjection::getInstance()->pyxisToXYZ(m_vecNode[n], &vecPoint[n]);
	}

	// Fill vecNormal with a normal for each line segment between points.
	std::vector<PYXCoord3DDouble> vecNormal(nPoints - 1);
	for (int n = 0; n != nPoints - 1; ++n)
	{
		vecNormal[n] = vecPoint[n].cross(vecPoint[n + 1]);
		vecNormal[n].normalize();
	}

	// Traverse the test cells using the PYXIS hierarchy.
	PYXPointer<PYXTileCollection> spTC(PYXTileCollection::create());
	const int nTargetResolution = geometry.getCellResolution();
	while (!vecTest.empty())
	{
		PYXIcosIndex& index = vecTest.back();
		bool bTargetRes = index.getResolution() == nTargetResolution;
		bool bIntersects = intersectsTest(index, vecPoint, vecNormal, !bTargetRes);
		if (!bTargetRes)
		{
			if (bIntersects)
			{
				PYXChildIterator it(index);
				vecTest.pop_back(); // invalidates index
				for (; !it.end(); it.next())
				{
					vecTest.push_back(it.getIndex());
				}
			}
			else
			{
				vecTest.pop_back();
			}
		}
		else
		{
			if (bIntersects)
			{
				spTC->addTile(index, nTargetResolution);
			}
			vecTest.pop_back();
		}
	}

	if (spTC->isEmpty())
	{
		return PYXEmptyGeometry::create();
	}

	return spTC;
}

/*!
Determine if this geometry has any intersection with the specified geometry.

\param	geometry		The geometry to intersect with this one.
\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

\return true if any intersection exists or false if none is found.
*/
bool PYXCurve::intersects(const PYXGeometry& geometry, bool bCommutative) const
{
	const int nPoints = static_cast<int>(m_vecNode.size());
	if (nPoints == 0)
	{
		assert(false);
		return false;
	}
	else if (nPoints == 1)
	{
		return geometry.intersects(PYXCell(m_vecNode.front()));
	}

	// Fill vecTest with the geometry's cells against which to test.
	std::vector<PYXIcosIndex> vecTest;
	if (const PYXTile* p = dynamic_cast<const PYXTile*>(&geometry))
	{
		vecTest.push_back(p->getRootIndex());
	}
	else if (const PYXEmptyGeometry* p = dynamic_cast<const PYXEmptyGeometry*>(&geometry))
	{
		return PYXEmptyGeometry::create();
	}
	else if (const PYXGlobalGeometry* p = dynamic_cast<const PYXGlobalGeometry*>(&geometry))
	{
		for (PYXIcosIterator it(2); !it.end(); it.next())
		{
			vecTest.push_back(it.getIndex());
		}
	}
	else
	{
		// Default implementation uses every cell.
		for (PYXPointer<PYXIterator> spIt = geometry.getIterator();
			!spIt->end(); spIt->next())
		{
			vecTest.push_back(spIt->getIndex());
		}
	}

	// Fill vecPoint with the curve's nodes in XYZ coordinates.
	std::vector<PYXCoord3DDouble> vecPoint(nPoints);
	for (int n = 0; n != nPoints; ++n)
	{
		SnyderProjection::getInstance()->pyxisToXYZ(m_vecNode[n], &vecPoint[n]);
	}

	// Fill vecNormal with a normal for each line segment between points.
	std::vector<PYXCoord3DDouble> vecNormal(nPoints - 1);
	for (int n = 0; n != nPoints - 1; ++n)
	{
		vecNormal[n] = vecPoint[n].cross(vecPoint[n + 1]);
		vecNormal[n].normalize();
	}

	// Traverse the test cells using the PYXIS hierarchy.
	const int nTargetResolution = geometry.getCellResolution();
	while (!vecTest.empty())
	{
		PYXIcosIndex& index = vecTest.back();
		bool bTargetRes = index.getResolution() == nTargetResolution;
		bool bIntersects = intersectsTest(index, vecPoint, vecNormal, !bTargetRes);
		if (!bTargetRes)
		{
			if (bIntersects)
			{
				PYXChildIterator it(index);
				vecTest.pop_back(); // invalidates index
				for (; !it.end(); it.next())
				{
					vecTest.push_back(it.getIndex());
				}
			}
			else
			{
				vecTest.pop_back();
			}
		}
		else
		{
			if (bIntersects)
			{
				return true;
			}
			vecTest.pop_back();
		}
	}

	return false;
}

/*!
Get the cell resolution.

\return	The cell resolution.
*/
int PYXCurve::getCellResolution() const
{
	int nResolution = 0;

	// get the resolution of the first node
	if (!m_vecNode.empty())
	{
		nResolution = m_vecNode[0].getResolution();
	}

	return nResolution;
}

/*!
Set the PYXIS resolution of cells in the geometry. This method changes the
resolution of each of the nodes in the line.

\param	nCellResolution	The cell resolution
*/
void PYXCurve::setCellResolution(int nCellResolution)
{
	int nIndex = 0;

	IndexVector::iterator it = m_vecNode.begin();
	for (; it != m_vecNode.end(); ++it)
	{
		if (nIndex == 0)
		{
			m_vecNode[nIndex++].setResolution(nCellResolution);
		}
		else
		{
			
			it->setResolution(nCellResolution);

			// TODO[kabiraman]: Ensure that consecutive duplicate nodes do not exist.  
			// Investigate why the below causes a failure of null indices in the vector.
			// don't permit duplicate nodes
			/*if (*it != m_vecNode[nIndex - 1])
			{
				m_vecNode[nIndex++] = *it;
			}*/
		}
	}

	m_vecNode.resize(nIndex);
}

/*!
Get an iterator to the individual cells in the geometry.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXIterator> PYXCurve::getIterator() const
{
	return PYXCurveIterator::create(m_vecNode);
}

/*!
Add a node to the curve.

\param	index	The node.
*/
void PYXCurve::addNode(const PYXIcosIndex& index)
{
	if (index.isNull())
	{
		PYXTHROW(PYXGeometryException, "Null index.");
	}

	// We don't store duplicate consecutive indices.
	if (m_vecNode.empty())
	{
		m_vecNode.push_back(index);
	}
	else if ( index != m_vecNode.back() )
	{
		CoordLatLon last;
		CoordLatLon current;
		SnyderProjection::getInstance()->pyxisToNative(m_vecNode.back(),&last);

		m_vecNode.push_back(index);	

		SnyderProjection::getInstance()->pyxisToNative(index,&current);

		m_length += GreatCircleArc::calcDistance(last,current);
	}
	else
	{
		++m_nNodeDrop;
	}
}

/*!
Copy a representation of this geometry into a tile collection.

\param	pTileCollection	The tile collection.
*/
inline void PYXCurve::copyTo(PYXTileCollection* pTileCollection) const
{
	copyTo(pTileCollection, getCellResolution());
}

/*!
Copy a representation of this geometry into a tile collection at the specified resolution.

\param	pTileCollection		The tile collection.
\param	nTargetResolution	The target resolution.
*/
inline void PYXCurve::copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
{
	assert(pTileCollection != 0);

	PYXTileCollection tileCollection;
	pTileCollection->clear();
	
	for (PYXPointer<PYXIterator> it = getIterator(); !it->end(); it->next())
	{
		tileCollection.addTile(it->getIndex(), getCellResolution());
	}
	tileCollection.copyTo(pTileCollection, nTargetResolution);
}

/*!
Serialize the curve to a stream.

\param	out	The stream to serialize to.
*/
void PYXCurve::serialize(std::basic_ostream< char>& out) const
{
	out << m_vecNode.size() << " ";

	boost::intrusive_ptr<PYXIterator> it = getIterator();
	for (; !it->end(); it->next())
	{
		std::string strIndex = it->getIndex().toString();
		const char* pIndex = strIndex.c_str();

		for (const char* pIndexIterator = pIndex; *pIndexIterator; ++pIndexIterator)
		{
			unsigned char c = *pIndexIterator;
			out << c;
		}

		out << " ";
	}
}

/*!
Deserialize a curve from a stream.

\param	in	The stream to deserialize from.
*/
void PYXCurve::deserialize(std::basic_istream< char>& in)
{
	std::string strIndex;
	int nNumberOfNodes = 0;

	in >> nNumberOfNodes;
	m_vecNode.clear();
	in.get();

	for (int i = 0; i < nNumberOfNodes; ++i)
	{
		while (true)
		{
			char tmp;
			in.get(tmp);
			unsigned char c = tmp;
			if(' ' == c)
			{
				addNode(PYXIcosIndex(strIndex));
				strIndex.clear();
				break;
			}
			strIndex.push_back(c);
		}
	}
}

/*!
Constructor initializes member variables.

\param	vecNode		The vector of nodes.
*/
PYXCurve::PYXCurveIterator::PYXCurveIterator(const PYXCurve::IndexVector& vecNode) :
	m_itDestBegin(vecNode.begin()),
	m_itDest(vecNode.begin()),
	m_itEnd(vecNode.end()),
	m_nDir(PYXMath::knDirectionZero)
{
#ifdef NEW_CURVE_METHOD
	if (m_itDest != m_itEnd)
	{
		// Set dest index.
		m_indexDest = *m_itDest;

		// Set current index.
		m_nextIndex = m_indexDest;
	
		m_index = m_indexDest;

		if (vecNode.size() == 1)
		{
			m_vecIndex.push_back(m_indexDest);
		}
		else
		{
			// Generate all indexes for this curve.
			getNextIndex();
		}
	}
#else
	if (m_itDest != m_itEnd)
	{
		// Set dest index.
		m_indexDest = *m_itDest;
		CoordLatLon coordDest;
		SnyderProjection::getInstance()->pyxisToNative(m_indexDest, &coordDest);
		m_apGca.reset(new GreatCircleArc(coordDest, coordDest));

		// Set current index.
		m_nextIndex = m_indexDest;
	
		m_index = m_indexDest;

		// Calculate step distance.
		m_fStepDist = SnyderProjection::getInstance()->resolutionToPrecision(m_nextIndex.getResolution());
	}
#endif
}

/*!
Move to the next cell.
*/
void PYXCurve::PYXCurveIterator::next()
{
#ifdef NEW_CURVE_METHOD
	if (!end())
	{
		m_index = m_vecIndex.back();
		m_vecIndex.pop_back();
	}
#else
	if (!end())
	{
		// Remove last item from vector.  Next has to go 1 past last item.
		if (!m_vecIndex.empty())
		{
			m_vecIndex.pop_back();
			m_vecDir.pop_back();
		}

		// Add indexes to vector until we have 3 or we have reached end of curve.
		while (m_vecIndex.size() < 3 && getNextIndex())
		{
//			m_vecIndex.push_back(m_nextIndex);
//			m_vecDir.push_back(m_nDir);
		}			

		// TEST: Culling of point

		// Check to see if current index is neighbour to index two previous.
		if (m_vecIndex.size() == 3)
		{
			// If latest index and two before it are neighbours,
			// remove the "extra" one between it,
			// but only if it isn't a destination.
			if (testIsNeighbour(m_vecIndex[0], m_vecIndex[2])
				&& m_itDest != m_itDestBegin
				&& *(m_itDest - 1) != m_vecIndex[1])
			{
				m_vecIndex.erase(m_vecIndex.begin() + 1);
				m_vecDir.erase(m_vecDir.begin() + 1);

				// Calculate new direction from our previous index.
				m_vecDir[1] = PYXIcosMath::getNeighbourDirection(m_vecIndex[0], m_vecIndex[1]);
			}
		}

		// Get next index from vector of calculated indexes.  Remove last item.
		if (!m_vecIndex.empty())
		{
			m_index = m_vecIndex[0];
			m_nTheDirection = m_vecDir[0];

			// TODO mlepage This is ridiculous, should use erase, but can't
			// because the actual removal is in another method.

			for (size_t i = 1; i < m_vecIndex.size(); ++i)
			{
				m_vecIndex[i-1] = m_vecIndex[i];
				m_vecDir[i-1] = m_vecDir[i];
			}
			// Last item is removed the next time we call "Next".
		}
	}
#endif
}

/*!
See if we have covered all the cells.

\return	true if finished the iteration, otherwise false.
*/
bool PYXCurve::PYXCurveIterator::end() const
{
#ifdef NEW_CURVE_METHOD
	return m_vecIndex.empty();
#else
	return (m_itDest == m_itEnd && m_index == m_indexDest && m_vecIndex.size() == 0);
#endif
}

/*!
Move to the next cell.

\return true if a new index was calculated.  false if at end of curve.
*/
bool PYXCurve::PYXCurveIterator::getNextIndex()
{
#ifdef NEW_CURVE_METHOD
	while(m_itDest != m_itEnd)
	{
		m_indexDest = *m_itDest;
		// We finished the current segment.
		if (++m_itDest != m_itEnd)
		{
			GCArc(m_vecIndex, m_indexDest, *m_itDest);
		}
		else
		{
			// No more segments.
			m_nDir = PYXMath::knDirectionZero;
			return false;
		}
	}
	// We are at end of curve.
	return false;
#else
	if (!(m_itDest == m_itEnd && m_nextIndex == m_indexDest))
	{
		if (m_nextIndex == m_indexDest)
		{
			// We finished the current segment.
			if (++m_itDest != m_itEnd)
			{
				// Start new segment.
				m_indexDest = *m_itDest;
				CoordLatLon coord;
				SnyderProjection::getInstance()->pyxisToNative(m_indexDest, &coord);
				m_apGca.reset(new GreatCircleArc(m_apGca->getPoint2(), coord));
				m_fPos = 0.0;
				m_fStep = m_fStepDist / m_apGca->getDistance();
			}
			else
			{
				// No more segments.
				m_nDir = PYXMath::knDirectionZero;
				return false;
			}
		}

		PYXIcosIndex index;

		while (m_fPos <= 1.0)
		{
			// Advance along the current segment.
		//	m_fPos += m_fStep;

			// Test the index at this spot.
			CoordLatLon coord = m_apGca->getPoint(m_fPos);
			SnyderProjection::getInstance()->nativeToPYXIS(	
										coord, 
										&index, 
										m_nextIndex.getResolution()	);
			if (index != m_nextIndex)
			{
				// We found a new index on this segment.
				{
					// Sanity check: new index should be neighbour of previous index
					std::vector<PYXIcosIndex> vec;
					PYXIcosMath::getNeighbours(m_nextIndex, &vec);
					if (std::find(vec.begin(), vec.end(), index) == vec.end())
					{
						// The new index is not a neighbour of the current index!
						// Try to find a mutual neighbour.
						std::vector<PYXIcosIndex> vec2;
						PYXIcosMath::getNeighbours(index, &vec2);
						std::sort(vec.begin(), vec.end());
						std::sort(vec2.begin(), vec2.end());
						std::vector<PYXIcosIndex> vec3;
						std::set_intersection(vec.begin(), vec.end(), vec2.begin(), vec2.end(), std::back_inserter(vec3));
						if (1 < vec3.size())
						{
							// We found two mutual neighbours.
							// Pick the best one and reset our position for the next loop.
							CoordLatLon coord0;
							SnyderProjection::getInstance()->pyxisToNative(vec3[0], &coord0);
							CoordLatLon coord1;
							SnyderProjection::getInstance()->pyxisToNative(vec3[1], &coord1);

							CoordLatLon coordSt = m_apGca->getPoint(m_fPos-m_fStep*2);

							double fDist1 = coord0.distanceToLine(coordSt, coord);
							double fDist2 = coord1.distanceToLine(coordSt, coord); 

							if ( fDist1 <= fDist2)
							{
								m_nDir = PYXIcosMath::getNeighbourDirection(m_nextIndex, vec3[0]);
								m_nextIndex = vec3[0];
							}
							else
							{
								m_nDir = PYXIcosMath::getNeighbourDirection(m_nextIndex, vec3[1]);
								m_nextIndex = vec3[1];
							}
							m_vecIndex.push_back(m_nextIndex);
							m_vecDir.push_back(m_nDir);
							break;
						}
						else if (!vec3.empty())
						{
							// We found a mutual neighbour.
							// Use it and reset our position for the next loop.
							m_nDir = PYXIcosMath::getNeighbourDirection(m_nextIndex, vec3.front());
							m_nextIndex = vec3.front();
							m_vecIndex.push_back(m_nextIndex);
							m_vecDir.push_back(m_nDir);
							break;
						}

						// We didn't find a mutual neighbour!
						assert(false && "Unable to find a mutual neighbor.");
					}
				}
				m_nDir = PYXIcosMath::getNeighbourDirection(m_nextIndex, index);
				m_nextIndex = index;
				m_vecIndex.push_back(m_nextIndex);
				m_vecDir.push_back(m_nDir);
				break;
			}

			m_fPos += m_fStep;

		}

		if (1.0 < m_fPos && m_nextIndex != m_indexDest)
		{
			// Last index on segment isn't dest index!
			// (This really shouldn't happen.)
			std::vector<PYXIcosIndex> vec;
			PYXIcosMath::getNeighbours(m_nextIndex, &vec);
			if (std::find(vec.begin(), vec.end(), m_indexDest) != vec.end())
			{
				// The dest index is a neighbour of the current index. Use it.
				m_nDir = PYXIcosMath::getNeighbourDirection(m_nextIndex, m_indexDest);
				m_vecDir.push_back(m_nDir);
				m_nextIndex = m_indexDest;
				m_vecIndex.push_back(m_nextIndex);
			}
			else
			{
				// The dest index isn't even a neighbour of the current index!
				assert(false && "Destination index not a neighbor of current index.");
			}
		}

		// A new cell was calculated.
		return true;
	}

	// We are at end of curve.
	return false;
#endif
}

namespace
{

/*!
Recursive routine to subdivide a curve with midpoint displacement.
Adds A and a randomized midpoint to the curve at the specified resolution.
Does not add B to the curve. (So add the final point after recursing.)

\param curve		The curve.
\param A			The first point.
\param B			The second point.
\param nRes			The resolution.
\param nMaxLevel	The max level to recurse.
\param nLevel		The current level.
*/
void curveDivide(PYXCurve& curve, PYXCoord3DDouble& A, PYXCoord3DDouble& B, int nRes, int nMaxLevel, int nLevel)
{
	if (nLevel == nMaxLevel)
	{
		// Add A to curve.
		PYXIcosIndex index;
		SnyderProjection::getInstance()->xyzToPYXIS(A, &index, nRes);
		curve.addNode(index);
	}
	else
	{
		// Displace M and recurse.
		double w = (double)rand()/RAND_MAX;
		PYXCoord3DDouble Aw = A;
		Aw.scale(w);
		PYXCoord3DDouble Bw = B;
		Bw.scale(1 - w);
		PYXCoord3DDouble N = A.cross(B);
		N.scale((double)rand()/RAND_MAX - 0.5);
		PYXCoord3DDouble M;
		// TODO my kingdom for an add routine
		M.set(	Aw.x() + Bw.x() + N.x(),
				Aw.y() + Bw.y() + N.y(),
				Aw.z() + Bw.z() + N.z()	);
		M.normalize();

		curveDivide(curve, A, M, nRes, nMaxLevel, nLevel + 1);
		curveDivide(curve, M, B, nRes, nMaxLevel, nLevel + 1);
	}
}

}

/*!
\param A			Start of curve.
\param B			End of curve.
\param nRes			Resolution of curve.
\param nDivisions	Number of divisions to perform.
*/
void PYXCurve::randomize(const PYXIcosIndex& A, const PYXIcosIndex& B, int nRes, int nDivisions)
{
	this->clear();

	// Subdivide curve recursively.
	PYXCoord3DDouble Axyz;
	SnyderProjection::getInstance()->pyxisToXYZ(A, &Axyz);
	PYXCoord3DDouble Bxyz;
	SnyderProjection::getInstance()->pyxisToXYZ(B, &Bxyz);
	curveDivide(*this, Axyz, Bxyz, nRes, nDivisions, 0);

	// Add B to curve (final point)
	PYXIcosIndex index;
	SnyderProjection::getInstance()->xyzToPYXIS(Bxyz, &index, nRes);
	this->addNode(index);
}

/*! 
Allows PYXCurve objects collection to be written to streams.

\param out		The stream to write to.
\param pyxCurve	The curve to write to the stream.

\return The stream after the operation.
*/
std::ostream& operator <<(	std::ostream& out,
							const PYXCurve& pyxCurve	)
{
	out << pyxCurve.getNodes().size();

	// iterate over all of the cells in the collection 
	std::vector<PYXIcosIndex>::const_iterator itNode =
		pyxCurve.getNodes().begin();

	for (; itNode != pyxCurve.getNodes().end(); ++itNode)
	{
		out << " " << *itNode;
	}

	return out;
}

/*!
Allows PYXCurve to be read from streams.

\param input	The stream to read from.
\param pyxCurve	The curve to write to the stream.

\return The stream after the operation.
*/
std::istream& operator >>(	std::istream& input, 
							PYXCurve& pyxCurve	)
{
	// delete all existing nodes 
	pyxCurve.clear();

	// determine the number of tiles in the collection
	int nCount = 0;
	input >> nCount;
	
	// read each of the indices from the stream
	PYXIcosIndex pyxIndex;
	for (; 0 < nCount; nCount--)
	{
		input >> pyxIndex;
		pyxCurve.addNode(pyxIndex);
	}
	return input;
}
