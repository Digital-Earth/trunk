/******************************************************************************
null_coverage.cpp

begin		: 2006-05-25
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE
#include "null_coverage.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/file_utils.h"

// {DACC06A2-D564-4b2b-ACBE-112B001D213F}
PYXCOM_DEFINE_CLSID(NullCoverage, 
0xdacc06a2, 0xd564, 0x4b2b, 0xac, 0xbe, 0x11, 0x2b, 0x0, 0x1d, 0x21, 0x3f);
PYXCOM_CLASS_INTERFACES(NullCoverage, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(NullCoverage, "Null Value", "A global data set of NULL values", "Development/Diagnostic",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

// Tester class
Tester<NullCoverage> gTester;

// Test method
void NullCoverage::test()
{
	boost::intrusive_ptr<IProcess> spCoverageProcess(new NullCoverage);
	NullCoverage* pCoverage = dynamic_cast<NullCoverage*>(spCoverageProcess.get());
	assert(pCoverage != 0);

	// write the process out
	boost::filesystem::path tempPath = AppServices::makeTempFile(".ppl");
	TRACE_INFO("Example NullCoverage file created: " << FileUtils::pathToString(tempPath));
	PipeManager::writeProcessToFile(FileUtils::pathToString(tempPath), spCoverageProcess);

	// TODO: Read in the process and verify it is the same as the written one
}

const char* const dataTypeStr[] =
{
	"Null_RBG_Value",
	"Null_Elevation_Value"
};

NullCoverage::dataType getDataTypeFromStr(const char* str)
{
	for (int n = 0; n != sizeof(dataTypeStr)/sizeof(dataTypeStr[0]); ++n)
	{
		if (strcmp(dataTypeStr[n], str) == 0)
		{
			return static_cast<NullCoverage::dataType>(n);
		}
	}
	return NullCoverage::knElevation;
}

void NullCoverage::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it;
	it = mapAttr.find("resolution");
	if (it != mapAttr.end())
	{
		m_nResolution = atoi(it->second.c_str());
	}

	it = mapAttr.find("data_type");
	if (it != mapAttr.end())
	{
		m_dataType = getDataTypeFromStr(it->second.c_str());
	}
}

std::map<std::string, std::string> NullCoverage::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["resolution"] = StringUtils::toString(m_nResolution);
	mapAttr["data_type"] = dataTypeStr[m_dataType];
	return mapAttr;
}

IProcess::eInitStatus NullCoverage::initImpl()
{
	// create a new field definition with the selected data type
	
	m_spCovDefn = PYXTableDefinition::create();
	if (m_dataType == NullCoverage::knRGB)
	{
		m_spCovDefn->addFieldDefinition(
			"Null_RGB_Value", 
			PYXFieldDefinition::knContextRGB, 
			PYXValue::knUInt8, 
			3);	
	}
	else
	{
		m_spCovDefn->addFieldDefinition(
			"Null_Elevation_Value", 
			PYXFieldDefinition::knContextElevation, 
			PYXValue::knDouble, 
			1);	
	}

	return IProcess::knInitialized;
}

/*!
Gets a field tile with a completely null cells over the entire cell. Since value tiles 
are called 

\param 	index  		The index that the tile is to be rooted at. 
\param 	nRes	  	The resolution of the tile.
\param	nFieldIndex	The field index that the tile is being quested for. 

\return a pointer to a value tile containing the cells completely null.
*/
PYXPointer<PYXValueTile> NullCoverage::getFieldTile(
	const PYXIcosIndex &index, int nRes, int nFieldIndex) const 
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	if (nFieldIndex == 0)
	{		
		return PYXValueTile::create(index, nRes, m_spCovDefn);
	}
	return PYXPointer<PYXValueTile>();
}

std::string NullCoverage::getAttributeSchema() const
{
	
	
	std::string strXSD = 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">";

    // add the data type
    strXSD +=  "<xs:simpleType name=\"dataType\"><xs:restriction base=\"xs:string\">";
	for (int n = 0; n != sizeof(dataTypeStr)/sizeof(dataTypeStr[0]); ++n)
	{
		strXSD += "<xs:enumeration value=\"";
		strXSD += dataTypeStr[n];
		strXSD += "\" />";
	}
	strXSD += "</xs:restriction></xs:simpleType>";

	// add the resolution type
    strXSD += "<xs:simpleType name=\"resolutionType\">"
                "<xs:restriction base=\"xs:int\">"
                    "<xs:minInclusive value=\"0\" />"
                    "<xs:maxExclusive value=\"40\" />"
                "</xs:restriction>"
            "</xs:simpleType>";

    // build the actual schema
    strXSD += 
		"<xs:element name=\"NullCoverage\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"data_type\" type=\"dataType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Data Type</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"resolution\" type=\"resolutionType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Resolution</friendlyName>"
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

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue NullCoverage::getCoverageValue(	const PYXIcosIndex& index,
											int nFieldIndex	) const
{
	return PYXValue();
}


