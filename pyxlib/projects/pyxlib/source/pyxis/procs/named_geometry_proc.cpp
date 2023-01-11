/******************************************************************************
named_geometry_proc.cpp

begin      : 10/18/2007 3:45:46 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"

// local includes
#include "pyxlib.h"
#include "named_geometry_proc.h"

// pyxlib includes
#include "pyxis/geometry/geometry_serializer.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

//standard includes
#include <sstream>
#include <iostream>


namespace {
	Tester<NamedGeometryProc> gTester;
}

void NamedGeometryProc::test()
{
	boost::intrusive_ptr<IProcess> spProc(new NamedGeometryProc);
	TEST_ASSERT(spProc);

	PYXPointer<PYXTileCollection> geometry = PYXTileCollection::create();
	geometry->addTile(PYXIcosIndex("A-0002"),15);
	geometry->addTile(PYXIcosIndex("A-0003"),15);
	geometry->addTile(PYXIcosIndex("A-0020"),15);
	geometry->addTile(PYXIcosIndex("A-0030"),15);
	geometry->addTile(PYXIcosIndex("A-0200"),15);
	geometry->addTile(PYXIcosIndex("A-0300"),15);
	geometry->addTile(PYXIcosIndex("A-0202"),15);
	geometry->addTile(PYXIcosIndex("A-0303"),15);

	boost::intrusive_ptr<IWritableFeature> feature = spProc->QueryInterface<IWritableFeature>();

	feature->setGeometry(geometry);
	feature->setID("Feature1");
	feature->setStyle("<style><Icon><IconIndex>1</IconIndex></Icon></style>");
	feature->addField("field1",PYXFieldDefinition::knContextNone,PYXValue::knString,1,PYXValue("Hello"));
	feature->addField("field2",PYXFieldDefinition::knContextElevation,PYXValue::knFloat,1,PYXValue(1.5f));

	TEST_ASSERT(spProc->initProc() == IProcess::knInitialized);

	TEST_ASSERT_EQUAL(feature->getFieldValueByName("field1"),PYXValue("Hello"));
	TEST_ASSERT_EQUAL(feature->getFieldValueByName("field2"),PYXValue(1.5f));
	TEST_ASSERT_EQUAL(feature->getDefinition()->getFieldCount(),2);
	TEST_ASSERT_EQUAL(feature->getDefinition()->getFieldDefinition(0).getContext(),PYXFieldDefinition::knContextNone);
	TEST_ASSERT_EQUAL(feature->getDefinition()->getFieldDefinition(1).getContext(),PYXFieldDefinition::knContextElevation);

	PYXTileCollection resultGeometry;
	feature->getGeometry()->copyTo(&resultGeometry);
	TEST_ASSERT(resultGeometry.isEqual(*geometry));

	std::string serialized;
	PipeManager::writePipelineToString(&serialized,spProc);

	//delete the process object
	feature.reset();
	spProc.reset();

	spProc = PipeManager::readPipelineFromString(serialized);

	TEST_ASSERT(spProc);
	TEST_ASSERT(spProc->initProc() == IProcess::knInitialized);

	boost::intrusive_ptr<IWritableFeature> feature2 = spProc->QueryInterface<IWritableFeature>();

	TEST_ASSERT_EQUAL(feature2->getFieldValueByName("field1"),PYXValue("Hello"));
	TEST_ASSERT_EQUAL(feature2->getFieldValueByName("field2"),PYXValue(1.5f));
	TEST_ASSERT_EQUAL(feature2->getDefinition()->getFieldCount(),2);
	TEST_ASSERT_EQUAL(feature2->getDefinition()->getFieldDefinition(0).getContext(),PYXFieldDefinition::knContextNone);
	TEST_ASSERT_EQUAL(feature2->getDefinition()->getFieldDefinition(1).getContext(),PYXFieldDefinition::knContextElevation);

	PYXTileCollection resultGeometry2;
	feature2->getGeometry()->copyTo(&resultGeometry2);
	TEST_ASSERT(resultGeometry2.isEqual(*geometry));
}

// {2309AE25-FE1A-482f-9642-D2ADD1520629}
PYXCOM_DEFINE_CLSID(NamedGeometryProc, 
0x2309ae25, 0xfe1a, 0x482f, 0x96, 0x42, 0xd2, 0xad, 0xd1, 0x52, 0x6, 0x29);

PYXCOM_CLASS_INTERFACES(NamedGeometryProc, IProcess::iid, IFeature::iid, IWritableFeature::iid, PYXCOM_IUnknown::iid);
IPROCESS_SPEC_BEGIN(NamedGeometryProc, "Region (Binary Format)", "A Single Feature Process. Generate a single process with Metadata and Geometry", "Development/Tools",
					IFeature::iid, IWritableFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

/*!
Set the geometry of the named geometry process. When the geometry is set it is also serialized
out to a file, via the PYXGeometerySerializer::serialize. 

\param spGeom A PYXPointer to the geometry to set for this process.
*/
void NamedGeometryProc::setGeometry(const PYXPointer<PYXGeometry> & spGeom)
{
	if (spGeom)
	{
		m_spGeom = spGeom;
		std::string geomStr = PYXGeometrySerializer::serialize(*spGeom);
		std::ostringstream outStrHex;
		writeHex(outStrHex, geomStr.c_str(), geomStr.length());
		setData(outStrHex.str());
	}
}

IProcess::eInitStatus NamedGeometryProc::initImpl()
{
	if (m_defaultID)
	{
		m_strID = "Named Geometry: " + m_strName + " " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	}

	std::string strData = getData();
	if (!strData.empty())
	{
		std::string strGeom = readHex(strData);
		m_spGeom = PYXGeometrySerializer::deserialize(strGeom);
	}
	else
	{
		setInitProcError<GenericProcInitError>("No geometry stored in the process");
		return knFailedToInit;
	}
	return knInitialized;
}


//! Set the id of this feature.
void STDMETHODCALLTYPE NamedGeometryProc::setID (const std::string & strID)
{
	m_strID = strID;
	m_defaultID = false;
}


/*!
Get the attributes associated with  this process. 

\return a map of standard string - standard string containing the attributes to be serialized.
*/
std::map<std::string, std::string> STDMETHODCALLTYPE NamedGeometryProc::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr.clear();
	mapAttr["GeometryName"] = m_strName;

	if (m_spDefn && m_spDefn->getFieldCount() > 0)
	{
		// note: typo in attribute ID
		mapAttr["TableDefintion"] = XMLUtils::toBase64(StringUtils::toString(*m_spDefn));
		mapAttr["TableValues"] = XMLUtils::toBase64(StringUtils::toString(m_vecValues));
	}

	if (m_strStyle.size() > 0)
	{
		mapAttr["Style"] = m_strStyle;
	}

	if (!m_defaultID)
	{
		mapAttr["FeatureID"] = m_strID;
	}

	return mapAttr;
}

/*!
Set the attributes for the process. 

\param mapAttr A map of standard string - standard string containing the attributes to be set
               for this process.
*/
void NamedGeometryProc::setAttributes(const std::map<std::string,std::string> &mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"GeometryName",m_strName);

	std::string tableDefiniton;
	std::string tableValues;

	// note: typo in attribute ID
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"TableDefintion",tableDefiniton);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"TableValues",tableValues);

	if (tableDefiniton.size() > 0)
	{
		PYXPointer<PYXTableDefinition> newDefinition = PYXTableDefinition::create();
		StringUtils::fromString(XMLUtils::fromBase64(tableDefiniton),newDefinition.get());
		m_spDefn = newDefinition;
		StringUtils::fromString(XMLUtils::fromBase64(tableValues),&m_vecValues);
	}

	m_strStyle = "";
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Style",m_strStyle);

	std::string userGivenID;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FeatureID",userGivenID);

	m_defaultID = userGivenID.size() == 0;
	if (!m_defaultID)
	{
		m_strID = userGivenID;
	}
}


//! Set whether this feature and it's data can be written to and changed.
void STDMETHODCALLTYPE NamedGeometryProc::setIsWritAble(bool bWritable)
{
	//why this thing inside the interface any way... no one never use it...
	m_bWritable = bWritable;
}

//! Set the style that this feature is supposed to be styled with.
void STDMETHODCALLTYPE NamedGeometryProc::setStyle(const std::string & style)
{
	m_strStyle = style;
}

//! Set the definition of the meta data for this feature.
void STDMETHODCALLTYPE NamedGeometryProc::setMetaDataDefinition(const PYXPointer<PYXTableDefinition> & spDef)
{
	//set new definition
	m_spDefn = spDef;
	m_vecValues.resize(m_spDefn->getFieldCount());

	//set default values
	for (int i = 0; i< m_spDefn->getFieldCount(); i++)
	{
		m_vecValues[i] = m_spDefn->getFieldDefinition(i).getTypeCompatibleValue();
	}
}
