/******************************************************************************
colourizer.cpp

begin		: 2007-05-03
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "colourizer.h"

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

// {8B64253C-7DA2-4d0c-988A-1148BADFF24F}
PYXCOM_DEFINE_CLSID(Colourizer, 
0x8b64253c, 0x7da2, 0x4d0c, 0x98, 0x8a, 0x11, 0x48, 0xba, 0xdf, 0xf2, 0x4f);
PYXCOM_CLASS_INTERFACES(Colourizer, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(Colourizer, "Colourizer", "A coverage that colours an input coverage across a range of values", "Styling/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Coverage to Colour", "The input coverage that is coloured by the process")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<Colourizer> gTester;

double fMinDefault = -11000;
double fMaxDefault = 9000;

}

Colourizer::Colourizer() :
	m_fMin(fMinDefault),
	m_fMax(fMaxDefault)
{
}

Colourizer::~Colourizer()
{
}

void Colourizer::test()
{
	// TODO test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE Colourizer::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["min"] = StringUtils::toString(m_fMin);
	mapAttr["max"] = StringUtils::toString(m_fMax);

	if (m_palette != "")
	{
		mapAttr["palette"] = m_palette;
	}

	return mapAttr;
}

std::string STDMETHODCALLTYPE Colourizer::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"Colourizer\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"min\" type=\"xs:double\" default=\"" + StringUtils::toString(m_fMin) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Minimum</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"max\" type=\"xs:double\" default=\"" + StringUtils::toString(m_fMax) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Maximum</friendlyName>"
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
}

void STDMETHODCALLTYPE Colourizer::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;

	// TODO should probably detect non-integer values.

	it = mapAttr.find("min");
	m_fMin = fMinDefault;
	if (it != mapAttr.end())
	{
		StringUtils::fromString(it->second, &m_fMin);
	}

	it = mapAttr.find("max");
	m_fMax = fMaxDefault;
	if (it != mapAttr.end())
	{
		StringUtils::fromString(it->second, &m_fMax);
	}

	it = mapAttr.find("palette");
	if (it != mapAttr.end())
	{
		m_palette = it->second;
	}
	else 
	{
		m_palette = "";
	}
}

IProcess::eInitStatus Colourizer::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	PYXPointer<State> newState = State::create(getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>(),m_fMin,m_fMax,m_palette);	
		
	if (!newState->m_spCov)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not acquire input coverage for colourizer.");
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
		m_spCovDefn->addFieldDefinition(
			"rgb", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);
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

PYXValue Colourizer::getCoverageValue(	const PYXIcosIndex& index,
										int nFieldIndex	) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;
	}

	PYXValue val;

	PYXValue valIn = state->m_spCov->getCoverageValue(index, nFieldIndex);
	if (!valIn.isNull())
	{
		uint8_t buf[3];
		state->m_palette->convert(valIn.getDouble(),buf,false); //false = no alpha		
		val = PYXValue(buf, 3);
	}

	return val;
}

PYXPointer<PYXValueTile> Colourizer::getFieldTile(	const PYXIcosIndex& index,
													int nRes,
													int nFieldIndex	) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;
	}

	PYXPointer<PYXValueTile> spValueTile;
	PYXPointer<PYXValueTile> spValueTileIn = state->m_spCov->getFieldTile(index, nRes, nFieldIndex);
	if (spValueTileIn)
	{
		spValueTile = PYXValueTile::create(index, nRes, state->m_definition);

		uint8_t buf[3];

		int nCellCount = spValueTile->getNumberOfCells();
		for (int nCell = 0; nCell != nCellCount; ++nCell)
		{
			PYXValue valIn = spValueTileIn->getValue(nCell, nFieldIndex);
			if (!valIn.isNull())
			{
				state->m_palette->convert(valIn.getDouble(),buf,false); //false = no alpha				
				spValueTile->setValue(nCell, 0, PYXValue(buf, 3));
			}
		}
	}

	return spValueTile;
}

PYXCost STDMETHODCALLTYPE Colourizer::getFieldTileCost(	const PYXIcosIndex& index,
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


////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void Colourizer::createGeometry() const
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
