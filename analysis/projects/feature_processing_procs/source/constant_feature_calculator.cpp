/******************************************************************************
constant_feature_calculator.cpp

begin      : 08/20/2012 4:42:06 PM 
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "constant_feature_calculator.h"
#include "pyxis/data/constant_record.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantFieldCalculator
////////////////////////////////////////////////////////////////////////////////////////////////////////

// {782659BF-AF07-4dad-AD31-CABC9ADBFB29}
PYXCOM_DEFINE_CLSID(ConstantFieldCalculator, 
					0x782659bf, 0xaf07, 0x4dad, 0xad, 0x31, 0xca, 0xbc, 0x9a, 0xdb, 0xfb, 0x29);

PYXCOM_CLASS_INTERFACES(ConstantFieldCalculator, IProcess::iid, IFeatureCalculator::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ConstantFieldCalculator, "Constant attribute", "Returns a constant attribute.", "Analysis/Features/Calculator",
					IFeatureCalculator::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END


PYXValue STDMETHODCALLTYPE ConstantFieldCalculator::calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const
{
	return m_outputRecord->getFieldValue(fieldIndex);
}


boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE ConstantFieldCalculator::calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const
{
	return m_outputRecord;
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE ConstantFieldCalculator::getOutputDefinition() const
{
	return m_outputDefinition;
}

std::string STDMETHODCALLTYPE ConstantFieldCalculator::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"

		"<xs:element name=\"ConstantFieldCalculator\">"
		"<xs:complexType>"
		"<xs:sequence>"

		"<xs:element name=\"AttributeName\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Attribute Name</friendlyName>"
		"<description>Name of the field for the const attribute.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"AttributeValue\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Attribute Value</friendlyName>"
		"<description>Value of the field for the const attribute.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";


	return strXSD;
}

std::map< std::string, std::string > STDMETHODCALLTYPE ConstantFieldCalculator::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["AttributeName"] = StringUtils::toString(m_attributeName);
	mapAttr["AttributeValue"] = StringUtils::toString(m_attributeValue);

	return mapAttr;
}

void STDMETHODCALLTYPE ConstantFieldCalculator::setAttributes( std::map< std::string, std::string > const & mapAttr )
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"AttributeName",m_attributeName);
	try
	{
		UPDATE_PROCESS_ATTRIBUTE(mapAttr,"AttributeValue",PYXValue,m_attributeValue);
	}
	catch(...)
	{
		m_attributeValue = PYXValue();
	}
}

IProcess::eInitStatus ConstantFieldCalculator::initImpl()
{

	m_outputDefinition = PYXTableDefinition::create();

	m_outputDefinition->addFieldDefinition(m_attributeName, PYXFieldDefinition::knContextNormal, m_attributeValue.getArrayType(), m_attributeValue.getArraySize() );

	std::vector<PYXValue> values;
	values.push_back(m_attributeValue);

	m_outputRecord = new ConstantRecord(m_outputDefinition,values);

	return knInitialized;
}

ConstantFieldCalculator::ConstantFieldCalculator()
{
	m_attributeName = "const";
	m_attributeValue = PYXValue(1);
}

