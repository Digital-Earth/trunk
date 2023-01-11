/******************************************************************************
point_aggregator_process.cpp

begin		: 2008-01-31
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "point_aggregator_process.h"

//pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/sub_index.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/procs/exceptions.h"
#include "pyxis/procs/default_feature.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// {EE604B45-F5AD-4290-8B66-31551BF2C8E1}
PYXCOM_DEFINE_CLSID(PointAggregatorProcess, 
0xee604b45, 0xf5ad, 0x4290, 0x8b, 0x66, 0x31, 0x55, 0x1b, 0xf2, 0xc8, 0xe1);

// {17689C66-3041-4c65-91CF-58BDEBE6340D}
PYXCOM_DEFINE_CLSID(AggregatedPointFeature, 
0x17689c66, 0x3041, 0x4c65, 0x91, 0xcf, 0x58, 0xbd, 0xeb, 0xe6, 0x34, 0xd);

PYXCOM_CLASS_INTERFACES(PointAggregatorProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

PYXCOM_CLASS_INTERFACES(AggregatedPointFeature, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);


IPROCESS_SPEC_BEGIN(PointAggregatorProcess, "Point Aggregator", "Aggregates all Features up to a lower resolution.", "Development/Old",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Features To Aggregate", "Collection of Features to Aggregate.")
IPROCESS_SPEC_END

/*!
A Feature iterator to iterate over all the features in the Feature collection process.
*/
class PointAggregatorFeatureCollectionIterator : public FeatureIterator
{
public:

	//! Dynamic Creator.
	static PYXPointer<PointAggregatorFeatureCollectionIterator> create(const std::vector<boost::intrusive_ptr<IFeature> >& vect)
	{
		return PYXNEW(PointAggregatorFeatureCollectionIterator, vect);
	}


	//! Default Constructor.
	PointAggregatorFeatureCollectionIterator(const std::vector<boost::intrusive_ptr<IFeature> >& vect) :
		m_vect(vect),
		m_vecIt(m_vect.begin())
	{
	}

	//! Destructor
	virtual ~PointAggregatorFeatureCollectionIterator(){;}

	//! Determine if we are done iterating over the features.
	virtual bool end() const
	{
		return m_vecIt == m_vect.end();
	}

	//! Move to the next feature.
	virtual void next()
	{
		++m_vecIt;
	}

	//! Get the current feature the iterator is on.
	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return *m_vecIt;
	}

private:
	
	//! A vector of features to iterate over.
	std::vector<boost::intrusive_ptr<IFeature> > m_vect;

	//! A const iterator for maintaining the position of the vector.
	std::vector<boost::intrusive_ptr<IFeature> >::const_iterator m_vecIt;

};



Tester<PointAggregatorProcess> gTester;
void PointAggregatorProcess::test()
{

	//TODO: More testing.
	{ //Test Aggregation from Res 6 to Res 5. 
		PYXTile tile (PYXIcosIndex("H-000"), 6); 
		boost::intrusive_ptr<DefaultFeatureCollection> spFC (new DefaultFeatureCollection());
		for (PYXPointer<PYXIterator> it = tile.getIterator(); !it->end(); it->next())
		{
			spFC->addFeature(new DefaultFeature(PYXCell::create(it->getIndex())));
		}

		boost::intrusive_ptr<PointAggregatorProcess> spAggregator (new PointAggregatorProcess());

		boost::intrusive_ptr<IProcess> spProc;
		spFC->QueryInterface(IProcess::iid, (void**) &spProc);
		
		spAggregator->getParameter(0)->addValue(spProc);
		spAggregator->initProc();
		spAggregator->m_nAggregatedResolution = 5;
		
		PYXPointer<FeatureIterator> spFeatureIt = spAggregator->getIterator();
		TEST_ASSERT(spFeatureIt->getFeature()->getGeometry()->getIterator()->getIndex() == PYXIcosIndex("H-0000"));

		for (PYXVertexIterator it(tile.getRootIndex()); !it.end(); ++it)
		{
			spFeatureIt->next();
			TEST_ASSERT(spFeatureIt->getFeature()->getGeometry()->getIterator()->getIndex() == it.getIndex());
		}
		PYXPointer<FeatureIterator> spDefaultFeatIt = spFC->getIterator();
		for (PYXPointer<FeatureIterator> spFeatIt = spAggregator->getIterator(); !spFeatIt->end(); spFeatIt->next())
		{
			boost::intrusive_ptr<IFeatureCollection> spAggregFC;
			spFeatIt->getFeature()->QueryInterface(IFeatureCollection::iid, (void**) &spAggregFC);

			for (PYXPointer<FeatureIterator> spIt = spAggregFC->getIterator(); !spIt->end(); spIt->next())
			{
				TEST_ASSERT(spDefaultFeatIt->getFeature()->getGeometry()->getIterator()->getIndex() == 
					spIt->getFeature()->getGeometry()->getIterator()->getIndex());
				spDefaultFeatIt->next();
			}
		}
	}


}

 
AggregatedPointFeature::AggregatedPointFeature()
{
	m_spDefn = PYXTableDefinition::create();
}

void AggregatedPointFeature::setGeometry(PYXPointer<PYXGeometry> spGeom)
{
	m_spGeom = spGeom;
}

void AggregatedPointFeature::addFeature(boost::intrusive_ptr<IFeature> spFeature)
{
	if (spFeature)
	{
		m_vecFeatures.push_back(spFeature);
	}
	calcMetaData();
}


 bool AggregatedPointFeature::isWritable() const
 {
	 return true;
 }

const std::string& AggregatedPointFeature::getID() const
{
	for (std::vector<boost::intrusive_ptr<IFeature> >::const_iterator it = m_vecFeatures.begin();
		it != m_vecFeatures.end(); ++it)
	{
		m_strID = m_strID + (*it)->getID();
	}
	return m_strID;
}

PYXPointer<PYXGeometry>  AggregatedPointFeature::getGeometry()
{
	return m_spGeom;
}

PYXPointer<const PYXGeometry>  AggregatedPointFeature::getGeometry() const
{
	return m_spGeom;
}

//! Get an iterator to all the features in this collection.
PYXPointer<FeatureIterator> AggregatedPointFeature::getIterator() const
{
	return PointAggregatorFeatureCollectionIterator::create(m_vecFeatures);	
}

void AggregatedPointFeature::calcMetaData() 
{
	m_spDefn = PYXTableDefinition::create();
	m_spDefn->addFieldDefinition("Name", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);
	setFieldValue(PYXValue("Aggregated Point"), 0);
	m_spDefn->addFieldDefinition("Description", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);
	setFieldValue(PYXValue("Represents all points Aggregated to Index: " + m_spGeom->getIterator()->getIndex().toString()), 1);
	m_spDefn->addFieldDefinition("Aggregated Point Count", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);
	setFieldValue(PYXValue("Points Aggregated: " + intToString(static_cast<int>(m_vecFeatures.size()),0)), 2);
}

std::string AggregatedPointFeature::getStyle() const
{
	if (m_vecFeatures.size() == 1)
	{
		return "<style><LineColour>red</LineColour><Icon>4</Icon></style>";
	}
	else if (m_vecFeatures.size() > 1 && m_vecFeatures.size() <= 5)
	{
		return "<style><LineColour>red</LineColour><Icon>3</Icon></style>";		
	}
	else if (m_vecFeatures.size() > 6)
	{
		return "<style><LineColour>red</LineColour><Icon>3</Icon></style>";		
	}
	
	return "";
}

std::string AggregatedPointFeature::getStyle(const std::string& strStyleToGet) const
{
	PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(getStyle());
	return styleDoc->getNodeText("/style/" + strStyleToGet);	
}


//! Get the feature with the specified ID.
boost::intrusive_ptr<IFeature> AggregatedPointFeature::getFeature(
		const std::string& strFeatureID) const
{
	std::vector<boost::intrusive_ptr<IFeature> >::const_iterator vecIt;

	for (vecIt = m_vecFeatures.begin(); vecIt != m_vecFeatures.end(); ++vecIt)
	{
		if ((*vecIt)->getID().compare(strFeatureID) == 0)
		{
			return (*vecIt);
		}
	}

	return boost::intrusive_ptr<IFeature>();
}

//! Get an iterator to all the features in this collection that intersect this geometry.
PYXPointer<FeatureIterator> AggregatedPointFeature::getIterator(const PYXGeometry& geometry) const
{
	std::vector<boost::intrusive_ptr<IFeature> > vecFeatureSet;
	for (std::vector<boost::intrusive_ptr<IFeature> >::const_iterator it = m_vecFeatures.begin();
		it != m_vecFeatures.end(); ++it)
	{
		if (geometry.intersects(*(*it)->getGeometry()))
		{
			vecFeatureSet.push_back(*it);
		}
	}

	return PointAggregatorFeatureCollectionIterator::create(vecFeatureSet);
}

PointAggregatorProcess::PointAggregatorProcess() : m_nAggregatedResolution(15)
{
}

IProcess::eInitStatus PointAggregatorProcess::initImpl()
{
	m_bIsAggregated = false;
	m_spGeom = PYXMultiCell::create();
	m_strId = procRefToStr(ProcRef(this));

	m_spDefn = PYXTableDefinition::create();
	if (m_spDefn)
	{
		m_spDefn->addFieldDefinition("Name", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);
		m_spDefn->addFieldDefinition("Description", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);
		setFieldValue(PYXValue("Aggregated Points"), 0);
		setFieldValue(PYXValue("Aggregates all data from Resolution n to x"), 1);
	}

	// TODO: Remove spurious spec verification (in all processes)
	if (getParameterCount() > 0 && getParameter(0)->getValueCount() > 0)
	{
		boost::intrusive_ptr<IProcess> spInputProc = getParameter(0)->getValue(0);
		
		//Attempt to determine what the input actually is. 

		boost::intrusive_ptr<ICoverage> spCov;
		spInputProc->getOutput()->QueryInterface(ICoverage::iid, (void**) &spCov);
		if (spCov)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Cannot plug a coverage into the Point Aggregator");
			return knFailedToInit;
		}

		m_spInputFeatureCollection = spInputProc->getOutput()->QueryInterface<IFeatureCollection>();
		assert(m_spInputFeatureCollection);
	}
	return knInitialized;
}

std::map<std::string, std::string> PointAggregatorProcess::getAttributes() const
{
	std::map<std::string,std::string> mapAttr;
	mapAttr.clear();
	
	mapAttr["AggregatedResolution"] = intToString(m_nAggregatedResolution, 0);

	return mapAttr;
}

void PointAggregatorProcess::setAttributes(const std::map<std::string,std::string> &attrMap)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it = attrMap.find("AggregatedResolution");

	if (it != attrMap.end())
	{
		int nRes = atoi(const_cast<const char*>(it->second.c_str()));
		assert(nRes > 0);
		if (m_nAggregatedResolution != nRes)
		{
			 if (nRes > 0 && nRes < 40)
			 {
				m_nAggregatedResolution = nRes;
				m_bIsAggregated = false;
			 }
			 else 
			 {
				 //TODO: Change to proper exception when have more time.
				PYXTHROW(PYXException, "Trying to aggregate to a resolution beyond which Pyxis can support.");
			 }
		}
	}
}

std::vector<boost::intrusive_ptr<AggregatedPointFeature> > PointAggregatorProcess::aggregate(int nAggregateResolution) const
{	
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	bool bWasAggregated = false;
	std::vector<boost::intrusive_ptr<AggregatedPointFeature> > vecAggregatedPts;
	for (PYXPointer<FeatureIterator> spFeatIt = m_spInputFeatureCollection->getIterator();
		!spFeatIt->end(); spFeatIt->next())
	{
		bWasAggregated = false;
		boost::intrusive_ptr<IFeature> spCurrentFeature = spFeatIt->getFeature();	
		for (std::vector<boost::intrusive_ptr<AggregatedPointFeature> >::const_iterator it = 
			vecAggregatedPts.begin(); it != vecAggregatedPts.end(); ++it)
		{	
			if ((*it)->getGeometry()->intersects(*(spCurrentFeature->getGeometry())))
			{
				
 				(*it)->addFeature(spCurrentFeature);
				 bWasAggregated = true;
			}
		}

		//This feature can't aggregate to a higher feature yet.
		if (!bWasAggregated)
		{
			boost::intrusive_ptr<AggregatedPointFeature> spAggregatedFeature(new AggregatedPointFeature);
		
			PYXPointer<PYXGeometry> spAggregatedCell = PYXCell::create(spCurrentFeature->getGeometry()->getIterator()->getIndex());
			spAggregatedCell->setCellResolution(nAggregateResolution);
			spAggregatedFeature->setGeometry(spAggregatedCell);
			vecAggregatedPts.push_back(spAggregatedFeature);
			if (spAggregatedFeature->getGeometry()->intersects(*(spCurrentFeature->getGeometry())))
			{
				spAggregatedFeature->addFeature(spCurrentFeature);
				bWasAggregated = true;
			}
		}
		assert(bWasAggregated && "Error found cell couldn't aggregate to");
	}
	
	m_bIsAggregated = true;
	return vecAggregatedPts;

}

std::string PointAggregatorProcess::getAttributeSchema() const
{
 	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"PointAggregatorProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"AggregatedResolution\" type=\"xs:int\" default=\"" + 
			  StringUtils::toString(m_nAggregatedResolution) + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Aggregated Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}


PYXPointer<FeatureIterator> PointAggregatorProcess::getIterator(const PYXGeometry& geometry) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (!m_bIsAggregated || geometry.getCellResolution() != m_nAggregatedResolution)
	{
		m_nAggregatedResolution = geometry.getCellResolution() - 7;
		m_bIsAggregated = false;
		m_vecAggregatedFeatures = aggregate(m_nAggregatedResolution);
	}
	std::vector<boost::intrusive_ptr<IFeature> > vecFeatureSet;
	for (std::vector<boost::intrusive_ptr<AggregatedPointFeature> >::const_iterator it = m_vecAggregatedFeatures.begin();
		it != m_vecAggregatedFeatures.end(); ++it)
	{
		if (geometry.intersects(*(*it)->getGeometry()))
		{
			boost::intrusive_ptr<IFeature> spFeat;
			(*it)->QueryInterface(IFeature::iid, (void**) &spFeat);
			if (spFeat)
			{
				vecFeatureSet.push_back(spFeat);
			}
		}
	}

	return PointAggregatorFeatureCollectionIterator::create(vecFeatureSet);
}


PYXPointer<FeatureIterator> PointAggregatorProcess::getIterator() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (!m_bIsAggregated)
	{
		m_vecAggregatedFeatures = aggregate(m_nAggregatedResolution);
	}
	return PointAggregatorFeatureCollectionIterator::create(convert(m_vecAggregatedFeatures));
}

//! Get the feature with the specified ID.
boost::intrusive_ptr<IFeature>  PointAggregatorProcess::getFeature(
	const std::string& strFeatureID) const
{
	if (!m_bIsAggregated)
	{
		m_vecAggregatedFeatures = aggregate(m_nAggregatedResolution);
	}
	std::vector<boost::intrusive_ptr<AggregatedPointFeature> >::const_iterator vecIt;

	for (vecIt = m_vecAggregatedFeatures.begin(); vecIt != m_vecAggregatedFeatures.end(); ++vecIt)
	{
		if ((*vecIt)->getID().compare(strFeatureID) == 0)
		{
			return (*vecIt);
		}
	}
	return boost::intrusive_ptr<IFeature>();
}


std::vector<boost::intrusive_ptr<IFeature> > PointAggregatorProcess::convert(std::vector<boost::intrusive_ptr<AggregatedPointFeature> > vecFeatures)const
{
	std::vector<boost::intrusive_ptr<IFeature> > vecIFeatures;

	for (std::vector<boost::intrusive_ptr<AggregatedPointFeature> >::const_iterator it = vecFeatures.begin(); it != vecFeatures.end(); ++it)
	{
		boost::intrusive_ptr<IFeature> spFeature;
		(*it)->QueryInterface(IFeature::iid, (void**) &spFeature);
		if (spFeature)
		{
			vecIFeatures.push_back(spFeature);
		}
	}
	return vecIFeatures;
}

const std::string& PointAggregatorProcess::getID() const
{
	return m_strId;
}

PYXPointer<PYXGeometry> PointAggregatorProcess::getGeometry()
{
	if (m_spGeom->isEmpty())
	{
		calcGeometry();
	}
	return m_spGeom;
}

PYXPointer<const PYXGeometry> PointAggregatorProcess::getGeometry() const
{
	return getGeometry();
}

std::string PointAggregatorProcess::getStyle() const
{
	return "";
}

std::string PointAggregatorProcess::getStyle(const std::string& strStyleToGet) const
{
	if (!m_bIsAggregated)
	{
		m_vecAggregatedFeatures = aggregate(m_nAggregatedResolution);
	}

	if(m_vecAggregatedFeatures.size() == 0)
	{
		assert(false && 
			"The features in the Point Aggregator process have not been aggregated!");
		return "";
	}
	else
	{
		return m_vecAggregatedFeatures[0]->getStyle(strStyleToGet);
	}	
}

void PointAggregatorProcess::calcGeometry() const
{
	m_spGeom = PYXMultiCell::create();
	if (!m_bIsAggregated)
	{
		m_vecAggregatedFeatures = aggregate(m_nAggregatedResolution);
	}

	if (m_spGeom->isEmpty())
	{
		for (std::vector<boost::intrusive_ptr<AggregatedPointFeature> >::const_iterator it = m_vecAggregatedFeatures.begin();
			it != m_vecAggregatedFeatures.end(); ++it)
		{
			boost::intrusive_ptr<AggregatedPointFeature> spFeat = (*it);
			m_spGeom->addCell(PYXCell::create(spFeat->getGeometry()->getIterator()->getIndex()));
		}
	}
}
