/******************************************************************************
resolution_filter.cpp

begin		: June 24, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
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

// {C7F50295-9883-4eb6-BE81-55347C96FAFE}
PYXCOM_DEFINE_CLSID(FeatureCollectionResolutionFilter, 
0xc7f50295, 0x9883, 0x4eb6, 0xbe, 0x81, 0x55, 0x34, 0x7c, 0x96, 0xfa, 0xfe);
PYXCOM_CLASS_INTERFACES(FeatureCollectionResolutionFilter, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureCollectionResolutionFilter, "Feature Collection Resolution Filter", "A feature collection that filters input feature collections based on resolution.", "Development/Broken",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Feature Collection", "A feature collection to filter based on its geometry resolution.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<FeatureCollectionResolutionFilter> gTester;

}

FeatureCollectionResolutionFilter::FeatureCollectionResolutionFilter()
{
	m_nMinAbsRes = -1;
	m_nMaxAbsRes = -1;
	m_nMinRelRes = -1;
	m_nMaxRelRes = -1;
}

FeatureCollectionResolutionFilter::~FeatureCollectionResolutionFilter()
{
}

void FeatureCollectionResolutionFilter::test()
{
	// TODO test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE FeatureCollectionResolutionFilter::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["min_rel_res"] = StringUtils::toString(m_nMinRelRes);
	mapAttr["max_rel_res"] = StringUtils::toString(m_nMaxRelRes);
	mapAttr["min_abs_res"] = StringUtils::toString(m_nMinAbsRes);
	mapAttr["max_abs_res"] = StringUtils::toString(m_nMaxAbsRes);

	return mapAttr;
}

std::string STDMETHODCALLTYPE FeatureCollectionResolutionFilter::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"FeatureCollectionResolutionFilter\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"min_rel_res\" type=\"xs:int\" default=\"" + 
			  StringUtils::toString(m_nMinRelRes) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Minimum Relative Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"max_rel_res\" type=\"xs:int\" default=\"" + 
			  StringUtils::toString(m_nMaxRelRes) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Maximum Relative Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"min_abs_res\" type=\"xs:int\" default=\"" + 
			  StringUtils::toString(m_nMinAbsRes) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Minimum Absolute Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"max_abs_res\" type=\"xs:int\" default=\"" + 
			  StringUtils::toString(m_nMaxAbsRes) + "\">"
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

void STDMETHODCALLTYPE FeatureCollectionResolutionFilter::setAttributes(const std::map<std::string, std::string>& mapAttr)
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

IProcess::eInitStatus FeatureCollectionResolutionFilter::initImpl()
{
	m_spInputFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();

	if (!m_spInputFC)
	{
		return knFailedToInit;
	}

	int nRes = m_spInputFC->getGeometry()->getCellResolution();

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

	m_spGeom = m_spInputFC->getGeometry();
	m_bWritable = m_spInputFC->isWritable();
	m_strID = m_spInputFC->getID();
	m_strStyle = m_spInputFC->getStyle();

	return knInitialized;
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> FeatureCollectionResolutionFilter::getIterator() const
{
	return m_spInputFC->getIterator();
}

PYXPointer<FeatureIterator> FeatureCollectionResolutionFilter::getIterator(const PYXGeometry& geometry) const
{
	int nRes = geometry.getCellResolution();
	if (m_nMinRes <= nRes && nRes <= m_nMaxRes)
	{
		return m_spInputFC->getIterator(geometry);
	}
	else
	{
		std::vector<boost::intrusive_ptr<IFeature> > vecFeatures;
		return createEmptyFeatureIterator(
			vecFeatures.begin(), vecFeatures.end());
	}
}


std::vector<FeatureStyle> FeatureCollectionResolutionFilter::getFeatureStyles() const
{
	return m_spInputFC->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> FeatureCollectionResolutionFilter::getFeature(const std::string& strFeatureID) const
{
	return m_spInputFC->getFeature(strFeatureID);	
}

PYXPointer<const PYXTableDefinition> FeatureCollectionResolutionFilter::getFeatureDefinition() const
{
	return m_spInputFC->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> FeatureCollectionResolutionFilter::getFeatureDefinition()
{
	return m_spInputFC->getFeatureDefinition();
}

bool FeatureCollectionResolutionFilter::canRasterize() const
{
	return m_spInputFC->canRasterize();
}