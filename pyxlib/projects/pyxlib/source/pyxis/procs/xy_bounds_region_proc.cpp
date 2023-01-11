/******************************************************************************
xy_bounds_region_proc.cpp

begin		: 2011-03-22
copyright	: (C) 2011 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"

#include "pyxlib.h"
#include "xy_bounds_region_proc.h"

#include "pyxis/region/xy_bounds_region.h"
#include "pyxis/geometry/vector_geometry.h"

// {69D60C62-9666-43bf-994E-D8574D050A91}
PYXCOM_DEFINE_CLSID(XYBoundsRegionProc, 
0x69d60c62, 0x9666, 0x43bf, 0x99, 0x4e, 0xd8, 0x57, 0x4d, 0x05, 0x0a, 0x91);

PYXCOM_CLASS_INTERFACES(XYBoundsRegionProc, IProcess::iid, IFeature::iid, PYXCOM_IUnknown::iid);
IPROCESS_SPEC_BEGIN(XYBoundsRegionProc, "Rectangle Region", "A single feature process whose region is a rectangle defined by the attributes.", "Utility",
	IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

////////////////////////////////////////////////////////////////////////////////
// ProcessImpl
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus XYBoundsRegionProc::initImpl()
{
	PYXPointer<PYXXYBoundsRegion> region = PYXXYBoundsRegion::create(
												PYXRect2DDouble(m_xMin, m_yMin, m_xMax, m_yMax),
												m_converter);

	m_spGeom = PYXVectorGeometry::create(region,16);

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::string XYBoundsRegionProc::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"XYBoundsRegionProc\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"xMin\" type=\"xs:double\" default=\"0\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>X Minimum</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"yMin\" type=\"xs:double\" default=\"0\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Y Minimum</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"xMax\" type=\"xs:double\" default=\"0\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>X Maximum</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"yMax\" type=\"xs:double\" default=\"0\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Y Maximum</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

std::map< std::string, std::string > XYBoundsRegionProc::getAttributes() const
{
	std::map< std::string, std::string > mapAttr;

	mapAttr["xMin"] = StringUtils::toString(m_xMin);
	mapAttr["yMin"] = StringUtils::toString(m_yMin);
	mapAttr["xMax"] = StringUtils::toString(m_xMax);
	mapAttr["yMax"] = StringUtils::toString(m_yMax);

	return mapAttr;
}

void XYBoundsRegionProc::setAttributes(
	std::map< std::string, std::string > const & mapAttr)
{
	std::map< std::string, std::string >::const_iterator it;

	it = mapAttr.find("xMin");
	m_xMin = (it == mapAttr.end()) ? 0 : StringUtils::fromString< int >(it->second);
	it = mapAttr.find("yMin");
	m_yMin = (it == mapAttr.end()) ? 0 : StringUtils::fromString< int >(it->second);
	it = mapAttr.find("xMax");
	m_xMax = (it == mapAttr.end()) ? 0 : StringUtils::fromString< int >(it->second);
	it = mapAttr.find("yMax");
	m_yMax = (it == mapAttr.end()) ? 0 : StringUtils::fromString< int >(it->second);

	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;
}

////////////////////////////////////////////////////////////////////////////////
// XYBoundsRegionProc
////////////////////////////////////////////////////////////////////////////////

XYBoundsRegionProc::XYBoundsRegionProc() :
m_xMin(0), m_yMin(0), m_xMax(0), m_yMax(0)
{}
