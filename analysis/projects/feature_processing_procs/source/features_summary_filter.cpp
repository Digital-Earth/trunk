/******************************************************************************
features_summary_filter.cpp

begin		: March 21, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "features_summary_filter.h"


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

#include "pyxis/region/circle_region.h"

#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"

#include "boost/scoped_ptr.hpp"

// standard includes
#include <cassert>


////////////////////////////////////////////////////////////////////////////////
// FeaturesSummaryAttributeRangeFilter
////////////////////////////////////////////////////////////////////////////////

// {94253CD3-AB9B-4c05-84D2-634FF31969F1}
PYXCOM_DEFINE_CLSID(FeaturesSummaryAttributeRangeFilter, 
0x94253cd3, 0xab9b, 0x4c05, 0x84, 0xd2, 0x63, 0x4f, 0xf3, 0x19, 0x69, 0xf1);

PYXCOM_CLASS_INTERFACES(FeaturesSummaryAttributeRangeFilter, IProcess::iid, IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeaturesSummaryAttributeRangeFilter, "Features Summary Filter", "Filter a features group by features properties.", "Development",
					IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureGroup::iid, 1, 1, "Input Feature Group", "A feature group.")
IPROCESS_SPEC_END

namespace
{
	//! Tester class
	Tester<FeaturesSummaryAttributeRangeFilter> gTester;
}

FeaturesSummaryAttributeRangeFilter::FeaturesSummaryAttributeRangeFilter()
{
	m_fieldIndex = 0;
	m_range = PYXValueRange(PYXValue(0),PYXValue(0),knClosed,knOpen);
}

FeaturesSummaryAttributeRangeFilter::~FeaturesSummaryAttributeRangeFilter()
{
}

void FeaturesSummaryAttributeRangeFilter::test()
{
	// TODO test something
}


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE FeaturesSummaryAttributeRangeFilter::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["MinValue"] = StringUtils::toString(m_range.min);
	mapAttr["MaxValue"] = StringUtils::toString(m_range.max);
	mapAttr["FieldIndex"] = StringUtils::toString(m_fieldIndex);
	return mapAttr;
}

std::string STDMETHODCALLTYPE FeaturesSummaryAttributeRangeFilter::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"FeaturesSummaryAttributeRangeFilter\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  
				  "<xs:element name=\"MinValue\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Minimum Value</friendlyName>"
						"<description>the minimum value to filter</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"MaxValue\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Minimum Value</friendlyName>"
						"<description>the minimum value to filter</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"
		  
			      "<xs:element name=\"FieldIndex\" type=\"xs:int\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Field Index</friendlyName>"
						"<description>Filed index to filter</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE FeaturesSummaryAttributeRangeFilter::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	PYXValue min;
	PYXValue max;

	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"MinValue",PYXValue,min);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"MaxValue",PYXValue,max);
	m_range = PYXValueRange::createClosedOpen(min,max);

	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"FieldIndex",int,m_fieldIndex);
}

class RangeFilter : public FeaturesSummaryFilterContext::Filter
{
private:
	PYXValueRange m_range;
	int m_fieldIndex;

public:
	static PYXPointer<RangeFilter> create(const PYXValueRange & range,int fieldIndex)
	{
		return PYXNEW(RangeFilter,range,fieldIndex);
	}

	RangeFilter(const PYXValueRange & range,int fieldIndex) : m_range(range), m_fieldIndex(fieldIndex)
	{
	}

public: 
	virtual Range<int> filter(boost::intrusive_ptr<IFeature> feature)
	{
		if (m_range.contains(feature->getFieldValue(m_fieldIndex)))
		{
			return Range<int>(1);
		}
		return Range<int>(0);
	}

	virtual Range<int> filter(boost::intrusive_ptr<IFeatureGroup> group)
	{
		PYXPointer<PYXHistogram> histogram = group->getFieldHistogram(m_fieldIndex);

		if (histogram)
		{
			return histogram->getFeatureCount(m_range);
		}

		return Range<int>::createClosedClosed(0,group->getFeaturesCount().max);
	}
};


IProcess::eInitStatus FeaturesSummaryAttributeRangeFilter::initImpl()
{	
	m_spInputFG = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureGroup>();

	if (!m_spInputFG)
	{
		return knFailedToInit;
	}

	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");
	const ProcessIdentityCache cache(cacheDir);
	const boost::filesystem::path pathCache = cache.getPath(getIdentity(), true);
	if (pathCache.empty())
	{
		PYXTHROW(PYXException, "Could not create cache directory.");
	}
	std::string strCacheDir = FileUtils::pathToString(pathCache);

	TRACE_INFO(getProcName() << " cache dir:" << strCacheDir);

	PYXPointer<PYXLocalStorage> storage = PYXProcessLocalStorage::create(getIdentity());

	m_context = FeaturesSummaryFilterContext::create(m_spInputFG,storage,RangeFilter::create(m_range,m_fieldIndex));
	m_rootGroup = m_context->getRootGroup();
	//m_rootGroup = GroupNode::create(m_spInputFG,storage,RangeFilter::create(m_range,m_fieldIndex));

	m_spGeom = m_spInputFG->getGeometry();
	m_bWritable = m_spInputFG->isWritable();
	m_strID = m_spInputFG->getID();
	m_strStyle = m_spInputFG->getStyle();

	return knInitialized;
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> FeaturesSummaryAttributeRangeFilter::getIterator() const
{
	return m_rootGroup->getIterator();
}

PYXPointer<FeatureIterator> FeaturesSummaryAttributeRangeFilter::getIterator(const PYXGeometry& geometry) const
{
	return m_rootGroup->getIterator(geometry);
}


std::vector<FeatureStyle> FeaturesSummaryAttributeRangeFilter::getFeatureStyles() const
{
	return m_spInputFG->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> FeaturesSummaryAttributeRangeFilter::getFeature(const std::string& strFeatureID) const
{
	return m_spInputFG->getFeature(strFeatureID);
}

PYXPointer<const PYXTableDefinition> FeaturesSummaryAttributeRangeFilter::getFeatureDefinition() const
{
	return m_spInputFG->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> FeaturesSummaryAttributeRangeFilter::getFeatureDefinition()
{
	return m_spInputFG->getFeatureDefinition();
}

bool FeaturesSummaryAttributeRangeFilter::canRasterize() const
{
	return m_spInputFG->canRasterize();
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureGroup
///////////////////////////////////////////////////////////////////////////////


Range<int> FeaturesSummaryAttributeRangeFilter::getFeaturesCount() const 
{
	return m_rootGroup->getFeaturesCount();
}

bool FeaturesSummaryAttributeRangeFilter::moreDetailsAvailable() const
{
	return m_rootGroup->moreDetailsAvailable();
}

PYXPointer<PYXHistogram> FeaturesSummaryAttributeRangeFilter::getFieldHistogram(int fieldIndex) const
{
	return m_rootGroup->getFieldHistogram(fieldIndex);
}

PYXPointer<PYXHistogram> FeaturesSummaryAttributeRangeFilter::getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const
{
	return m_rootGroup->getFieldHistogram(geometry, fieldIndex);
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeaturesSummaryAttributeRangeFilter::getGroupIterator() const
{
	return m_rootGroup->getGroupIterator();
}
	
PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeaturesSummaryAttributeRangeFilter::getGroupIterator(const PYXGeometry& geometry) const
{
	return m_rootGroup->getGroupIterator(geometry);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE FeaturesSummaryAttributeRangeFilter::getFeatureGroup(const std::string & groupId) const
{
	return m_rootGroup->getFeatureGroup(groupId);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE FeaturesSummaryAttributeRangeFilter::getFeatureGroupForFeature(const std::string & featureId) const
{
	return getFeatureGroup(m_spInputFG->getFeatureGroupForFeature(featureId)->getID());
}

///////////////////////////////////////////////////////////////////////////////
// FeaturesSummaryFilterContext
///////////////////////////////////////////////////////////////////////////////

class FeaturesSummaryFilterContext::GroupFactory : public PYXDataCollection::DataItemFactory<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>>
{
private:
	const PYXPointer<const FeaturesSummaryFilterContext> & m_context;
	const PYXPointer<GenericFeaturesGroup::NodeData> & m_data;
	const std::string & m_name;
public:
	GroupFactory(const PYXPointer<const FeaturesSummaryFilterContext> & context,
				 const PYXPointer<GenericFeaturesGroup::NodeData> & data,
				 const std::string & name) : m_context(context), m_data(data), m_name(name)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>> & item,int & itemSize) const
	{
		item = PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>::create(new GenericFeaturesGroup(m_context,m_data));
		itemSize = 20; //to indicate we allocated some memory...
	}
};

class FeaturesSummaryFilterContext::InputGroupFactory : public PYXDataCollection::DataItemFactory<PYXObjectWrapper<boost::intrusive_ptr<IFeatureGroup>>>
{
private:
	const PYXPointer<const FeaturesSummaryFilterContext> & m_context;
	const std::string & m_id;
	const std::string & m_name;
public:
	InputGroupFactory(const PYXPointer<const FeaturesSummaryFilterContext> & context,
					  const std::string & id,
					  const std::string & name) : m_context(context), m_id(id), m_name(name)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXObjectWrapper<boost::intrusive_ptr<IFeatureGroup>>> & item,int & itemSize) const
	{
		item = PYXObjectWrapper<boost::intrusive_ptr<IFeatureGroup>>::create(m_context->m_inputFG->getFeatureGroup(m_id));
		itemSize = 4;
	}
};

class FilteredChildrenFactory : public PYXDataCollection::DataItemFactory<GenericFeaturesGroup::ChildrenList>
{
private:
	const PYXPointer<PYXLocalStorage> & m_storage;
	const std::string & m_id;
	static const std::string m_name;
public:
	FilteredChildrenFactory(const PYXPointer<PYXLocalStorage> & storage,const std::string & id) : m_storage(storage), m_id(id)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<GenericFeaturesGroup::ChildrenList> & item,int & itemSize) const
	{
		boost::scoped_ptr<PYXConstWireBuffer> dataBuffer(m_storage->get("g:"+m_id+":cl"));
		if (dataBuffer)
		{
			PYXPointer<GenericFeaturesGroup::ChildrenList> cl = GenericFeaturesGroup::ChildrenList::create();
			(*dataBuffer) >> *cl;

			item = cl;
			itemSize = dataBuffer->size();
		}
	}
};

const std::string FilteredChildrenFactory::m_name = "cl";

class GeometryFactory : public PYXDataCollection::DataItemFactory<PYXTileCollection>
{
private:
	const GenericFeaturesGroup::NodeData & m_nodeData;
	const std::string & m_name;
public:
	GeometryFactory(const GenericFeaturesGroup::NodeData & nodeData,const std::string & geomName) : m_nodeData(nodeData), m_name(geomName)
	{
	}

	virtual const std::string & getName() const { return m_name; }

	virtual void createItem(PYXPointer<PYXTileCollection> & item,int & itemSize) const
	{
		const GenericFeaturesGroup::GroupData * groupData = dynamic_cast<const GenericFeaturesGroup::GroupData *>(& m_nodeData);

		if (groupData != 0)
		{
			PYXConstWireBuffer buffer(groupData->serializedGeometry);

			PYXPointer<PYXGeometry> geom;
			buffer >> geom;

			assert(geom->getCellResolution() > 1);

			item = boost::dynamic_pointer_cast<PYXTileCollection>(geom);
			itemSize = buffer.size()*100;
		}
	}
};

void FeaturesSummaryFilterContext::mergeSubGroup(boost::intrusive_ptr<IFeatureGroup> group,
			GenericFeaturesGroup::GroupData & data,
			GenericFeaturesGroup::ChildrenList & children,
			PYXPointer<PYXTileCollection> geom,
			boost::recursive_mutex & mutex) const
{
	boost::intrusive_ptr<GenericFeaturesGroup> filteredGroup = generateFilteredGroup(group);

	if (filteredGroup->getFeaturesCount().max != 0)
	{
		boost::recursive_mutex::scoped_lock lock(mutex);

		children.childrenNodes.push_back(filteredGroup->getNodeData());
		Range<int> groupCount = filteredGroup->getFeaturesCount();
		data.featuresCount.min += groupCount.min;
		data.featuresCount.max += groupCount.max;
		PYXPointer<PYXTileCollection> otherGeom = PYXTileCollection::create();
		filteredGroup->getGeometry()->copyTo(otherGeom.get(),geom->getCellResolution());

		geom->addGeometry(*otherGeom);
	}
}

boost::intrusive_ptr<GenericFeaturesGroup> FeaturesSummaryFilterContext::generateFilteredGroup(boost::intrusive_ptr<IFeatureGroup> inputGroup) const
{
	boost::recursive_mutex mutex;

	Range<int> featureCountEstimation = getFilter()->filter(m_inputFG);

	PYXPointer<GenericFeaturesGroup::GroupData> data = GenericFeaturesGroup::GroupData::create();
	GenericFeaturesGroup::ChildrenList children;
	
	//[shatzi:04/09/2014] this is a bug, as inputGroup->getID() of the root group is FS:{.....} and not empty string.
	data->id = inputGroup->getID();
	data->featuresCount = 0;

	PYXPointer<PYXTileCollection> geom = PYXTileCollection::create();
	geom->setCellResolution(inputGroup->getGeometry()->getCellResolution());

	PYXTaskGroup taskGroup;

	if (featureCountEstimation.max>0)
	{
		PYXPointer<FeatureIterator> it = inputGroup->getGroupIterator();
		
		while(!it->end())
		{
			boost::intrusive_ptr<IFeature> feature = it->getFeature();
			boost::intrusive_ptr<IFeatureGroup> group = feature->QueryInterface<IFeatureGroup>();

			if (group)
			{
				taskGroup.addTask(boost::bind(&FeaturesSummaryFilterContext::mergeSubGroup,this,group,boost::ref(*data),boost::ref(children),geom,boost::ref(mutex)));
			}
			else if (getFilter()->filter(feature).min>0)
			{
				boost::recursive_mutex::scoped_lock lock(mutex);

				PYXPointer<GenericFeaturesGroup::NodeData> featureNode = GenericFeaturesGroup::NodeData::create();

				featureNode->id = feature->getID();
				featureNode->circle = feature->getGeometry()->getBoundingCircle();
				featureNode->nodeType = GenericFeaturesGroup::knFeature;

				children.childrenNodes.push_back(featureNode);

				data->featuresCount.min++;
				data->featuresCount.max++;

				PYXPointer<PYXTileCollection> otherGeom = PYXTileCollection::create();
				feature->getGeometry()->copyTo(otherGeom.get(),geom->getCellResolution());

				geom->addGeometry(*otherGeom);
			}

			it->next();
		}
	}

	taskGroup.joinAll();

	//serialize geom (could be empty tile collection)
	PYXStringWireBuffer buffer;
	buffer << *geom;
	data->serializedGeometry = *buffer.getBuffer();

	//if we found some features... write down cl
	if (data->featuresCount.max>0)
	{
		data->circle = geom->getBoundingCircle();	

		buffer.clear();
		buffer << children;
		m_storage->set("g:"+data->id+":cl",buffer);
	}

	return new GenericFeaturesGroup(this,data);
}

boost::intrusive_ptr<GenericFeaturesGroup> FeaturesSummaryFilterContext::getRootGroup() const
{
	int version = 0;
	{
		boost::scoped_ptr<PYXWireBuffer> versionBuffer(m_storage->get("tree:version").release());
		if (versionBuffer)
		{
			(*versionBuffer) >> version;
		}
	}

	boost::scoped_ptr<PYXWireBuffer> dataBuffer(m_storage->get("g:").release());
	
	if (version != knCurrentVersion || !dataBuffer)
	{
		m_storage->removeAll();

		PYXPointer<PYXLocalStorage> old = m_storage;
		m_storage = PYXBufferedLocalStorage::create(old);

		boost::intrusive_ptr<GenericFeaturesGroup> root = generateFilteredGroup(m_inputFG);

		PYXStringWireBuffer buffer;
		buffer << *(root->getNodeData());
		m_storage->set("g:",buffer);
		
		PYXStringWireBuffer versionBuffer;
		versionBuffer << knCurrentVersion;
		m_storage->set("tree:version",versionBuffer);

		m_storage = old;
	}

	dataBuffer.reset(m_storage->get("g:").release());
	if (dataBuffer)
	{
		PYXPointer<GenericFeaturesGroup::GroupData> data = GenericFeaturesGroup::GroupData::create();
		(*dataBuffer) >> *data;

		return new GenericFeaturesGroup(this,data);
	}

	PYXTHROW(PYXException,"we should never get here");
}

PYXPointer<GenericFeaturesGroup::ChildrenList> FeaturesSummaryFilterContext::getChildren(const GenericFeaturesGroup & parent) const
{	
	return getDataCollectionForGroup(parent).getItem(FilteredChildrenFactory(m_storage,parent.getID()));
}

Range<int> FeaturesSummaryFilterContext::getFeaturesCount(const GenericFeaturesGroup & parent) const
{
	return dynamic_cast<const GenericFeaturesGroup::GroupData*>(&getDataForGroup(parent))->featuresCount;
}

boost::intrusive_ptr<IFeature> FeaturesSummaryFilterContext::getFeature(const GenericFeaturesGroup & parent, const std::string & featureId) const
{
	return getInputGroup(parent)->getFeature(featureId);
}

std::string FeaturesSummaryFilterContext::getFeatureGroupIdForFeature(const std::string & featureId) const
{
	return m_inputFG->getFeatureGroupForFeature(featureId)->getID();
}

boost::intrusive_ptr<IFeature> FeaturesSummaryFilterContext::getFeature(const GenericFeaturesGroup & parent, const PYXPointer<GenericFeaturesGroup::NodeData> & featureData) const
{
	return getInputGroup(parent)->getFeature(featureData->id);
}


boost::intrusive_ptr<GenericFeaturesGroup> FeaturesSummaryFilterContext::getGroup(const GenericFeaturesGroup & parent, const PYXPointer<GenericFeaturesGroup::NodeData> & groupData) const
{
	return getDataCollectionForGroup(parent).getItem(GroupFactory(this,groupData,"g:"+groupData->id))->value;
}

PYXPointer<PYXTableDefinition> FeaturesSummaryFilterContext::getFeatureDefinition(const GenericFeaturesGroup & parent) const
{
	return m_inputFG->getFeatureDefinition();
}

PYXValue FeaturesSummaryFilterContext::getFieldValue(const GenericFeaturesGroup & parent,int index) const
{
	return PYXValue();
}


PYXPointer<PYXGeometry> FeaturesSummaryFilterContext::getGeometry(const GenericFeaturesGroup & parent) const
{
	return getDataCollectionForGroup(parent).getItem(GeometryFactory(getDataForGroup(parent),"geom"));
}

std::string FeaturesSummaryFilterContext::getStyle(const GenericFeaturesGroup & parent) const
{
	return m_inputFG->getStyle();
}

boost::intrusive_ptr<IFeatureGroup> FeaturesSummaryFilterContext::getInputGroup(const GenericFeaturesGroup & parent) const
{
	return getDataCollectionForGroup(parent).getItem(InputGroupFactory(this,parent.getID(),"i"))->value;
}

////////////////////////////////////////////////////////////////////////////////
// FeaturesSummaryGeometryFilter
////////////////////////////////////////////////////////////////////////////////

// {93CD5A3B-26CB-4643-9CE8-CFAC201447DF}
PYXCOM_DEFINE_CLSID(FeaturesSummaryGeometryFilter, 
0x93cd5a3b, 0x26cb, 0x4643, 0x9c, 0xe8, 0xcf, 0xac, 0x20, 0x14, 0x47, 0xdf);

PYXCOM_CLASS_INTERFACES(FeaturesSummaryGeometryFilter, IProcess::iid, IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeaturesSummaryGeometryFilter, "Features Summary Geometry Filter", "Filter a features group by a geometry.", "Development",
					IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureGroup::iid, 1, 1, "Input Feature Group", "A feature group.")
	IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "Filter Geometry", "A geometry to filter with.")
IPROCESS_SPEC_END

namespace
{
	//! Tester class
	Tester<FeaturesSummaryGeometryFilter> gGeometryTester;
}

FeaturesSummaryGeometryFilter::FeaturesSummaryGeometryFilter()
{	
}

FeaturesSummaryGeometryFilter::~FeaturesSummaryGeometryFilter()
{
}

void FeaturesSummaryGeometryFilter::test()
{
	// TODO test something
}


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE FeaturesSummaryGeometryFilter::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	
	return mapAttr;
}

std::string STDMETHODCALLTYPE FeaturesSummaryGeometryFilter::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"FeaturesSummaryGeometryFilter\">"
		  "<xs:complexType>"
			"<xs:sequence>"			  

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE FeaturesSummaryGeometryFilter::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;
}

class GeometryFilter : public FeaturesSummaryFilterContext::Filter
{
private:
	PYXPointer<PYXGeometry> m_geometry;
	PYXPointer<PYXTileCollection> m_geometryAsTileCollection;

public:
	static PYXPointer<GeometryFilter> create(const PYXPointer<PYXGeometry>  & geometry)
	{
		return PYXNEW(GeometryFilter,geometry);
	}

	GeometryFilter(const PYXPointer<PYXGeometry>  & geometry) : m_geometry(geometry)
	{
	}

public: 
	virtual Range<int> filter(boost::intrusive_ptr<IFeature> feature)
	{
		if (m_geometry->intersects(*feature->getGeometry()))
		{
			return Range<int>(1);
		}
		return Range<int>(0);
	}

	virtual Range<int> filter(boost::intrusive_ptr<IFeatureGroup> group)
	{
		//first check if the geometry intersects at all...
		if (!m_geometry->intersects(*group->getGeometry()))
		{
			return Range<int>(0);
		}

		if (!m_geometryAsTileCollection)
		{
			m_geometryAsTileCollection = PYXTileCollection::create(m_geometry);
		}

		if (m_geometryAsTileCollection->contains(*PYXTileCollection::create(group->getGeometry())))
		{
			return Range<int>(group->getFeaturesCount().max);
		}

		return Range<int>::createClosedClosed(0,group->getFeaturesCount().max);
	}
};


IProcess::eInitStatus FeaturesSummaryGeometryFilter::initImpl()
{	
	m_spInputFG = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureGroup>();
	m_spInputGeometry = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IFeature>();

	if (!m_spInputFG)
	{
		setInitProcError<GenericProcInitError>("Can't initialize Input Feature Group from Paramter 0");
		return knFailedToInit;
	}

	if (!m_spInputGeometry)
	{
		setInitProcError<GenericProcInitError>("Can't initialize Input Filter Geometry from Paramter 1");
		return knFailedToInit;
	}

	PYXPointer<PYXGeometry> inputGeometry = m_spInputGeometry->getGeometry();

	if (!inputGeometry )
	{
		setInitProcError<GenericProcInitError>("Can't initialize Input Filter Geometry from Paramter 1");
		return knFailedToInit;
	}


	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");
	const ProcessIdentityCache cache(cacheDir);
	const boost::filesystem::path pathCache = cache.getPath(getIdentity(), true);
	if (pathCache.empty())
	{
		PYXTHROW(PYXException, "Could not create cache directory.");
	}
	std::string strCacheDir = FileUtils::pathToString(pathCache);

	TRACE_INFO(getProcName() << " cache dir:" << strCacheDir);

	PYXPointer<PYXLocalStorage> storage = PYXProcessLocalStorage::create(getIdentity());

	m_context = FeaturesSummaryFilterContext::create(m_spInputFG,storage,GeometryFilter::create(inputGeometry));
	m_rootGroup = m_context->getRootGroup();
	//m_rootGroup = GroupNode::create(m_spInputFG,storage,RangeFilter::create(m_range,m_fieldIndex));

	m_spGeom = inputGeometry; //take the filter geometry as our geometry
	m_bWritable = m_spInputFG->isWritable();
	m_strID = m_spInputFG->getID();
	m_strStyle = m_spInputFG->getStyle();

	return knInitialized;
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> FeaturesSummaryGeometryFilter::getIterator() const
{
	return m_rootGroup->getIterator();
}

PYXPointer<FeatureIterator> FeaturesSummaryGeometryFilter::getIterator(const PYXGeometry& geometry) const
{
	return m_rootGroup->getIterator(geometry);
}


std::vector<FeatureStyle> FeaturesSummaryGeometryFilter::getFeatureStyles() const
{
	return m_spInputFG->getFeatureStyles();
}

boost::intrusive_ptr<IFeature> FeaturesSummaryGeometryFilter::getFeature(const std::string& strFeatureID) const
{
	return m_spInputFG->getFeature(strFeatureID);
}

PYXPointer<const PYXTableDefinition> FeaturesSummaryGeometryFilter::getFeatureDefinition() const
{
	return m_spInputFG->getFeatureDefinition();
}

PYXPointer<PYXTableDefinition> FeaturesSummaryGeometryFilter::getFeatureDefinition()
{
	return m_spInputFG->getFeatureDefinition();
}

bool FeaturesSummaryGeometryFilter::canRasterize() const
{
	return m_spInputFG->canRasterize();
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureGroup
///////////////////////////////////////////////////////////////////////////////


Range<int> FeaturesSummaryGeometryFilter::getFeaturesCount() const 
{
	return m_rootGroup->getFeaturesCount();
}

bool FeaturesSummaryGeometryFilter::moreDetailsAvailable() const
{
	return m_rootGroup->moreDetailsAvailable();
}

PYXPointer<PYXHistogram> FeaturesSummaryGeometryFilter::getFieldHistogram(int fieldIndex) const
{
	return m_rootGroup->getFieldHistogram(fieldIndex);
}

PYXPointer<PYXHistogram> FeaturesSummaryGeometryFilter::getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const
{
	return m_rootGroup->getFieldHistogram(geometry, fieldIndex);
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeaturesSummaryGeometryFilter::getGroupIterator() const
{
	return m_rootGroup->getGroupIterator();
}
	
PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeaturesSummaryGeometryFilter::getGroupIterator(const PYXGeometry& geometry) const
{
	return m_rootGroup->getGroupIterator(geometry);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE FeaturesSummaryGeometryFilter::getFeatureGroup(const std::string & groupId) const
{
	return m_rootGroup->getFeatureGroup(groupId);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE FeaturesSummaryGeometryFilter::getFeatureGroupForFeature(const std::string & featureId) const
{
	return getFeatureGroup(m_spInputFG->getFeatureGroupForFeature(featureId)->getID());
}
