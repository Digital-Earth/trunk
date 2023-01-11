/******************************************************************************
generic_feature_group.cpp

begin		: Apr 09, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "generic_feature_group.h"


#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/thread_pool.h"
#include "pyxis/utility/numeric_histogram.h"
#include "pyxis/utility/string_histogram.h"

#include "pyxis/region/circle_region.h"

#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/data/impl/numeric_histogram_impl.h"
#include "pyxis/data/impl/string_histogram_impl.h"
#include "pyxis/data/feature_iterator_linq.h"

#include "boost/scoped_ptr.hpp"

// standard includes
#include <cassert>

///////////////////////////////////////////////////////////////////////////////
// GenericFeaturesGroup::GroupIterator
///////////////////////////////////////////////////////////////////////////////

GenericFeaturesGroup::GroupIterator::GroupIterator(const PYXPointer<const GenericFeaturesGroup> & group,const PYXPointer<PYXGeometry> & geometry)
	: m_group(group), m_geometry(geometry), m_children(group->m_context->getChildren(*group))
{
	m_childrenIt = m_children->childrenNodes.begin();
	findNextMatch();
}

GenericFeaturesGroup::GroupIterator::~GroupIterator()
{
}

bool GenericFeaturesGroup::GroupIterator::end() const
{
	return m_childrenIt == m_children->childrenNodes.end();
}

void GenericFeaturesGroup::GroupIterator::findNextMatch()
{
	if (!m_geometry)
		return;

	while (m_childrenIt != m_children->childrenNodes.end())
	{
		if (m_geometry->intersects(PYXVectorGeometry((*m_childrenIt)->circle)))
		{
			return;
		}
		++m_childrenIt;
	}
}

void GenericFeaturesGroup::GroupIterator::next()
{
	//move to the next item..
	if (m_childrenIt != m_children->childrenNodes.end())
	{
		++m_childrenIt;
	}

	findNextMatch();
}

boost::intrusive_ptr<IFeature> GenericFeaturesGroup::GroupIterator::getFeature() const
{
	if (m_childrenIt != m_children->childrenNodes.end())
	{
		if ((*m_childrenIt)->nodeType == knFeature)
		{
			return m_group->m_context->getFeature(*m_group,*m_childrenIt);
		}
		else if ((*m_childrenIt)->nodeType == knGroup)
		{
			return m_group->m_context->getGroup(*m_group,*m_childrenIt);
		}
		PYXTHROW(PYXException,"unknown node type");
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// GenericFeaturesGroup::Iterator
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> expandFeatures(const boost::intrusive_ptr<IFeature> & feature)
{
	boost::intrusive_ptr<IFeatureGroup> & featureGroup = feature->QueryInterface<IFeatureGroup>();
	if (featureGroup)
	{
		return PYXFeatureIteratorLinq(featureGroup->getGroupIterator()).selectMany(boost::bind(expandFeatures,_1));
	}
	else 
	{
		return PYXFeatureIteratorLinq(feature);
	}
}

PYXPointer<FeatureIterator> expandFeaturesWithGeometry(const boost::intrusive_ptr<IFeature> & feature, PYXPointer<PYXGeometry> geometry )
{

	boost::intrusive_ptr<IFeatureGroup> & featureGroup = feature->QueryInterface<IFeatureGroup>();
	if (featureGroup)
	{
		return PYXFeatureIteratorLinq(featureGroup->getGroupIterator(*geometry)).selectMany(boost::bind(expandFeaturesWithGeometry,_1,geometry));
	}
	else 
	{
		return PYXFeatureIteratorLinq(feature);
	}
}


PYXPointer<FeatureIterator> STDMETHODCALLTYPE GenericFeaturesGroup::getIterator() const
{
	return PYXFeatureIteratorLinq(getGroupIterator()).selectMany(boost::bind(expandFeatures,_1));

	//return Iterator::create(this);
}


PYXPointer<FeatureIterator> STDMETHODCALLTYPE GenericFeaturesGroup::getIterator( const PYXGeometry& geometry ) const
{
	return PYXFeatureIteratorLinq(getGroupIterator(geometry)).selectMany(boost::bind(expandFeaturesWithGeometry,_1,geometry.clone()));

	//return Iterator::create(this,geometry.clone());
}

/*
[shatzi]:Replaced by using FeatureIteratorLinq::selectMany.

GenericFeaturesGroup::Iterator::Iterator(const PYXPointer<const GenericFeaturesGroup> & group,const PYXPointer<PYXGeometry> & geometry)
	: m_group(group), m_geometry(geometry), m_children(group->m_context->getChildren(*group))
{
	m_childrenIt = m_children->childrenNodes.begin();
	
	findNextMatch();

	//special case when there are no matching features in the group - we need find the next feature in the sub group
	if (m_childrenIt != m_children->childrenNodes.end() && (*m_childrenIt)->nodeType == knGroup)
	{
		next();
	}
}

GenericFeaturesGroup::Iterator::~Iterator()
{
}

bool GenericFeaturesGroup::Iterator::end() const
{
	return m_childrenIt == m_children->childrenNodes.end() && !m_curChildIt;
}

void GenericFeaturesGroup::Iterator::findNextMatch()
{
	if (!m_geometry)
		return;

	while (m_childrenIt != m_children->childrenNodes.end())
	{
		if (m_geometry->intersects(PYXVectorGeometry((*m_childrenIt)->circle)))
		{
			return;
		}
		++m_childrenIt;
	}
}

void GenericFeaturesGroup::Iterator::next()
{
	if (m_curChildIt && !m_curChildIt->end())
	{
		m_curChildIt->next();

		if (m_curChildIt->end())
		{
			m_curChildIt.reset();

			//if next time is a group - get it. if the next feature is a feature - we already pointing to it...
			if (m_childrenIt != m_children->childrenNodes.end() && (*m_childrenIt)->nodeType == knGroup)
			{
				next();
			}
		}
	}
	else if (m_childrenIt != m_children->childrenNodes.end())
	{
		if ((*m_childrenIt)->nodeType == knGroup)
		{
			m_curChildIt = Iterator::create(m_group->getGroup(*m_childrenIt),m_geometry);
			++m_childrenIt;
			findNextMatch();

			if (m_curChildIt->end())
			{
				m_curChildIt.reset();
				next();
			}
		}
		else
		{
			++m_childrenIt;
			findNextMatch();
		}
	}
}

boost::intrusive_ptr<IFeature> GenericFeaturesGroup::Iterator::getFeature() const
{
	if (m_curChildIt && !m_curChildIt->end())
	{
		return m_curChildIt->getFeature();
	}

	if (m_childrenIt != m_children->childrenNodes.end() && (*m_childrenIt)->nodeType == knFeature)
	{
		return m_group->m_context->getFeature(*m_group,*m_childrenIt);
	}

	return 0;
}
*/

///////////////////////////////////////////////////////////////////////////////
// GenericFeaturesGroup::NodeData
///////////////////////////////////////////////////////////////////////////////

PYXWireBuffer & operator>>(PYXWireBuffer & buffer,GenericFeaturesGroup::NodeData & data)
{
	double x,y,z,r;	

	unsigned char type = data.nodeType;

	buffer >> type >> data.id >> x >> y >> z >> r >> data.serializedData;

	data.circle = PYXBoundingCircle(PYXCoord3DDouble(x,y,z),r);

	data.nodeType = (GenericFeaturesGroup::NodeType)type;
	data.readSerializedData();

	return buffer;
}

PYXWireBuffer & operator<<(PYXWireBuffer & buffer,const GenericFeaturesGroup::NodeData & data)
{
	data.createSerializedData();

	unsigned char type = (unsigned char) data.nodeType;
	buffer << type;
	buffer << data.id;
	buffer << data.circle.getCenter().x() << data.circle.getCenter().y() << data.circle.getCenter().z() << data.circle.getRadius();
	buffer << data.serializedData;


	return buffer;
}

PYXWireBuffer & operator>>(PYXWireBuffer & buffer,GenericFeaturesGroup::ChildrenList & data)
{
	int count;

	buffer >> PYXCompactInteger(count);

	data.childrenNodes.clear();
	data.childrenNodes.reserve(count);

	for(int i=0;i<count;++i)
	{
		PYXPointer<GenericFeaturesGroup::NodeData> child = GenericFeaturesGroup::NodeData::create();

		buffer >> *child;

		//HACK: this should be implemented somewhere else.. but it easy to write it here
		if (child->nodeType == GenericFeaturesGroup::knGroup)
		{
			//convert the node into GroupData...
			PYXPointer<GenericFeaturesGroup::GroupData> groupChild = GenericFeaturesGroup::GroupData::create();
			groupChild->id = child->id;
			groupChild->circle = child->circle;
			groupChild->serializedData = child->serializedData;
			groupChild->readSerializedData();

			data.childrenNodes.push_back(groupChild);
		}
		else
		{
			data.childrenNodes.push_back(child);
		}
	}

	return buffer;
}

PYXWireBuffer & operator<<(PYXWireBuffer & buffer,const GenericFeaturesGroup::ChildrenList & data)
{
	int count = (int)data.childrenNodes.size();

	buffer << PYXCompactInteger(count);

	for(int i=0;i<count;++i)
	{
		buffer << *(data.childrenNodes[i]);
	}
	
	return buffer;
}

////////////////////////////////////////////////////////////////////////////////
// PYXNumericHistogram
////////////////////////////////////////////////////////////////////////////////

//class PYXNumericHistogram : public PYXHistogram
//{
//protected:
//	NumericHistogram<double> m_histogram;
//
//public:
//	static PYXPointer<PYXNumericHistogram> create(const NumericHistogram<double> & other)
//	{
//		return PYXNEW(PYXNumericHistogram,other);
//	}
//
//	PYXNumericHistogram(const NumericHistogram<double> & other) : m_histogram(other)
//	{
//	}
//
//public:
//	virtual Range<int> getFeatureCount() const
//	{
//		return Range<int>(m_histogram.count());
//	}
//	
//	virtual Range<int> getFeatureCount(Range<PYXValue> range) const 
//	{
//		return m_histogram.count(Range<double>::createClosedOpen(range.min.getDouble(),range.max.getDouble()));
//	}
//
//	// get summation of the field values (not supported for not numeric types)
//	virtual PYXValue getSum() const 
//	{
//		return PYXValue(m_histogram.getSum());
//	}
//
//	// get average of the field values (not supported for not numeric types)
//	virtual PYXValue getAverage() const 
//	{
//		return PYXValue(m_histogram.getAverage());
//	}
//
//	// get summation of the field values square (not supported for not numeric types)
//	virtual PYXValue getSumSquare() const 
//	{
//		return PYXValue(m_histogram.getSumSquare());
//	}
//
//	virtual Range<PYXValue> getBoundaries() const
//	{
//		Range<double> boundary(m_histogram.getBoundaries());
//
//		return Range<PYXValue>::createClosedClosed(PYXValue(boundary.min),PYXValue(boundary.max));
//	}
//
//	virtual std::vector<PYXHistogramBin> getBins() const
//	{
//		std::vector<NumericHistogram<double>::LeafBin> bins;
//
//		m_histogram.getLeafBins(bins);
//
//		std::vector<PYXHistogramBin> result(bins.size());
//
//		int i=0;
//		for(std::vector<NumericHistogram<double>::LeafBin>::iterator it=bins.begin();it!=bins.end();++it,++i)
//		{
//			result[i].range = Range<PYXValue>::createClosedOpen(PYXValue(it->range.min),PYXValue(it->range.max));
//			result[i].count = it->count;
//		}
//
//		return result;
//	}
//
//	virtual void add(const PYXValue & value)
//	{
//		m_histogram.add(NumericHistogram<double>(value.getDouble()));
//	}
//
//	virtual void add(const PYXHistogram & histogram)
//	{
//		const PYXNumericHistogram * other = dynamic_cast<const PYXNumericHistogram*>(&histogram);
//
//		if (!other)
//		{
//			PYXTHROW(PYXException,"other histogram is not PYXNumericHistogram");
//		}
//
//		m_histogram.add(other->m_histogram);
//	}
//
//	NumericHistogram<double> & getDoubleHistogram() { return m_histogram; }
//
//	const NumericHistogram<double> & getDoubleHistogram() const { return m_histogram; }
//};
//
////////////////////////////////////////////////////////////////////////////////
// StringFeaturesSummaryFieldHistogram
////////////////////////////////////////////////////////////////////////////////


class NumericHistogramFactory : public PYXDataCollection::DataItemFactory<PYXHistogram>
{
private:
	const GenericFeaturesGroup & m_group;
	const PYXPointer<PYXLocalStorage> & m_storage;
	int m_fieldIndex;
	const std::string & m_name;

	mutable boost::recursive_mutex m_mutex;

public:
	NumericHistogramFactory(const GenericFeaturesGroup & group,
							const PYXPointer<PYXLocalStorage> & storage,
							int fieldIndex,
							const std::string & name) 
		: m_group(group), m_storage(storage), m_fieldIndex(fieldIndex),m_name(name)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXHistogram> & item,int & itemSize) const
	{
		std::string key = "hist:"+StringUtils::toString(m_fieldIndex)+":"+m_group.getID();
		boost::scoped_ptr<PYXWireBuffer> dataBuffer(m_storage->get(key));

		boost::scoped_ptr<NumericHistogram<double>> nodeHist(new NumericHistogram<double>());

		if (dataBuffer)
		{
			*dataBuffer >> *nodeHist;

			item = PYXNumericHistogram::create(*nodeHist);
			itemSize = sizeof(NumericHistogram<double>);
		}
		else
		{
			PYXPointer<PYXNumericHistogram> hist = PYXNumericHistogram::create(*nodeHist);

			m_group.visitFeatures(boost::bind(&NumericHistogramFactory::addFeatureToHistogram,this,_1,boost::ref(hist)));
			m_group.visitSubGroupsParallel(boost::bind(&NumericHistogramFactory::addGroupHistogram,this,_1,boost::ref(hist)))->join();

			hist->getDoubleHistogram().limit(1000);

			dataBuffer.reset(new PYXStringWireBuffer);
			*dataBuffer << hist->getDoubleHistogram();

			m_storage->set(key,*dataBuffer);

			item = hist;
			itemSize = sizeof(NumericHistogram<double>);
		}

		assert(item->getFeatureCount() == m_group.getFeaturesCount());
	}

private:
	void addFeatureToHistogram(const boost::intrusive_ptr<IFeature> & feature,const PYXPointer<PYXNumericHistogram> & histogram) const
	{
		histogram->add(feature->getFieldValue(m_fieldIndex));
	}

	void addGroupHistogram(const boost::intrusive_ptr<GenericFeaturesGroup> & group,const PYXPointer<PYXNumericHistogram> & histogram) const
	{
		PYXPointer<PYXHistogram> groupHist = group->getFieldHistogram(m_fieldIndex);

		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			histogram->add(*groupHist);
		}
	}
};


class NumericHistogramSpatialFactory 
{
private:
	const GenericFeaturesGroup & m_group;
	const PYXPointer<PYXLocalStorage> & m_storage;
	const PYXGeometry & m_geometry;
	int m_fieldIndex;

	mutable boost::recursive_mutex m_mutex;
	
public:
	NumericHistogramSpatialFactory(const GenericFeaturesGroup & group,
							const PYXPointer<PYXLocalStorage> & storage,
							const PYXGeometry & geometry,
							int fieldIndex) 
		: m_group(group), m_storage(storage), m_geometry(geometry), m_fieldIndex(fieldIndex)
	{
	}

	PYXPointer<PYXHistogram> createHistogram() const
	{
		return createHistogram(m_group);
	}

	PYXPointer<PYXHistogram> createHistogram(const GenericFeaturesGroup & group) const
	{
		PYXPointer<PYXNumericHistogram> hist = PYXNumericHistogram::create(NumericHistogram<double>());

		//root geometry have invalide geometries
		if (group.getID() != "")
		{
			//we have valid geometry - check for optimizations...
			if (m_geometry.contains(*group.getGeometry()))
			{
				return group.getFieldHistogram(m_fieldIndex);
			}

			if (!m_geometry.intersects(*group.getGeometry()))
			{
				return hist;
			}
		}
		
		group.visitFeatures(boost::bind(&NumericHistogramSpatialFactory::addFeatureToHistogram,this,_1,boost::ref(hist)),m_geometry);
		group.visitSubGroupsParallel(boost::bind(&NumericHistogramSpatialFactory::addGroupHistogram,this,_1,boost::ref(hist)),m_geometry)->join();

		assert(hist->getFeatureCount().max <= group.getFeaturesCount().max);

		hist->getDoubleHistogram().limit(1000);		

		return hist;
	}

private:
	void addFeatureToHistogram(const boost::intrusive_ptr<IFeature> & feature,const PYXPointer<PYXNumericHistogram> & histogram) const
	{
		if (feature->getGeometry()->intersects(m_geometry))
		{
			histogram->add(feature->getFieldValue(m_fieldIndex));
		}
	}

	void addGroupHistogram(const boost::intrusive_ptr<GenericFeaturesGroup> & group,const PYXPointer<PYXNumericHistogram> & histogram) const
	{
		PYXPointer<PYXHistogram> groupHist = createHistogram(*group);

		//add into total result if histogram has any values inside it
		if (groupHist->getFeatureCount().max > 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			histogram->add(*groupHist);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// GenericFeaturesGroup::ContextWithHistograms
///////////////////////////////////////////////////////////////////////////////

class StringHistogramFactory : public PYXDataCollection::DataItemFactory<PYXHistogram>
{
private:
	const GenericFeaturesGroup & m_group;
	const PYXPointer<PYXLocalStorage> & m_storage;
	int m_fieldIndex;
	const std::string & m_name;

	mutable boost::recursive_mutex m_mutex;

public:
	StringHistogramFactory(const GenericFeaturesGroup & group,
							const PYXPointer<PYXLocalStorage> & storage,
							int fieldIndex,
							const std::string & name) 
		: m_group(group), m_storage(storage), m_fieldIndex(fieldIndex),m_name(name)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXHistogram> & item,int & itemSize) const
	{
		std::string key = "hist:"+StringUtils::toString(m_fieldIndex)+":"+m_group.getID();
		boost::scoped_ptr<PYXWireBuffer> dataBuffer(m_storage->get(key));

		if (dataBuffer)
		{
			StringHistogram stringHist;
			*dataBuffer >> stringHist;

			item = PYXStringHistogram::create(stringHist);
			itemSize = sizeof(StringHistogram);
		}
		else
		{
			PYXPointer<PYXStringHistogram> hist = PYXStringHistogram::create();

			m_group.visitFeatures(boost::bind(&StringHistogramFactory::addFeatureToHistogram,this,_1,boost::ref(hist)));
			m_group.visitSubGroupsParallel(boost::bind(&StringHistogramFactory::addGroupHistogram,this,_1,boost::ref(hist)))->join();

			hist->getStringHistogram().limit(4000);

			dataBuffer.reset(new PYXStringWireBuffer);
			*dataBuffer << hist->getStringHistogram();

			m_storage->set(key,*dataBuffer);

			item = hist;
			itemSize = sizeof(StringHistogram);
		}
	}

private:
	void addFeatureToHistogram(const boost::intrusive_ptr<IFeature> & feature,const PYXPointer<PYXStringHistogram> & histogram) const
	{
		histogram->add(feature->getFieldValue(m_fieldIndex));
	}

	void addGroupHistogram(const boost::intrusive_ptr<GenericFeaturesGroup> & group,const PYXPointer<PYXStringHistogram> & histogram) const
	{
		PYXPointer<PYXHistogram> groupHist = group->getFieldHistogram(m_fieldIndex);

		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			histogram->add(*groupHist);			
		}
	}
};

class StringHistogramSpatialFactory 
{
private:
	const GenericFeaturesGroup & m_group;
	const PYXPointer<PYXLocalStorage> & m_storage;
	const PYXGeometry & m_geometry;
	int m_fieldIndex;

	mutable boost::recursive_mutex m_mutex;

public:
	StringHistogramSpatialFactory(const GenericFeaturesGroup & group,
							const PYXPointer<PYXLocalStorage> & storage,
							const PYXGeometry & geometry,
							int fieldIndex) 
		: m_group(group), m_storage(storage), m_geometry(geometry), m_fieldIndex(fieldIndex)
	{
	}

	PYXPointer<PYXHistogram> createHistogram() const
	{
		return createHistogram(m_group);
	}

	PYXPointer<PYXHistogram> createHistogram(const GenericFeaturesGroup & group) const
	{
		PYXPointer<PYXStringHistogram> hist = PYXStringHistogram::create();

		//root geometry have invalide geometries
		if (group.getID() != "")
		{
			//we have valid geometry - check for optimizations...
			if (m_geometry.contains(*group.getGeometry()))
			{
				return group.getFieldHistogram(m_fieldIndex);
			}

			/*
			 * [SHATZI]: Disabled as this causes bugs of small resolutions intersecting with TileCollections
			 *
			if (!m_geometry.intersects(*group.getGeometry()))
			{
				return hist;
			}
			*/
		}

		group.visitFeatures(boost::bind(&StringHistogramSpatialFactory::addFeatureToHistogram,this,_1,boost::ref(hist)),m_geometry);
		group.visitSubGroupsParallel(boost::bind(&StringHistogramSpatialFactory::addGroupHistogram,this,_1,boost::ref(hist)),m_geometry)->join();

		hist->getStringHistogram().limit(4000);

		return hist;
	}

private:
	void addFeatureToHistogram(const boost::intrusive_ptr<IFeature> & feature,const PYXPointer<PYXStringHistogram> & histogram) const
	{
		if (feature->getGeometry()->intersects(m_geometry))
		{
			histogram->add(feature->getFieldValue(m_fieldIndex));
		}
	}

	void addGroupHistogram(const boost::intrusive_ptr<GenericFeaturesGroup> & group,const PYXPointer<PYXStringHistogram> & histogram) const
	{
		PYXPointer<PYXHistogram> groupHist = createHistogram(*group);

		//add into total result if histogram has any values inside it
		if (groupHist->getFeatureCount().max > 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			histogram->add(*groupHist);
		}
	}
};

PYXPointer<PYXHistogram> GenericFeaturesGroup::ContextWithHistograms::getFieldHistogram(const GenericFeaturesGroup & parent,const PYXGeometry & geometry, int nFieldIndex) const
{
	if (getFeatureDefinition(parent)->getFieldDefinition(nFieldIndex).isNumeric())
	{
		return NumericHistogramSpatialFactory(parent,m_storage,geometry,nFieldIndex).createHistogram();
	}
	else 
	{
		return StringHistogramSpatialFactory(parent,m_storage,geometry,nFieldIndex).createHistogram();
	}
}

PYXPointer<PYXHistogram> GenericFeaturesGroup::ContextWithHistograms::getFieldHistogram(const GenericFeaturesGroup & parent,int nFieldIndex) const
{
	std::string key = "h:"+StringUtils::toString(nFieldIndex);
	if (getFeatureDefinition(parent)->getFieldDefinition(nFieldIndex).isNumeric())
	{
		return getDataCollectionForGroup(parent).getItem(NumericHistogramFactory(parent,m_storage,nFieldIndex,key));
	}
	else 
	{
		return getDataCollectionForGroup(parent).getItem(StringHistogramFactory(parent,m_storage,nFieldIndex,key));
	}
}
