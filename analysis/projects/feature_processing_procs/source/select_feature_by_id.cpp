/******************************************************************************
select_feature_by_id.cpp

begin		: 2013-04-18
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "select_feature_by_id.h"

#include "feature_collection_process.h"

// Required by tests
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/procs/default_feature.h"

// {D6B564E3-6D66-40A4-AB5F-F7D068568D31}
PYXCOM_DEFINE_CLSID(SelectFeatureByIdProcess,
0xd6b564e3, 0x6d66, 0x40a4, 0xab, 0x5f, 0xf7, 0xd0, 0x68, 0x56, 0x8d, 0x31);

PYXCOM_CLASS_INTERFACES(SelectFeatureByIdProcess, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(SelectFeatureByIdProcess, "Select Feature By Id", "extract a single feature from a feature collection by the feature ID.", "Analysis/Features",
					IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)	
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A collection of features to select a feature from.")
IPROCESS_SPEC_END


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE SelectFeatureByIdProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["FeatureID"] = m_featureID;
	return mapAttr;
}

std::string STDMETHODCALLTYPE SelectFeatureByIdProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"SelectFeatureByIdProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"

				  "<xs:element name=\"FeatureID\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>FeatureID</friendlyName>"
						"<description>Feature ID to extract from the input feature collection.</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE SelectFeatureByIdProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"FeatureID",m_featureID);	
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;	
}

//! Initialize the process.
IProcess::eInitStatus SelectFeatureByIdProcess::initImpl()
{
	boost::intrusive_ptr<IFeatureCollection> inputFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();

	m_outputFeature = inputFC->getFeature(m_featureID);

	if (!m_outputFeature)
	{
		setInitProcError<GenericProcInitError>("Feature with ID '" + m_featureID + "' was not found in input feature collection");
		m_initState = knFailedToInit;
	}

	return knInitialized;
}
