/******************************************************************************
xy_coverage_translator.cpp

begin		: 2010-04-19
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_SAMPLING_SOURCE
#include "stdafx.h"
#include "xy_coverage_translator.h"


// pyxlib includes
#include "pyxis/utility/pyxcom.h"
#include "pyxis/pipe/process.h"

// {9EBF1F8B-C200-48e9-9329-E15F67850A97}
PYXCOM_DEFINE_CLSID(XYCoverageTranslator, 
0x9ebf1f8b, 0xc200, 0x48e9, 0x93, 0x29, 0xe1, 0x5f, 0x67, 0x85, 0xa, 0x97);

PYXCOM_CLASS_INTERFACES(XYCoverageTranslator, IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);



// {B26A382D-151C-4149-8F03-00C1740E56B4}
PYXCOM_DEFINE_CLSID(XYCoverageTranslatorProcess, 
0xb26a382d, 0x151c, 0x4149, 0x8f, 0x3, 0x0, 0xc1, 0x74, 0xe, 0x56, 0xb4);
PYXCOM_CLASS_INTERFACES(XYCoverageTranslatorProcess, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(XYCoverageTranslatorProcess, "XYCoverage Value Translator", "Translate values from XYCoverage", "Reader",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IXYCoverage::iid, 1, 1, "Input XY Coverage", "A XY Coverage to use as an input.");	
IPROCESS_SPEC_END


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

XYCoverageTranslatorProcess::XYCoverageTranslatorProcess() : m_outputType(PYXValue::knDouble), m_translateOperation("Cast")
{
}

//! Get the attributes in this process.
std::map<std::string, std::string> STDMETHODCALLTYPE XYCoverageTranslatorProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["Operation"] = m_translateOperation;
	mapAttr["OutputType"] = PYXValue::getTypeAsString(m_outputType);	

	return mapAttr;
}

//! Set the attributes in this process.
void STDMETHODCALLTYPE XYCoverageTranslatorProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Operation",m_translateOperation);
	
	std::string outputTypeName;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"OutputType",outputTypeName);

	m_outputType = PYXValue::getType(outputTypeName);
}

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string XYCoverageTranslatorProcess::getAttributeSchema() const
{
	return
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"TranslateOperation\">"
			"<xs:restriction base=\"xs:string\">"
				"<xs:enumeration value=\"Cast\" />"
				"<xs:enumeration value=\"Reinterpret\" />"				
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:simpleType name=\"PYXValueType\">"
			"<xs:restriction base=\"xs:string\">"
				"<xs:enumeration value=\"uint8_t\" />"
				"<xs:enumeration value=\"uint16_t\" />"
				"<xs:enumeration value=\"uint32_t\" />"
				"<xs:enumeration value=\"int8_t\" />"
				"<xs:enumeration value=\"int16_t\" />"
				"<xs:enumeration value=\"int32_t\" />"
				"<xs:enumeration value=\"bool\" />"
				"<xs:enumeration value=\"char\" />"
				"<xs:enumeration value=\"float\" />"
				"<xs:enumeration value=\"double\" />"				
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"XYCoverageTranslatorProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Operation\" type=\"TranslateOperation\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Translation</friendlyName>"
					"<description>The translation method to use, where to perfrom a safe cast or reinterpret.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			  "<xs:element name=\"OutputType\" type=\"PYXValueType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Output Type</friendlyName>"
					"<description>The output type to translate to.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>" 			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Prepare the process so that it is able to provide data.

*/
IProcess::eInitStatus XYCoverageTranslatorProcess::initImpl()
{
	std::vector<PYXPointer<ISingleValueTranslator>> translators;

	boost::intrusive_ptr<IXYCoverage> inputCoverage;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IXYCoverage::iid, (void**) &inputCoverage);

	if  (!inputCoverage)
	{
		setInitProcError<GenericProcInitError>("Unable to get the input XY Coverage");
		return knFailedToInit;
	}	

	PYXPointer<PYXTableDefinition> inputDefinition = inputCoverage->getCoverageDefinition();

	for(int i=0;i<inputDefinition->getFieldCount();i++)
	{
		if (m_translateOperation == "Cast")
		{
			translators.push_back(CastValueTranslator::create(inputDefinition->getFieldDefinition(i),m_outputType));
		}
		else 
		{
			translators.push_back(ReinterpretValueTranslator::create(inputDefinition->getFieldDefinition(i),m_outputType));
		}
	}

	m_spXYCoverage = new XYCoverageTranslator(inputCoverage,SimpleDefinitionTranslator::create(translators));
	
	if  (!m_spXYCoverage)
	{
		setInitProcError<GenericProcInitError>("Unable to create an instance of a XYCoverageTranslator. Fatal Error.");
		return knFailedToInit;
	}	

	return knInitialized;
}
