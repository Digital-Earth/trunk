/******************************************************************************
mass_conserving_sampler.cpp

begin		: 2006-03-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "mass_conserving_sampler.h"

// local includes
#include "default_coverage.h"

// pyxlib includes
#include "pyxis/derm/coord_converter.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"

// standard includes
#include <cassert>

//! Constructor
PYXMassConservingSampler::PYXMassConservingSampler()
{
	m_sampleResolution = -1;
	m_bCalculated = false;
	m_spCache = PYXDefaultCoverage::create();
}

//! Destructor
PYXMassConservingSampler::~PYXMassConservingSampler()
{
}

/*!
Method for getting the resolution for the data source.

\return	The resolution that the data will be sampled at.
*/
int PYXMassConservingSampler::getResolution() const
{
	if (m_sampleResolution != -1)
	{
		return m_sampleResolution; 
	}

	return PYXSampler::getResolution();
}

/*!
Set up the default coverage for read/write access.
*/
void PYXMassConservingSampler::openCache() const
{
	// get, and interpret input coverage meta data
	PYXPointer<const PYXTableDefinition> spTblDef = getInput()->getCoverageDefinition();
	int nFieldCount = spTblDef->getFieldCount();
	std::vector<PYXValue::eType> vecTypes;
	std::vector<int> vecCounts;
	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		const PYXFieldDefinition& fieldDef = spTblDef->getFieldDefinition(nField);
		vecTypes.push_back(fieldDef.getType());
		vecCounts.push_back(fieldDef.getCount());
	}

	// make a new PYXDefaultCoverage data source and open it read/write
	m_spCache = PYXDefaultCoverage::create();
	m_spCache->openReadWrite("temp", *getInput()->getDefinition().get(), 
									getInput()->getFieldValues(),
									*spTblDef.get(),
									PYXTile::knDefaultTileDepth,
									m_sampleResolution);
}

//! run the calculation sum all the data values into Pyxis space
void PYXMassConservingSampler::runCalculation() const
{
	if (m_bCalculated)
		return;
		
	// we only want to be in this code once.
	static boost::mutex calculationGuard;
	boost::mutex::scoped_lock lock(calculationGuard);
	if (!m_bCalculated) 
	{
		if (m_sampleResolution == -1)
		{
			m_sampleResolution = PYXSampler::getResolution();
		}
		openCache();
#if 0
		PYXPointer<PYXXYIterator> spIt(getInput()->getXYIterator());
#else
		// TODO work on this
		assert(false && "TODO");
		PYXPointer<PYXXYIterator> spIt;
#endif
		PYXIcosIndex itIndex;
		PYXValue currentValue;
		if (spIt != 0)
		{
			for (; !spIt->end(); spIt->next())
			{
				PYXCoord2DDouble xy = spIt->getPoint();
				getInput()->getCoordConverter()->nativeToPYXIS(xy, &itIndex, m_sampleResolution);
				currentValue = m_spCache->getCoverageValue(itIndex);
				if (currentValue.isNull())
				{
					currentValue = spIt->getFieldValue();
				}
				else
				{
					for (int i = 0; i < currentValue.getArraySize(); ++i)
					{
						currentValue.setDouble(i, currentValue.getDouble(i) + spIt->getFieldValue().getDouble(i));
					}
				}
				m_spCache->setCoverageValue(currentValue, itIndex);
			}
		}
		m_bCalculated = true;
	}
}

/*!
Get the coverage value at the specified PYXIS index. Implements mass conserving
sampling.  This will be an expensive sampler, but will be best to use for data
sets like a population.  It itterates over the whole XY data set and adds the values
into a coverage at the given resolution.  The first time a data value is retrieved, the
retrieval time will be long, after that the retrival time will be much shorter.

\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	The value.
*/
PYXValue PYXMassConservingSampler::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	assert((getInput().get() != 0) && "No input coverage set.");

	runCalculation();

	// get the value
	return m_spCache->getCoverageValue(index, nFieldIndex);
}
