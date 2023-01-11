/******************************************************************************
checker_coverage.cpp

begin      : 21/03/2007 11:00:31 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE
#include "checker_coverage.h" 

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"

// boost includes
#include <boost/filesystem/path.hpp>

// {1716ED04-9238-42f6-9CC2-C0749A828CA8}
PYXCOM_DEFINE_CLSID(CheckerCoverage,
0x1716ed04, 0x9238, 0x42f6, 0x9c, 0xc2, 0xc0, 0x74, 0x9a, 0x82, 0x8c, 0xa8);
PYXCOM_CLASS_INTERFACES(CheckerCoverage, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(CheckerCoverage, "Checkerboard Coverage", "A global data set that returns different colours for vertex, origin and pentagon cells.", "Development/Diagnostic",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

// Tester class
Tester<CheckerCoverage> gTester;

// Test method
void CheckerCoverage::test()
{
	boost::intrusive_ptr<IProcess> spProc(new CheckerCoverage);
	CheckerCoverage* pCov = dynamic_cast<CheckerCoverage*>(spProc.get());
	TEST_ASSERT(pCov != 0);

	CheckerCoverage testCoverage;
	TEST_ASSERT(testCoverage == *pCov);
	TEST_ASSERT(testCoverage.getOriginChildValue() == pCov->getOriginChildValue());
	TEST_ASSERT(testCoverage.getVertexChildValue() == pCov->getVertexChildValue());
	TEST_ASSERT(testCoverage.getPentagonValue() == pCov->getPentagonValue());

	PYXIcosIndex pentagonIndex("1-000");
	PYXIcosIndex vertexIndex("1-0002");
	PYXIcosIndex originIndex("1-00020");

	TEST_ASSERT(pCov->getCoverageValue(pentagonIndex) == pCov->getPentagonValue());
	TEST_ASSERT(pCov->getCoverageValue(vertexIndex) == pCov->getVertexChildValue());
	TEST_ASSERT(pCov->getCoverageValue(originIndex) == pCov->getOriginChildValue());

	// write out a sample process
	boost::filesystem::path tempPath = AppServices::makeTempFile(".ppl");
	TRACE_INFO("Example Checkerboard Coverage file created: " << FileUtils::pathToString(tempPath));
	PipeManager::writeProcessToFile(FileUtils::pathToString(tempPath), spProc);

	// TODO: Verify the process is read in from file correctly.
	
	int nNotAColour = 9;
	PYXValue invalidValue(nNotAColour);
	TEST_ASSERT(!pCov->setOriginChildValue(invalidValue));
	TEST_ASSERT(testCoverage == *pCov);

	unsigned char nValue1[3] = {12, 255, 55};
	PYXValue originValue(nValue1, 3);
	TEST_ASSERT(pCov->setOriginChildValue(originValue));
	TEST_ASSERT(testCoverage != *pCov);
	
	unsigned char nValue2[3] = {7, 255, 99};
	PYXValue vertexValue(nValue2, 3);
	TEST_ASSERT(pCov->setVertexChildValue(vertexValue));

	unsigned char nValue3[3] = {11, 255, 111};
	PYXValue pentagonValue(nValue3, 3);
	TEST_ASSERT(pCov->setPentagonValue(pentagonValue));

	TEST_ASSERT(pCov->getCoverageValue(pentagonIndex) == pentagonValue);
	TEST_ASSERT(pCov->getCoverageValue(vertexIndex) == vertexValue);
	TEST_ASSERT(pCov->getCoverageValue(originIndex) == originValue);
}

/*!
Creates a single channel constant coverage at resolution '10'  that returns an
array of 3 unsigend characters (RGB) with values of '42'.
*/
CheckerCoverage::CheckerCoverage() :
	m_nResolution(10)
{

	// TODO: Enhance the coverage to allow different contexts and types (like const coverage).
	// create the coverage definition
	m_spCovDefn->addFieldDefinition(
		"colour", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);

	// initialize the feature definition.
	m_strID = "Checkerboard Coverage: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// default green pentagons
	unsigned char nValue[3] = {0, 255, 0};
	m_pentagonColour = PYXValue(nValue, 3);

	// default red origin child cells
	nValue[1] = 0;
	nValue[0] = 255;
	m_originColour = PYXValue(nValue, 3);

	// default blue vertex child cells
	nValue[0] = 0;
	nValue[2] = 255;
	m_vertexColour = PYXValue(nValue, 3);

	setGeometryResolution(m_nResolution);
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue CheckerCoverage::getCoverageValue(	const PYXIcosIndex& index,
											int nFieldIndex	) const
{
	if (nFieldIndex != 0)
	{
		PYXTHROW(
			CheckerCoverageException, 
			"Invalid field index '" << 
			nFieldIndex << 
			"'. Only '0' is allowed.");
	}

	if (!index.hasVertexChildren())
	{
		return m_vertexColour;
	}
	else if (index.isPentagon())
	{
		return m_pentagonColour;
	}

	return m_originColour;
}

/*
Gets a field tile. Constructs a field tile of the desired size and depth as requested rooted at the index specified. 
As we iterate over all of the indices of the tile the tile's values get coloured according to whether the cell is a
vertex/pentagon/orgin cell. 

\param 	index  		The index that the tile is to be rooted at. 
\param 	nRes	    The resolution of the tile.
\param	nFieldIndex	The field index that the tile is being quested for. 

\return A pointer to a value tile containing the cells appropriately coloured. 
*/
PYXPointer<PYXValueTile> CheckerCoverage::getFieldTile(const PYXIcosIndex& index, int nRes, int nFieldIndex) const
{
	boost::recursive_mutex::scoped_lock lock (m_coverageMutex);

	PYXPointer<PYXTableDefinition> spCovDef = PYXTableDefinition::create();
	spCovDef->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spOutTile = PYXValueTile::create(index, nRes, spCovDef);

	for (PYXPointer<PYXIterator> spIt = spOutTile->getIterator(); !spIt->end(); spIt->next())
	{
		PYXIcosIndex index = spIt->getIndex();
		if (index.hasVertexChildren())
		{
			spOutTile->setValue(index, nFieldIndex, m_vertexColour);
			continue;
		}
		else if (index.isPentagon())
		{
			spOutTile->setValue(index, nFieldIndex, m_pentagonColour);
			continue;
		}
		else
		{
			spOutTile->setValue(index, nFieldIndex, m_originColour);
			continue;
		}
	}
	return spOutTile;
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::string STDMETHODCALLTYPE CheckerCoverage::getAttributeSchema() const
{
 	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"colour\">"
			"<xs:restriction base=\"xs:string\">"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"CheckerCoverage\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"resolution\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"pentagonColour\" type=\"colour\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Pentagon Colour</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"originColour\" type=\"colour\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Origin Colour</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"vertexColour\" type=\"colour\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Vertex Colour</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

std::map<std::string, std::string> CheckerCoverage::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["resolution"] = StringUtils::toString(m_nResolution);

	std::ostringstream ost;
	ost << m_pentagonColour; 
	mapAttr["pentagonColour"] = ost.str();
	ost.str("");
	ost << m_originColour; 
	mapAttr["originColour"] = ost.str();
	ost.str("");
	ost << m_vertexColour; 
	mapAttr["vertexColour"] = ost.str();
	return mapAttr;
}

void CheckerCoverage::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;
	it = mapAttr.find("resolution");
	if (it != mapAttr.end())
	{
		setGeometryResolution(atoi(it->second.c_str()));
	}
	else
	{
		PYXTHROW(CheckerCoverageException, "Cant find resolution in attributes.");
	}
	it = mapAttr.find("originColour");
	if (it != mapAttr.end())
	{
		StringUtils::fromString(it->second, &m_originColour);
		assert(isColour(m_originColour));
	}
	else
	{
		PYXTHROW(CheckerCoverageException, "Cant find origin colour in attributes.");
	}
	it = mapAttr.find("pentagonColour");
	if (it != mapAttr.end())
	{
		StringUtils::fromString(it->second, &m_pentagonColour);
	}
	else
	{
		PYXTHROW(CheckerCoverageException, "Cant find pentagon colour in attributes.");
	}
	it = mapAttr.find("vertexColour");
	if (it != mapAttr.end())
	{
		StringUtils::fromString(it->second, &m_vertexColour);
	}
	else
	{
		PYXTHROW(CheckerCoverageException, "Cant find vertex colour in attributes.");
	}
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

/*!
Sets the PYXValue that is returned for every origin child cell that is
requested.

\param value	The colour value. Must be an array of 3 unsigned characters.

\return true if the value was changed, otherwise false.
*/
bool CheckerCoverage::setOriginChildValue(const PYXValue& value)
{
	if (isColour(value))
	{
		m_originColour = value;
		return true;
	}
	return false;
}

/*!
Sets the PYXValue that is returned for every vertex child cell that is
requested.

\param value	The colour value. Must be an array of 3 unsigned characters.

\return true if the value was changed, otherwise false.
*/
bool CheckerCoverage::setVertexChildValue(const PYXValue& value)
{
	if (isColour(value))
	{
		m_vertexColour = value;
		return true;
	}
	return false;
}

/*!
Sets the PYXValue that is returned for every pentagon cell that is
requested.

\param value	The colour value. Must be an array of 3 unsigned characters.

\return true if the value was changed, otherwise false.
*/
bool CheckerCoverage::setPentagonValue(const PYXValue& value)
{
	if (isColour(value))
	{
		m_pentagonColour = value;
		return true;
	}
	return false;
}

/*!
Verify that the passed value is in a form that can be displayed as a colour.

\param value	The colour value to verify.
*/
bool CheckerCoverage::isColour(const PYXValue& value)
{
	if (value.getArraySize() == 3 && value.getArrayType() == PYXValue::knUInt8)
	{
		return true;
	}
	return false;
}

//! The equality operator.
bool operator ==(const CheckerCoverage& lhs, const CheckerCoverage& rhs)
{
	if (	lhs.getProcVersion() == rhs.getProcVersion() &&
			lhs.m_nResolution == rhs.m_nResolution &&
			lhs.m_pentagonColour == rhs.m_pentagonColour &&
			lhs.m_originColour == rhs.m_originColour &&
			lhs.m_vertexColour == rhs.m_vertexColour )
	{
		return true;
	}
	return false;
}

//! The inequality operator.
bool operator !=(const CheckerCoverage& lhs, const CheckerCoverage& rhs)
{
	return !(lhs == rhs);
}
