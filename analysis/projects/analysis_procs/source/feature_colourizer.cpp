/******************************************************************************
feature_colourizer.cpp

begin		: 2007-06-28
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "feature_colourizer.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>
#include <iostream>

// {593DBFEE-28E7-42ab-9853-FF3946118D6D}
PYXCOM_DEFINE_CLSID(FeatureColourizer, 
0x593dbfee, 0x28e7, 0x42ab, 0x98, 0x53, 0xff, 0x39, 0x46, 0x11, 0x8d, 0x6d);
PYXCOM_CLASS_INTERFACES(FeatureColourizer, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureColourizer, "Feature Colourizer", "A coverage that colourizes its input feature raster", "Development/Old",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Feature Raster", "A rasterized feature to colourize")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<FeatureColourizer> gTester;

uint8_t colours[][3] =
{
	// http://en.wikipedia.org/wiki/Web_colors
	{ 0x00, 0xff, 0xff }, // aqua
	{ 0x00, 0x00, 0x00 }, // black
	{ 0x00, 0x00, 0xff }, // blue
	{ 0xff, 0x00, 0xff }, // fuschia
	{ 0x00, 0x80, 0x00 }, // green
	{ 0x80, 0x80, 0x80 }, // grey
	{ 0x00, 0xff, 0x00 }, // lime
	{ 0x80, 0x00, 0x00 }, // maroon
	{ 0x00, 0x00, 0x80 }, // navy
	{ 0x80, 0x80, 0x00 }, // olive
	{ 0x80, 0x00, 0x80 }, // purple
	{ 0xff, 0x00, 0x00 }, // red
	{ 0xc0, 0xc0, 0xc0 }, // silver
	{ 0x00, 0x80, 0x80 }, // teal
	{ 0xff, 0xff, 0xff }, // white
	{ 0xff, 0xff, 0x00 }  // yellow
};

}

FeatureColourizer::FeatureColourizer() :
	m_nColourIndex(-1)
{
}

FeatureColourizer::~FeatureColourizer()
{
}

void FeatureColourizer::test()
{	
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE FeatureColourizer::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["col_idx"] = StringUtils::toString(m_nColourIndex);

	return mapAttr;
}

void STDMETHODCALLTYPE FeatureColourizer::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;
	it = mapAttr.find("col_idx");

	if (it != mapAttr.end())
	{
		m_nColourIndex = atoi(it->second.c_str());
		if (m_nColourIndex != -1)
		{
			m_nColourIndex = m_nColourIndex % 16;
		}
	}
}

IProcess::eInitStatus FeatureColourizer::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	// Set up input coverage
	m_spCov = 0;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &m_spCov);

	// Set up input feature collection
	m_spFC = 0;

	// TODO[kabiraman]: The feature colourizer should never have to look 
	// at its input's input, m_mapFeaturesRules and any other metadata should 
	// be passed up the process chain.  See 
	// http://euclid:9000/WorldView/ticket/1213.
	boost::intrusive_ptr<IProcess> spProc = getParameter(0)->getValue(0);
	if(spProc->getParameterCount() > 0)
	{
		spProc->getParameter(0)->getValue(0)->getOutput()->QueryInterface(
			IFeatureCollection::iid, (void**) &m_spFC);

		// Set up coverage definition
		m_spCovDefn = PYXTableDefinition::create();
		m_spCovDefn->addFieldDefinition(
			"rgb", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);

		return knInitialized;
	}
	else
	{
		return knFailedToInit;
	}
}

PYXValue FeatureColourizer::getCoverageValue(	const PYXIcosIndex& index,
												int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// Output value
	PYXValue vo;

	// Input value
	PYXValue vi = m_spCov->getCoverageValue(index, nFieldIndex);

	if (!vi.isNull())
	{
		if (m_nColourIndex != -1)
		{
			vo.swap(PYXValue(colours[m_nColourIndex], 3));
		}
		else
		{
			int nCount = FIDStr::count(vi.getStringPtr(0));
			if (nCount == 1)
			{
				std::vector<std::string> vecFID;
				FIDStr::decode(vi.getStringPtr(0), &vecFID);

				colourize(vecFID.front(), vo);
			}
			else
			{
				vo.swap(PYXValue(colours[6], 3));
			}
		}
	}

	return vo;
}

PYXPointer<PYXValueTile> FeatureColourizer::getFieldTile(	const PYXIcosIndex& index,
															int nRes,
															int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// Output value tile
	PYXPointer<PYXValueTile> spVTO;

	// Input value tile
	PYXPointer<PYXValueTile> spVTI = m_spCov->getFieldTile(index, nRes, nFieldIndex);

	if (spVTI)
	{
		spVTO = PYXValueTile::create(index, nRes, getCoverageDefinition());

		PYXValue v;
		std::string str;
		std::vector<std::string> vecFID;

		for (int n = 0; n != spVTI->getNumberOfCells(); ++n)
		{
			v = spVTI->getValue(n, 0);
			if (!v.isNull())
			{
				if (m_nColourIndex != -1)
				{
					v.swap(PYXValue(colours[m_nColourIndex], 3));
				}
				else
				{
					FIDStr::decode(v.getStringPtr(0), &vecFID);

					if (vecFID.size() == 1)
					{
						colourize(vecFID.front(), v);
					}
					else
					{
						v.swap(PYXValue(colours[14], 3)); // 14 is white
					}
				}

				spVTO->setValue(n, 0, v);
			}
		}
	}

	return spVTO;
}

inline void FeatureColourizer::colourize(const std::string& strFeatureID, 
										 PYXValue& v) const
{
	// Colour by feature ID.
	int rgba = StringUtils::toColour(strFeatureID);
	v.swap(PYXValue(reinterpret_cast<uint8_t*>(&rgba) + 1, 3));
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void FeatureColourizer::createGeometry() const
{
	// TODO[mlepage] should probably do something more sensible

	if (m_spCov && m_spCov->getGeometry())
	{
		m_spGeom = m_spCov->getGeometry()->clone();
	}
	else
	{
		m_spGeom = PYXGlobalGeometry::create(10);
	}		
}
