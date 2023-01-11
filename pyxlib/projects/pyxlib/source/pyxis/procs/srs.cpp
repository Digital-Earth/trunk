/******************************************************************************
srs.cpp

begin		: 2007-04-16
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/procs/srs.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <algorithm>
#include <cctype>

// {7BE2766C-3B77-44e1-B15F-76D8F6E54D06}
PYXCOM_DEFINE_IID(ISRS, 
0x7be2766c, 0x3b77, 0x44e1, 0xb1, 0x5f, 0x76, 0xd8, 0xf6, 0xe5, 0x4d, 0x6);

// {25D72D8D-B26F-428b-9FDF-408C7C73791D}
PYXCOM_DEFINE_CLSID(SRSProc, 
0x25d72d8d, 0xb26f, 0x428b, 0x9f, 0xdf, 0x40, 0x8c, 0x7c, 0x73, 0x79, 0x1d);
PYXCOM_CLASS_INTERFACES(SRSProc, ISRS::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(SRSProc, "Spatial Reference System", "Defines a 'Spatial Reference System' (datum, projection, etc) for a data file.", "Reader",
					ISRS::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

namespace
{

inline std::string tolower(const std::string& str)
{
	std::string str2;
	str2.resize(str.size());
	std::transform(str.begin(), str.end(), str2.begin(), std::tolower);
	return str2;
}

std::map<PYXSpatialReferenceSystem::eSystem, std::string> mapSystemToStr;
std::map<std::string, PYXSpatialReferenceSystem::eSystem> mapStrToSystem;

std::map<PYXSpatialReferenceSystem::eDatum, std::string> mapDatumToStr;
std::map<std::string, PYXSpatialReferenceSystem::eDatum> mapStrToDatum;

std::map<PYXSpatialReferenceSystem::eProjection, std::string> mapProjectionToStr;
std::map<std::string, PYXSpatialReferenceSystem::eProjection> mapStrToProjection;

struct AutoInit
{
	AutoInit()
	{
		initSystem(PYXSpatialReferenceSystem::knSystemProjected, "projected");
		initSystem(PYXSpatialReferenceSystem::knSystemGeographical, "geographical");
		initSystem(PYXSpatialReferenceSystem::knSystemNone, "none");

		initDatum(PYXSpatialReferenceSystem::knDatumNAD27, "nad27");
		initDatum(PYXSpatialReferenceSystem::knDatumNAD83, "nad83");

		//backward compatibility - it used be wgs73 - and it need to be wgs72 - so we handle both for now.
		initDatum(PYXSpatialReferenceSystem::knDatumWGS72, "wgs73");
		initDatum(PYXSpatialReferenceSystem::knDatumWGS72, "wgs72");
		initDatum(PYXSpatialReferenceSystem::knDatumWGS84, "wgs84");
		initDatum(PYXSpatialReferenceSystem::knDatumNone, "none");

		initProjection(PYXSpatialReferenceSystem::knProjectionUTM, "utm");
		initProjection(PYXSpatialReferenceSystem::knProjectionTM, "tm");
		initProjection(PYXSpatialReferenceSystem::knProjectionLCC, "lcc");
		initProjection(PYXSpatialReferenceSystem::knCustomProjection, "custom");
		initProjection(PYXSpatialReferenceSystem::knProjectionNone, "none");
	}
	void initSystem(PYXSpatialReferenceSystem::eSystem knSystem, const std::string& str)
	{
		mapSystemToStr[knSystem] = str;
		mapStrToSystem[str] = knSystem;
	}
	void initDatum(PYXSpatialReferenceSystem::eDatum knDatum, const std::string& str)
	{
		mapDatumToStr[knDatum] = str;
		mapStrToDatum[str] = knDatum;
	}
	void initProjection(PYXSpatialReferenceSystem::eProjection knProjection, const std::string& str)
	{
		mapProjectionToStr[knProjection] = str;
		mapStrToProjection[str] = knProjection;
	}
} autoInit;

}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE SRSProc::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["system"] = mapSystemToStr[m_spSRS->getSystem()];
	mapAttr["datum"] = mapDatumToStr[m_spSRS->getDatum()];
	mapAttr["projection"] = mapProjectionToStr[m_spSRS->getProjection()];
	if (m_spSRS->getProjection() == PYXSpatialReferenceSystem::knProjectionUTM)
	{
		mapAttr["utm_hemi"] = m_spSRS->getIsUTMNorth() ? "north" : "south";
		mapAttr["utm_zone"] = StringUtils::toString(m_spSRS->getZone());
	}
	return mapAttr;
}

std::string STDMETHODCALLTYPE SRSProc::getAttributeSchema() const
{
	std::string strHemi = "";

	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

	if (m_spSRS->getProjection() == PYXSpatialReferenceSystem::knProjectionUTM)
	{
		std::string strUTMHemiDef = 
			m_spSRS->getIsUTMNorth() ? "north" : "south";

		strHemi = "<xs:element name=\"utm_hemi\" type=\"hemisphereType\" "
			"default=\"" + strUTMHemiDef + "\">";		
	}
	else
	{
		strHemi = "<xs:element name=\"utm_hemi\" type=\"hemisphereType\">";
	}

	strHemi += "<xs:annotation>"
		  "<xs:appinfo>"
			"<friendlyName>UTM Hemisphere</friendlyName>"
			"<description></description>"
		  "</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>";

	// add an enum type for hemisphere
	std::string strHemiType = "<xs:simpleType name=\"hemisphereType\"><xs:"
		"restriction base=\"xs:string\"><xs:enumeration value=\"north\" />"
		"<xs:enumeration value=\"south\" /></xs:restriction></xs:simpleType>";
	strXSD += strHemiType;

	// add an enum type for datum
	std::string strDatumType = "<xs:simpleType name=\"datumType\"><xs:"
		"restriction base=\"xs:string\"><xs:enumeration value=\"nad27\" />"
		"<xs:enumeration value=\"nad83\" /><xs:enumeration value=\"wgs72\""
		" /><xs:enumeration value=\"wgs84\" /><xs:enumeration value=\"none\" "
		"/></xs:restriction></xs:simpleType>";
	strXSD += strDatumType;

	// add an enum type for projection
	std::string strProjectionType = "<xs:simpleType name=\"projectionType\">"
		"<xs:restriction base=\"xs:string\"><xs:enumeration value=\"utm\" />"
		"<xs:enumeration value=\"tm\" /><xs:enumeration value=\"lcc\" /><xs:"
		"enumeration value=\"none\" /></xs:restriction></xs:simpleType>";
	strXSD += strProjectionType;

	// add an enum type for system
	std::string strSystemType = "<xs:simpleType name=\"systemType\"><xs:"
		"restriction base=\"xs:string\"><xs:enumeration value=\"projected\" "
		"/><xs:enumeration value=\"geographical\" /></xs:restriction>"
		"</xs:simpleType>";
	strXSD += strSystemType;

	// add an enum type for utm zone
	std::string strUTMZoneType = "<xs:simpleType name=\"utmZoneType\"><xs:"
		"restriction base=\"xs:string\">";

	for(int i = 1; i < 61; ++i)
	{
		strUTMZoneType += "<xs:enumeration value=\"" + intToString(i, 0) 
			+ "\" />";
	}
		
	strUTMZoneType += "</xs:restriction></xs:simpleType>";
	strXSD += strUTMZoneType;

	strXSD += "<xs:element name=\"SRS\">"
	  "<xs:complexType>"
		"<xs:sequence>"
		  "<xs:element name=\"system\" type=\"systemType\">"
			"<xs:annotation>"
			  "<xs:appinfo>"
				"<friendlyName>System</friendlyName>"
				"<description></description>"
			  "</xs:appinfo>"
			"</xs:annotation>"
		  "</xs:element>"
		  "<xs:element name=\"datum\" type=\"datumType\">"
			"<xs:annotation>"
			  "<xs:appinfo>"
				"<friendlyName>Datum</friendlyName>"
				"<description></description>"
			  "</xs:appinfo>"
			"</xs:annotation>"
		  "</xs:element>"
		  "<xs:element name=\"projection\" type=\"projectionType\">"
			"<xs:annotation>"
			  "<xs:appinfo>"
				"<friendlyName>Projection</friendlyName>"
				"<description></description>"
			  "</xs:appinfo>"
			"</xs:annotation>"
		  "</xs:element>";
		  
		strXSD += strHemi;
		strXSD += "<xs:element name=\"utm_zone\" type=\"utmZoneType\" default=\"" + 
			StringUtils::toString(m_spSRS->getZone()) + "\">"
			"<xs:annotation>"
			  "<xs:appinfo>"
				"<friendlyName>Projection</friendlyName>"
				"<description></description>"
			  "</xs:appinfo>"
			"</xs:annotation>"
		  "</xs:element>"
		"</xs:sequence>"		  
	  "</xs:complexType>"
	  "</xs:element>"
	  "</xs:schema>";

	return strXSD;	
}

void STDMETHODCALLTYPE SRSProc::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;
	if ((it = mapAttr.find("system")) != mapAttr.end())
	{
		if (mapStrToSystem.find(tolower(it->second)) == mapStrToSystem.end())
		{
			PYXTHROW(PYXException, "unrecognized system");
		}
		m_spSRS->setSystem(mapStrToSystem[tolower(it->second)]);
	}
	if ((it = mapAttr.find("datum")) != mapAttr.end())
	{
		if (mapStrToDatum.find(tolower(it->second)) == mapStrToDatum.end())
		{
			PYXTHROW(PYXException, "unrecognized datum");
		}
		m_spSRS->setDatum(mapStrToDatum[tolower(it->second)]);
	}
	if ((it = mapAttr.find("projection")) != mapAttr.end())
	{
		if (mapStrToProjection.find(tolower(it->second)) == mapStrToProjection.end())
		{
			PYXTHROW(PYXException, "unrecognized projection");
		}
		m_spSRS->setProjection(mapStrToProjection[tolower(it->second)]);
	}
	if ((it = mapAttr.find("custom_projection")) != mapAttr.end())
	{
		setData(it->second);
	}
	if (m_spSRS->getSystem() == PYXSpatialReferenceSystem::knSystemProjected
		&& m_spSRS->getProjection() == PYXSpatialReferenceSystem::knProjectionUTM)
	{
		bool bNorth = true;
		if ((it = mapAttr.find("utm_hemi")) != mapAttr.end())
		{
			if (tolower(it->second) == "north")
			{
				// already true
			}
			else if (tolower(it->second) == "south")
			{
				bNorth = false;
			}
			else
			{
				PYXTHROW(PYXException, "unrecognized utm_hemi");
			}
		}
		m_spSRS->setIsUTMNorth(bNorth);

		int nZone = 0;
		if ((it = mapAttr.find("utm_zone")) != mapAttr.end())
		{
			nZone = atoi(it->second.c_str());
		}
		if (nZone < 1 || 60 < nZone)
		{
			PYXTHROW(PYXException, "unrecognized zone");
		}
		m_spSRS->setZone(nZone);
	}
}

std::string STDMETHODCALLTYPE SRSProc::getData() const
{
	return m_spSRS->getWKT();
}

void STDMETHODCALLTYPE SRSProc::setData(const std::string& strData)
{
	// TODO: just store the string, the user must init to take effect
	m_initState = IProcess::knNeedsInit;
	m_spSRS->setWKT(strData);
}
