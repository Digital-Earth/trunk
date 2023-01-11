/******************************************************************************
resolution_filter.cpp

begin		: 2007-04-27
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "resolution_filter.h"

#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

// {FE0A6DAD-AA1E-4489-9AAB-24564CF39A2C}
PYXCOM_DEFINE_CLSID(ResolutionFilter, 
0xfe0a6dad, 0xaa1e, 0x4489, 0x9a, 0xab, 0x24, 0x56, 0x4c, 0xf3, 0x9a, 0x2c);
PYXCOM_CLASS_INTERFACES(ResolutionFilter, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ResolutionFilter, "Resolution Filter", "A coverage that filters input coverages based on resolution.", "Utility",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "A coverage to filter based on its geometry resolution.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<ResolutionFilter> gTester;

}

ResolutionFilter::ResolutionFilter()
{
	m_nMinAbsRes = -1;
	m_nMaxAbsRes = -1;
	m_nMinRelRes = -1;
	m_nMaxRelRes = -1;
}

ResolutionFilter::~ResolutionFilter()
{
}

void ResolutionFilter::test()
{
	// TODO test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE ResolutionFilter::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["min_rel_res"] = StringUtils::toString(m_nMinRelRes);
	mapAttr["max_rel_res"] = StringUtils::toString(m_nMaxRelRes);
	mapAttr["min_abs_res"] = StringUtils::toString(m_nMinAbsRes);
	mapAttr["max_abs_res"] = StringUtils::toString(m_nMaxAbsRes);

	return mapAttr;
}

std::string STDMETHODCALLTYPE ResolutionFilter::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"ResolutionFilter\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"min_rel_res\" type=\"xs:int\" default=\"" + StringUtils::toString(m_nMinRelRes) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Minimum Relative Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"max_rel_res\" type=\"xs:int\" default=\"" + StringUtils::toString(m_nMaxRelRes) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Maximum Relative Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"min_abs_res\" type=\"xs:int\" default=\"" + StringUtils::toString(m_nMinAbsRes) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Minimum Absolute Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"max_abs_res\" type=\"xs:int\" default=\"" + StringUtils::toString(m_nMaxAbsRes) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Maximum Absolute Resolution</friendlyName>"					
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE ResolutionFilter::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;

	// TODO should probably detect non-integer values.

	it = mapAttr.find("min_rel_res");
	m_nMinRelRes = it != mapAttr.end() ? atoi(it->second.c_str()) : -1;
	it = mapAttr.find("max_rel_res");
	m_nMaxRelRes = it != mapAttr.end() ? atoi(it->second.c_str()) : -1;
	it = mapAttr.find("min_abs_res");
	m_nMinAbsRes = it != mapAttr.end() ? atoi(it->second.c_str()) : -1;
	it = mapAttr.find("max_abs_res");
	m_nMaxAbsRes = it != mapAttr.end() ? atoi(it->second.c_str()) : -1;
}

IProcess::eInitStatus ResolutionFilter::initImpl()
{
	m_spCov = getInput(0);
	m_spCovDefn = m_spCov->getCoverageDefinition()->clone();
	int nRes = m_spCov->getGeometry()->getCellResolution();

	// Choose min filter res.
	if (m_nMinAbsRes != -1)
	{
		m_nMinRes = m_nMinAbsRes;
	}
	else if (m_nMinRelRes != -1)
	{
		m_nMinRes = nRes - m_nMinRelRes;
	}
	else
	{
		m_nMinRes = -1;
	}

	// Choose max filter res.
	if (m_nMaxAbsRes != -1)
	{
		m_nMaxRes = m_nMaxAbsRes;
	}
	else if (m_nMaxRelRes != -1)
	{
		m_nMaxRes = nRes + m_nMaxRelRes;
	}
	else
	{
		m_nMaxRes = -1;
	}

	// Clamp filter res to limits.
	if (m_nMinRes < PYXIcosIndex::knMinSubRes)
	{
		m_nMinRes = PYXIcosIndex::knMinSubRes;
	}

	// Clamp filter res to limits.
	if (PYXMath::knMaxAbsResolution < m_nMaxRes || m_nMaxRes == -1)
	{
		m_nMaxRes = PYXMath::knMaxAbsResolution;
	}

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue ResolutionFilter::getCoverageValue(	const PYXIcosIndex& index,
												int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	int nRes = index.getResolution();
	if(m_nMinRes <= nRes && nRes <= m_nMaxRes)
	{
		return m_spCov->getCoverageValue(index, nFieldIndex);
	}
	else
	{
		return PYXValue();
	}
}

PYXPointer<PYXValueTile> ResolutionFilter::getFieldTile(	const PYXIcosIndex& index,
															int nRes,
															int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	if(m_nMinRes <= nRes && nRes <= m_nMaxRes)
	{
		return m_spCov->getFieldTile(index, nRes, nFieldIndex);
	}
	else
	{
		return PYXPointer<PYXValueTile>();
	}
}

PYXCost ResolutionFilter::getFieldTileCost(	const PYXIcosIndex& index,
											int nRes,
											int nFieldIndex ) const
{
	if(m_nMinRes <= nRes && nRes <= m_nMaxRes)
	{
		return m_spCov->getFieldTileCost(index, nRes, nFieldIndex);
	}
	else
	{
		return PYXCost::knImmediateCost;
	}
}

PYXPointer<PYXValueTile> ResolutionFilter::getCoverageTile(const PYXTile& tile) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	int nRes = tile.getCellResolution();
	if(m_nMinRes <= nRes && nRes <= m_nMaxRes)
	{
		return m_spCov->getCoverageTile(tile);
	}
	else
	{
		return PYXPointer<PYXValueTile>();
	}
}

PYXCost ResolutionFilter::getTileCost(const PYXTile& tile) const
{
	int nRes = tile.getCellResolution();
	if(m_nMinRes <= nRes && nRes <= m_nMaxRes)
	{
		return m_spCov->getTileCost(tile);
	}
	else
	{
		return PYXCost::knImmediateCost;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

boost::intrusive_ptr<ICoverage> ResolutionFilter::getInput(int n) const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);
	boost::intrusive_ptr<ICoverage> spCov;
	assert(getParameter(0) &&
		getParameter(0)->getValue(n) &&
		getParameter(0)->getValue(n)->getOutput());
	getParameter(0)->getValue(n)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &spCov);
	return spCov;
}

void ResolutionFilter::createGeometry() const
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