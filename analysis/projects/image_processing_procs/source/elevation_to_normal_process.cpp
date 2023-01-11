/******************************************************************************
elevation_to_normal_process.cpp

begin		: 2010-03-09
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "elevation_to_normal_process.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/derm/neighbour_iterator.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

// standard includes
#include <cassert>

// {DB041009-1DA3-4f8e-AA28-0E8EFBA5F6F8}
PYXCOM_DEFINE_CLSID(ElevationToNormalProcess, 
0xdb041009, 0x1da3, 0x4f8e, 0xaa, 0x28, 0xe, 0x8e, 0xfb, 0xa5, 0xf6, 0xf8);

PYXCOM_CLASS_INTERFACES(ElevationToNormalProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ElevationToNormalProcess, "Elevation to Normal", "A coverage that takes elevation values and converts them to surface normal.", "Analysis/Elevations",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Elevation Coverage", "The elevation input coverage.")
IPROCESS_SPEC_END



namespace
{

//! Tester class
Tester<ElevationToNormalProcess> gTester;

}

ElevationToNormalProcess::ElevationToNormalProcess()
{
	m_pSnyder = SnyderProjection::getInstance();
}

ElevationToNormalProcess::~ElevationToNormalProcess()
{
}

void ElevationToNormalProcess::test()
{
	//TODO [shatzi]: add some unit testing!
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus ElevationToNormalProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Elevation To Normal" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the input coverage.
	m_spCov = 0;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &m_spCov	);
	
	if (!m_spCov)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input coverage not set");
		return knFailedToInit;
	}

	// Get the input coverage definition.
	PYXPointer<PYXTableDefinition> spCovDefn = m_spCov->getCoverageDefinition();

	// Create a new field definition, in which the greyscale fields are verified and converted to rgb.
	m_spCovDefn = PYXTableDefinition::create();
	if (spCovDefn->getFieldCount() == 0)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input coverage should always have at least one field");
		return knFailedToInit;
	}
	else
	{
		for (int nOffset = 0; nOffset < spCovDefn->getFieldCount(); ++nOffset)
		{
			// For each greyscale UInt8[1] field, add a 3 RGB UInt8[3] field.
			const PYXFieldDefinition& field = spCovDefn->getFieldDefinition(nOffset);
			m_spCovDefn->addFieldDefinition(
				field.getName(), PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 3	);
		}
	}
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue ElevationToNormalProcess::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	assert(m_spCov);

	std::vector<PYXIcosIndex> neighbours;
	PYXIcosMath::getNeighbours(index,&neighbours);

	PYXMath::eHexDirection gapDirection;
	if (PYXIcosMath::getCellGap(index,&gapDirection))
	{
		neighbours.erase(neighbours.begin()+(int)gapDirection-1);
	}
	
	return findNormal(neighbours[0],m_spCov->getCoverageValue(neighbours[0], nFieldIndex),
					  neighbours[2],m_spCov->getCoverageValue(neighbours[2], nFieldIndex),
					  neighbours[4],m_spCov->getCoverageValue(neighbours[4], nFieldIndex));
}

PYXPointer<PYXValueTile> ElevationToNormalProcess::getFieldTile(
	const PYXIcosIndex& index,
	int nRes,
	int nFieldIndex	) const
{
	assert(m_spCov);

	// get the tile from the input coverage
	PYXPointer<PYXValueTile> spInputValueTile = m_spCov->getFieldTile(index, nRes, nFieldIndex);

	for(PYXNeighbourIterator it(index);!it.end();it.next())
	{
		m_spCov->getFieldTile(it.getIndex(), nRes, nFieldIndex);
	}
	// Return null tiles right away.
	if (!spInputValueTile)
	{
		return spInputValueTile;
	}

	// create the tile to return to the caller
	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, spCovDefn);

	
	//int nMaxCount = spValueTile->getNumberOfCells();
	//calculatePartialTile(0,nMaxCount-1,index,spInputValueTile.get(),spValueTile.get(),nFieldIndex,nRes);

	
	int nCellCount = spValueTile->getNumberOfCells();
	int cellsPerThread = (nCellCount + N_THREADS - 1) / N_THREADS;
	int firstIndex = 0;
	boost::thread_group threads;
	for (int count = 0; count < N_THREADS; ++count)
	{
		int lastIndex = firstIndex + cellsPerThread - 1;
		if (lastIndex >= nCellCount)
		{
			lastIndex = nCellCount - 1;
		}

		// just call it directly if you want single threaded... and comment out the create_thread below
		//calculatePartialTile(firstIndex,lastIndex,index,spInputValueTile.get(),spValueTile.get(),nFieldIndex,nRes);

		threads.create_thread(
			boost::bind (&ElevationToNormalProcess::calculatePartialTile,this,
				firstIndex, lastIndex,index,spInputValueTile.get(),spValueTile.get(),nFieldIndex,nRes));
				
		firstIndex += cellsPerThread;
	}

	// wait for all the threads to finish.
	threads.join_all();
	
	
	return spValueTile;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void ElevationToNormalProcess::calculatePartialTile (int firstIndex, int lastIndex,
													 const PYXIcosIndex & index,
													 PYXValueTile* spInputTile,
													 PYXValueTile* spOutputTile,
													 int nFieldIndex,
													 int nRes) const
{	
	// convert all of the values from the input coverage tile
	PYXValue inVal = m_spCov->getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	PYXValue outVal = getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();

#ifdef FAST_NORMAL
	PYXCoord3DDouble aDelta;
	PYXCoord3DDouble bDelta;
	PYXCoord3DDouble cDelta;

	calcDelta(PYXIcosMath::calcIndexFromOffset(index,nRes,0),aDelta,bDelta,cDelta);
#endif
	
	//all cell of the root center child are safe in the meaning of finding neighboors
	int nSafeCount = PYXIcosMath::getCellCount(index,nRes-2);

	//works only for centeriod cells, for vertex children - the safe region is smaller
	if (!index.hasVertexChildren())
	{
		nSafeCount = PYXIcosMath::getCellCount(index,nRes-3);
	}

	PYXMath::eHexDirection dir1 = PYXMath::knDirectionOne;
	PYXMath::eHexDirection dir2 = PYXMath::knDirectionThree;
	PYXMath::eHexDirection dir3 = PYXMath::knDirectionFive;
	PYXMath::eHexDirection gapDirection;
	
	if (PYXIcosMath::getCellGap(index,&gapDirection))
	{
		if ((int)gapDirection % 2)
		{
			dir1 = PYXMath::knDirectionTwo;
			dir2 = PYXMath::knDirectionFour;
			dir3 = PYXMath::knDirectionSix;
		}
	}

	// TODO: we should be able to be more efficient than this -- look for
	// or add support for setting the index in the iterator to part way along.
	PYXPointer<PYXIterator> it = spOutputTile->getIterator();

	for (int n = 0; n < firstIndex; ++n)
	{
		it->next();
	}	
	
	for (int n = firstIndex; n <= lastIndex; ++n)
	{
		PYXIcosIndex curIndex(it->getIndex());
		it->next();
		
		PYXIcosIndex neighbour1(PYXIcosMath::move(curIndex,dir1));
		PYXIcosIndex neighbour2(PYXIcosMath::move(curIndex,dir2));
		PYXIcosIndex neighbour3(PYXIcosMath::move(curIndex,dir3));	

		PYXValue value1(inVal);
		PYXValue value2(inVal);
		PYXValue value3(inVal);

		if (n<nSafeCount)
		{			
			if (!spInputTile->getValue(neighbour1,0, &value1) ||
				!spInputTile->getValue(neighbour2,0, &value2) ||
				!spInputTile->getValue(neighbour3,0, &value3))
			{
				continue;
			}		
		}
		else
		{
			if (!safeFetchValue(&value1,index,neighbour1,spInputTile,nFieldIndex) ||
				!safeFetchValue(&value2,index,neighbour2,spInputTile,nFieldIndex) ||
				!safeFetchValue(&value3,index,neighbour3,spInputTile,nFieldIndex))
			{
				continue;
			}
		}
						
		if (!value1.isNull() && !value2.isNull() && !value3.isNull())
		{
#ifdef FAST_NORMAL
			spOutputTile->setValue(n, 0, findNormalFast(curIndex,
													   aDelta,value1,
													   bDelta,value2,
													   cDelta,value3));
#else			
			spOutputTile->setValue(n, 0, findNormal(neighbour1,value1,
												   neighbour2,value2,
												   neighbour3,value3));
		   
#endif		
		}		
	}
}


inline bool ElevationToNormalProcess::safeFetchValue(PYXValue * pValue,const PYXIcosIndex & root,const PYXIcosIndex & index,const PYXPointer<PYXValueTile> & valueTile,int nFieldIndex) const
{
	if (root.isAncestorOf(index))
	{
		return valueTile->getValue(index,0, pValue);
	} 
	else 
	{

		// lock - the input maybe not thread safe...
		boost::recursive_mutex::scoped_lock lock(m_inputCovMutex);

		*pValue = m_spCov->getCoverageValue(index, nFieldIndex);
		return true;
	}
}

void ElevationToNormalProcess::createGeometry() const
{
	assert(m_spCov);
	PYXPointer<PYXGeometry> spGeometry = m_spCov->getGeometry();
	if (!spGeometry)
	{
		m_spGeom = PYXEmptyGeometry::create();
	}
	else
	{
		m_spGeom = spGeometry->clone();
	}
}

#ifdef FAST_NORMAL

void ElevationToNormalProcess::calcDelta(const PYXIcosIndex & root,PYXCoord3DDouble & aDelta,PYXCoord3DDouble & bDelta,PYXCoord3DDouble & cDelta) const
{
	std::vector<PYXIcosIndex> neighbours;
	PYXIcosMath::getNeighbours(root,&neighbours);

	PYXMath::eHexDirection gapDirection;
	if (PYXIcosMath::getCellGap(root,&gapDirection))
	{
		neighbours.erase(neighbours.begin()+(int)gapDirection-1);
	}

	PYXCoord3DDouble rootXYZ;
	CoordLatLon latLon;

	m_pSnyder->pyxisToNative(root,&latLon);	
	SphereMath::llxyz(latLon,&rootXYZ);

	m_pSnyder->pyxisToNative(neighbours[0],&latLon);	
	SphereMath::llxyz(latLon,&aDelta);

	m_pSnyder->pyxisToNative(neighbours[1],&latLon);	
	SphereMath::llxyz(latLon,&bDelta);

	m_pSnyder->pyxisToNative(neighbours[2],&latLon);	
	SphereMath::llxyz(latLon,&cDelta);

	aDelta.subtract(rootXYZ);
	bDelta.subtract(rootXYZ);
	cDelta.subtract(rootXYZ);
}

PYXValue ElevationToNormalProcess::findNormalFast(const PYXIcosIndex & root,
												  const PYXCoord3DDouble & aDelta, const PYXValue& aElevation,
												  const PYXCoord3DDouble & bDelta, const PYXValue& bElevation,
												  const PYXCoord3DDouble & cDelta, const PYXValue& cElevation) const
{
	PYXCoord3DDouble rootXYZ;
	
	CoordLatLon latLon;
	m_pSnyder->pyxisToNative(root,&latLon);	
	SphereMath::llxyz(latLon,&rootXYZ);

	PYXCoord3DDouble aXYZ(rootXYZ);
	PYXCoord3DDouble bXYZ(rootXYZ);
	PYXCoord3DDouble cXYZ(rootXYZ);

	aXYZ.translate(aDelta);
	bXYZ.translate(bDelta);
	cXYZ.translate(cDelta);

	aXYZ.normalize();
	bXYZ.normalize();
	cXYZ.normalize();

	//add elevation
	aXYZ.scale( (aElevation.getDouble()/SphereMath::knEarthRadius+1.0));
	bXYZ.scale( (bElevation.getDouble()/SphereMath::knEarthRadius+1.0));
	cXYZ.scale( (cElevation.getDouble()/SphereMath::knEarthRadius+1.0));

	//calculate ba and ca
	bXYZ.subtract(aXYZ);
	cXYZ.subtract(aXYZ);

	PYXCoord3DDouble normal = bXYZ.cross(cXYZ);

	if (aXYZ.dot(normal) < 0)
	{		
		normal.negate();
	}

	//normalize
	normal.normalize();

	double normalValues[3];
	normalValues[0] = normal[0];
	normalValues[1] = normal[1];
	normalValues[2] = normal[2];

	return PYXValue(normalValues,3);
}

#endif

// TODO: This continues to run on a thread after PYXLib deinitializes, causing
// m_pSnyder to point to garbage: see ticket #2555.
PYXValue ElevationToNormalProcess::findNormal(const PYXIcosIndex & a,const PYXValue& aElevation,
										   const PYXIcosIndex & b,const PYXValue& bElevation,
										   const PYXIcosIndex & c,const PYXValue& cElevation
										   ) const
{	
	PYXCoord3DDouble aXYZ;
	PYXCoord3DDouble bXYZ;
	PYXCoord3DDouble cXYZ;
	
	CoordLatLon latLon;
	m_pSnyder->pyxisToNative(a,&latLon);	
	SphereMath::llxyz(latLon,&aXYZ);

	m_pSnyder->pyxisToNative(b,&latLon);	
	SphereMath::llxyz(latLon,&bXYZ);

	m_pSnyder->pyxisToNative(c,&latLon);	
	SphereMath::llxyz(latLon,&cXYZ);

	//add elevation
	aXYZ.scale( (aElevation.getDouble()/SphereMath::knEarthRadius+1.0));
	bXYZ.scale( (bElevation.getDouble()/SphereMath::knEarthRadius+1.0));
	cXYZ.scale( (cElevation.getDouble()/SphereMath::knEarthRadius+1.0));

	//calcaulte ba and ca
	bXYZ.subtract(aXYZ);
	cXYZ.subtract(aXYZ);

	PYXCoord3DDouble normal = bXYZ.cross(cXYZ);

	if (aXYZ.dot(normal) < 0)
	{		
		normal.negate();
	}

	//normalize
	normal.normalize();

	double normalValues[3];
	normalValues[0] = normal[0];
	normalValues[1] = normal[1];
	normalValues[2] = normal[2];

	return PYXValue(normalValues,3);
}