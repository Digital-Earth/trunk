/******************************************************************************
channel_combiner.cpp

begin		: 2007-04-26
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "channel_combiner.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

// {6BCC2C4D-1353-442b-A7B2-6D57ED42C12D}
PYXCOM_DEFINE_CLSID(ChannelCombiner, 
0x6bcc2c4d, 0x1353, 0x442b, 0xa7, 0xb2, 0x6d, 0x57, 0xed, 0x42, 0xc1, 0x2d);
PYXCOM_CLASS_INTERFACES(ChannelCombiner, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ChannelCombiner, "Channel Combiner", "A coverage that combines multiple input coverages into a single coverage with multiple fields.", "Utility",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 0, -1, "Channel Input Coverage(s)", "A coverage to amalgomate with the other input coverages.")
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 0, -1, "Input Feature Collection(s)", "Assorted feature collections.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<ChannelCombiner> gTester;

}

ChannelCombiner::ChannelCombiner()
{
}

ChannelCombiner::~ChannelCombiner()
{
}

void ChannelCombiner::test()
{
	// TODO test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE ChannelCombiner::getAttributes() const
{
	return std::map<std::string, std::string>();
}

void STDMETHODCALLTYPE ChannelCombiner::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
}

std::string ChannelCombiner::getData() const
{
	return "";
}

void ChannelCombiner::setData(const std::string& strData)
{
}

IProcess::eInitStatus ChannelCombiner::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Channel Combiner: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	int nInputCount = getParameter(0)->getValueCount();
	m_spCovDefn = PYXTableDefinition::create();
	for (int nInput = 0; nInput != nInputCount; ++nInput)
	{
		// Each field must have a unique name, so rename them.
		const PYXFieldDefinition& fieldDefn = getInput(nInput)->getCoverageDefinition()->getFieldDefinition(0);
		m_spCovDefn->addFieldDefinition(StringUtils::toString(nInput), fieldDefn.getContext(), 
			fieldDefn.getType(), fieldDefn.getCount());
	}
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue ChannelCombiner::getCoverageValue(	const PYXIcosIndex& index,
											int nFieldIndex	) const
{
	return getInput(nFieldIndex)->getCoverageValue(index, 0);
}

PYXCost ChannelCombiner::getFieldTileCost(	const PYXIcosIndex& index,
											int nRes,
											int nFieldIndex) const
{
	return getInput(nFieldIndex)->getFieldTileCost(index, nRes, 0);
}

PYXPointer<PYXValueTile> ChannelCombiner::getFieldTile(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex	) const
{
	return getInput(nFieldIndex)->getFieldTile(index, nRes, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

boost::intrusive_ptr<ICoverage> ChannelCombiner::getInput(int n) const
{
	// TODO: this should be done once at init time, or though lazy evaluation
	// but it should be cached because QueryInterface is slow.

	boost::recursive_mutex::scoped_lock lock(m_procMutex);
	boost::intrusive_ptr<ICoverage> spCov;
	assert(getParameter(0) &&
		getParameter(0)->getValue(n) &&
		getParameter(0)->getValue(n)->getOutput());
	getParameter(0)->getValue(n)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &spCov);
	return spCov;
}

void ChannelCombiner::createGeometry() const
{
	// TODO[mlepage] should probably do something more sensible

	if (0 < getParameter(0)->getValueCount() && getInput(0)->getGeometry())
	{
		m_spGeom = getInput(0)->getGeometry()->clone();
	}
	else
	{
		m_spGeom = PYXGlobalGeometry::create(10);
	}
}
