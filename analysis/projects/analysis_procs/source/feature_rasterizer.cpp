/******************************************************************************
feature_rasterizer.cpp

begin		: 2007-05-30
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "feature_rasterizer.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/curve.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

// {90A2B533-CFD2-4afb-AA89-8AD9A4E8FCB8}
PYXCOM_DEFINE_CLSID(FeatureRasterizer, 
0x90a2b533, 0xcfd2, 0x4afb, 0xaa, 0x89, 0x8a, 0xd9, 0xa4, 0xe8, 0xfc, 0xb8);
PYXCOM_CLASS_INTERFACES(FeatureRasterizer, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureRasterizer, "Feature to Coverage", "A coverage that rasterizes its input features", "Development/Old",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A feature collection to rasterize")
IPROCESS_SPEC_END

#define SKIP_PRACTICALLY_GLOBAL_GEOMETRIES 0

namespace
{

//! Tester class
Tester<FeatureRasterizer> gTester;

}

FeatureRasterizer::FeatureRasterizer() :
	m_bMask(false)
{
}

FeatureRasterizer::~FeatureRasterizer()
{
}

void FeatureRasterizer::test()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE FeatureRasterizer::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	if (m_bMask)
	{
		mapAttr["mode"] = "mask";
	}
	else
	{
		mapAttr["mode"] = "filtr";
	}

	return mapAttr;
}

void STDMETHODCALLTYPE FeatureRasterizer::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;
	it = mapAttr.find("mode");

	m_bMask = false;
	if (it != mapAttr.end() && it->second == "mask")
	{
		m_bMask = true;
	}
}

IProcess::eInitStatus FeatureRasterizer::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_spFC = 0;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		IFeatureCollection::iid, (void**) &m_spFC);

	if (!m_spFC)
	{
		PYXTHROW(NullFeatureCollectionException, "Failed to get the feature \
								collection to rasterize from the parameter.");
	}

	// Set up coverage definition
	m_spCovDefn = PYXTableDefinition::create();
	if (m_bMask)
	{
		m_spCovDefn->addFieldDefinition(
			"mask", PYXFieldDefinition::knContextNone, PYXValue::knBool, 1);
	}
	else
	{
		m_spCovDefn->addFieldDefinition(
			"filtr", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);
	}
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue FeatureRasterizer::getCoverageValue(	const PYXIcosIndex& index,
												int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	PYXValue vo;
	std::string str;

	// TODO mlepage needs to edit this code to support mask mode as in getFieldTile

	PYXPointer<FeatureIterator> spFit = m_spFC->getIterator(PYXCell(index));

	for (; !spFit->end(); spFit->next())
	{
		boost::intrusive_ptr<IFeature> spF = spFit->getFeature();
		const std::string& strFID = spF->getID();
#if 0
		// Going to assume that if we get a feature, it intersects this cell.
		// Oops, bad assumption, doesn't seem to work well.
		FIDStr::add(&str, strFID);
#else

		PYXPointer<const PYXGeometry> spGeom = spF->getGeometry();
		assert(spGeom);

		// Intersect geometry with cell
		{
			PYXCell cell(index);
			spGeom = spGeom->intersection(cell);
		}

		PYXPointer<PYXIterator> spIt = spGeom->getIterator();
		for (; !spIt->end(); spIt->next())
		{
			const PYXIcosIndex& index2 = spIt->getIndex();

			// TODO should be able to just use the intersection list without
			// going through each feature's geometry?

			if (index2 == index)
			{
				FIDStr::uncheckedAdd(&str, strFID);
			}
		}
#endif
	}

	if (!str.empty())
	{
		vo.setType(PYXValue::knString);
		vo.getStringPtr(0)->swap(str);
	}

	return vo;
}

bool isPracticallyGlobal(PYXPointer<const PYXGeometry> spGeom)
{
	// Detect global geometry trivially.
	if (dynamic_cast<const PYXGlobalGeometry*>(spGeom.get()))
	{
		return true;
	}

	// Pick a simple cell count threshold.
	const int nRes = 3;
	const int knTotalCellCount = PYXIcosMath::getCellCount(nRes);
	const int knThreshCellCount = static_cast<int>(0.5 * knTotalCellCount);

	// Count cells.
	PYXCell cell;
	int nCellCount = 0;
	for (PYXIcosIterator it(nRes); !it.end(); it.next())
	{
		cell.setIndex(it.getIndex());
		if (spGeom->intersects(cell))
		{
			if (knThreshCellCount <= ++nCellCount)
			{
				return true;
			}
		}
	}

	return false;
}

PYXPointer<PYXValueTile> FeatureRasterizer::getFieldTile(	const PYXIcosIndex& index,
															int nRes,
															int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);
	if (!m_spFC->canRasterize())
	{
		return 0;
	}
	
	// Output value tile
	PYXPointer<PYXValueTile> spVTO;

	if (m_bMask)
	{
#if 0
		PYXValue vtrue(true);
		PYXValue v;

		PYXTile tile(index, nRes);
		PYXPointer<FeatureIterator> spFit = m_spFC->getIterator(tile);
		if (!spFit->end())
		{
			spVTO = PYXValueTile::create(index, nRes, getCoverageDefinition());
		}

		const int knTotalCellCount = tile.getCellCount();
		int nSetCellCount = 0;

		for (bool bDone = false; !bDone && !spFit->end(); spFit->next())
		{
			boost::intrusive_ptr<IFeature> spF = spFit->getFeature();

			PYXExhaustiveIterator it(index, nRes);
			PYXCell cell;

			for (int nOffset = 0; !it.end(); it.next(), ++nOffset)
			{
				if (spVTO->getValue(nOffset, 0, &v))
				{
					// Already set.
					continue;
				}
				cell.setIndex(it.getIndex());
				if (spF->getGeometry()->intersects(cell))
				{
					spVTO->setValue(nOffset, 0, vtrue);
					if (++nSetCellCount == knTotalCellCount)
					{
						bDone = true;
						break;
					}
				}
			}
			bDone = true; // TEMP do only first feature for testing
		}
#else
		PYXValue v; // dummy
		PYXValue vtrue(true);
		PYXPointer<FeatureIterator> spFit = m_spFC->getIterator(PYXTile(index, nRes));

		if (!spFit->end())
		{
			spVTO = PYXValueTile::create(index, nRes, getCoverageDefinition());
		}

		const int knTotalCellCount = PYXIcosMath::getCellCount(index, nRes);
		int nSetCellCount = 0;

		for (; !spFit->end() && nSetCellCount != knTotalCellCount; spFit->next())
		{

			boost::intrusive_ptr<IFeature> spF = spFit->getFeature();
			const std::string& strFID = spF->getID();

			PYXPointer<const PYXGeometry> spGeom = spF->getGeometry();
			assert(spGeom);

#if SKIP_PRACTICALLY_GLOBAL_GEOMETRIES
			// Skip almost-global geometries for now.
			if (isPracticallyGlobal(spGeom))
			{
				continue;
			}
#endif

			bool bCurve = dynamic_cast<const PYXCurve*>(spGeom.get()) ? true : false;

			if (bCurve)
			{
				const PYXCurve * curve = dynamic_cast<const PYXCurve*>(spGeom.get());

				bCurve = curve->getLength() < PYXIcosMath::UnitSphere::calcTileCircumRadius(index);

				/*
				double expectedAmountOfCellsInLine = curve->getLength() / PYXIcosMath::UnitSphere::calcCellCircumRadius(nRes);
				
				//if amount of cells is smaller then amount of cells inside the tile - use the curve iteraotr.
				//else, the amount of cells is bigger then amount of cells inside the tile - do intersection (it will be faster).
				bCurve = expectedAmountOfCellsInLine < knTotalCellCount;
				*/
			}

			if (!bCurve)
			{
				// Intersect geometry with tile
				PYXTile tile(index, nRes);

				PYXPointer<PYXGeometry> intersectionGeom = spGeom->intersection(tile);

				if (!intersectionGeom->isEmpty())
				{
					intersectionGeom->setCellResolution(nRes);
					spGeom = intersectionGeom;
				}
				else
				{
					//it's empty - lets move to the next feature.
					continue;
				}
			}

			PYXPointer<PYXIterator> spIt = spGeom->getIterator();
			for (; !spIt->end(); spIt->next())
			{
				const PYXIcosIndex& index2 = spIt->getIndex();
				if (bCurve && !index.isAncestorOf(index2))
				{
					// TODO 2008-02-04 mlepage when curve intersection is working
					// properly, we won't need this check
					continue;
				}
				int nPos = PYXIcosMath::calcCellPosition(index, index2);
				if (!spVTO->getValue(nPos, 0, &v))
				{
					spVTO->setValue(nPos, 0, vtrue);
					if (++nSetCellCount == knTotalCellCount)
					{
						break;
					}
				}
			}
		}
#endif
	}
	else
	{
		PYXPointer<FeatureIterator> spFit = m_spFC->getIterator(PYXTile(index, nRes));

		if (!spFit->end())
		{
			spVTO = PYXValueTile::create(index, nRes, getCoverageDefinition());
		}

		for (; !spFit->end(); spFit->next())
		{
			boost::intrusive_ptr<IFeature> spF = spFit->getFeature();
			const std::string& strFID = spF->getID();

			PYXPointer<const PYXGeometry> spGeom = spF->getGeometry();
			assert(spGeom);

#if 0
			// Intersect geometry with tile
			{
				PYXTile tile(index, nRes);
				spGeom = spGeom->intersection(tile);
			}

			PYXPointer<PYXIterator> spIt = spGeom->getIterator();
			for (; !spIt->end(); spIt->next())
			{
				const PYXIcosIndex& index2 = spIt->getIndex();

				if (index.isAncestorOf(index2))
				{
					int nPos = PYXIcosMath::calcCellPosition(index, index2);

					PYXValue v = spVTO->getValue(nPos, 0);

					if (v.isNull())
					{
						v.setType(PYXValue::knString);
					}

					FIDStr::uncheckedAdd(v.getStringPtr(0), strFID);
					spVTO->setValue(nPos, 0, v);
				}
			}
#else
			// Try faster(?) method of asking intersects per tile cell
			PYXExhaustiveIterator it(index, nRes);
			PYXCell cell;

			for (; !it.end(); it.next())
			{
				cell.setIndex(it.getIndex());

				if (spGeom->intersects(cell))
				{
					int nPos = PYXIcosMath::calcCellPosition(index, it.getIndex());

					PYXValue v = spVTO->getValue(nPos, 0);

					if (v.isNull())
					{
						v.setType(PYXValue::knString);
					}

					FIDStr::uncheckedAdd(v.getStringPtr(0), strFID);
					spVTO->setValue(nPos, 0, v);
				}
			}
#endif
		}
	}
	return spVTO;
}


////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void FeatureRasterizer::createGeometry() const
{
	// TODO[mlepage] should probably do something more sensible

	if (m_spFC && m_spFC->getGeometry())
	{
		m_spGeom = m_spFC->getGeometry()->clone();
	}
	else
	{
		m_spGeom = PYXGlobalGeometry::create(10);
	}	
}
