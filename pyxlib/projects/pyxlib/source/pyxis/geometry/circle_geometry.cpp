/******************************************************************************
circle_geometry.cpp

begin		: 2007-11-19
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"

//local includes
#include "circle_geometry.h"
#include "circle_intersection_test.h"
#include "icos_test_traverser.h"

// pyxlib includes
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <limits>

//! The unit test class
Tester<PYXCircleGeometry> gTester;
void PYXCircleGeometry::test()
{

	{ // Test Normal Operation 
		PYXIcosIndex index("3-020020300000");
		PYXPointer<PYXCircleGeometry> spGeom = PYXCircleGeometry::create(index, 50000);
		TEST_ASSERT(spGeom->getCellResolution() == index.getResolution());
		
		int nGeometryCount = spGeom->getGeometryCount();
		spGeom->setRadius(60000);
		TEST_ASSERT(nGeometryCount < spGeom->getGeometryCount());
		spGeom->setRadius(50000);
		TEST_ASSERT(nGeometryCount == spGeom->getGeometryCount());

		nGeometryCount = spGeom->getGeometryCount();
		spGeom->setCellResolution(15);
		TEST_ASSERT(nGeometryCount < spGeom->getGeometryCount());

		PYXPointer<PYXGeometry> spCloneGeom = spGeom->clone();
		TEST_ASSERT(spCloneGeom);

		PYXPointer<PYXCircleGeometry> spCircClone = boost::dynamic_pointer_cast<PYXCircleGeometry, PYXGeometry> (spCloneGeom);
		TEST_ASSERT(spCircClone);

		TEST_ASSERT(spCircClone->getCellResolution() == spGeom->getCellResolution());
		TEST_ASSERT(spCircClone->getGeometryCount() == spGeom->getGeometryCount());
		TEST_ASSERT(spCircClone->getCentre() == spGeom->getCentre());
		TEST_ASSERT(spCircClone->getRadius() == spGeom->getRadius());
		
		PYXTileCollection tc;
		spGeom->copyTo(&tc);
		TEST_ASSERT(!tc.isEmpty());
		std::vector<PYXIcosIndex> vectIndex;
		spGeom->calcPerimeter(&vectIndex);
		TEST_ASSERT(!vectIndex.empty()); 
	}
	//TODO: Should have some more unit testing here.
}

/*!
Default constructor, constructs a Circle Geometry object. 

\param centreIndex The PYXIcosIndex which is the centre of the circle.
\param fRadius	   The radius of the circle in metres.
*/
PYXCircleGeometry::PYXCircleGeometry(const PYXIcosIndex& centreIndex, double fRadius) :
	m_centre(centreIndex),
	m_fRadius(fRadius)
{
}

/*!
Calculates the circular geometry by setting up and peforming a circle intersection test.
The circle intersection test returns all the cells that have an intersection from the centre
index and a given radius as a tile collection. 

\sa CircleIntersectionTest
*/
void PYXCircleGeometry::calcGeometry() const
{
	boost::recursive_mutex::scoped_lock lock (m_mutex);
	if (m_spGeometry)
	{
		m_spGeometry->clear();
	}
	CircleIntersectionTest geometryTest(SnyderProjection::getInstance());
	geometryTest.setCentre(m_centre);
	geometryTest.setDataResolution(m_centre.getResolution());
	geometryTest.setTargetResolution(m_centre.getResolution());
	geometryTest.setRadiusInMetres(m_fRadius);

	PYXIcosTestTraverser traverser;
	traverser.setTest(geometryTest);
	traverser.traverse(PYXIcosIndex::knMinSubRes, m_centre.getResolution());
	m_spGeometry = traverser.getTileCollection();
}

/*!
Set the PYXIS resolution of cells in the geometry. A change 
in the cell resolution results in the recalculation of all of 
the cells which make up this geometry.

\param	nCellResolution	The cell resolution.
*/
void PYXCircleGeometry::setCellResolution(int nCellResolution)
{
	boost::recursive_mutex::scoped_lock lock (m_mutex);
	if (m_centre.getResolution() != nCellResolution)
	{
		m_centre.setResolution(nCellResolution);
		m_spGeometry.reset();
	}
}

/*!
Clones this geometry object, by constructing a new Circular geometry 
with the same centre point and radius of the current object. Constructing 
a new object with the same centre and radius is garunteed to generate the 
same tile collction. 

\return a PYXPointer to a Geometry which represents a clone of the current geometry.
*/
PYXPointer<PYXGeometry> PYXCircleGeometry::clone() const
{
	return boost::dynamic_pointer_cast<PYXGeometry, PYXCircleGeometry>(
		PYXCircleGeometry::create(m_centre, m_fRadius));
}

/*!
Sets the radius of the circle. Each change in the radius results
in a new circle being created and therefore the need to recalculated the geometry.

\param fRadius The radius in metres of the circle to create.
*/
void PYXCircleGeometry::setRadius(double fRadius)
{
	boost::recursive_mutex::scoped_lock lock (m_mutex);
	if (fRadius != m_fRadius)
	{
		m_fRadius = fRadius;
		m_spGeometry.reset();
	}
}

/*!
Gets an iterator to all the geometries in this collection. The geometry
iterator is returned by proxying through to the PYXTileCollection's getGeometryIterator 
method, and returning that.

\return A PYXPointer to a Geometry iterator to enable iterating over all geometries
        in this collection.
*/
PYXPointer<PYXGeometryIterator> PYXCircleGeometry::getGeometryIterator() const
{
	boost::recursive_mutex::scoped_lock lock (m_mutex);
	if (!m_spGeometry)
	{
		calcGeometry();
	}

	return m_spGeometry->getGeometryIterator();
}

/*!
Calculates the perimeter of this geometry. The permiters is calculated 
by proxying the call through to the PYXTileCollection calcPerimeter method
which populates a vector of PYXIcosIndices with all the indices that make 
up the perimeter of this geometry.

\param pVecIndex The container to hold the returned indices.

\param A pointer to a standard vector of PYXIcosIndices to fill.
*/
void PYXCircleGeometry::calcPerimeter(std::vector<PYXIcosIndex> *pVecIndex) const
{
	boost::recursive_mutex::scoped_lock lock (m_mutex);
	if (!m_spGeometry)
	{
		calcGeometry();
	}

	m_spGeometry->calcPerimeter(pVecIndex);
}

/*!
Copies this geometry into a PYXTileCollection. This method proxies the call
through to the PYXTileCollection copyTo method.

\param pTileCollection	The geometry to copy this geometry into.
*/
void PYXCircleGeometry::copyTo(PYXTileCollection *pTileCollection) const
{
	boost::recursive_mutex::scoped_lock lock (m_mutex);
	if (!m_spGeometry)
	{
		calcGeometry();
	}
	m_spGeometry->copyTo(pTileCollection, getCellResolution());
}

/*!
Copies this geometry into a PYXTileCollection at the specified resolution. This method proxies the call
through to the PYXTileCollection copyTo method.

\param	pTileCollection		The geometry to copy this geometry into.
\param	nTargetResolution	The target resolution.
*/
void PYXCircleGeometry::copyTo(PYXTileCollection *pTileCollection, int nTargetResolution) const
{
	boost::recursive_mutex::scoped_lock lock (m_mutex);
	if (!m_spGeometry)
	{
		calcGeometry();
	}

	assert(pTileCollection != 0);

	m_spGeometry->copyTo(pTileCollection, nTargetResolution);
}

/*!
Gets the cell resolution of this geometry by proxying a call to reutrn 
the cell resolution through to the PYXTileCollection get cell resolution.

\return An integer which is the cell resolution of this geometry.
*/
int PYXCircleGeometry::getCellResolution() const 
{	
	return m_centre.getResolution();
}

void PYXCircleGeometry::serialize(std::basic_ostream< char> &out) const
{
	out << getRadius() << std::endl;
	
	PYXIcosIndex centreIndex = getCentre();
//	std::basic_string<unsigned char> strCompatible(reinterpret_cast<const unsigned char*>(centreIndex.toString().c_str()));
	out << centreIndex.toString().c_str() << std::endl;
}

void PYXCircleGeometry::deserialize(std::basic_istream< char> &in)
{
	in >> m_fRadius;
	
	std::basic_string<char> strCompatIn;
	in >> strCompatIn;
	//std::string strIndex(reinterpret_cast<const char*>(strCompatIn.c_str()));
	m_centre = PYXIcosIndex(strCompatIn.c_str());
	m_spGeometry.reset();
}

void PYXCircleGeometry::setCentrePoint(const PYXIcosIndex &centreIndex)
{
	if (centreIndex != m_centre)
	{
		m_centre = centreIndex;
		m_spGeometry.reset();
	}
}

bool PYXCircleGeometry::intersects(const PYXGeometry &geometry, bool bCommutative) const
{
	return ! intersection(geometry,bCommutative)->isEmpty();
}


PYXPointer<PYXGeometry> PYXCircleGeometry::intersection(const PYXGeometry& geometry, bool bCommutative) const
{
	CircleIntersectionTest geometryTest(SnyderProjection::getInstance());
	geometryTest.setCentre(m_centre);
	geometryTest.setDataResolution(geometry.getCellResolution());
	geometryTest.setTargetResolution(geometry.getCellResolution());
	geometryTest.setRadiusInMetres(m_fRadius);

	PYXIcosTestTraverser traverser;
	traverser.setTest(geometryTest);
	
	const PYXTile * tile = dynamic_cast<const PYXTile*>(&geometry);
	if (tile != 0)
	{
		traverser.traverse(tile->getRootIndex(), tile->getCellResolution());
	}
	else
	{
		traverser.setGeometry(geometry.clone());
		traverser.traverse(geometry.getCellResolution(), geometry.getCellResolution());
	}

	return traverser.getTileCollection();
}

PYXBoundingCircle PYXCircleGeometry::getBoundingCircle() const
{
	PYXCoord3DDouble center;

	SnyderProjection::getInstance()->pyxisToXYZ(m_centre, &center);	

	return PYXBoundingCircle(center,m_fRadius/SphereMath::knEarthRadius);
}