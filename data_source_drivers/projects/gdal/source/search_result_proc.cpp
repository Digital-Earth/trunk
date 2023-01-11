/******************************************************************************
search_result_proc.cpp

begin		: 2008-06-09
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "search_result_proc.h"

// local includes
#include "coord_converter_impl.h"

//publib includes
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/utility/string_utils.h"


// {6F576501-FAD1-46d7-A360-CF6A35769445}
PYXCOM_DEFINE_CLSID(SearchResultProcess, 
0x6f576501, 0xfad1, 0x46d7, 0xa3, 0x60, 0xcf, 0x6a, 0x35, 0x76, 0x94, 0x45);

PYXCOM_CLASS_INTERFACES(SearchResultProcess, IProcess::iid, IFeature::iid, 
						IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(SearchResultProcess, "Search Result", 
					"Outputs a feature which is the result of a search result.", "Utility", 
					IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

//TODO: Unit test

///////////////////////////////////////////////////////////////////////////////
// IProcess
///////////////////////////////////////////////////////////////////////////////

//! Get the output of this process.
boost::intrusive_ptr<const PYXCOM_IUnknown> SearchResultProcess::getOutput() const
{
	return reinterpret_cast<const PYXCOM_IUnknown*>(m_spOutputFeature.get());
}

//! Get the output of this process.
boost::intrusive_ptr<PYXCOM_IUnknown> SearchResultProcess::getOutput()
{
	return reinterpret_cast<PYXCOM_IUnknown *>(m_spOutputFeature.get());
}

/*!
Gets attribute schema of this process. An empty attribute schema is returned 
to prevent editing of this process' attributes. 

\return An empty attribute schema.
*/
std::string SearchResultProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"SearchResultProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"ResultName\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Search result item title</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"ResultDescription\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Search result item description</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"ResultUrl\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Search result item Url</friendlyName>"
					"<description>An Url for additional information about the search result item</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"Latitude\" type=\"xs:double\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Latitude</friendlyName>"
					"<description>Latitude in WGS84</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"Longitude\" type=\"xs:double\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Longitude</friendlyName>"
					"<description>Longitude in WGS84</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"FeatId\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Search result ID</friendlyName>"
					"<description>a unique ID for this search result</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}


/*!
Collects all the attributes associated with this process and returns them as 
key-value pairs for serialization of the attributes with this process. 

\return A map of key-value pairs containing all the attributes of the process.
*/
std::map<std::string, std::string> SearchResultProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr.clear();
	mapAttr["Latitude"] = StringUtils::toString(m_fLat);
	mapAttr["Longitude"] = StringUtils::toString(m_fLon);
	mapAttr["Writable"] = m_bWritable ? "true" : "false";
	mapAttr["Style"] = m_strStyle;
	mapAttr["FeatId"] = m_strID;
	mapAttr["ResultName"] = m_strName;
	mapAttr["ResultDescription"] = m_strDesc;
	mapAttr["ResultUrl"] = m_strUrl;
	return mapAttr;
}


/*!
Sets the attributes of this process, in order to get this process ready for 
initialization.

\param mapAttr  A key-value pairing of this process' attribute to value settings.
*/
void SearchResultProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator mapIt = mapAttr.find("Writable");

	if (mapIt != mapAttr.end())
	{
		std::string strWrit = mapIt->second;
		m_bWritable = strWrit.compare("true") == 0 ? true : false;
	}

	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"Latitude",double,m_fLat);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"Longitude",double,m_fLon);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Style",m_strStyle);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FeatId",m_strID);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"ResultName",m_strName);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"ResultDescription",m_strDesc);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"ResultUrl",m_strUrl);
}

/*!
Initalizes this process, during initalization the process' attributes are read 
and the feature this process outputs is constructed. If a WritableFeature can 
be successfully constructed from the attributes then the process has been 
successfully initialized.  If a WritableFeature cannot be successfully 
constructed or an exception occurs during construction then the process is not 
successfully constructed.

\return IProcess::eInitStatus indicating that initialization was successful or not.
*/
IProcess::eInitStatus SearchResultProcess::initImpl()
{
	m_initState = knInitializing;
	try
	{
		if (!m_spGeom)
		{
			PYXIcosIndex index; 
			PYXPointer<PYXSpatialReferenceSystem> spSRS = 
				PYXSpatialReferenceSystem::create();
			spSRS->setDatum(PYXSpatialReferenceSystem::knDatumWGS84);
			spSRS->setProjection(PYXSpatialReferenceSystem::knProjectionNone);
			spSRS->setSystem(PYXSpatialReferenceSystem::knSystemGeographical);
			
			boost::intrusive_ptr<CoordConverterImpl> spCoordConv =
				boost::intrusive_ptr<CoordConverterImpl>(new CoordConverterImpl());
			spCoordConv->initialize(const_cast<const PYXSpatialReferenceSystem&>(*spSRS));
			PYXCoord2DDouble coord(m_fLon, m_fLat);
			spCoordConv->nativeToPYXIS(coord, &index, 25);
			
			m_spGeom = PYXCell::create(index)->clone();
		}

		PYXPointer<PYXTableDefinition> spDef = PYXTableDefinition::create();
		spDef->addFieldDefinition(
			"name", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);

		spDef->addFieldDefinition(
			"description", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);

		spDef->addFieldDefinition(
			"url", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);

		m_spOutputFeature = boost::intrusive_ptr<WritableSearchFeature>(
			new WritableSearchFeature(
			m_spGeom, m_strID, m_strStyle, m_bWritable, spDef));
		m_spOutputFeature->setFieldValueByName(PYXValue(m_strUrl), "url");
		m_spOutputFeature->setFieldValueByName(PYXValue(m_strDesc), "description");
		m_spOutputFeature->setFieldValueByName(PYXValue(m_strName), "name");

		if (!m_spOutputFeature)
		{
			return IProcess::knFailedToInit;
		}
	}
	catch(PYXException&)
	{
		return IProcess::knFailedToInit;
	}

	return IProcess::knInitialized;
}

///////////////////////////////////////////////////////////////////////////////
// IRecord
///////////////////////////////////////////////////////////////////////////////

PYXPointer<const PYXTableDefinition> SearchResultProcess::getDefinition() const
{
	return m_spOutputFeature->getDefinition();
}

PYXPointer<PYXTableDefinition> SearchResultProcess::getDefinition()
{
	return m_spOutputFeature->getDefinition();
}

PYXValue SearchResultProcess::getFieldValue(int nFieldIndex) const
{
	return m_spOutputFeature->getFieldValue(nFieldIndex);
}

void SearchResultProcess::setFieldValue(PYXValue value, int nFieldIndex)
{
	return m_spOutputFeature->setFieldValue(value, nFieldIndex);
}

PYXValue SearchResultProcess::getFieldValueByName(const std::string& strName) const
{
	return m_spOutputFeature->getFieldValueByName(strName);
}

void SearchResultProcess::setFieldValueByName(PYXValue value, const std::string& strName)
{
	return m_spOutputFeature->setFieldValueByName(value, strName);
}

std::vector<PYXValue> SearchResultProcess::getFieldValues() const
{
	return m_spOutputFeature->getFieldValues();
}

void SearchResultProcess::setFieldValues(const std::vector<PYXValue>& vecValues)
{
	return m_spOutputFeature->setFieldValues(vecValues);
}

void SearchResultProcess::addField(const std::string& strName,
								PYXFieldDefinition::eContextType nContext,
								PYXValue::eType nType,
								int nCount,
								PYXValue value
								)
{
	return m_spOutputFeature->addField(strName,
		nContext,
		nType,
		nCount,
		value
		);
}