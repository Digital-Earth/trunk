/******************************************************************************
data_collection.cpp

begin		: March 12, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "data_collection.h"

#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/geometry_serializer.h"
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
#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "sqlite3.h"

// standard includes
#include <cassert>

PYXDataCollection::CacheManager* PYXDataCollection::CacheManager::m_pInstance = 0;




void PYXDataCollection::CacheManager::add(const PYXPointer<PYXDataCollection::CollectionInfo> & info)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	info->m_cacheIterator = m_collections.insert(m_collections.end(),info);
}

void PYXDataCollection::CacheManager::remove(const PYXPointer<PYXDataCollection::CollectionInfo> & info)
{
	memoryConsumedChanged(-info->freeMemory());
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (m_checkIt == info->m_cacheIterator)
		{
			++m_checkIt;
		}
		m_collections.erase(info->m_cacheIterator);	
	}
}

void PYXDataCollection::CacheManager::memoryConsumedChanged(int memoryUsageDelta)
{
	if (memoryUsageDelta!=0)
	{
		m_totalMemoryUsed.memoryChangedDelta(memoryUsageDelta);
	}
}

void PYXDataCollection::CacheManager::freeMemory()
{
	std::vector<PYXPointer<PYXDataCollection::CollectionInfo>> memoryToFree;

	//collect 10 data collection not been used...
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		if (m_collections.size() == 0)
		{
			return;
		}

		if (m_checkIt == m_collections.end())
		{
			m_checkIt = m_collections.begin();
		}

		unsigned int count = m_collections.size();
		unsigned int amountToFree = std::max(count/4,(unsigned int)1);
		for(unsigned int i=0;i<count;++i)
		{
			PYXDataCollection::CollectionInfo & item = **m_checkIt; 

			//find item with memoryUsed and that it hasn't been accessed since last check..
			if ((item.m_memoryUsed > 0 && item.m_accessed == item.m_checked))
			{
				//TODO: this is not safe because the datacollection could be deleted or in the process of delete it self. need better solution..
				memoryToFree.push_back(*m_checkIt);
			}

			//marked as checked...
			item.m_checked = item.m_accessed;

			//move to the next item (cyclic manner)
			++m_checkIt;
			if (m_checkIt == m_collections.end())
			{
				m_checkIt = m_collections.begin();
			}

			//check if we have enough items..
			if (memoryToFree.size()>=amountToFree)
				break;
		}
	}

	//found nothing to free
	if (memoryToFree.size()==0)
		return;

	//free memory...
	int memoryFreed = 0;
	for(std::vector<PYXPointer<PYXDataCollection::CollectionInfo>>::iterator it = memoryToFree.begin();it != memoryToFree.end(); ++it)
	{
		memoryFreed += (**it).freeMemory();
	}

	//TRACE_INFO("Freeing PYXDataCollection resource: free " << memoryToFree.size() << " collections with " << memoryFreed/1024/1024 << " MB");

	//report to memory change to memory manager
	memoryConsumedChanged(-memoryFreed);

	//TRACE_INFO("Freeing PYXDataCollection total memory usage is :" << m_totalMemoryUsed.getByteCount()/1024/1024 << " MB");
}



Tester<PYXDataCollection> gTester;

class Data : public PYXObject
{
public:
	Data() {}
	static PYXPointer<Data> create(){
		return PYXNEW(Data);
	}
	std::list<std::string> list;
};

class DataFactory : public PYXDataCollection::SerializiedDataItemFactory<Data>
{
public:
	static const std::string name;

	DataFactory(PYXLocalStorage & storage) : SerializiedDataItemFactory(storage,name)
	{
	}

protected:

	virtual PYXPointer<Data> createNew() const
	{
		return Data::create();
	}

	virtual void generate(PYXPointer<Data> & item) const
	{
		item->list.push_back("Hello");
		item->list.push_back("World");
		item->list.push_back("This");
		item->list.push_back("Is");
		item->list.push_back("Great");
		for(int i =0;i<1000;++i)
		{
			item->list.push_back("Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer"
				"Very Long String indeed - this is used to generate a very big buffer");
		}
	}

	virtual void load(PYXWireBuffer & buffer,PYXPointer<Data> & item) const
	{		
		buffer >> item->list;
	}
	
	virtual void save(PYXWireBuffer & buffer,PYXPointer<Data> & item) const
	{
		buffer << item->list;
	}
};

const std::string DataFactory::name = "list";

void PYXDataCollection::test()
{
	PYXPointer<PYXLocalStorage> storage = PYXTempLocalStorage::create();
	PYXPointer<PYXDataCollection> collection = PYXDataCollection::create();

	PYXPointer<Data> item = collection->getItem(DataFactory(*storage));

	PYXPointer<PYXDataCollection> collection2 = PYXDataCollection::create();

	PYXPointer<Data> item2 = collection2->getItem(DataFactory(*storage));

	std::list<PYXPointer<PYXDataCollection>> bigList;

	/*
	while(1)
	{
		PYXPointer<PYXDataCollection> collection3 = PYXDataCollection::create(storage);

		PYXPointer<Data> item2 = collection3->getItem(DataFactory());

		bigList.push_back(collection3);
	}
	*/
}