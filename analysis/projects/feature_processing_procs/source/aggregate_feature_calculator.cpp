/******************************************************************************
aggregate_feature_calculator.cpp

begin      : 08/22/2012 4:42:06 PM 
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "aggregate_feature_calculator.h"
#include "pyxis/data/constant_record.h"

#include "pyxis/geometry/vector_geometry2.h"

#include <boost/bind.hpp>
#include "pyxis/utility/value_math.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantFieldCalculator
////////////////////////////////////////////////////////////////////////////////////////////////////////


// {AEEBE10E-8A51-45d7-9BBD-FDB6D584C245}
PYXCOM_DEFINE_CLSID(AggregateFeatureCalculator,
					0xaeebe10e, 0x8a51, 0x45d7, 0x9b, 0xbd, 0xfd, 0xb6, 0xd5, 0x84, 0xc2, 0x45);


PYXCOM_CLASS_INTERFACES(AggregateFeatureCalculator, IProcess::iid, IFeatureCalculator::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(AggregateFeatureCalculator, "Aggregate Features", "Different aggregation of features properties in a feature group.", "Analysis/Features/Calculator",
					IFeatureCalculator::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureGroup::iid, 1, 1, "Input Features", "A collection of features to perform a aggregation on.")
IPROCESS_SPEC_END


PYXValue STDMETHODCALLTYPE AggregateFeatureCalculator::calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const
{
	PYXPointer<PYXGeometry> geometry = spFeature->getGeometry();
	std::auto_ptr<IAggregationOperator> aggregator = m_operatorFactory->create();
	const PYXVectorGeometry2* const vectorGeom2 = dynamic_cast<const PYXVectorGeometry2*>(geometry.get());

	if (vectorGeom2 != 0)
	{
		aggregateFeatures(m_featureGroup,*vectorGeom2,*aggregator);
	}
	else
	{
		PYXTileCollection geomAsTileCollection;
		geometry->copyTo(&geomAsTileCollection);
		aggregateFeatures(m_featureGroup,geomAsTileCollection,*aggregator);
	}
	return aggregator->getResult();
}


void AggregateFeatureCalculator::aggregateFeatures(const boost::intrusive_ptr<IFeatureGroup> & group,const PYXVectorGeometry2 & geometry,IAggregationOperator & aggregator) const
{
	PYXPointer<FeatureIterator> iterator = group->getGroupIterator(geometry);

	for(;!iterator->end();iterator->next())
	{
		boost::intrusive_ptr<IFeature> feature = iterator->getFeature();
		boost::intrusive_ptr<IFeatureGroup> subGroup = feature->QueryInterface<IFeatureGroup>();

		if (subGroup)
		{	
		
			switch (geometry.getRegion()->getVisitor()->intersects(subGroup->getGeometry()->getBoundingCircle()))
			{
			case knIntersectionComplete:
				aggregator.aggregate(subGroup,m_InputAttributeIndex);
				break;
			case knIntersectionPartial:
				 aggregateFeatures(subGroup,geometry, aggregator);
				break;
			case knIntersectionNone:
				break;
			}
		}
		else
		{
			if (feature->getGeometry()->intersects(geometry))
			{
				aggregator.aggregate(feature,m_InputAttributeIndex);
			}
		}
	}
}


void AggregateFeatureCalculator::aggregateFeatures(const boost::intrusive_ptr<IFeatureGroup> & group,const PYXTileCollection & geometry,IAggregationOperator & aggregator) const
{
	PYXPointer<FeatureIterator> iterator = group->getGroupIterator(geometry);
	
	for(;!iterator->end();iterator->next())
	{
		boost::intrusive_ptr<IFeature> feature = iterator->getFeature();
		boost::intrusive_ptr<IFeatureGroup> subGroup = feature->QueryInterface<IFeatureGroup>();

		if (subGroup)
		{
			PYXTileCollection groupGeom;
			subGroup->getGeometry()->copyTo(&groupGeom,geometry.getCellResolution());

			if (geometry.contains(groupGeom))
			{
				aggregator.aggregate(group,m_InputAttributeIndex);
			}
			else
			{
				aggregateFeatures(subGroup,geometry, aggregator);
			}
		}
		else
		{
			if (feature->getGeometry()->intersects(geometry))
			{
				aggregator.aggregate(feature,m_InputAttributeIndex);
			}
		}
	}
}

PYXValue generateValue(const AggregateFeatureCalculator & counter,boost::intrusive_ptr<IFeature> spFeature, int index)
{
	return counter.calculateValue(spFeature,index);
}

boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE AggregateFeatureCalculator::calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const
{
	return new ConstantRecord(m_outputDefinition,boost::bind(generateValue,boost::ref(*this),spFeature,_1));
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE AggregateFeatureCalculator::getOutputDefinition() const
{
	return m_outputDefinition;
}

std::string STDMETHODCALLTYPE AggregateFeatureCalculator::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"

		"<xs:simpleType name=\"OperatorType\">"
		"<xs:restriction base=\"xs:string\">"
			"<xs:enumeration value=\"Feature Count\" />"
			"<xs:enumeration value=\"Minimum\" />"
			"<xs:enumeration value=\"Maximum\" />"
			"<xs:enumeration value=\"Sum\" />"
			"<xs:enumeration value=\"Average\" />"
		"</xs:restriction>"
		"</xs:simpleType>"

		"<xs:simpleType name=\"InputProperty\">"
		"<xs:restriction base=\"xs:string\">";
	
	boost::intrusive_ptr<IFeatureGroup> featureGroup;
	if(getParameter(0)->getValueCount()>0)
	{
		featureGroup = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureGroup>();
		if (featureGroup)
		{
			PYXPointer<PYXTableDefinition> definition = featureGroup ->getFeatureDefinition();
			for (int i=0; i < definition ->getFieldCount() ; ++i)
			{
				if( definition->getFieldDefinition(i).isNumeric())
				{
					strXSD +="<xs:enumeration value=\"" + XMLUtils::toSafeXMLText(definition->getFieldDefinition(i).getName(), true) + "\" />";
				}
			}
		}
	}
	strXSD +=
		"</xs:restriction>"
		"</xs:simpleType>"

		"<xs:element name=\"AggregateFeatureCalculator\">"
		"<xs:complexType>"
		"<xs:sequence>"

		"<xs:element name=\"InputPropertyName\" type=\"InputProperty\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Input Property Name</friendlyName>"
		"<description>Name of the property to perform aggregation on.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"Operator\" type=\"OperatorType\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Aggregation Operator</friendlyName>"
		"<description>Type of the aggregation.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"OutputPropertyName\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Output Property Name</friendlyName>"
		"<description>Name of the field for the const attribute.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";


	return strXSD;
}

std::map< std::string, std::string > STDMETHODCALLTYPE AggregateFeatureCalculator::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["OutputPropertyName"] = StringUtils::toString(m_OutputAttributeName);
	mapAttr["InputPropertyName"] = StringUtils::toString(m_inputName);
	mapAttr["Operator"] = StringUtils::toString(m_operatorName);

	return mapAttr;
}

void STDMETHODCALLTYPE AggregateFeatureCalculator::setAttributes( std::map< std::string, std::string > const & mapAttr )
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"InputPropertyName",m_inputName);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Operator",m_operatorName);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"OutputPropertyName",m_OutputAttributeName);
	
	
	if ("Feature Count" == m_operatorName) 
	{
		m_operatorFactory.reset(new AggregationOperatorFactory<CountOperator>());
	}
	else if ("Minimum" == m_operatorName) 
	{
		m_operatorFactory.reset( new AggregationOperatorFactory<MinOperator>());
	}
	else if ("Maximum" == m_operatorName) 
	{
		m_operatorFactory.reset( new AggregationOperatorFactory<MaxOperator>());
	}
	else if ("Sum" == m_operatorName) 
	{
		m_operatorFactory.reset( new AggregationOperatorFactory<SumOperator>());
	}
	else if ("Average" == m_operatorName) 
	{
		m_operatorFactory.reset( new AggregationOperatorFactory<AverageOperator>());
	}
	else
	{
		m_operatorFactory.reset( new AggregationOperatorFactory<CountOperator>());
		m_operatorName = "Feature Count";
	}
}

IProcess::eInitStatus AggregateFeatureCalculator::initImpl()
{
	m_outputDefinition = PYXTableDefinition::create();
	PYXPointer<PYXTableDefinition> definition= m_operatorFactory->create()->getOutputDefinition();
	if (""==m_OutputAttributeName)
	{
		m_OutputAttributeName=definition->getFieldDefinition(0).getName();
	}
	m_outputDefinition->addFieldDefinition(m_OutputAttributeName, PYXFieldDefinition::knContextNormal, definition->getFieldDefinition(0).getType(), definition->getFieldCount() );

	
	m_featureGroup = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureGroup>();

	if (!m_featureGroup)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input is not IFeatureGroup");
		return knFailedToInit;
	}

	if (m_operatorName!="Feature Count")
	{
		m_InputAttributeIndex = m_featureGroup->getFeatureDefinition()->getFieldIndex(m_inputName);
		if (-1 == m_InputAttributeIndex ||
			!m_featureGroup->getFeatureDefinition()->getFieldDefinition(m_InputAttributeIndex).isNumeric())
		{
			setInitProcError<GenericProcInitError>("Field name not found!");
			return knFailedToInit;
		}
	}

	return knInitialized;
}

AggregateFeatureCalculator::AggregateFeatureCalculator()
{
}

//////////////////////////////////////////////////////////////////////////
//CountOperator
//////////////////////////////////////////////////////////////////////////
void CountOperator::aggregate( const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex )
{
	m_value+= group->getFeaturesCount().max;
}

void CountOperator::aggregate( const boost::intrusive_ptr<IFeature> & feature, int fieldIndex )
{
	m_value++;
}

PYXValue CountOperator::getResult() const
{
	return PYXValue(m_value);
}

PYXPointer<PYXTableDefinition> CountOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Count", PYXFieldDefinition::knContextNormal, PYXValue::knInt32, 1 );

	}
	return m_outputDefinition;
}


//////////////////////////////////////////////////////////////////////////
//MinOperator
//////////////////////////////////////////////////////////////////////////
void MinOperator::aggregate( const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex )
{
	if (m_foundValue)
	{
		m_value = std::min(m_value,group->getFieldHistogram(fieldIndex)->getBoundaries().min.getDouble());
	}
	else
	{
		m_foundValue = true;
		m_value = group->getFieldHistogram(fieldIndex)->getBoundaries().min.getDouble();
	}
}

void MinOperator::aggregate( const boost::intrusive_ptr<IFeature> & feature, int fieldIndex )
{
	if (m_foundValue)
	{
		m_value = std::min(m_value, feature->getFieldValue(fieldIndex).getDouble());
	}
	else
	{
		m_foundValue = true;
		m_value = feature->getFieldValue(fieldIndex).getDouble();
	}
}

PYXPointer<PYXTableDefinition> MinOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Min", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );

	}
	return m_outputDefinition;
}

//////////////////////////////////////////////////////////////////////////
//MaxOperator
//////////////////////////////////////////////////////////////////////////
void MaxOperator::aggregate( const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex )
{
	if (m_foundValue)
	{
		m_value = std::max(m_value,group->getFieldHistogram(fieldIndex)->getBoundaries().max.getDouble());
	}
	else
	{
		m_foundValue = true;
		m_value = group->getFieldHistogram(fieldIndex)->getBoundaries().max.getDouble();
	}
}

void MaxOperator::aggregate( const boost::intrusive_ptr<IFeature> & feature, int fieldIndex )
{
	if (m_foundValue)
	{
		m_value = std::max(m_value, feature->getFieldValue(fieldIndex).getDouble());
	}
	else
	{
		m_foundValue = true;
		m_value = feature->getFieldValue(fieldIndex).getDouble();
	}
}

PYXPointer<PYXTableDefinition> MaxOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Max", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );
	}
	return m_outputDefinition;
}

//////////////////////////////////////////////////////////////////////////
//SumOperator
//////////////////////////////////////////////////////////////////////////
void SumOperator::aggregate( const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex )
{
	m_value += group->getFieldHistogram(fieldIndex)->getSum().getDouble();
}

void SumOperator::aggregate( const boost::intrusive_ptr<IFeature> & feature, int fieldIndex )
{
	m_value += feature->getFieldValue(fieldIndex).getDouble();
}

PYXPointer<PYXTableDefinition> SumOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Sum", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );
	}
	return m_outputDefinition;
}


//////////////////////////////////////////////////////////////////////////
//AverageOperator
//////////////////////////////////////////////////////////////////////////
void AverageOperator::aggregate( const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex )
{
	m_sum.aggregate(group,fieldIndex);
	m_count.aggregate(group,fieldIndex);
}

void AverageOperator::aggregate( const boost::intrusive_ptr<IFeature> & feature, int fieldIndex )
{
	m_sum.aggregate(feature,fieldIndex);
	m_count.aggregate(feature,fieldIndex);
}

PYXValue AverageOperator::getResult() const
{
	return PYXValue(m_sum.getResult().getDouble()/m_count.getResult().getDouble());
}

PYXPointer<PYXTableDefinition> AverageOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Average", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );
	}
	return m_outputDefinition;
}
