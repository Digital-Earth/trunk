/******************************************************************************
features_summary.cpp

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "features_summary.h"

#include "icos_tree.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/thread_pool.h"
#include "pyxis/utility/profile.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/procs/geopacket_source.h"

#include "boost/algorithm/string.hpp"
#include "boost/scoped_ptr.hpp"

// standard includes
#include <cassert>
#include "pyxis/utility/ssl_utils.h"
#include "pyxis/utility/local_storage_impl.h"


#define MULTITHREAD_IMPORT
#define FEATURE_SUMMARY_CHANNEL_ID "FSv6"

// {E6C3802D-E7B3-431c-A41F-FBAB79E1CA2D}
PYXCOM_DEFINE_CLSID(FeaturesSummary,
					0xe6c3802d, 0xe7b3, 0x431c, 0xa4, 0x1f, 0xfb, 0xab, 0x79, 0xe1, 0xca, 0x2d);

PYXCOM_CLASS_INTERFACES(FeaturesSummary, IProcess::iid, IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, IGeoPacketSource::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeaturesSummary, "Features Summary", "A feature collection that filters input feature collections based on resolution.", "Development",
					IFeatureGroup::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, IGeoPacketSource::iid, PYXCOM_IUnknown::iid)
					IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Feature Collection", "A feature collection.")
					IPROCESS_SPEC_END

					namespace
{
	//! Tester class
	Tester<FeaturesSummary> gTester();
}

////////////////////////////////////
// GroupCreator
////////////////////////////////////

class GroupCreator : public PYXObject
{
private:
	PYXIcosIndex m_root;
	std::string m_key;
	long m_totalFeaturesCount;
	long m_localFeaturesCount;
	std::vector<PYXPointer<GroupCreator>> m_subGroups;
	GenericFeaturesGroup::ChildrenList m_newFeatures;
	bool m_modified;
	PYXPointer<PYXLocalStorage> m_storage;

	static const int OptimalGroupSize = 100;

public:
	GroupCreator(const PYXIcosIndex & root,const PYXPointer<PYXLocalStorage> & storage) : m_root(root), m_storage(storage), m_modified(false), m_totalFeaturesCount(0), m_localFeaturesCount(0)
	{
		m_key = "g:" + m_root.toString() + ":cl";
	}

	static PYXPointer<GroupCreator> create(const PYXIcosIndex & root,const PYXPointer<PYXLocalStorage> & storage)
	{
		return PYXNEW(GroupCreator,root,storage);
	}

public:
	static PYXPointer<GenericFeaturesGroup::NodeData> createNodeData(const boost::intrusive_ptr<IFeature> & feature)
	{
		PYXPointer<GenericFeaturesGroup::NodeData> nodeData = GenericFeaturesGroup::NodeData::create();

		nodeData->nodeType = GenericFeaturesGroup::knFeature;
		std::string id = feature->getID();
		nodeData->id = id;
		nodeData->serializedData = *FeaturesSummary::GenericFeature::serializeFeature(feature);
		nodeData->circle = feature->getGeometry()->getBoundingCircle();

		return nodeData;
	}

	static PYXIcosIndex getFeatureIndex(const PYXPointer<GenericFeaturesGroup::NodeData> & feature)
	{
		PYXIcosIndex index;

		SnyderProjection::getInstance()->nativeToPYXIS(
			SphereMath::xyzll(feature->circle.getCenter()),&index,std::max(2,PYXBoundingCircle::estimateResolutionFromRadius(feature->circle.getRadius())));

		return index;
	}

	void addFeature(const PYXPointer<GenericFeaturesGroup::NodeData> & feature,const PYXIcosIndex & group)
	{
		m_modified = true;
		m_totalFeaturesCount++;
		//we are the request group
		if (m_root == group)
		{
			m_localFeaturesCount++;
			m_newFeatures.childrenNodes.push_back(feature);
			return;
		}

		//add to this group if we are small enough group, and we have no sub groups,.
		if (m_totalFeaturesCount < OptimalGroupSize &&
			m_subGroups.empty() &&
			m_root.getResolution() > group.getResolution() - 11)
		{
			m_localFeaturesCount++;
			m_newFeatures.childrenNodes.push_back(feature);
			return;
		}
		if ( m_subGroups.empty())
		{
			splitGroup();
		}
		int subGroupIndex = group.getSubIndex().getDigit(m_root.getResolution()-1);
		if (!(m_subGroups[subGroupIndex]))
		{
			PYXIcosIndex subIndex = m_root;
			subIndex.getSubIndex().appendDigit(subGroupIndex);
			m_subGroups[subGroupIndex] = PYXNEW(GroupCreator,subIndex,m_storage);
		}
		m_subGroups[subGroupIndex]->addFeature(feature,group);
	}

public:
	void splitGroup()
	{
		if (!m_subGroups.empty())
		{
			// already divided
			return;
		}
		m_modified = true;
		m_subGroups.resize(7);

		bool bufferFound = false;
		GenericFeaturesGroup::ChildrenList previousFeatures;

		if (m_localFeaturesCount > (long)m_newFeatures.childrenNodes.size())
		{
			auto buffer = m_storage->get(m_key);
			if (buffer.get() != nullptr)
			{
				bufferFound = true;
				*buffer >> previousFeatures;
			}
		}

		GenericFeaturesGroup::ChildrenList oldNewFeatures;
		std::swap(oldNewFeatures.childrenNodes,m_newFeatures.childrenNodes);

		// redistribute children
		m_localFeaturesCount -= previousFeatures.childrenNodes.size();
		m_localFeaturesCount -= oldNewFeatures.childrenNodes.size();
		m_totalFeaturesCount -= previousFeatures.childrenNodes.size();
		m_totalFeaturesCount -= oldNewFeatures.childrenNodes.size();

		for(auto & child : previousFeatures.childrenNodes)
		{
			addFeature(child,getFeatureIndex(child));
		}

		for(auto & child : oldNewFeatures.childrenNodes)
		{
			addFeature(child,getFeatureIndex(child));
		}

		//clear children list.
		//note, addFeature could add feature to m_newFeatures for this group as well (aka - big features).
		//we will write down m_newFeatures in the next serialization event.
		previousFeatures.childrenNodes.clear();

		if (bufferFound)
		{
			PYXStringWireBuffer buffer;
			buffer << previousFeatures;
			m_storage->set(m_key,buffer);
		}
	}

public:
	void flush(bool fast=true)
	{
		if (!m_modified)
		{
			return;
		}

		for(auto & subGroup : m_subGroups)
		{
			if (subGroup)
			{
				subGroup->flush(fast);
			}
		}

		//check if we have something to update
		if (m_newFeatures.childrenNodes.empty())
		{
			m_modified = false;
			return;
		}

		GenericFeaturesGroup::ChildrenList previousFeatures;

		if (m_localFeaturesCount>0)
		{
			auto buffer = m_storage->get(m_key);
			if (buffer.get() != nullptr)
			{
				*buffer >> previousFeatures;
			}
		}

		//add new children
		for(auto & child : m_newFeatures.childrenNodes)
		{
			previousFeatures.childrenNodes.push_back(child);
		}

		if (fast && previousFeatures.childrenNodes.empty())
		{
			//on fast mode - we don't store empty groups - waste of effort
		}
		else
		{
			PYXStringWireBuffer buffer;
			buffer << previousFeatures;
			m_storage->set(m_key,buffer);
		}

		m_newFeatures.childrenNodes.clear();
		m_modified = false;
	}

	int subGroupCount()
	{
		int count = 0;
		for(auto & child : m_subGroups)
		{
			if (child) count++;
		}
		return count;
	}

	class GroupFinalData : public PYXObject
	{
	public:
		GroupFinalData()
		{
			groupData = GenericFeaturesGroup::GroupData::create();
			groupGeometry = PYXTileCollection::create();
		}

		static PYXPointer<GroupFinalData> create()
		{
			return PYXNEW(GroupFinalData);
		}

		PYXPointer<GenericFeaturesGroup::GroupData> groupData;
		PYXPointer<PYXTileCollection> groupGeometry;
	};

private:
	void borrowChildFeature(const PYXPointer<GroupCreator> & child)
	{
		try
		{
			if (child->m_localFeaturesCount==0)
			{
				//no children to borrow
				return;
			}
			bool bufferFound = false;

			GenericFeaturesGroup::ChildrenList childPreviousFeatures;
			{
				auto buffer = m_storage->get(child->m_key);
				if (buffer.get() != nullptr)
				{
					*buffer >> childPreviousFeatures;
					bufferFound = true;
				}
			}
			for(auto & feature : childPreviousFeatures.childrenNodes)
			{
				m_newFeatures.childrenNodes.push_back(feature);
				m_localFeaturesCount++;
			}
			for(auto & feature : child->m_newFeatures.childrenNodes)
			{
				m_newFeatures.childrenNodes.push_back(feature);
				m_localFeaturesCount++;
			}

			if (bufferFound)
			{
				m_storage->remove(child->m_key);
			}

			m_modified = true;
		}
		CATCH_AND_RETHROW("failed to borrow child group features (parent=" << this->m_key << " child=" << child->m_key << ")");
	}

public:
	void compressChildren()
	{
		int childCount = 0;
		PYXPointer<GroupCreator> lastChild;

		PYXTaskGroup compressChildrenTasks;
		for(auto & child : m_subGroups)
		{
			if (!child) continue;

			compressChildrenTasks.addTask(boost::bind(&GroupCreator::compressChildren,child.get()));
			//child->compressChildren();
		}
		compressChildrenTasks.joinAll();

		try
		{
			for(auto & child : m_subGroups)
			{
				if (!child) continue;

				if (child->m_totalFeaturesCount < OptimalGroupSize/10)
				{
					borrowChildFeature(child);
					child.reset();
				}
				else
				{
					childCount++;
					lastChild = child;
				}
			}

			//we have only 1 child borrow its children
			if (childCount == 1 && lastChild->m_localFeaturesCount + m_localFeaturesCount < OptimalGroupSize)
			{
				//take its features
				borrowChildFeature(lastChild);

				//take its children
				m_subGroups = lastChild->m_subGroups;
				lastChild.reset();
			}

			if (m_totalFeaturesCount > OptimalGroupSize*10)
			{
				flush(false);
			}
		}
		CATCH_AND_RETHROW("failed to compress features group (key=" << this->m_key << ")");
	}

private:
	static void finalizeGroupTask(PYXPointer<GroupFinalData> & finalData,PYXPointer<GroupCreator> group)
	{
		finalData = group->finalize();
	}

public:
	PYXPointer<GroupFinalData> finalize()
	{
		flush(false);

		PYXPointer<GroupFinalData> group = GroupFinalData::create();

		group->groupData->id = m_root.toString();
		group->groupData->featuresCount = m_totalFeaturesCount;

		group->groupGeometry->setCellResolution(m_root.getResolution()+5);

		if (!m_subGroups.empty())
		{
			PYXPointer<GroupFinalData> subGroupsFinalData[7];

			PYXTaskGroup finalizeSubGroupTasks;

			for(unsigned int i=0;i<m_subGroups.size();++i)
			{
				auto & child = m_subGroups[i];

				if (!child) continue;

				assert(child->m_key[0] == 'g');

				finalizeSubGroupTasks.addTask(boost::bind(&GroupCreator::finalizeGroupTask,boost::ref(subGroupsFinalData[i]),child));
			}

			finalizeSubGroupTasks.joinAll();

			for(unsigned int i=0;i<7;++i)
			{
				if (!subGroupsFinalData[i]) continue;

				auto childFinalData = subGroupsFinalData[i];

				m_newFeatures.childrenNodes.push_back(childFinalData->groupData);

				group->groupData->circle += childFinalData->groupData->circle;

				PYXTileCollection geomCopy;

				childFinalData->groupGeometry->copyTo(&geomCopy,group->groupGeometry->getCellResolution() );
				group->groupGeometry->addGeometry(geomCopy);
			}

			//free memory of sub groups.
			m_subGroups.clear();
		}

		PYXStringWireBuffer tempBuffer;
		tempBuffer << group->groupData->id;
		PYXPointer<PYXConstBufferSlice> bufferId = tempBuffer.getBuffer();
		std::map<std::string,PYXConstBufferSlice> fids;

		//add all the previous features in the group
		GenericFeaturesGroup::ChildrenList previousFeatures;
		{
			auto buffer = m_storage->get(m_key);
			if (buffer.get() != nullptr)
			{
				*buffer >> previousFeatures;
			}
		}

		for(auto & child : previousFeatures.childrenNodes)
		{
			m_newFeatures.childrenNodes.push_back(child);

			group->groupData->circle += child->circle;

			fids["fid:" + child->id] = *bufferId;

			PYXPointer<PYXGeometry> featureGeometry = FeaturesSummary::GenericFeature::deserializeGeometryOnly(child->serializedData);
			PYXTileCollection geomCopy;
			featureGeometry->copyTo(&geomCopy,group->groupGeometry->getCellResolution() );
			group->groupGeometry->addGeometry(geomCopy);
		}

		//write down the final child list
		{
			PYXStringWireBuffer buffer;
			buffer << m_newFeatures;
			m_storage->set(m_key,buffer);
		}

		//write down feature id lookup
		m_storage->setMany(fids);

		//write down the final geometry list
		{
			PYXStringWireBuffer buffer;
			buffer << *(group->groupGeometry);
			group->groupData->serializedGeometry = *buffer.getBuffer();
		}

		return group;
	}
};

////////////////////////////////////
// FeaturesSummaryCreator
////////////////////////////////////

class FeaturesSummaryCreator
{
private:
	PYXPointer<PYXLocalStorage> m_storage;
	boost::intrusive_ptr<IFeatureCollection> m_fc;
	boost::detail::atomic_count m_totalFeaturesImported;
	int m_memoryUsage;
	std::map<int,PYXPointer<GroupCreator>> m_rootGroups;

public:
	FeaturesSummaryCreator(const boost::intrusive_ptr<IFeatureCollection> & fc, const PYXPointer<PYXLocalStorage> & storage) : m_fc(fc), m_storage(storage), m_totalFeaturesImported(0), m_memoryUsage(0)
	{
	}

private:
	void handleFeature(PYXPointer<GenericFeaturesGroup::NodeData> & nodeData,boost::intrusive_ptr<IFeature> feature)
	{
		if (!feature)
		{
			PYXTHROW(PYXException,"Failed to CreateNodeData because feature was null");
		}

		try
		{
			nodeData = GroupCreator::createNodeData(feature);
			++m_totalFeaturesImported;
		}
		CATCH_AND_RETHROW("Failed to CreateNodeData for feature " << feature->getID());
	}

	void storeFeature(const PYXPointer<GenericFeaturesGroup::NodeData> & nodeData)
	{
		try
		{
			PYXIcosIndex index = GroupCreator::getFeatureIndex(nodeData);

			m_memoryUsage += nodeData->serializedData.size();

			int rootIndex = index.getPrimaryResolution();

			auto mapIt = m_rootGroups.find(rootIndex);
			if (mapIt == m_rootGroups.end())
			{
				PYXIcosIndex groupIndex = index;
				groupIndex.setResolution(1);
				m_rootGroups[rootIndex] = GroupCreator::create(groupIndex ,m_storage);
				mapIt = m_rootGroups.find(rootIndex);
			}
			mapIt->second->addFeature(nodeData,index);
		}
		CATCH_AND_RETHROW("Failed to add feature into group tree " << nodeData->id);
	}

public:

	void generate(int memoryUsage)
	{
		PYXHighQualityTimer timer;
		timer.start();

		m_storage->removeAll();
		auto iterator = m_fc->getIterator();

		const int ChunkSize = 1000;
		PYXPointer<GenericFeaturesGroup::NodeData> featuresCache[ChunkSize];

		while(!iterator->end())
		{
			PYXTaskGroup importTasks;

			int loaded=0;
			for(;loaded<ChunkSize && !iterator->end();++loaded)
			{
				importTasks.addTask(boost::bind(&FeaturesSummaryCreator::handleFeature,this,boost::ref(featuresCache[loaded]),iterator->getFeature()));
				iterator->next();
			}

			importTasks.joinAll();

			for(int i=0;i<loaded;++i)
			{
				storeFeature(featuresCache[i]);
			}

			// if we consumed too much memory - flush.
			try
			{
				if (m_memoryUsage > memoryUsage)
				{
					for(auto item : m_rootGroups)
					{
						item.second->flush();
					}
					TRACE_INFO("import " << m_totalFeaturesImported << " features took " << timer.tick() << "[sec]");
					m_memoryUsage=0;
				}
			}
			CATCH_AND_RETHROW("Serialization to disk failed");
		}

		TRACE_INFO("import " << m_totalFeaturesImported << " features took " << timer.tick() << "[sec], compressing groups...");

		GenericFeaturesGroup::GroupData rootData;
		GenericFeaturesGroup::ChildrenList rootChildren;

		for(auto item : m_rootGroups)
		{
			item.second->compressChildren();

			auto groupData = item.second->finalize();

			rootChildren.childrenNodes.push_back(groupData->groupData);

			rootData.circle += groupData->groupData->circle;

			Range<int> groupCount = groupData->groupData->featuresCount;
			rootData.featuresCount.min += groupCount.min;
			rootData.featuresCount.max += groupCount.max;
		}

		//write root node info
		PYXStringWireBuffer groupDataBuffer;
		groupDataBuffer << rootData;
		m_storage->set("g:",groupDataBuffer);

		//write root children (note. the ChildrenFactory reads g:id:cl. and the root id is "".
		groupDataBuffer.clear();
		groupDataBuffer << rootChildren;
		m_storage->set("g::cl",groupDataBuffer);

		timer.stop();

		TRACE_INFO("import " << m_totalFeaturesImported << " features took " << timer.getTime() << "[sec]");
	}
};

/*
transform original storage keys to file based keys for better performance

here are examples of keys transformation
hist:0:1-020301002 -> hist:0:1-0203:01002
g:1-0              -> group:root:1-0
g:1-020301002      -> group:1-0203:01002
g:1-02030102       -> group:1-0203:0102
fid:100            -> fid:100
fid:1150           -> fid:1:150
fid:123456         -> fid:1:123:456

this is used to provide file chunks that have around maximum of 1000~4000 keys per chunk.
*/
class PYXLocalStorageTransform : public PYXLocalStorage
{
private:
	PYXPointer<PYXLocalStorage> m_storage;

public:
	static PYXPointer<PYXLocalStorageTransform> create(const PYXPointer<PYXLocalStorage> & storage)
	{
		return PYXNEW(PYXLocalStorageTransform,storage);
	}

	PYXLocalStorageTransform(const PYXPointer<PYXLocalStorage> & storage) : m_storage(storage)
	{
	}

	//we assume feature index is pyxis index -> so 5 keys produce around 1
	std::string getIndexPartition(const std::string & index)
	{
		//divide string into parts
		if (index.size() <= 5) 
		{
			return "root:"+index;
		}
		else 
		{
			int size = index.size() - 1;
			int pos = size - size % 5;
			return index.substr(0,pos) + ":" + index.substr(pos);
		}
	}

	//we assume feature id is a natural number. so 3 digits will provide 1000 keys per chunk.
	std::string getFeatureIdPartition(const std::string & fid)
	{
		if (fid.size()<=3) 
		{
			return fid;
		}
		else
		{
			int pos = fid.size() - 3;
			return fid.substr(0,pos) + ":" + fid.substr(pos);
		}
	}

	// split a given tree into first partition and leave the rest
	// Note: this function modify the input reference std::string key.
	//
	// usage:
	//   key = "hello:world:lovely:day";
	//   first = shiftFirstPartition(key);
	//   second = shiftFirstPartition(key);
	// result:
	//   //first = "hello" second="world" key = "lovely:day"
	std::string shiftFirstPartition(std::string & key)
	{
		auto index = key.find_first_of("\\/:");

		if (index == std::string::npos)
		{
			return "";
		}
		auto partition = key.substr(0,index);
		key = key.substr(index+1);
		return partition;
	}

	/*
	transform origial storage keys to file based keys for better performance

	here are examples of keys transformation
	hist:0:1-020301002 -> hist:0:1-0203:01002
	g:1-0              -> group:root:1-0
	g:1-020301002      -> group:1-0203:01002
	g:1-02030102       -> group:1-0203:0102
	fid:100            -> fid:100
	fid:1150           -> fid:1:150
	fid:123456         -> fid:1:123:456

	this is used to provide file chunks that have around maximum of 1000~4000 keys per chunk.
	*/
	std::string transformKey(const std::string & key)
	{
		auto rest = key;
		auto first = shiftFirstPartition(rest);

		if (first == "hist")
		{
			auto second = shiftFirstPartition(rest);
			return "hist:"+ second + ":" + getIndexPartition(rest);
		}

		if (first == "g")
		{
			auto index = shiftFirstPartition(rest);

			if (!rest.empty()) 
			{
				return "group:" + getIndexPartition(index) + "-" + rest;
			}
			else
			{
				return "group:" + getIndexPartition(index);
			}
		}
		else if (first == "fid")
		{
			return "fid:" + getFeatureIdPartition(rest);
		}
		
		return key;
	}


	virtual std::auto_ptr<PYXConstWireBuffer> get(const std::string & key)
	{
		return m_storage->get(transformKey(key));
	}

	virtual void set(const std::string &key, PYXWireBuffer & data)
	{
		m_storage->set(transformKey(key),data);
	}

	virtual void setMany(const std::map<std::string,PYXConstBufferSlice> & data)
	{
		for(auto & keyValue : data) 
		{
			set(keyValue.first,PYXConstWireBuffer(keyValue.second));
		}
	}

	virtual void remove(const std::string & key)
	{
		m_storage->remove(transformKey(key));
	}

	virtual void removeAll()
	{
		m_storage->removeAll();
	}

	virtual void applyChanges(const std::vector<PYXPointer<PYXLocalStorageChange>> & changes)
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}
};

////////////////////////////////////////////////////////////////////////////////
// PYXLocalStorageWithPyxnetChannel
////////////////////////////////////////////////////////////////////////////////

class PYXLocalStorageWithPyxnetChannel : public PYXLocalStorage
{
public:
	static PYXPointer<PYXLocalStorageWithPyxnetChannel> create(const PYXPointer<PYXLocalStorage> & storage, const PYXPointer<PYXNETChannel> & channel)
	{
		return PYXNEW(PYXLocalStorageWithPyxnetChannel,storage,channel);
	}

	PYXLocalStorageWithPyxnetChannel(const PYXPointer<PYXLocalStorage> & storage, const PYXPointer<PYXNETChannel> & channel)
		: m_storage(storage), m_channel(channel)
	{
	}

public:
	virtual std::auto_ptr<PYXConstWireBuffer> get(const std::string & key)
	{
		std::auto_ptr<PYXConstWireBuffer> buffer = m_storage->get(key);
		if (buffer.get()!=0)
		{
			return buffer;
		}

		if (m_channel)
		{
			//TRACE_INFO("downloading key from pyxnet: " << key);
			buffer = m_channel->getKey(key);
		}

		if (buffer.get()!=0)
		{
			m_storage->set(key,*buffer);
		}
		return buffer;
	}

	virtual void set(const std::string &key, PYXWireBuffer & data)
	{
		m_storage->set(key,data);
	}

	virtual void setMany(const std::map<std::string,PYXConstBufferSlice> & data)
	{
		m_storage->setMany(data);
	}

	virtual void remove(const std::string & key)
	{
		m_storage->remove(key);
	}

	virtual void removeAll()
	{
		m_storage->removeAll();
	}

	virtual void applyChanges(const std::vector<PYXPointer<PYXLocalStorageChange>> & changes)
	{
		m_storage->applyChanges(changes);
	}

private:
	PYXPointer<PYXLocalStorage> m_storage;
	PYXPointer<PYXNETChannel> m_channel;
};

////////////////////////////////////////////////////////////////////////////////
// FeaturesSummary
////////////////////////////////////////////////////////////////////////////////

FeaturesSummary::FeaturesSummary()
{
}

FeaturesSummary::~FeaturesSummary()
{
}

void FeaturesSummary::test()
{
	//Test issue with PYXTileCollection
	PYXTileCollection collection;

	collection.setCellResolution(20);

	collection.addTile(PYXIcosIndex("A-0101010101000"), 20);
	collection.addTile(PYXIcosIndex("A-0101010201"), 20);
	collection.addTile(PYXIcosIndex("A-010101010102"), 20);
	collection.addTile(PYXIcosIndex("A-0101010"), 20);

	std::string seralized = PYXGeometrySerializer::serialize(collection);

	PYXPointer<PYXGeometry> deGeom = PYXGeometrySerializer::deserialize(seralized);

	PYXPointer<PYXTileCollection> deCollection = boost::dynamic_pointer_cast<PYXTileCollection>(deGeom);

	TEST_ASSERT(deCollection->isEqual(collection));

	// TODO test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE FeaturesSummary::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	return mapAttr;
}

std::string STDMETHODCALLTYPE FeaturesSummary::getAttributeSchema() const
{
	return
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		"<xs:element name=\"FeaturesSummary\">"
		"<xs:complexType>"
		"<xs:sequence>"

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE FeaturesSummary::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;
}

class PYXNETChannelKeyProviderFromLocalStorage : public PYXNETChannelKeyProvider
{
public:
	static PYXPointer<PYXNETChannelKeyProviderFromLocalStorage> create(const PYXPointer<PYXLocalStorage> & storage)
	{
		return PYXNEW(PYXNETChannelKeyProviderFromLocalStorage,storage);
	}

	PYXNETChannelKeyProviderFromLocalStorage(const PYXPointer<PYXLocalStorage> & storage) : m_storage(storage)
	{
	}

	virtual std::string getKey(const std::string & key)
	{
		std::auto_ptr<PYXConstWireBuffer> buffer = m_storage->get(key);

		if (buffer.get() == 0)
		{
			return "[NULL]"; //this is illegal base64
		}

		PYXPointer<PYXConstBufferSlice> slice = buffer->getBuffer();

		return XMLUtils::toBase64(std::string((char*)slice->begin(),slice->size()));
	}

private:
	PYXPointer<PYXLocalStorage> m_storage;
};

/*!
Overrides Process::initProc() to ignore errors on inputs until this process is initialized.
*/
IProcess::eInitStatus FeaturesSummary::initProc(bool bRecursive)
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	if (m_initState == knInitializing)
	{
		// we are in the middle of initializing.
		return knInitializing;
	}

	// unless we are already initialized, then set our state to initializing.
	if (m_initState != knInitialized)
	{
		m_initState = knInitializing;
	}

	// clear out the current error state
	m_spInitError = boost::intrusive_ptr<IProcessInitError>();

	// process all of the children first
	if (bRecursive)
	{
		std::vector<PYXPointer<Parameter>> parameters = static_cast<std::vector<PYXPointer<Parameter>>> (getParameters());
		// cycle through each of the children
		std::vector<PYXPointer<Parameter>>::iterator it =
			parameters.begin();
		for (; it != parameters.end(); ++it)
		{
			for (int n = 0; n < (*it)->getValueCount(); ++n)
			{
				assert((*it)->getValue(n));
				(*it)->getValue(n)->initProc(true);
			}
		}
	}

	// if the process is already initialized, return
	if (m_initState != knInitialized)
	{
		if (!verifySpec(this))
		{
			m_initState = knDoesNotMeetSpec;
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new ProcSpecFailure());
			return m_initState;
		}

		try
		{
			// perform the actual initialization and examine the state
			m_initState = initImpl();
			if (m_initState == knFailedToInit)
			{
				if (!m_spInitError)
				{
					TRACE_ERROR("A process failed to initialize but no error state was set, creating generic error for " << getProcName());
					setInitProcError<GenericProcInitError>("Generic process initialization error.");
				}
			}
			else if (m_initState != knInitialized)
			{
				TRACE_ERROR("Process initialization should only return pass or fail, Policy Breach!");
				assert(false && "Process initialization should only return pass or fail, Policy Breach!");
			}
		}
		catch (PYXException& e)
		{
			TRACE_ERROR("A PYXIS exception was thrown during process initialization. Processes should never throw during init.");
			setInitProcError<GenericProcInitError>("A PYXIS error occurred during process initialization: \n" + e.getFullErrorString());
			m_initState = knFailedToInit;
		}
		catch (...)
		{
			TRACE_ERROR("A generic exception occurred during process initialization for: " << getProcName());
			setInitProcError<GenericProcInitError>("A generic error occurred during process initialization");
			m_initState = knFailedToInit;
		}
		assert(	(m_initState == knInitialized || m_spInitError) &&
			"Policy Breach! invalid error and state combination.");
	}
	return m_initState;
}

bool FeaturesSummary::inputIsOK() const
{
	boost::intrusive_ptr<IProcess> spProc = getParameter(0)->getValue(0);

	if(spProc->getInitState() == IProcess::knNeedsInit)
	{
		spProc->initProc(true);
	}
	return (spProc->getInitState() == IProcess::knInitialized);
}

bool FeaturesSummary::inputHasMissingSRS()
{
	return PipeUtils::findFirstError(this,"{68F0FC89-2D83-439C-BD4E-72A8A9CCDCED}");
}

bool FeaturesSummary::inputHasMissingUserCredentials()
{
	return PipeUtils::findFirstError(this,"{100B9867-D8E2-4A72-81DE-DE8FEB5187FD}");
}

bool FeaturesSummary::inputHasMissingGeometry()
{
	return PipeUtils::findFirstError(this,"{62878998-D2B8-4F98-BA48-7ECAD2B523F0}");
}

bool FeaturesSummary::inputHasMissingWorldFile()
{
	return PipeUtils::findFirstError(this,"{ACA50CE2-E822-49D2-AFE1-1AE5BA7966E9}");
}

IProcess::eInitStatus FeaturesSummary::initImpl()
{
	m_strID = "FS: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	m_bWritable = false;

	int memoryLimit = 50;
	const int MB = 1024 * 1024;
	
	if (!AppServices::getConfiguration(AppServicesConfiguration::importMemoryLimit).empty()) 
	{
		memoryLimit = std::max(memoryLimit,StringUtils::fromString<int>(AppServices::getConfiguration(AppServicesConfiguration::importMemoryLimit)));
	}

	if (inputIsOK())
	{
		//don't init the pipeline line before we have a stable identity
		PipeUtils::waitUntilPipelineIdentityStable(this);

		//validate input can expose a feature collection class
		m_spInputFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();

		if (!m_spInputFC)
		{
			setInitProcError<InputInitError>("input failed to return IFeatureCollection");
			return knFailedToInit;
		}

		//borrow input metadata
		m_spGeom = m_spInputFC->getGeometry();
		m_strStyle = m_spInputFC->getStyle();
		m_spDefn = m_spInputFC->getDefinition();
		if (!m_spDefn)
		{
			m_spDefn = PYXTableDefinition::create();
		}
		m_vecValues = m_spInputFC->getFieldValues();
		m_strStyle = m_spInputFC->getStyle();
		m_featuresDefinition = m_spInputFC->getFeatureDefinition();

		initializeLocalStorage();

		//create the channel, but we not using it on the context.
		if (PYXNETChannelProvider::getInstance())
		{
			m_channel = PYXNETChannelProvider::getInstance()->getOrCreateChannel(ProcRef(getProcID(),getProcVersion()),FEATURE_SUMMARY_CHANNEL_ID);
		}

		m_context = Context::create(m_spInputFC,m_featuresDefinition,m_strStyle,m_localStorage,memoryLimit * MB);
	}
	else
	{
		if (!PipeUtils::isPipelineIdentityStable(this))
		{
			setInitProcError<GenericProcInitError>("Can't initialize FeatureSummary process with remote sources only with not stable identity.");
			return knFailedToInit;
		}

		// Missing SRS, no need to look for data over PyxNet.  After the SRS
		// is added, if there is still local data missing, this code will be
		// ignored and data will be looked for over PyxNet.
		// TODO: Replace hard-coded GUID with a constant, and do the same for other hard-coded GUIDs.
		if (inputHasMissingSRS())
		{
			setInitProcError<GenericProcInitError>("Input has missing SRS, initialization aborted.");
			return knFailedToInit;
		}

		// Missing UserCredentials, no need to look for data over PyxNet.
		// Try to open the dataset only if we have valid name and password
		if (inputHasMissingUserCredentials())
		{
			setInitProcError<GenericProcInitError>("Input has missing user credentials, initialization aborted.");
			return knFailedToInit;
		}

		// Missing geometry in file, no need to look for data over PyxNet.
		// Try to open the dataset only if we have a file with a valid geometry.
		if (inputHasMissingGeometry())
		{
			setInitProcError<GenericProcInitError>("Input has missing geometry, initialization aborted.");
			return knFailedToInit;
		}

		// Missing world file, no need to look for data over PyxNet.
		// Try to open the dataset only if we have a valid world file (or geotransform is included in data file)
		if (inputHasMissingWorldFile())
		{
			setInitProcError<GenericProcInitError>("Input has missing world file, initialization aborted.");
			return knFailedToInit;
		}

		if (!PYXNETChannelProvider::getInstance())
		{
			setInitProcError<GenericProcInitError>("Initialization of remote FeatureSummary aborted because PyxNet channel provider was not initialized.");
			return knFailedToInit;
		}

		initializeLocalStorage();

		m_channel = PYXNETChannelProvider::getInstance()->getOrCreateChannel(ProcRef(getProcID(),getProcVersion()),FEATURE_SUMMARY_CHANNEL_ID);
		m_localStorageWithPyxnet = PYXLocalStorageWithPyxnetChannel::create(m_localStorage,m_channel);

		try
		{
			downloadMetadata();
		}
		catch(PYXException& ex)
		{
			setInitProcError<GenericProcInitError>("Failed to download features summary metadata due to an error: " + ex.getFullErrorString());
			return knFailedToInit;
		}
		catch(...)
		{
			setInitProcError<GenericProcInitError>("Failed to download features summary metadata.");
			return knFailedToInit;
		}

		m_context = Context::create(0,m_featuresDefinition,m_strStyle,m_localStorageWithPyxnet,memoryLimit * MB);
	}

	auto defaultBuffer = m_localStorageBuffered->getBufferSize();
	m_localStorageBuffered->setBufferSize(memoryLimit * MB);

	m_rootGroup = GenericFeaturesGroup::create(m_context);

	m_localStorageBuffered->setBufferSize(defaultBuffer);

	storeMetadata();

	PYXThreadPool::addTask(boost::bind(&FeaturesSummary::pointerSafePublishPyxnetChannel,boost::intrusive_ptr<FeaturesSummary>(this)));
	//measureLocalStorageUsage();

	return knInitialized;
}

void FeaturesSummary::initializeLocalStorage()
{
	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");
	const ProcessIdentityCache cache(cacheDir);
	const boost::filesystem::path pathCache = cache.getPath(getIdentity(), true);
	if (pathCache.empty())
	{
		PYXTHROW(PYXException, "Could not create cache directory.");
	}

	/*
	SSLUtils::Checksum checksum("SHA256");
	checksum.generate(getIdentity());
	auto id = checksum.toHexString().substr(0, 32);

	m_localStorage = m_localStorageBuffered = PYXBufferedLocalStorage::create(PYXLocalStorageFactory::createREST(id));
*	*/
	
	m_localStorage = m_localStorageBuffered = PYXBufferedLocalStorage::create(PYXProcessLocalStorage::create(getIdentity()));

	//add keys transformation if we are using files storage
	if (AppServices::getConfiguration(AppServicesConfiguration::localStorageFormat) == AppServicesConfiguration::localStorageFormat_files)
	{
		m_localStorage = PYXLocalStorageTransform::create(m_localStorageBuffered);
	}

}

void FeaturesSummary::storeMetadata()
{
	PYXStringWireBuffer buffer;

	buffer << *m_spGeom;

	m_localStorage->set("fs:geom",buffer);

	std::string def;
	std::string defValues;
	std::string featureDef;

	{
		std::stringstream stream;
		stream << *m_spDefn;
		def = stream.str();
	}

	{
		std::stringstream stream;
		stream << m_vecValues;
		defValues = stream.str();
	}

	if (m_featuresDefinition)
	{
		std::stringstream stream;
		stream << *m_featuresDefinition;
		featureDef = stream.str();
	}

	buffer.clear();
	buffer << def << defValues << featureDef << m_strStyle;
	m_localStorage->set("fs:def",buffer);

	m_localStorageBuffered->commit();
}

void FeaturesSummary::downloadMetadata()
{
	std::auto_ptr<PYXConstWireBuffer> buffer = m_localStorageWithPyxnet->get("fs:geom");

	if (buffer.get() == 0)
	{
		PYXTHROW(PYXException,"Faild to download pipeline geometry");
	}

	(*buffer) >> m_spGeom;

	buffer = m_localStorageWithPyxnet->get("fs:def");

	if (buffer.get() == 0)
	{
		PYXTHROW(PYXException,"Faild to download pipeline metadata");
	}

	std::string def;
	std::string defValues;
	std::string featureDef;
	(*buffer) >> def >> defValues >> featureDef >> m_strStyle;

	{
		std::stringstream stream(def);
		m_spDefn = PYXTableDefinition::create();
		stream >> *m_spDefn;
	}

	{
		std::stringstream stream(defValues);
		std::vector<PYXValue> values;
		stream >> values;
		std::swap(m_vecValues,values);
	}

	m_featuresDefinition.reset();
	if (!featureDef.empty())
	{
		std::stringstream stream(featureDef);
		m_featuresDefinition = PYXTableDefinition::create();
		stream >> *m_featuresDefinition;
	}

	m_localStorageBuffered->commit();
}

void FeaturesSummary::pointerSafePublishPyxnetChannel(boost::intrusive_ptr<FeaturesSummary> self)
{
	self->publishPyxnetChannel();
}

void FeaturesSummary::publishPyxnetChannel()
{
	while(this->getInitState() == knNeedsInit && PipeManager::exists())
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}

	if (PipeManager::exists())
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}

	if (this->getInitState() == knInitialized && PipeManager::exists())
	{
		if (m_channel) {
			TRACE_INFO("publishing features summary for " << getProcName() );
		
			m_channel->publish();
			m_channel->attachLocalProvider(PYXNETChannelKeyProviderFromLocalStorage::create(m_localStorage));
		}
	}
}

/*
void FeaturesSummary::measureLocalStorageUsage()
{
//TODO: this not working anymore... we have a new schema...
std::list<boost::intrusive_ptr<IFeatureGroup>> groups;
groups.push_back(m_rootGroup);

int groupMetaDataSize = 0;
int featuresDataSize  = 0;
int groupGeometrySize = 0;

int featuresMetaDataSize = 0;
int featuresGeometrySize = 0;

while(!groups.empty())
{
boost::intrusive_ptr<IFeatureGroup> group = groups.front();
groups.pop_front();
PYXPointer<FeatureIterator> it = group->getGroupIterator();

while(!it->end())
{
boost::intrusive_ptr<IFeature> feature = it->getFeature();
boost::intrusive_ptr<IFeatureGroup> featureIsGroup = feature->QueryInterface<IFeatureGroup>();

if (featureIsGroup)
{
groups.push_back(featureIsGroup);
}
it->next();
}

std::string groupId = group->getID();

std::auto_ptr<PYXConstWireBuffer> buffer = m_context->getStorage()->get(groupId);
groupMetaDataSize += buffer.get()!=0?buffer->size():0;

buffer = m_context->getStorage()->get(groupId+":geom");
groupGeometrySize += buffer.get()!=0?buffer->size():0;

buffer = m_context->getStorage()->get(groupId+":features");
featuresDataSize += buffer.get()!=0?buffer->size():0;

if (buffer.get() != 0)
{
std::map<std::string,PYXConstBufferSlice> featuresData;
(*buffer) >> featuresData;

for(std::map<std::string,PYXConstBufferSlice>::iterator it = featuresData.begin();it != featuresData.end(); ++it)
{
PYXConstWireBuffer constBuffer(it->second);
PYXConstBufferSlice chunk;
constBuffer >> chunk;
featuresMetaDataSize += chunk.size();

constBuffer >> chunk;
featuresGeometrySize += chunk.size();
}
}
}

int total = groupMetaDataSize+groupGeometrySize+featuresDataSize;

TRACE_INFO("Features summary usage: total=" << total << " ( metaData=" << groupMetaDataSize << " groupGeom=" << groupGeometrySize << " features=" << featuresDataSize << "[metadata=" << featuresMetaDataSize << " geom=" << featuresGeometrySize << "])");
}
*/

///////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
///////////////////////////////////////////////////////////////////////////////

PYXPointer<FeatureIterator> FeaturesSummary::getIterator() const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_rootGroup->getIterator();
}

PYXPointer<FeatureIterator> FeaturesSummary::getIterator(const PYXGeometry& geometry) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_rootGroup->getIterator(geometry);
}

std::vector<FeatureStyle> FeaturesSummary::getFeatureStyles() const
{
	PYXTHROW_NOT_IMPLEMENTED();
}

boost::intrusive_ptr<IFeature> FeaturesSummary::getFeature(const std::string& strFeatureID) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);

	std::auto_ptr<PYXConstWireBuffer> buffer = m_context->getStorage()->get("fid:"+strFeatureID);

	//failed to locate feature
	if (buffer.get()==0)
	{
		return 0;
	}
	std::string groupId;
	*buffer >> groupId;

	return m_rootGroup->findGroup(groupId)->getFeature(strFeatureID);
}

PYXPointer<const PYXTableDefinition> FeaturesSummary::getFeatureDefinition() const
{
	return m_featuresDefinition;
}

PYXPointer<PYXTableDefinition> FeaturesSummary::getFeatureDefinition()
{
	return m_featuresDefinition;
}

bool FeaturesSummary::canRasterize() const
{
	//TODO: should we do something better here? why not always to rasterize the features? or this should be inside the style...
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// IFeatureGroup
///////////////////////////////////////////////////////////////////////////////

Range<int> FeaturesSummary::getFeaturesCount() const
{
	return m_rootGroup->getFeaturesCount();
}

bool FeaturesSummary::moreDetailsAvailable() const
{
	return m_rootGroup->moreDetailsAvailable();
}

PYXPointer<PYXHistogram> FeaturesSummary::getFieldHistogram(int fieldIndex) const
{
	notifyProcessing(ProcessProcessingEvent::Processing);
	return m_rootGroup->getFieldHistogram(fieldIndex);
}

PYXPointer<PYXHistogram> FeaturesSummary::getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const
{
	notifyProcessing(ProcessProcessingEvent::Processing);
	return m_rootGroup->getFieldHistogram(geometry,fieldIndex);
}

boost::intrusive_ptr<IFeatureGroup> FeaturesSummary::getFeatureGroup(const std::string & groupId) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	if (m_rootGroup->getID() == groupId)
	{
		return m_rootGroup;
	}
	return m_rootGroup->getFeatureGroup(groupId);
}

boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE FeaturesSummary::getFeatureGroupForFeature(const std::string & featureId) const
{
	return getFeatureGroup(m_context->getFeatureGroupIdForFeature(featureId));
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeaturesSummary::getGroupIterator() const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_rootGroup->getGroupIterator();
}

PYXPointer<FeatureIterator> STDMETHODCALLTYPE FeaturesSummary::getGroupIterator(const PYXGeometry& geometry) const
{
	notifyProcessing(ProcessProcessingEvent::Fetching);
	return m_rootGroup->getGroupIterator(geometry);
}

///////////////////////////////////////////
// FeaturesSummary::Context::GroupFactory
///////////////////////////////////////////

class FeaturesSummary::Context::GroupFactory : public PYXDataCollection::DataItemFactory<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>>
{
private:
	const PYXPointer<const FeaturesSummary::Context> & m_context;
	const PYXPointer<GenericFeaturesGroup::NodeData> & m_data;
	const std::string & m_name;
public:
	GroupFactory(const PYXPointer<const FeaturesSummary::Context> & context,
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

class ChildrenFactory : public PYXDataCollection::DataItemFactory<GenericFeaturesGroup::ChildrenList>
{
private:
	const PYXPointer<PYXLocalStorage> & m_storage;
	const std::string & m_id;
	static const std::string m_name;
public:
	ChildrenFactory(const PYXPointer<PYXLocalStorage> & storage,const std::string & id) : m_storage(storage), m_id(id)
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
			itemSize = 20;
			//itemSize = dataBuffer->size();
		}
	}
};

const std::string ChildrenFactory::m_name = "cl";

///////////////////////////////////////////
// GeometryFactory
///////////////////////////////////////////

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
			itemSize = 20;
		}
	}
};

///////////////////////////////////////////
// FeaturesSummary::Context
///////////////////////////////////////////

boost::intrusive_ptr<GenericFeaturesGroup> FeaturesSummary::Context::getRootGroup() const
{
	int version = 0;
	{
		boost::scoped_ptr<PYXWireBuffer> versionBuffer(m_storage->get("tree:version"));
		if (versionBuffer)
		{
			(*versionBuffer) >> version;
		}
	}

	int completedLocally = 0;
	if (m_fc)
	{
		//this will try to load the data locally
		boost::scoped_ptr<PYXWireBuffer> completedLocallyBuffer(m_storage->get("tree:completedLocally"));
		if (completedLocallyBuffer)
		{
			(*completedLocallyBuffer) >> completedLocally;
		}
	}

	boost::scoped_ptr<PYXWireBuffer> dataBuffer(m_storage->get("g:"));

	bool needToGenerateHistograms = false;

	if (version != knCurrentVersion ||  //this is not the right version...
		!dataBuffer ||					//we failed to get root group
		(m_fc && completedLocally == 0)) //we have the file locally, but we didn't process it locally.
		//this can happen if someone start to download a FC over the network and then try to open it locally.
	{
		m_storage->removeAll();

		if (m_fc) {
			//we have valid input - recreate geo-packets
			try
			{
				FeaturesSummaryCreator creator(m_fc,m_storage);
				creator.generate(m_memoryLimit);

				PYXStringWireBuffer versionBuffer;
				versionBuffer << knCurrentVersion;
				m_storage->set("tree:version",versionBuffer);

				if (m_fc)
				{
					PYXStringWireBuffer completedLocallyBuffer;
					completedLocallyBuffer << 1;
					m_storage->set("tree:completedLocally",completedLocallyBuffer);
				}

				needToGenerateHistograms = true;
			}
			catch(PYXException & ex)
			{
				m_storage->removeAll();
				PYXTHROW(PYXException,"Failed to generate feature summary due to the following error:" << ex.getFullErrorString());
			}
			catch(std::exception & stdex)
			{
				m_storage->removeAll();
				PYXTHROW(PYXException,"Failed to generate feature summary due to the following error:" << stdex.what());
			}
			catch(...)
			{
				m_storage->removeAll();
				PYXTHROW(PYXException,"Failed to generate feature summary");
			}
		} else {
			//we don't have valid input - mark our new version and start download again
			PYXStringWireBuffer versionBuffer;
			versionBuffer << knCurrentVersion;
			m_storage->set("tree:version",versionBuffer);
		}
	}

	dataBuffer.reset(m_storage->get("g:").release());
	if (dataBuffer)
	{
		PYXPointer<GenericFeaturesGroup::GroupData> groupData = GenericFeaturesGroup::GroupData::create();
		(*dataBuffer) >> *groupData;

		auto rootGroup = new GenericFeaturesGroup(this,groupData);

		if (needToGenerateHistograms)
		{
			//generate all histograms...
			int count = rootGroup->getFeatureDefinition()->getFieldCount();
			
			for(int i=0;i<count;i++)
			{
				TRACE_INFO("Generating histogram (" << (i+1) << " from " << count << ")");
				rootGroup->getFieldHistogram(i);
			}
		}

		return rootGroup;
	}

	PYXTHROW(PYXException,"we should never get here");
}

boost::intrusive_ptr<IFeature> FeaturesSummary::Context::getFeature(const GenericFeaturesGroup & parent, const std::string & featureId) const
{
	PYXPointer<GenericFeaturesGroup::NodeData> feature = getChildren(parent)->findFeature(featureId);

	if (feature)
	{
		boost::intrusive_ptr<IFeature> result = GenericFeature::deserialize(
			getFeatureDefinition(parent),
			featureId,
			feature->serializedData);

		return result;
	}

	PYXTHROW(PYXException,"Feature ID " << featureId << " wasn't found, we might need to search outside of the group children");
}

std::string FeaturesSummary::Context::getFeatureGroupIdForFeature(const std::string & featureId) const
{
	std::auto_ptr<PYXConstWireBuffer> buffer = getStorage()->get("fid:"+featureId);

	//failed to locate feature
	if (buffer.get()==0)
	{
		return "";
	}

	std::string groupId;
	*buffer >> groupId;

	return groupId;
}

PYXPointer<GenericFeaturesGroup::ChildrenList> FeaturesSummary::Context::getChildren(const GenericFeaturesGroup & parent) const
{
	return getDataCollectionForGroup(parent).getItem(ChildrenFactory(m_storage,parent.getID()));
}

Range<int> FeaturesSummary::Context::getFeaturesCount(const GenericFeaturesGroup & parent) const
{
	return dynamic_cast<const GenericFeaturesGroup::GroupData*>(&getDataForGroup(parent))->featuresCount;
}

boost::intrusive_ptr<IFeature> FeaturesSummary::Context::getFeature(const GenericFeaturesGroup & parent, const PYXPointer<GenericFeaturesGroup::NodeData> & feature) const
{
	boost::intrusive_ptr<IFeature> result = GenericFeature::deserialize(
		getFeatureDefinition(parent),
		feature->id,
		feature->serializedData);

	return result;
}

boost::intrusive_ptr<GenericFeaturesGroup> FeaturesSummary::Context::getGroup(const GenericFeaturesGroup & parent, const PYXPointer<GenericFeaturesGroup::NodeData> & groupData) const
{
	PYXPointer<PYXObjectWrapper<boost::intrusive_ptr<GenericFeaturesGroup>>> groupItem = getDataCollectionForGroup(parent).getItem(GroupFactory(this,groupData,"g:"+groupData->id));
	return groupItem->value;
}

PYXPointer<PYXTableDefinition> FeaturesSummary::Context::getFeatureDefinition(const GenericFeaturesGroup & parent) const
{
	return m_featureDefinition;
}

PYXValue FeaturesSummary::Context::getFieldValue(const GenericFeaturesGroup & parent,int index) const
{
	return PYXValue();
}

PYXPointer<PYXGeometry> FeaturesSummary::Context::getGeometry(const GenericFeaturesGroup & parent) const
{
	return getDataCollectionForGroup(parent).getItem(GeometryFactory(getDataForGroup(parent),"geom"));
}

std::string FeaturesSummary::Context::getStyle(const GenericFeaturesGroup & parent) const
{
	return m_strStyle;
}

///////////////////////////////////////////
// FeaturesSummary::GenericFeature
///////////////////////////////////////////

PYXPointer<PYXConstBufferSlice> FeaturesSummary::GenericFeature::serializeFeature(const boost::intrusive_ptr<IFeature> & feature)
{
	std::string geometry = PYXGeometrySerializer::serialize(feature->getGeometry());

	PYXStringWireBuffer metadataBuffer;

	PYXPointer<PYXTableDefinition> definition = feature->getDefinition();
	for(int i=0; i < definition->getFieldCount(); ++i)
	{
		const PYXFieldDefinition & fieldDefinition = definition->getFieldDefinition(i);
		PYXValue value = feature->getFieldValue(i);

		if (value.isNull())
		{
			metadataBuffer << (unsigned char)0;
			continue;
		}
		else
		{
			metadataBuffer << (unsigned char)1;
		}

		if (value.isArrayNullable())
		{
			//TODO: what we should do if the array is nullable?
			PYXTHROW(PYXException,"We don't support nullable array yet...");
		}

		for(int v=0;v<fieldDefinition.getCount();++v)
		{
			switch (fieldDefinition.getType())
			{
			case PYXValue::knBool:
				metadataBuffer << static_cast<unsigned char>(value.getBool(v));
				break;
			case PYXValue::knChar:
				metadataBuffer << static_cast<unsigned char>(value.getChar(v));
				break;
			case PYXValue::knInt8:
			case PYXValue::knInt16:
			case PYXValue::knInt32:
				metadataBuffer << value.getInt32(v);
				break;

			case PYXValue::knUInt8:
			case PYXValue::knUInt16:
			case PYXValue::knUInt32:
				metadataBuffer << static_cast<int>(value.getInt32(v));
				break;

			case PYXValue::knReservedForInt64:
			case PYXValue::knReservedForUInt64:
				PYXTHROW(PYXException,"unsupported format " << fieldDefinition.getType());

			case PYXValue::knFloat:
			case PYXValue::knDouble:
				metadataBuffer << value.getDouble(v);
				break;

			case PYXValue::knString:
				metadataBuffer <<value.getString(v);
			}
		}
	}

	std::string metadata;
	metadataBuffer.copyToString(metadata);

	PYXStringWireBuffer buffer;
	buffer << metadata << geometry;

	return buffer.getBuffer();
}

PYXPointer<PYXGeometry> FeaturesSummary::GenericFeature::deserializeGeometryOnly(const PYXConstBufferSlice & in)
{
	PYXConstWireBuffer buffer(in);

	PYXConstBufferSlice metadata;
	PYXConstBufferSlice geometry;

	buffer >> metadata >> geometry;

	std::string str((const char *)geometry.begin(),geometry.size());
	return PYXGeometrySerializer::deserialize(str);
}

FeaturesSummary::GenericFeature::GenericFeature(const PYXPointer<PYXTableDefinition> featureDefinition,
												const std::string & featureID,
												const PYXConstBufferSlice & in)
												: m_spDefn(featureDefinition), m_strID(featureID)
{
	PYXConstWireBuffer buffer(in);

	PYXConstBufferSlice metadata;

	buffer >> metadata >> m_serializedGeometry;

	PYXConstWireBuffer metadataBuffer(metadata);

	m_vecValues.resize(featureDefinition->getFieldCount());

	for(int i=0; i < featureDefinition->getFieldCount(); ++i)
	{
		unsigned char hasValue;

		metadataBuffer >> hasValue;

		if (!hasValue)
		{
			continue;
		}

		const PYXFieldDefinition & fieldDefinition = featureDefinition->getFieldDefinition(i);

		if (fieldDefinition.getType() == PYXValue::knString && fieldDefinition.getCount()==1)
		{
			//HACK: if the type is string with count = 1, then fieldDefinition.getTypeCompatibleValue() fails...
			std::string temp;
			metadataBuffer >> temp;

			swap(m_vecValues[i],PYXValue(temp));
			continue;
		}

		PYXValue value = fieldDefinition.getTypeCompatibleValue();

		for(int v=0;v<fieldDefinition.getCount();++v)
		{
			switch (fieldDefinition.getType())
			{
			case PYXValue::knBool:
				{
					unsigned char c;
					metadataBuffer >> c;
					value.setBool(v,c!=0);
				}
				break;
			case PYXValue::knChar:
				{
					unsigned char c;
					metadataBuffer >> c;
					value.setChar(v,static_cast<char>(c));
				}
				break;
			case PYXValue::knInt8:
			case PYXValue::knInt16:
			case PYXValue::knInt32:
				{
					int temp;
					metadataBuffer >> temp;
					value.setInt(v,temp);
				}
				break;

			case PYXValue::knUInt8:
			case PYXValue::knUInt16:
			case PYXValue::knUInt32:
				{
					int temp;
					metadataBuffer >> temp;
					value.setUInt(v,static_cast<unsigned int>(temp));
				}
				break;

			case PYXValue::knReservedForInt64:
			case PYXValue::knReservedForUInt64:
				PYXTHROW(PYXException,"unsupported format " << fieldDefinition.getType());

			case PYXValue::knFloat:
			case PYXValue::knDouble:
				{
					double temp;
					metadataBuffer >> temp;
					value.setDouble(v,temp);
				}
				break;

			case PYXValue::knString:
				{
					std::string temp;
					metadataBuffer >> temp;
					value.setString(v,temp);
				}
				break;
			}
		}

		swap(m_vecValues[i],value);
	}
}

void FeaturesSummary::GenericFeature::createGeometry() const
{
	if (!m_spGeom)
	{
		m_spGeom = PYXGeometrySerializer::deserialize(m_serializedGeometry);
	}
}