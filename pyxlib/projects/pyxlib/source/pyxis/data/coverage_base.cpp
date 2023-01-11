/******************************************************************************
coverage_base.cpp

begin		: 2007-03-27
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "coverage_base.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/exhaustive_iterator.h"

PYXPointer<PYXValueTile> STDMETHODCALLTYPE CoverageBase::getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, spCovDefn);

	PYXExhaustiveIterator it(index, nRes);
	for (int nIndexOffset = 0; !it.end(); it.next(), ++nIndexOffset)
	{
		spValueTile->setValue(
			nIndexOffset, 
			0,
			getCoverageValue(it.getIndex(), nFieldIndex));
	}
	return spValueTile;
}

PYXCost STDMETHODCALLTYPE CoverageBase::getFieldTileCost(const PYXIcosIndex& index,
													     int nRes,
													     int nFieldIndex ) const 
{
	return PYXCost::knDefaultCost;
}

PYXPointer<PYXValueTile> CoverageBase::getCoverageTile(const PYXTile& tile) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);
	int nFieldCount = getCoverageDefinition()->getFieldCount();

	if (nFieldCount == 1)
	{
		return getFieldTile(tile.getRootIndex(), tile.getCellResolution(), 0);
	}

	// fill every field for every cell in the tile.
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(tile, getCoverageDefinition());
	int nCellCount = tile.getCellCount();
	for (int nFieldIndex = 0; nFieldIndex < nFieldCount; ++nFieldIndex)
	{
		PYXPointer<PYXValueTile> spTile = getFieldTile(tile.getRootIndex(), tile.getCellResolution(), nFieldIndex);
		if (spTile)
		{
			for (int nIndexOffset = 0; nIndexOffset < nCellCount; ++nIndexOffset)
			{
				spValueTile->setValue(nIndexOffset, nFieldIndex, spTile->getValue(nIndexOffset, 0));
			}
		}
	}

	return spValueTile;
}

PYXCost CoverageBase::getTileCost(const PYXTile& tile) const
{
	int nFieldCount = getCoverageDefinition()->getFieldCount();

	PYXCost cost;
	for (int nFieldIndex = 0; nFieldIndex < nFieldCount; ++nFieldIndex)
	{
		cost += getFieldTileCost(tile.getRootIndex(), tile.getCellResolution(), nFieldIndex);
	}
	return cost;
}

void CoverageBase::setCoverageValue(	const PYXValue& value,	
										const PYXIcosIndex& index,
										int nFieldIndex	)
{
	assert(false && "Not implemented for base class.");
}

void CoverageBase::setCoverageTile(PYXPointer<PYXValueTile> spValueTile)
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	if (!isWritable())
	{
		PYXTHROW(PYXDataException, "Coverage is not writable, can't set the tile.");
	}

	// iterate over each value in the tile and write it to the coverage
	int nFieldCount = getCoverageDefinition()->getFieldCount();
	if (!spValueTile->isValueTileCompatible(getCoverageDefinition()))
	{
		PYXTHROW(PYXDataException, "Tile and coverage are not compatible.");
	}

	PYXPointer<PYXIterator> spIt = spValueTile->getTile().getIterator();
	for (int nIndexOffset = 0; !spIt->end(); spIt->next(), ++nIndexOffset)
	{
		for (int nIndex = 0; nIndex < nFieldCount; ++nIndex)
		{
			setCoverageValue(
				spValueTile->getValue(nIndexOffset, nIndex),
				spIt->getIndex(), 
				nIndex);
		}
	}
}
