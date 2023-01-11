#pragma once
/******************************************************************************
pipelines_tiles.h

begin		: 2010-05-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model_api.h"
#include "storage/visible_store.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <map>
#include <vector>
#include <set>


namespace Storage
{
namespace Memento
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// VisiblePipelineTiles implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class VisiblePipelineTiles : public PYXObject
{
protected:
	typedef std::map<Storage::VisibleStore::VisibleKey,T> MementosMap;
protected:
	boost::recursive_mutex m_mutex;
	MementosMap m_mementos;
	PYXPointer<Storage::VisibleStore> m_dataStore;

public:
	VisiblePipelineTiles(const PYXPointer<Storage::VisibleStore> & dataStore) : m_dataStore(dataStore)
	{
		m_dataStore->getTileRemovedNotifier().attach(this,&VisiblePipelineTiles::tileRemoved);
		m_dataStore->getPipelineRemovedNotifier().attach(this,&VisiblePipelineTiles::pipelineRemoved);
	}

	virtual ~VisiblePipelineTiles()
	{
		m_dataStore->getTileRemovedNotifier().detach(this,&VisiblePipelineTiles::tileRemoved);
		m_dataStore->getPipelineRemovedNotifier().detach(this,&VisiblePipelineTiles::pipelineRemoved);
	}

	virtual T & get(const PYXPointer<Storage::VisibleStore::VisibleKey> & key)
	{
		assert(key);

		boost::recursive_mutex::scoped_lock lock(m_mutex);

		//this is same as m_mementos[tile->getIndex()]; - however, it seems that operator[] is creating new T even if item already exists
		MementosMap::iterator it = m_mementos.find(*key);
		if (it == m_mementos.end())
		{
			m_mementos.insert(std::make_pair(*key,T()));
			it = m_mementos.find(tile.get());
		}

		return it->second;
	}

	void pipelineRemoved(PYXPointer<NotifierEvent> notifierEvent)
	{
		VisiblePipelineEvent * pipeline = dynamic_cast<Storage::VisiblePipelineEvent *>(notifierEvent.get());

		if (pipeline != 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			VisibleStore::VisibleKeysVector keys((*m_dataStore)[pipeline->getProcRef()]->getKeys());
			for(VisibleStore::VisibleKeysVector::iterator it = keys.begin();it != keys.end();++it)
			{
				m_mementos.erase(*it);
			}
		}
	}

	void tileRemoved(PYXPointer<NotifierEvent> notifierEvent)
	{
		VisibleTileEvent * tile = dynamic_cast<Stroage::VisibleTileEvent *>(notifierEvent.get());

		if (tile != 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			VisibleStore::VisibleKeysVector keys((*m_dataStore)[tile->getIndex()]->getKeys());
			for(VisibleStore::VisibleKeysVector::iterator it = keys.begin();it != keys.end();++it)
			{
				m_mementos.erase(*it);
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// CachedPipelineTiles implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class CachedPipelineTiles : public PYXObject
{
protected:
	boost::recursive_mutex m_mutex;
	PYXPointer<Storage::VisibleStore> m_dataStore;
	std::map<Storage::VisibleStore::VisibleKey,T> m_mementos;
	std::list<Storage::VisibleStore::VisibleKey> m_usage;
	const unsigned int m_cacheSize;

public:
	CachedPipelineTiles(const PYXPointer<Storage::VisibleStore> & dataStore, int unsigned cacheSize) : m_dataStore(dataStore), m_cacheSize(cacheSize)
	{	
	}

	virtual ~CachedPipelineTiles()
	{	
	}

	virtual T & get(const PYXPointer<Storage::VisibleStore::VisibleKey> & tile)
	{
		assert(tile);

		boost::recursive_mutex::scoped_lock lock(m_mutex);
		
		//this is same as m_mementos[tile->getIndex()]; - however, it seems that operator[] is creating new T even if item already exists
		std::map<Storage::VisibleStore::VisibleKey,T>::iterator it = m_mementos.find(*tile);
		if (it == m_mementos.end())
		{
			m_mementos.insert(std::make_pair(*tile,T()));
			it = m_mementos.find(*tile);
		}

		updateUsage(tile);
		
		limitCache();			

		return it->second;
	}		

private:
	void updateUsage(const PYXPointer<Storage::VisibleStore::VisibleKey> & tile)
	{
		//update m_list
		std::list<Storage::VisibleStore::VisibleKey>::iterator it = std::find(m_usage.begin(),m_usage.end(),*tile);
		if (it != m_usage.end())
		{
			m_usage.erase(it);
		}
		//first item is the oldest
		m_usage.push_back(*tile);
	}

	void limitCache()
	{
		std::list<Storage::VisibleStore::VisibleKey>::iterator it = m_usage.begin();
		while (m_mementos.size() > m_cacheSize && it != m_usage.end())
		{
			std::list<Storage::VisibleStore::VisibleKey>::iterator temp_it = it;
			++it;
			if (!temp_it->isVisible())
			{				
				m_mementos.erase(*temp_it);				
				//Note. because we advance ++it. it won't be invalidated after this erase
				m_usage.erase(temp_it);				
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipline Tile Memento with Priorities Filler thread implementation (base class)
///////////////////////////////////////////////////////////////////////////////////////////////////////////

class VisiblePipelineTilesWithFillerThreadBase : public PYXObject
{
protected:
	boost::recursive_mutex m_mutex;
	typedef std::map<Storage::VisibleStore::VisibleKey,PYXPointer<PYXObject>> MementoMap;
	typedef std::multimap<PYXIcosIndex,Storage::VisibleStore::VisibleKey> MementoToFillMap;

	MementoMap m_mementos;
	MementoToFillMap m_mementosToFill;

	PYXPointer<VisibleStore> m_dataStore;

	boost::thread			  m_fillerThread;
	boost::mutex			  m_threadMutex;
	boost::condition_variable m_hasTilesToFillCondition;
	bool m_stop;

public:
	VisiblePipelineTilesWithFillerThreadBase (const PYXPointer<Storage::VisibleStore> & dataStore) : m_dataStore(dataStore), m_stop(false)
	{
		m_dataStore->getTileRemovedNotifier().attach(this,&VisiblePipelineTilesWithFillerThreadBase ::tileRemoved);
		m_dataStore->getPipelineRemovedNotifier().attach(this,&VisiblePipelineTilesWithFillerThreadBase ::pipelineRemoved);

		m_fillerThread = boost::thread(boost::bind(&VisiblePipelineTilesWithFillerThreadBase ::fillerThreadFunction,this));
	}

	virtual ~VisiblePipelineTilesWithFillerThreadBase()
	{
		m_dataStore->getTileRemovedNotifier().detach(this,&VisiblePipelineTilesWithFillerThreadBase::tileRemoved);
		m_dataStore->getPipelineRemovedNotifier().detach(this,&VisiblePipelineTilesWithFillerThreadBase::pipelineRemoved);

		m_stop = true;
		{
			boost::mutex::scoped_lock threadLock(m_threadMutex);
			m_hasTilesToFillCondition.notify_one();
		}		
		m_fillerThread.join();
	}	

	PYXPointer<PYXObject> baseGet(const PYXPointer<Storage::VisibleStore::VisibleKey> & tile)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		PYXPointer<PYXObject> result = m_mementos[*tile];
		
		if (! result || ! baseIsUpToDate(tile,result))
		{				
			insertMementoToFill(*tile);
			{
				m_hasTilesToFillCondition.notify_all();
			}
		}

		return result;
	}

	void pipelineRemoved(PYXPointer<NotifierEvent> notifierEvent)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		VisiblePipelineEvent * pipeline = dynamic_cast<Storage::VisiblePipelineEvent *>(notifierEvent.get());

		if (pipeline != 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			VisibleStore::VisibleKeysVector keys((*m_dataStore)[pipeline->getProcRef()]->getKeys());

			//remove mementos
			for(VisibleStore::VisibleKeysVector::iterator it = keys.begin();it != keys.end();++it)
			{
				m_mementos.erase((**it));
			}

			//remove needed mementos
			for(MementoToFillMap::iterator it = m_mementosToFill.begin();it != m_mementosToFill.end(); )
			{
				MementoToFillMap::iterator remove_it = it;
				++it;

				if (remove_it->second.getProcRef() == pipeline->getProcRef())
				{
					m_mementosToFill.erase(remove_it);
				} 
			}
		}
	}

	void tileRemoved(PYXPointer<NotifierEvent> notifierEvent)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		VisibleTileEvent * tile = dynamic_cast<Storage::VisibleTileEvent *>(notifierEvent.get());

		if (tile != 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			VisibleStore::VisibleKeysVector keys((*m_dataStore)[tile->getIndex()]->getKeys());
			for(VisibleStore::VisibleKeysVector::iterator it = keys.begin();it != keys.end();++it)
			{
				m_mementos.erase((**it));
				m_mementosToFill.erase((*it)->getIndex());
			}
		}
	}	

protected:
	//! check if the current memento is still uptodate 
	virtual bool baseIsUpToDate(const PYXPointer<Storage::VisibleStore::VisibleKey> & tile,const PYXPointer<PYXObject> & memento)
	{
		return true;
	}

	//! called by the fillter thread
	virtual PYXPointer<PYXObject> baseCreateMemento(const PYXPointer<Storage::VisibleStore::VisibleKey> & key,const PYXPointer<PYXObject> & oldMemento)
	{
		PYXTHROW(PYXException,"Function should never be called");
		return PYXPointer<PYXObject>();
	}

protected:

	PYXPointer<Storage::VisibleStore::VisibleKey> findNeededTile()
	{
		//Note: this operation look the dataStore, therefore we do it outside the private look operation
		std::vector<PYXIcosIndex> visibleTiles(m_dataStore->getVisibleTiles());

		{
			//We need to look the m_mementosToFill inorder to access it.
			boost::recursive_mutex::scoped_lock lock(m_mutex);
				
			for(std::vector<PYXIcosIndex>::iterator it = visibleTiles.begin(); it != visibleTiles.end(); ++it)
			{
				MementoToFillMap::iterator itToFill = m_mementosToFill.find(*it);
				if (itToFill != m_mementosToFill.end())
				{
					return PYXNEW(VisibleStore::VisibleKey,itToFill->second);
				}
			}		
		}

		return PYXPointer<Storage::VisibleStore::VisibleKey>();
	}

	void removeMementoToFill(const Storage::VisibleStore::VisibleKey & key)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		MementoToFillMap::iterator it = m_mementosToFill.find(key.getIndex());
		while (it != m_mementosToFill.end())
		{
			if (it->second == key)
			{
				m_mementosToFill.erase(it);
				return;
			}

			//check if we would still find it
			if (it->first != key.getIndex())
			{
				break;
			}

			++it;
		}
	}

	void insertMementoToFill(const Storage::VisibleStore::VisibleKey & key)
	{		
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		MementoToFillMap::iterator it = m_mementosToFill.find(key.getIndex());
		while (it != m_mementosToFill.end())
		{
			//check if we found the key
			if (it->second == key)
			{				
				return;
			}
			
			//check if we would still find it
			if (it->first != key.getIndex())
			{
				break;
			}

			++it;
		}

		m_mementosToFill.insert(std::make_pair(key.getIndex(),key));
	}

	void fillerThreadFunction()
	{		
		while (!m_stop)
		{
			//try to find a needed tile to fill
			PYXPointer<Storage::VisibleStore::VisibleKey> key = findNeededTile();
			if (key)
			{
				try
				{
					//fetch old memento
					PYXPointer<PYXObject> oldMemento;
					{
						boost::recursive_mutex::scoped_lock lock(m_mutex);

						MementoMap::iterator it = m_mementos.find(*key);
						if (it != m_mementos.end())
						{
							oldMemento = it->second;
						}						
					}
					//we found one - create it.
					PYXPointer<PYXObject> result = baseCreateMemento(key,oldMemento);

					{
						//mark that we have a new memento
						boost::recursive_mutex::scoped_lock lock(m_mutex);

						//check if we still need this tile, then update the memento
						if (m_mementos.find(*key) != m_mementos.end())
						{
							m_mementos[*key] = result;														
						}

						removeMementoToFill(*key);
					}
				}
				catch(...)
				{
					TRACE_ERROR("Failed to create memento for tile " << key->getIndex());
				}
			}
			else
			{
				//wait until we notify about a need tile
				boost::mutex::scoped_lock threadLock(m_threadMutex);
				m_hasTilesToFillCondition.wait(threadLock);
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pipline Tile Memento with Priorities Filler thread implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
class VisiblePipelineTilesWithFillerThread : public VisiblePipelineTilesWithFillerThreadBase
{
public:
	VisiblePipelineTilesWithFillerThread(const PYXPointer<Storage::VisibleStore> & dataStore) : VisiblePipelineTilesWithFillerThreadBase(dataStore)
	{
	}

	PYXPointer<T> get(const PYXPointer<VisibleStore::VisibleKey> & key)
	{
		return dynamic_cast<T*>(baseGet(key).get());
	}

protected:
	//!check if the current memento is still uptodate 
	virtual bool isUpToDate(const PYXPointer<Storage::VisibleStore::VisibleKey> & key,const PYXPointer<T> & memento)
	{
		return true;
	}

	//! called by the fillter thread
	virtual PYXPointer<T> createMemento(const PYXPointer<Storage::VisibleStore::VisibleKey> & key,const PYXPointer<T> & oldMemento) = 0;

protected:
	virtual bool baseIsUpToDate(const PYXPointer<Storage::VisibleStore::VisibleKey> & key,const PYXPointer<PYXObject> & memento)
	{
		return isUpToDate(key,dynamic_cast<T*>(memento.get()));
	}

	virtual PYXPointer<PYXObject> baseCreateMemento(const PYXPointer<Storage::VisibleStore::VisibleKey> & key,const PYXPointer<PYXObject> & oldMemento)
	{
		return createMemento(key,dynamic_cast<T*>(oldMemento.get()));
	}
};

//namespace Memento
}
//namepsace Storage
}