/******************************************************************************
greyscale_to_rgb_process.cpp

begin		: 2007-10-11
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "greyscale_to_rgb_process.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

// {BC985EE7-B31E-4c2a-B9AA-3419276A6B3A}
PYXCOM_DEFINE_CLSID(GreyscaleToRGBProcess, 
0xbc985ee7, 0xb31e, 0x4c2a, 0xb9, 0xaa, 0x34, 0x19, 0x27, 0x6a, 0x6b, 0x3a);
PYXCOM_CLASS_INTERFACES(GreyscaleToRGBProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GreyscaleToRGBProcess, "Greyscale to RGB", "A coverage with all greyscale values of its input coverages converted to RGB values", "Drop", // "Utility",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Greyscale Coverage", "The 256 bit greyscale input coverage.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<GreyscaleToRGBProcess> gTester;

}

GreyscaleToRGBProcess::GreyscaleToRGBProcess()
{
}

GreyscaleToRGBProcess::~GreyscaleToRGBProcess()
{
}

void GreyscaleToRGBProcess::test()
{
	// Test getCoverageValue.
	{
		// Some index.
		PYXIcosIndex index = "A-01000";

		// The data for the input greyscale value.
		uint8_t arrayValue[] = {42};

		boost::intrusive_ptr<ConstCoverage> spInputCoverage(new ConstCoverage);
		assert(spInputCoverage);
		spInputCoverage->setReturnValue(PYXValue(arrayValue, 1), PYXFieldDefinition::knContextGreyScale);

		boost::intrusive_ptr<IProcess> spGreyscaleToRGBProcess;
		PYXCOMCreateInstance(GreyscaleToRGBProcess::clsid, 0, IProcess::iid, (void**) &spGreyscaleToRGBProcess);
		assert(spGreyscaleToRGBProcess);

		boost::intrusive_ptr<ICoverage> spGreyscaleToRGBCoverage;
		spGreyscaleToRGBProcess->QueryInterface(ICoverage::iid, (void**) &spGreyscaleToRGBCoverage);
		assert(spGreyscaleToRGBCoverage);

		spGreyscaleToRGBProcess->getParameter(0)->addValue(spInputCoverage);
		spGreyscaleToRGBProcess->initProc();

		// The data for the output RGB values.
		uint8_t resultArrayValue[] = {42, 42, 42};

		PYXValue resultValue(&resultArrayValue[0], 3);
		TEST_ASSERT(resultValue == spGreyscaleToRGBCoverage->getCoverageValue(index));
	}
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus GreyscaleToRGBProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Greyscale to RGB" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the input coverage.
	m_spCov = 0;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &m_spCov	);
	assert(m_spCov);

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
			if (field.getContext() == PYXFieldDefinition::knContextGreyScale) 
			{
				if (field.getType() != PYXValue::knUInt8)
				{
					m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
					m_spInitError->setError("This process can only accept 8 bit greyscale input.");
					return knFailedToInit;
				}
				m_spCovDefn->addFieldDefinition(
					field.getName(), PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3	);
			}
			else
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Expected a field '" + StringUtils::toString(nOffset) + "' to be greyscale.");
				return knFailedToInit;
			}
		}
	}
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue GreyscaleToRGBProcess::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	assert(m_spCov);
	return convert(m_spCov->getCoverageValue(index, nFieldIndex));
}

PYXPointer<PYXValueTile> GreyscaleToRGBProcess::getFieldTile(
	const PYXIcosIndex& index,
	int nRes,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	assert(m_spCov);

	// get the tile from the input coverage
	PYXPointer<PYXValueTile> spInputValueTile = m_spCov->getFieldTile(index, nRes, nFieldIndex);

	// Return null tiles right away.
	if (!spInputValueTile)
	{
		return spInputValueTile;
	}

	// create the tile to return to the caller
	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, spCovDefn);

	// convert all of the values from the input coverage tile
	PYXValue inVal = m_spCov->getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	PYXValue outVal = getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	uint8_t* pInVal = (uint8_t*) inVal.getPtr(0);
	uint8_t* pOutVal = (uint8_t*) outVal.getPtr(0);
	assert(pInVal != 0);
	assert(pOutVal != 0);
	int nMaxCount = spValueTile->getNumberOfCells();
	for (int n = 0; n < nMaxCount; ++n)
	{
		if (spInputValueTile->getValue(n, 0, &inVal))
		{
			pOutVal[0] = pOutVal[1] = pOutVal[2] = pInVal[0];
			spValueTile->setValue(n, 0, outVal);
		}
	}
	return spValueTile;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void GreyscaleToRGBProcess::createGeometry() const
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

//! Convert a greyscale value to an RGB value.
PYXValue GreyscaleToRGBProcess::convert(const PYXValue& valIn) const
{
	if (!valIn.isNull())
	{
		uint8_t buf[3];
		buf[0] = buf[1] = buf[2] = valIn.getUInt8();
		return PYXValue(buf, 3);
	}
	return valIn;
}
