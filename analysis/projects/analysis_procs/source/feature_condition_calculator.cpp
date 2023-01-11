/******************************************************************************
feature condition calculator.cpp

begin		: 2013-4-22
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE

#include "feature_condition_calculator.h"

// local includes
#include "exceptions.h"


#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"


// standard includes
#include <cassert>
#include "pyxis/data/constant_record.h"

#include <boost/bind.hpp>
#include "pyxis/data/feature_group.h"

//////////////////////////////////////////////////////////////////////////
// ComparisonCondition
//////////////////////////////////////////////////////////////////////////

// {E1DAB3AB-F9D2-4550-9FAA-C8A1279E0B1A}
PYXCOM_DEFINE_CLSID(FeatureConditionCalculator,
					0xe1dab3ab, 0xf9d2, 0x4550, 0x9f, 0xaa, 0xc8, 0xa1, 0x27, 0x9e, 0xb, 0x1a);

PYXCOM_CLASS_INTERFACES(FeatureConditionCalculator, IFeatureCalculator::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureConditionCalculator, "Comparison Condition", "A process that can act as a condition", "Analysis/Features",IFeatureCalculator::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_END

Tester<FeatureConditionCalculator> gComparisonConditionTester;
void FeatureConditionCalculator::test()
{

}

std::string STDMETHODCALLTYPE FeatureConditionCalculator::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"

		"<xs:simpleType name=\"ComparisonOperations\">"
		"<xs:restriction base=\"xs:string\">"
		"<xs:enumeration value=\"Equals\" />"
		"<xs:enumeration value=\"Greater than\" />"
		"<xs:enumeration value=\"Less than\" />"
		"<xs:enumeration value=\"Equal or Greater\" />"
		"<xs:enumeration value=\"Equal or Less\" />"
		"<xs:enumeration value=\"Between\" />"
		"<xs:enumeration value=\"Not Equals\" />"
		"<xs:enumeration value=\"No Condition\" />"
		"</xs:restriction>"
		"</xs:simpleType>";

	strXSD += 
		"<xs:element name=\"FeatureConditionCalculator\">"
		"<xs:complexType>"
		"<xs:sequence>"

		"<xs:element name=\"FieldIndex\" type=\"xs:int\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Field index</friendlyName>"
		"<description>S>Field index to which the condition is applied</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"Comparison\" type=\"ComparisonOperations\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Comparison operation</friendlyName>"
		"<description>Select the comparison operation</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"Value1\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Compare with</friendlyName>"
		"<description>Value to compare with</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"


		"<xs:element name=\"Value2\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>And</friendlyName>"
		"<description>Maximum value in a between condition </description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";

	return strXSD;
}

std::map<std::string, std::string> STDMETHODCALLTYPE FeatureConditionCalculator::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	auto it = s_operationName.find(m_operation);
	if (it != s_operationName.end())
	{
		mapAttr["Comparison"] = it->second;
	}
	else
	{
		TRACE_ERROR("Comparison Operation Not Found For Operation : " << m_operation);
	}
	
	mapAttr["Value1"] = m_compareTo.getString();
	mapAttr["Value2"] = m_compareToTop.getString();
	mapAttr["FieldIndex"] = StringUtils::toString(m_fieldIndex);
	return mapAttr;
}

void STDMETHODCALLTYPE FeatureConditionCalculator::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	auto it = mapAttr.find("Comparison");
	if (it != mapAttr.end())
	{
		auto operation = it->second;
		for (auto name = s_operationName.begin(); name!= s_operationName.end(); name ++)
		{
			if (operation==name->second)
			{
				m_operation = name->first;
			}
		}
	}

	it = mapAttr.find("Value1");
	if (it != mapAttr.end())
	{
		if(StringUtils::isNumeric(it->second))
		{
			m_compareTo = PYXValue(StringUtils::fromString<double>( it->second));
		}
		else
		{
			m_compareTo = PYXValue( it->second);
		}
	}
	it = mapAttr.find("Value2");
	if (it != mapAttr.end())
	{
		if(StringUtils::isNumeric(it->second))
		{
			m_compareToTop = PYXValue(StringUtils::fromString<double>( it->second));
		}
		else
		{
			m_compareToTop = PYXValue( it->second);
		}
	}
	it = mapAttr.find("FieldIndex");
	if (it != mapAttr.end())
	{
		m_fieldIndex = (int)(StringUtils::fromString<int>( it->second));
	}
}

bool FeatureConditionCalculator::isTrue( Range<PYXValue> range ) const
{
	switch (m_operation)
	{
	case Operation::NO_CONDITION:
		return true;

	case Operation::GREATER_THAN_OR_EQUAL:
		return m_compareTo.compare(range.min) != Operation::LESS_THAN;

	case  Operation::LESS_THAN_OR_EQUAL:
		return m_compareTo.compare(range.max) != Operation::GREATER_THAN;

	case  Operation::NOT_EQUAL:
		return m_compareTo.compare(range.min) != Operation::EQUALS || 
			m_compareTo.compare(range.max) != Operation::EQUALS;

	case  Operation::BETWEEN:
		return m_compareToTop.compare(range.min) == Operation::LESS_THAN && 
			m_compareTo.compare(range.max) == Operation::GREATER_THAN;

	case  Operation::LESS_THAN:
		return m_compareTo.compare(range.min) == Operation::LESS_THAN;

	case  Operation::GREATER_THAN:
		return m_compareTo.compare(range.max) == Operation::GREATER_THAN;

	case  Operation::EQUALS:
		return range.contains(m_compareTo);

	default:
		PYXTHROW(PYXException,"unknown condition operation : " << m_operation);
	}
}

PYXValue STDMETHODCALLTYPE FeatureConditionCalculator::calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const
{
	if(fieldIndex==0)
	{ 
		Range<PYXValue> fieldValue;
		boost::intrusive_ptr<IFeatureGroup> & featureGroup = spFeature->QueryInterface<IFeatureGroup>();

		if(featureGroup)
		{
			fieldValue = featureGroup->getFieldHistogram(m_fieldIndex)->getBoundaries();
		}
		else
		{
			//PYXValue converts to Range implicitly
			fieldValue = spFeature->getFieldValue(m_fieldIndex);
		}	
		return PYXValue(isTrue(fieldValue));
	}
	PYXTHROW(PYXException, "invalid field index");
}

boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE FeatureConditionCalculator::calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const
{
	std::vector<PYXValue> values;
	values.push_back(calculateValue(spFeature,0));
	return new ConstantRecord(m_outputDefinition,values);
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE FeatureConditionCalculator::getOutputDefinition() const
{
	return m_outputDefinition;
}

const std::map<Operation, char const*> FeatureConditionCalculator::s_operationName  =  boost::assign::map_list_of
	(Operation::EQUALS, "Equals") 
	(Operation::GREATER_THAN, "Greater than") 
	(Operation::LESS_THAN,"Less than")
	(Operation::GREATER_THAN_OR_EQUAL,"Equal or Greater")
	(Operation::LESS_THAN_OR_EQUAL,"Equal or Less")
	(Operation::NO_CONDITION,"No Condition")
	(Operation::BETWEEN,"Between")
	(Operation::NOT_EQUAL,"Not Equals")
	;
