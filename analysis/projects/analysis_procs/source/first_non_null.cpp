/******************************************************************************
first_non_null.cpp

begin		: 2007-05-02
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "first_non_null.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/geometry/geometry_serializer.h"

// standard includes
#include <cassert>

// {79E1D5B2-F816-449e-876B-9EAF0B1CE118}
PYXCOM_DEFINE_CLSID(FirstNonNull, 
					0x79e1d5b2, 0xf816, 0x449e, 0x87, 0x6b, 0x9e, 0xaf, 0xb, 0x1c, 0xe1, 0x18);
PYXCOM_CLASS_INTERFACES(FirstNonNull, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FirstNonNull, "Select First Value", "A coverage that returns the first non-null value of its input coverages.",  "Analysis/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
					IPROCESS_SPEC_PARAMETER(ICoverage::iid, 0, -1, "Input Coverage(s)", "An input coverage.")
					IPROCESS_SPEC_END

					namespace
{

	//! Tester class
	Tester<FirstNonNull> gTester;

}

FirstNonNull::FirstNonNull()
{
}

FirstNonNull::~FirstNonNull()
{
}

void FirstNonNull::test()
{
	// TODO test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus FirstNonNull::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);
	m_strID = "First Non Null: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	PYXPointer<State> newState = State::create();

	// Set up inputs
	for (int nInput = 0; nInput != getParameter(0)->getValueCount(); ++nInput)
	{
		boost::intrusive_ptr<ICoverage> spCov = getParameter(0)->getValue(nInput)->getOutput()->QueryInterface<ICoverage>();
		assert(spCov);
		newState->addInput(spCov);
	}

	// Below, we ensure that all input coverages have identical table definition.

	// Set up coverage definition
	m_spCovDefn = PYXTableDefinition::create();

	for (int nOffset = 0; nOffset < newState->getInputCount(); ++nOffset)
	{
		// get the cov defn. for n
		PYXPointer<PYXTableDefinition> currentCov = 
			newState->getInput(nOffset)->getCoverageDefinition();

		assert(0 < currentCov->getFieldCount());
		if (m_spCovDefn->getFieldCount() == 0)
		{
			assert(currentCov->getFieldDefinition(0).getType() != PYXValue::knNull &&
				"Can't have null field definition.");

			// if it's a valid coverage, clone it and break
			m_spCovDefn = newState->getInput(nOffset)->getCoverageDefinition()->clone();
		}
		else
		{
			// verify that the input matches the output coverage defn.
			if (m_spCovDefn->getFieldCount() != currentCov->getFieldCount())
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Field counts between coverages do not match.");
				return knFailedToInit;
			}

			// Ensure that all field definitions match between coverages.
			for (int nIndex = 0; nIndex < m_spCovDefn->getFieldCount(); ++nIndex)
			{
				if ((currentCov->getFieldDefinition(nIndex).getType() !=
					m_spCovDefn->getFieldDefinition(nIndex).getType()) ||
					(currentCov->getFieldDefinition(nIndex).getCount() !=
					m_spCovDefn->getFieldDefinition(nIndex).getCount()))
				{
					m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
					m_spInitError->setError("Input processes do not have compatible fields.");
					return knFailedToInit;
				}
			}
		}
	}

	if (m_spCovDefn->getFieldCount() == 0)
	{
		// add a null field
		m_spCovDefn->addNullField();
	}

	newState->setCoverageDefinition(m_spCovDefn->clone());

	m_state = newState;
	m_spGeom.reset();

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue FirstNonNull::getCoverageValue(	const PYXIcosIndex& index,
										int nFieldIndex	) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		state = m_state;
	}

	PYXValue v;

	for (int nInput = 0; nInput != state->getInputCount(); ++nInput)
	{
		v = state->getInput(nInput)->getCoverageValue(index, nFieldIndex);
		if (!v.isNull())
		{
			break;
		}
	}

	return v;
}

PYXCost STDMETHODCALLTYPE FirstNonNull::getFieldTileCost(	const PYXIcosIndex& index,
														 int nRes,
														 int nFieldIndex ) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		state = m_state;
	}

	int nTotalInputs = state->getInputCount();

	if (nTotalInputs == 1)
	{
		return state->getInput(0)->getFieldTileCost(index, nRes, nFieldIndex);
	}

	PYXCost cost;
	for (int nInput = 0; nInput != nTotalInputs; ++nInput)
	{
		cost += state->getInput(nInput)->getFieldTileCost(index, nRes, nFieldIndex);		
	}
	//TODO: realy caclulate the mixCost
	PYXCost findFirstNotNullValueCost = PYXCost::knImmediateCost;

	return cost + findFirstNotNullValueCost;
}

PYXPointer<PYXValueTile> FirstNonNull::getFieldTile(	const PYXIcosIndex& index,
													int nRes,
													int nFieldIndex	) const
{
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		state = m_state;
	}

	int nTotalInputs = state->getInputCount();

	if (nTotalInputs == 1)
	{
		return state->getInput(0)->getFieldTile(index, nRes, nFieldIndex);
	}
	auto tile = PYXTile(index,nRes);

	if (!m_spGeom->intersects(tile))
	{
		return 0;
	}

	// our goal tile, which we are constructing	
	PYXPointer<PYXValueTile> spValueTile;

	if (nTotalInputs != 0)
	{
		// the list of input data sources.
		std::vector<PYXPointer<PYXValueTile> > vecInputTile;

		int firstNonNullTileIndex = 0;

		do
		{
			// get the first input source
			vecInputTile.push_back(
				state->getInput(firstNonNullTileIndex)->getFieldTile(index, nRes, nFieldIndex));

			if (vecInputTile[firstNonNullTileIndex])
			{
				// copy all values into our output tile.
				spValueTile = vecInputTile[firstNonNullTileIndex]->cloneFieldTile(0);

				// Short cut if we only got data on the last tile
				// we don't need to look at anything else.
				if (firstNonNullTileIndex+1 == nTotalInputs)
				{
					return spValueTile;
				}
			}
			else
			{
				// Look for data in the next tile.
				firstNonNullTileIndex++;

				// since the tile intersects the bigger geometry (m_spGeom)
				// If we got to the end and didn't get a tile with data.
				// we should return an empty ValueTile
				if (firstNonNullTileIndex == nTotalInputs)
				{
					spValueTile = PYXValueTile::create(index,nRes,state->getCoverageDefinition());
					return spValueTile;
				}
			}
		}
		while (!spValueTile);

		// get the FNN value for each cell in our tile.
		int nCellCount = spValueTile->getNumberOfCells();
		PYXValue pv = state->getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
		bool bFoundSecondNonNullTile = false;
		for (int nCell = 0; nCell != nCellCount; ++nCell)
		{
			// TODO: this could be faster if we implemented a "null field iterator"
			// for a value tile.
			if (!spValueTile->getValue(nCell, 0, &pv))
			{
				// there was no data in the first tile.
				for (int nInput = firstNonNullTileIndex+1; nInput < nTotalInputs; ++nInput)
				{
					// check if field tile has been initialized.
					if (static_cast<int>(vecInputTile.size()) <= nInput)
					{
						// request tile from the input.
						vecInputTile.push_back(state->getInput(nInput)->getFieldTile(index, nRes, nFieldIndex));
						if (vecInputTile[nInput])
						{
							bFoundSecondNonNullTile = true;
						}
						else if (nInput+1 == nTotalInputs && !bFoundSecondNonNullTile)
						{
							// we just found out that we only have one tile supplying 
							// data for this section of the earth, so just return that.
							return spValueTile;
						}
					}

					// check to make sure the tile is not null before retrieving a value from it.
					if(vecInputTile[nInput])
					{
						// if the cell value is not null.
						if (vecInputTile[nInput]->getValue(nCell, 0, &pv))
						{
							// place in tile and break.
							spValueTile->setValue(nCell, 0, pv);
							break;
						}
					}
				}
			}
		}
	}

	return spValueTile;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void FirstNonNull::createGeometry() const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	//load geometry if we have a stable identity...
	PYXPointer<PYXLocalStorage> storage;

	//TODO[shatzi]: our default pipelines doesn't have stable id (there is no checksum on the input files)
	//if (PipeUtils::isPipelineIdentityStable(const_cast<Blender*>(this)))
	{
		storage = PYXProcessLocalStorage::create(getIdentity());

		std::auto_ptr<PYXConstWireBuffer> geomBuffer = storage->get("first_not_null:geom");

		if (geomBuffer.get() != 0)
		{
			(*geomBuffer) >> m_spGeom;

			if (m_spGeom)
			{
				return;
			}
		}
	}

	//create the geometry from inputs
	m_spGeom = PYXEmptyGeometry::create();
	for (int nInput = 0; nInput < m_state->getInputCount(); ++nInput)
	{
		PYXPointer<PYXGeometry> spGeometry = m_state->getInput(nInput)->getGeometry();
		if (0 != spGeometry)
		{
			// Union between itself and the geometry.
			m_spGeom = m_spGeom->disjunction(*spGeometry);
		}
	}

	//write the result to the storage if we can..
	if (storage)
	{
		PYXStringWireBuffer buffer;
		buffer << *m_spGeom;

		storage->set("first_not_null:geom",buffer);
	}
}
