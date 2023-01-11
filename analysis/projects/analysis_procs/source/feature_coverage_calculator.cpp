/******************************************************************************
feature_rasterizer.cpp

begin		: 2008-03-28
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "feature_coverage_calculator.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/derm/iterator.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/data/constant_record.h"

// standard includes
#include <algorithm>
#include <cassert>

// {7C2EA0D2-2DA9-431b-8747-357D1B24977B}
PYXCOM_DEFINE_CLSID(FeatureCoverageCalculator, 
0x7c2ea0d2, 0x2da9, 0x431b, 0x87, 0x47, 0x35, 0x7d, 0x1b, 0x24, 0x97, 0x7b);
PYXCOM_CLASS_INTERFACES(FeatureCoverageCalculator, IFeatureCalculator::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureCoverageCalculator, "Feature Coverage Calculator", "Calculates the ratio of a feature that is covered by non null data from a coverage.", "Development/Broken",
					IFeatureCalculator::iid, IProcess::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "The coverage that will be used in the coverage ratio calculation")
IPROCESS_SPEC_END

FeatureCoverageCalculator::FeatureCoverageCalculator()
{
}

FeatureCoverageCalculator::~FeatureCoverageCalculator()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus FeatureCoverageCalculator::initImpl()
{
	m_spCov = 0;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &m_spCov);
	if (!m_spCov)
	{
		PYXTHROW(AnalysisException,
			"Failed to get the coverage from the parameter.");
	}

	m_outputDefinition = PYXTableDefinition::create();

	m_outputDefinition->addFieldDefinition("ratio" , PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );


	return knInitialized;
}

/*!
Calculate the ratio of the feature geometry that is co-located with non null data in
the input coverage.
*/

PYXValue STDMETHODCALLTYPE FeatureCoverageCalculator::calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const
{
	int nCellCount = 0;
	int nPassCount = 0;
	PYXValue v;

	PYXPointer<PYXIterator> spIt = spFeature->getGeometry()->getIterator();
	for (; !spIt->end(); spIt->next())
	{
		++nCellCount;
		v.swap(m_spCov->getCoverageValue(spIt->getIndex()));
		if (!v.isNull())
		{
			++nPassCount;
		}
	}

	// calculate the ratio and return it
	double fRatio = 0;
	if (nCellCount != 0)
	{
		fRatio = static_cast<double>(nPassCount) / nCellCount;
	}
	else
	{
		TRACE_DEBUG("No cells in geometry for feature: " << spFeature->getID());
	}
	return PYXValue(fRatio);
}



boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE FeatureCoverageCalculator::calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const
{
	std::vector<PYXValue> values;
	values.push_back(calculateValue(spFeature,0));
	return new ConstantRecord(m_outputDefinition,values);
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE FeatureCoverageCalculator::getOutputDefinition() const
{
	return m_outputDefinition;
}
