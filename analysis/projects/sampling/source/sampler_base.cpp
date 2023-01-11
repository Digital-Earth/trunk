/******************************************************************************
sampler_base.cpp

begin		: 2007-03-12
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_SAMPLING_SOURCE
#include "stdafx.h"
#include "sampler_base.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/sampling/xy_bounds_geometry.h"
#include "pyxis/sampling/xy_coverage.h"

void SamplerBase::createGeometry() const
{
	boost::intrusive_ptr<IXYCoverage> spXYCov = getXYCoverage();
	assert(spXYCov && "input coverage must be set");

	//assert(spXYCov->hasSpatialReferenceSystem());

	double fPrecision = spXYCov->getSpatialPrecision();
	if (0 <= fPrecision)
	{
		// convert spatial precision in metres to a PYXIS resolution
		double fArcRadians = fPrecision / ReferenceSphere::kfRadius;
		int nResolution = SnyderProjection::getInstance()->precisionToResolution(fArcRadians);

		assert(spXYCov->getCoordConverter());

		// create the geometry at that resolution
		PYXPointer<PYXVectorGeometry> vecGeom = PYXXYBoundsGeometry::create(
													spXYCov->getBounds(),
													*(spXYCov->getCoordConverter()),
													nResolution	);

		m_spGeom = vecGeom;

		m_spRegion = boost::dynamic_pointer_cast<PYXXYBoundsRegion>(vecGeom->getRegion());
	}
}

PYXPointer<PYXValueTile> SamplerBase::getFieldTile(const PYXIcosIndex& index,
												   int nRes,
												   int nFieldIndex) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	if (getXYRegion()->intersects(index,true) == PYXRegion::knNone)
	{
		return 0;
	}

	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, spCovDefn);

	PYXValue workingVal = getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	PYXValue nullVal;
	
	// Send a "preload hint" down to the XY Coverage so that it can load up an area
	// of the data into memory for quicker access.
	getXYCoverage()->tileLoadHint(PYXTile(index, nRes));

	ValueTileConsumer consumer(*this,spValueTile);

	PYXPointer<XYAsyncValueGetter> getter = getXYCoverage()->getAsyncCoverageValueGetter(consumer,getSamplingMatrixSize(),getSamplingMatrixSize());

	//TRACE_INFO("starting populating tasks");

	if (nRes - index.getResolution() > 5)
	{
		int count = 0;
		PYXExhaustiveIterator it(index, nRes-5);

		for (int nIndexOffset = 0; !it.end(); it.next(), ++nIndexOffset,++count)
		{
			//getter->addAsyncRequest(it.getIndex());
			getter->addAsyncRequests(PYXTile(it.getIndex(),nRes));
		}
	}
	else 
	{
		getter->addAsyncRequests(PYXTile(index,nRes));
	}

	//TRACE_INFO("done populating tasks " << count );

	if (getter->join())
	{
		//copy values into tile
		consumer.copyValuesToValueTile();

		// Let the coverage know we are done with this tile.
		getXYCoverage()->tileLoadDoneHint(PYXTile(index, nRes));
	}
	else
	{
		// Let the coverage know we are done with this tile.
		getXYCoverage()->tileLoadDoneHint(PYXTile(index, nRes));
		
		//let the caller know we have failed
		PYXTHROW(PYXException,"Failed to sample tile");
	}
	
	return spValueTile;
}

PYXValue SamplerBase::getCoverageValue(	const PYXIcosIndex& index,
										int nFieldIndex) const
{
	if (getXYRegion()->intersects(index,false) == PYXRegion::knNone)
	{
		return PYXValue();
	}

	PYXValue returnVal = getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	if (getCoverageValue(index, &returnVal, nFieldIndex))
	{
		return returnVal;
	}
	return PYXValue();
}

void SamplerBase::geometryHint(PYXPointer<PYXGeometry> spGeom)
{
	getXYCoverage()->geometryHint(spGeom);
}

void SamplerBase::endGeometryHint(PYXPointer<PYXGeometry> spGeom)
{
	getXYCoverage()->endGeometryHint(spGeom);
}
