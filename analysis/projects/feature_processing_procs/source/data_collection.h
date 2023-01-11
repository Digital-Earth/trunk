#ifndef DATA_COLLECTION_H
#define DATA_COLLECTION_H
/******************************************************************************
data_collection.h

begin		: March 12, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/utility/local_storage.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/memory_manager.h"

#include <map>

#include "boost/scoped_ptr.hpp"
#include <boost/thread/recursive_mutex.hpp>

///////////////////////////////////////////////////////////////////////////////
// PYXDataCollection
///////////////////////////////////////////////////////////////////////////////

class PYXDataCollection : public PYXObject
{
public:
	template<class T>
	class DataItemFactory
	{
	public:
		virtual ~DataItemFactory()
		{
		}

	public:
		virtual const std::string & getName() const = 0;

		virtual void createItem(PYXPointer<T> & item,int & itemSize) const = 0;
	};

	template<class T>
	class SerializiedDataItemFactory : public DataItemFactory<T>
	{
	private:
		PYXLocalStorage & m_localStorage;
		const std::string & m_name;

	public:
		SerializiedDataItemFactory(PYXLocalStorage & localStorage,const std::string & name) : m_name(name), m_localStorage(localStorage)
		{
		}

		virtual const std::string & getName() const { return m_name; }

		virtual void createItem(PYXPointer<T> & item,int & itemSize) const 
		{
			item = createNew();

			boost::scoped_ptr<PYXWireBuffer> buffer(m_localStorage.get(m_name));

			if (buffer)
			{
				load(*buffer,item);
			}
			else 
			{
				generate(item);

				buffer.reset(new PYXStringWireBuffer());
				save(*buffer,item);
				m_localStorage.set(m_name,*buffer);
			}

			//assuming unseralized value would take twice as much as the serlized value...
			itemSize = buffer->size()*2;
		}

	protected:

		//create a new and empty item
		virtual PYXPointer<T> createNew() const = 0;
		
		//generate information for the item - this get called when the item can't be loaded from storage
		virtual void generate(PYXPointer<T> & item) const = 0;

		//load item from the storage
		virtual void load(PYXWireBuffer & buffer,PYXPointer<T> & item) const = 0;

		//save item to the storage
		virtual void save(PYXWireBuffer & buffer,PYXPointer<T> & item) const = 0;
	};

private:

	//forward decalarion.
	class CollectionInfo;

	class CacheManager : public MemoryConsumer
	{
	public:
		typedef std::list<PYXPointer<PYXDataCollection::CollectionInfo>> List;

		static CacheManager * getInstance()
		{
			if (m_pInstance == 0)
			{
				m_pInstance = new CacheManager();
			}
			assert(m_pInstance);
			return m_pInstance;
		}

	private:
		static CacheManager* m_pInstance;

		boost::recursive_mutex m_mutex;
		
		List m_collections;
		List::iterator m_checkIt;

		VaryingMemoryUsed m_totalMemoryUsed;

		CacheManager() : m_totalMemoryUsed(0)
		{
			m_checkIt = m_collections.end();
		}

	public:
		void add(const PYXPointer<PYXDataCollection::CollectionInfo> & info);
		void remove(const PYXPointer<PYXDataCollection::CollectionInfo> & info);

		void memoryConsumedChanged(int memoryUsageDelta);

		static const int MAX_ITEMS_TO_FREE = 30;

		virtual void freeMemory();
	};

private:
	class CollectionInfo : public PYXObject
	{
	public:
		CacheManager::List::iterator m_cacheIterator;
		int m_accessed;
		int m_checked;
		int m_memoryUsed;

		boost::recursive_mutex m_mutex;
		typedef std::map<std::string,PYXPointer<PYXObject>> DataItemsMap;

		DataItemsMap m_dataItems;

		CollectionInfo() : m_accessed(0), m_checked(0), m_memoryUsed(0)
		{
		}

		int freeMemory()
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			m_dataItems.clear();

			int freed = m_memoryUsed;
			m_memoryUsed = 0;

			return freed;
		}
	};

	PYXPointer<CollectionInfo> m_info;

public:

	template<class T>
	PYXPointer<T> getItem(const DataItemFactory<T> & factory)
	{
		int memoryDelta = 0;
		PYXPointer<T> item;

		{
			boost::recursive_mutex::scoped_lock lock(m_info->m_mutex);

			//mark as accessed..
			m_info->m_accessed++;

			CollectionInfo::DataItemsMap::iterator it = m_info->m_dataItems.find(factory.getName());
			
			if (it != m_info->m_dataItems.end())
			{
				return (T*)(it->second.get());
			}
		}

		factory.createItem(item,memoryDelta);

		{
			boost::recursive_mutex::scoped_lock lock(m_info->m_mutex);

			CollectionInfo::DataItemsMap::iterator it = m_info->m_dataItems.find(factory.getName());
			
			if (it != m_info->m_dataItems.end())
			{
				return (T*)(it->second.get());
			}

			m_info->m_dataItems[factory.getName()] = item;
			m_info->m_memoryUsed += memoryDelta;
		}

		CacheManager::getInstance()->memoryConsumedChanged(memoryDelta);
		return item;
	}

private:

	int freeMemory()
	{
		return m_info->freeMemory();
	}

private:
	PYXDataCollection() :
	   m_info(new CollectionInfo())
	{
		CacheManager::getInstance()->add(m_info);
	}

	PYXDataCollection(const PYXDataCollection &);

	PYXDataCollection & operator=(const PYXDataCollection &);

public:
	static PYXPointer<PYXDataCollection> create()
	{
		return PYXNEW(PYXDataCollection);
	}

public:
	virtual ~PYXDataCollection()
	{
		CacheManager::getInstance()->remove(m_info);
	}	

	static void test();
};

#endif // guard
