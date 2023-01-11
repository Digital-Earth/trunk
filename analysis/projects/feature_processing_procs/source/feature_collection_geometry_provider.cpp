/******************************************************************************
feature_collection_geometry-provider.cpp

begin		: 2013-6-13
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "stdafx.h"
#include "feature_collection_geometry_provider.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/feature_collection_index_proc.h"
#include "pyxis/pipe/pipe_utils.h"

// {BFFF86DE-90F9-4EA6-9BC1-60075FD91E79}
PYXCOM_DEFINE_CLSID(FeatureCollectionGeometryProvider, 
					0xbfff86de, 0x90f9, 0x4ea6, 0x9b, 0xc1, 0x60, 0x7, 0x5f, 0xd9, 0x1e, 0x79);


PYXCOM_CLASS_INTERFACES(FeatureCollectionGeometryProvider, IGeometryProvider::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureCollectionGeometryProvider, "Feature Collection Geometry Provider", "Provides a geometry for a given IRecord based on the input feature collection.", "Analysis/Features/Geotagging",
					IGeometryProvider::iid, PYXCOM_IUnknown::iid)
					IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Feature Collection", "A collection of features based on which a geometry is provided for a given record.")

					IPROCESS_SPEC_END

					IProcess::eInitStatus FeatureCollectionGeometryProvider::initImpl()
{
	if(m_featureFieldIndices.size() == 0  && m_featureFieldIndices.size() == 0)
	{
		setInitProcError<GenericProcInitError>("At least one index should be provided to match a record with a feature");
		return knFailedToInit;
	}
	if(m_featureFieldIndices.size() != m_featureFieldIndices.size()) 
	{
		setInitProcError<GenericProcInitError>("Number of feature indices and record indices do not match");
		return knFailedToInit;
	}

	return knInitialized;
}



std::map<std::string, std::string> STDMETHODCALLTYPE FeatureCollectionGeometryProvider::getAttributes() const
{

	std::map<std::string, std::string> mapAttr;
	std::string fields;

	vectorToString(m_recordFieldIndices, fields);
	mapAttr["RecordFieldIndices"] = fields;

	vectorToString(m_featureFieldIndices, fields);
	mapAttr["FeatureFieldsIndices"] = fields;

	return mapAttr;
}

PYXPointer<PYXGeometry> STDMETHODCALLTYPE FeatureCollectionGeometryProvider::getGeometry(boost::intrusive_ptr<IRecord> & record) const
{
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		if(!m_featureCollectionIndex)
		{
			//creating index for the the input feature collection
			auto indexProcessFactory = ProcessInitHelper("AFE6F82A-8E82-41CA-9764-B45DA5264D76"); //feature collection index process
			indexProcessFactory.addInput(0,getParameter(0)->getValue(0));
			indexProcessFactory.setAttribute("FieldsIndices", StringUtils::toString(m_featureFieldIndices[0])); 
			m_featureCollectionIndex = indexProcessFactory.getProcess(true)->getOutput()->QueryInterface<IFeatureCollectionIndex>();
		}
	}

	// we might have several fields but FeatureCollectionindex doesn know about fields 
	// so we search one and them make sure the others also match. 
	// this can lead to bad cases where index return huge amount of features which we need to later check linearly.

	auto it = m_featureCollectionIndex->getIterator(record->getFieldValue(m_recordFieldIndices[0]));

	for(; !it->end(); it->next())
	{
		auto feature = it->getFeature();
		bool featureMatches = true;
		for (unsigned int i = 0; i < m_featureFieldIndices.size(); i++)
		{
			if(feature->getFieldValue(m_featureFieldIndices[i]).compare(record->getFieldValue(m_recordFieldIndices[i])) != 0)
			{
				featureMatches = false;
				break;
			}
		}
		if(featureMatches)
		{
			return  feature->getGeometry();
		}
	}
	return  NULL;
}

void STDMETHODCALLTYPE FeatureCollectionGeometryProvider::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::string fields;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FeatureFieldsIndices",fields);
	stringToVector(fields,m_featureFieldIndices);

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"RecordFieldIndices",fields);
	stringToVector(fields,m_recordFieldIndices);
}


std::string STDMETHODCALLTYPE FeatureCollectionGeometryProvider::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		"<xs:element name=\"FeatureCollectionGeometryProvider\">"
		"<xs:complexType>"
		"<xs:sequence>"

		"<xs:element name=\"FeatureFieldsIndices\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Feature Fields Indices</friendlyName>"
		"<description>List of fields Indices of the feature collection that should match the specified fields of the record</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"RecordFieldIndices\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Record Fields Indices</friendlyName>"
		"<description>List of fields Indices ofthe record that should match the specified fields of the feature collection</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";
}

void FeatureCollectionGeometryProvider::stringToVector(const std::string & input,std::vector<int> & output) const
{
	output.clear();

	std::stringstream fieldsStream(input);

	while(fieldsStream.good())
	{
		int field = -1;
		fieldsStream >> field;

		if (field>=0)
		{
			output.push_back(field);
		}
	}
}

void FeatureCollectionGeometryProvider::vectorToString(const std::vector<int>  & input, std::string & output) const
{
	output = "";
	for(auto & fieldIndex : input)
	{
		output += StringUtils::toString(fieldIndex) + " ";
	}
	output = StringUtils::trimRight(output);
}
