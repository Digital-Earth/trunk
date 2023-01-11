/******************************************************************************
count_feature_calculator.cpp

begin      : 08/21/2012 4:42:06 PM 
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "count_feature_calculator.h"
#include "pyxis/data/constant_record.h"

#include "pyxis/geometry/vector_geometry2.h"

#include <boost/bind.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantFieldCalculator
////////////////////////////////////////////////////////////////////////////////////////////////////////


// {C94D4C6C-E1CA-400b-91AC-1AB3B2D79F0F}
PYXCOM_DEFINE_CLSID(CountFeatureCalculator,
					0xc94d4c6c, 0xe1ca, 0x400b, 0x91, 0xac, 0x1a, 0xb3, 0xb2, 0xd7, 0x9f, 0xf);


PYXCOM_CLASS_INTERFACES(CountFeatureCalculator, IProcess::iid, IFeatureCalculator::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(CountFeatureCalculator, "Count Features", "Returns number of features in a feature group.", "Analysis/Features/Calculator",
					IFeatureCalculator::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureGroup::iid, 1, 1, "Input Features", "A collection of features to perform a count on.")
IPROCESS_SPEC_END


PYXValue STDMETHODCALLTYPE CountFeatureCalculator::calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const
{
	PYXPointer<PYXGeometry> geometry = spFeature->getGeometry();

	const PYXVectorGeometry2* const vectorGeom2 = dynamic_cast<const PYXVectorGeometry2*>(geometry.get());

	if (vectorGeom2 != 0)
	{
		int result = countFeatures(m_featureGroup,*vectorGeom2);
		return PYXValue(result);
	}
	else
	{
		PYXTileCollection geomAsTileCollection;
		geometry->copyTo(&geomAsTileCollection);
		int result = countFeatures(m_featureGroup,geomAsTileCollection);
		return PYXValue(result);
	}
}

int CountFeatureCalculator::countFeatures(const boost::intrusive_ptr<IFeatureGroup> & group,const PYXVectorGeometry2 & geometry) const
{
	int result = 0;
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
				result += subGroup->getFeaturesCount().max;
				break;
			case knIntersectionPartial:
				result += countFeatures(subGroup,geometry);
				break;
			case knIntersectionNone:
				break;
			}
		}
		else
		{
			if (feature->getGeometry()->intersects(geometry))
			{
				result++;
			}
		}
	}
	return result;
}


int CountFeatureCalculator::countFeatures(const boost::intrusive_ptr<IFeatureGroup> & group,const PYXTileCollection & geometry) const
{
	int result = 0;
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
				result += subGroup->getFeaturesCount().max;
			}
			else
			{
				result += countFeatures(subGroup,geometry);
			}
		}
		else
		{
			if (feature->getGeometry()->intersects(geometry))
			{
				result++;
			}
		}
	}
	return result;
}

PYXValue generateValue(const CountFeatureCalculator & counter,boost::intrusive_ptr<IFeature> spFeature, int index)
{
	return counter.calculateValue(spFeature,index);
}

boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE CountFeatureCalculator::calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const
{
	return new ConstantRecord(m_outputDefinition,boost::bind(generateValue,boost::ref(*this),spFeature,_1));
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE CountFeatureCalculator::getOutputDefinition() const
{
	return m_outputDefinition;
}

std::string STDMETHODCALLTYPE CountFeatureCalculator::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"

		"<xs:element name=\"CountFeatureCalculator\">"
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

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";


	return strXSD;
}

std::map< std::string, std::string > STDMETHODCALLTYPE CountFeatureCalculator::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["AttributeName"] = StringUtils::toString(m_attributeName);

	return mapAttr;
}

void STDMETHODCALLTYPE CountFeatureCalculator::setAttributes( std::map< std::string, std::string > const & mapAttr )
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"AttributeName",m_attributeName);
}

IProcess::eInitStatus CountFeatureCalculator::initImpl()
{

	m_outputDefinition = PYXTableDefinition::create();

	m_outputDefinition->addFieldDefinition(m_attributeName, PYXFieldDefinition::knContextNormal, PYXValue::knInt32, 1 );

	m_featureGroup = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureGroup>();

	if (!m_featureGroup)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input is not IFeatureGroup");
		return knFailedToInit;
	}

	return knInitialized;
}

CountFeatureCalculator::CountFeatureCalculator()
{
	m_attributeName = "count";
}

