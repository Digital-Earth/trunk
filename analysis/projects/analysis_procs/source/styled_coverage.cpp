/******************************************************************************
styled_coverage.cpp

begin		: 2011-09-08
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "styled_coverage.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// TEMP
#include "pyxis/pipe/pipe_utils.h"

// standard includes
#include <cassert>

// {43FFAAE3-0A08-45f8-80DA-2E75B31EB96F}
PYXCOM_DEFINE_CLSID(StyledCoverage, 
0x43ffaae3, 0xa08, 0x45f8, 0x80, 0xda, 0x2e, 0x75, 0xb3, 0x1e, 0xb9, 0x6f);
PYXCOM_CLASS_INTERFACES(StyledCoverage, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(StyledCoverage, "Styled Coverage", "apply style for how to visualize the coverage information", "Styling/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Coverage to Style", "The input coverage")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<StyledCoverage> gTester;

}

StyledCoverage::StyledCoverage() :
	m_showAsElevation(false)
{
}

StyledCoverage::~StyledCoverage()
{
}

void StyledCoverage::test()
{
	// TODO test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE StyledCoverage::getAttributes() const
{	
	std::map<std::string, std::string> mapAttr;

	mapAttr["show_as_elevation"] = m_showAsElevation?"1":"0";
	mapAttr["palette"] = m_palette;	

	return mapAttr;
}

std::string STDMETHODCALLTYPE StyledCoverage::getAttributeSchema() const
{
	std::string xsd = 
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"StyledCoverage\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			"<xs:element name=\"show_as_elevation\" type=\"xs:boolean\" default=\"" + StringUtils::toString(m_showAsElevation?1:0) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Show as Elevation</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"palette\" type=\"xs:string\" >"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Palette</friendlyName>"
					"<description>Palette information to use</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return xsd;
}

void STDMETHODCALLTYPE StyledCoverage::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"palette",m_palette);
	std::string value;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"show_as_elevation",value);

	m_showAsElevation = value=="bool 1" || value=="1" || value=="true";

	m_strStyle = "<style><Coverage><ShowAsElevation>";
	m_strStyle += (m_showAsElevation?"1":"0");
	m_strStyle += "</ShowAsElevation><Palette>" + m_palette + "</Palette></Coverage></style>";
}


IProcess::eInitStatus StyledCoverage::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	PYXPointer<State> newState = State::create(getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>(),m_palette);

	if (!newState->m_spCov)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not acquire input coverage for StyledCoverage.");
		return knFailedToInit;
	}

	// Set up coverage definition
	m_spCovDefn = PYXTableDefinition::create();

	PYXPointer<PYXTableDefinition> spCurrentCovDefn = 
		newState->m_spCov->getCoverageDefinition();

	if (spCurrentCovDefn->getFieldCount() <= 0)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input has no data fields");
		return knFailedToInit;
	}

	if (spCurrentCovDefn->getFieldDefinition(0).getType() != PYXValue::knNull)
	{
		// if it's a valid coverage, clone it and break
		m_spCovDefn->addFieldDefinition(spCurrentCovDefn->getFieldDefinition(0));
	}

	if (m_spCovDefn->getFieldCount() == 0)
	{
		// add a null field
		m_spCovDefn->addNullField();
	}

	newState->m_definition = m_spCovDefn->clone();

	m_state = newState;

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue StyledCoverage::getCoverageValue(	const PYXIcosIndex& index,
										int nFieldIndex	) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;
	}

	return state->m_spCov->getCoverageValue(index, nFieldIndex);
}

PYXPointer<PYXValueTile> StyledCoverage::getFieldTile(	const PYXIcosIndex& index,
													int nRes,
													int nFieldIndex	) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;
	}
	return state->m_spCov->getFieldTile(index, nRes, nFieldIndex);
}

PYXCost STDMETHODCALLTYPE StyledCoverage::getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex ) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;
	}

	//TODO: realy caclulate the cololor cost
	PYXCost cololorCost = PYXCost::knImmediateCost;

	return state->m_spCov->getFieldTileCost(index,nRes,nFieldIndex) + cololorCost;
}

PYXPointer<PYXValueTile> StyledCoverage::getCoverageTile(const PYXTile& tile) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;
	}

	return state->m_spCov->getCoverageTile(tile);
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void StyledCoverage::createGeometry() const
{
	// TODO[mlepage] should probably do something more sensible

	if (0 < getParameter(0)->getValueCount() && m_state->m_spCov->getGeometry())
	{
		m_spGeom = m_state->m_spCov->getGeometry()->clone();
	}
	else
	{
		m_spGeom = PYXGlobalGeometry::create(10);
	}
}
