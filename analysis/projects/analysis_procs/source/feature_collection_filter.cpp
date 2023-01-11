/******************************************************************************
feature_collection_filter.cpp

begin		: 2013-4-22
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE

#include "feature_collection_filter.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/data/feature_iterator_linq.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/xtime.hpp>

// standard includes
#include <cassert>

// {3679A9BC-5111-4238-BA8B-5F441DA7AB5B}
PYXCOM_DEFINE_CLSID(FeatureCollectionFilter,
					0x3679a9bc, 0x5111, 0x4238, 0xba, 0x8b, 0x5f, 0x44, 0x1d, 0xa7, 0xab, 0x5b);

PYXCOM_CLASS_INTERFACES(FeatureCollectionFilter, IProcess::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureCollectionFilter, "Filter Features", "A process to filter features of a feature collection using a set of conditions", "Analysis/Features",
					IFeatureCollection::iid,  IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, -1, "Input Features", "A feature collection to filter")
	IPROCESS_SPEC_PARAMETER(IFeatureCalculator::iid, 0, -1, "Input Conditions", "A collection of input conditions")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<FeatureCollectionFilter> gTester;

}

FeatureCollectionFilter::FeatureCollectionFilter() : m_conditions() , m_inputFC(), m_inputFGroup()
{
}

FeatureCollectionFilter::~FeatureCollectionFilter()
{
}

void FeatureCollectionFilter::test()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::string STDMETHODCALLTYPE FeatureCollectionFilter::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

	strXSD += 
		"<xs:element name=\"FeatureCollectionFilter\">"
		  "<xs:complexType>"
			  "<xs:sequence>"

			  "</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strXSD;
}

std::map<std::string, std::string> STDMETHODCALLTYPE FeatureCollectionFilter::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	return mapAttr;
}

void STDMETHODCALLTYPE FeatureCollectionFilter::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
}

IProcess::eInitStatus FeatureCollectionFilter::initImpl()
{
	boost::intrusive_ptr<IFeatureCollection> fc = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();
	if (!fc)
	{
		setInitProcError<GenericProcInitError>("Failed to get the feature collection to rasterize from the parameter.");
		return IProcess::knFailedToInit;
	}
	m_inputFGroup = fc->QueryInterface<IFeatureGroup>();
	m_inputFC = fc;

	m_conditions.clear();
	for (int i = 0; i < getParameter(1)->getValueCount(); i++)
	{
		boost::intrusive_ptr<IFeatureCalculator> condition = getParameter(1)->getValue(i)->QueryInterface<IFeatureCalculator>();
		if (!condition)
		{
			setInitProcError<GenericProcInitError>("Failed to get the conditions in feature filter process");
			return IProcess::knFailedToInit;
		}

		m_conditions.push_back(condition);
	}
	m_strID = guidToStr(getProcID());

	return knInitialized;
}

bool checkFeature(boost::intrusive_ptr<IFeature> feature,const std::vector<boost::intrusive_ptr<IFeatureCalculator>> conditions)
{
	for (unsigned int i = 0; i < conditions.size(); i++)
	{
		if (!conditions[i]->calculateValue(feature, 0).getBool()) 
		{
			return false;
		}
	}
	return true;
}


bool checkFeatureInGeometry(boost::intrusive_ptr<IFeature> feature, const std::vector<boost::intrusive_ptr<IFeatureCalculator>> conditions, PYXPointer<PYXGeometry> geometry)
{
	if(!feature->getGeometry()->intersects(*geometry))
	{
		return false;
	}
	return checkFeature(feature,conditions);
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeatureCollectionFilter::getIterator() const
{
	return PYXFeatureIteratorLinq(m_inputFC->getIterator()).filter(boost::bind(checkFeature,_1,m_conditions));
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeatureCollectionFilter::getIterator( const PYXGeometry& geometry ) const
{
	
	auto geometryClone = geometry.clone();
	if (m_inputFGroup)
	{
		return  PYXFeatureIteratorLinq(m_inputFGroup->getGroupIterator(*geometryClone))
			.selectMany(boost::bind(PYXFeatureIteratorLinq::expandGroupToGeometry,_1,geometryClone))
			.filter(boost::bind(checkFeature,_1,m_conditions));
	}

	return PYXFeatureIteratorLinq(m_inputFC->getIterator(geometry))
		.filter(boost::bind(checkFeature,_1,m_conditions));
}

std::vector<FeatureStyle> STDMETHODCALLTYPE FeatureCollectionFilter::getFeatureStyles() const
{
	return m_inputFC->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE FeatureCollectionFilter::getFeature( const std::string& strFeatureID ) const
{
	return m_inputFC->getFeature(strFeatureID);
}

PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE FeatureCollectionFilter::getFeatureDefinition() const
{
	return m_inputFC->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE FeatureCollectionFilter::getFeatureDefinition()
{
	return m_inputFC->getFeatureDefinition();
}

bool STDMETHODCALLTYPE FeatureCollectionFilter::canRasterize() const
{
	return m_inputFC->canRasterize();
}