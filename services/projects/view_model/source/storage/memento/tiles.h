#pragma once
/******************************************************************************
tiles.h

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
// TileMementoStore implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class VisibleTiles : public PYXObject
{
protected:
	typedef std::map<PYXIcosIndex,T> MementosMap;
protected:
	boost::recursive_mutex m_mutex;
	MementosMap m_mementos;
	PYXPointer<VisibleStore> m_dataStore;

public:
	VisibleTiles(const PYXPointer<VisibleStore> & dataStore) : m_dataStore(dataStore)
	{
		m_dataStore->getTileRemovedNotifier().attach(this,&VisibleTiles::tileRemoved);		
	}

	virtual ~VisibleTiles()
	{
		m_dataStore->getTileRemovedNotifier().detach(this,&VisibleTiles::tileRemoved);		
	}

	virtual T & get(const PYXIcosIndex & index)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		//this is same as m_mementos[tile->getIndex()]; - however, it seems that operator[] is creating new T even if item already exists
		MementosMap::iterator it = m_mementos.find(index);
		if (it == m_mementos.end())
		{
			m_mementos.insert(std::make_pair(index,T()));
			it = m_mementos.find(index);
		}

		return it->second;
	}

	virtual T & get(const PYXPointer<VisibleStore::VisibleKey> & key)
	{
		assert(key);
		return get(key->getIndex());
	}

	void tileRemoved(PYXPointer<NotifierEvent> notifierEvent)
	{
		VisibleTileEvent * tile = dynamic_cast<VisibleTileEvent *>(notifierEvent.get());

		if (tile != 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			m_mementos.erase(tile->getIndex());
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// CachedTilesMementoStore implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class CachedTiles : public PYXObject
{
protected:
	boost::recursive_mutex m_mutex;
	PYXPointer<VisibleStore> m_dataStore;
	std::map<PYXIcosIndex,T> m_mementos;
	std::list<PYXIcosIndex> m_usage;
	const unsigned int m_cacheSize;

public:
	CachedTiles(const PYXPointer<VisibleStore> & dataStore, int unsigned cacheSize) : m_dataStore(dataStore), m_cacheSize(cacheSize)
	{	
	}

	virtual ~CachedTiles()
	{	
	}

	virtual T & get(const PYXPointer<VisibleStore::VisibleKey> & tile)
	{
		assert(tile);
		return get(tile->getIndex());
	}

	virtual T & get(const PYXIcosIndex & index)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		
		//this is same as m_mementos[tile->getIndex()]; - however, it seems that operator[] is creating new T even if item already exists
		std::map<PYXIcosIndex,T>::iterator it = m_mementos.find(index);
		if (it == m_mementos.end())
		{
			m_mementos.insert(std::make_pair(index,T()));
			it = m_mementos.find(index);
		}

		updateUsage(index);
		
		limitCache();			

		return it->second;
	}		

private:
	void updateUsage(const PYXIcosIndex & index)
	{
		//update m_list
		std::list<PYXIcosIndex>::iterator it = std::find(m_usage.begin(),m_usage.end(),index);
		if (it != m_usage.end())
		{
			m_usage.erase(it);
		}
		//first item is the oldest
		m_usage.push_back(index);
	}

	void limitCache()
	{
		std::list<PYXIcosIndex>::iterator it = m_usage.begin();
		while (m_mementos.size() > m_cacheSize && it != m_usage.end())
		{
			std::list<PYXIcosIndex>::iterator temp_it = it;
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
// Visible Tile Memento with Priorities Filler thread implementation (base class)
///////////////////////////////////////////////////////////////////////////////////////////////////////////

class VisibleTilesWithFillerThreadBase : public PYXObject
{
protected:
	boost::recursive_mutex m_mutex;
	typedef std::map<PYXIcosIndex,PYXPointer<PYXObject>> MementoMap;
	typedef std::set<PYXIcosIndex> MementoToFillMap;

	MementoMap m_mementos;
	MementoToFillMap m_mementosToFill;

	PYXPointer<VisibleStore> m_dataStore;

	boost::thread			  m_fillerThread;
	boost::mutex			  m_threadMutex;
	boost::condition_variable m_hasTilesToFillCondition;
	bool m_stop;

public:
	VisibleTilesWithFillerThreadBase(const PYXPointer<VisibleStore> & dataStore) : m_dataStore(dataStore), m_stop(false)
	{
		m_dataStore->getTileRemovedNotifier().attach(this,&VisibleTilesWithFillerThreadBase::tileRemoved);
	
		m_fillerThread = boost::thread(boost::bind(&VisibleTilesWithFillerThreadBase::fillerThreadFunction,this));
	}

	virtual ~VisibleTilesWithFillerThreadBase()
	{
		m_dataStore->getTileRemovedNotifier().detach(this,&VisibleTilesWithFillerThreadBase::tileRemoved);
	
		m_stop = true;
		m_hasTilesToFillCondition.notify_one();
		m_fillerThread.join();
	}	


protected:
	//! check if the current memento is still uptodate 
	virtual bool baseIsUpToDate(const PYXIcosIndex & index,const PYXPointer<PYXObject> & memento)
	{
		return true;
	}

	//! called by the fillter thread
	virtual PYXPointer<PYXObject> baseCreateMemento(const PYXIcosIndex & index,const PYXPointer<PYXObject> & oldMemento)
	{
		PYXTHROW(PYXException,"Function should never be called");
		return PYXPointer<PYXObject>();
	}

protected:
	PYXPointer<PYXObject> baseGet(const PYXIcosIndex & index)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		PYXPointer<PYXObject> result = m_mementos[index];
		
		if (! result || ! baseIsUpToDate(index,result))
		{
			m_mementosToFill.insert(index);
			m_hasTilesToFillCondition.notify_one();
		}

		return result;
	}

	void tileRemoved(PYXPointer<NotifierEvent> notifierEvent)
	{
		VisibleTileEvent * tile = dynamic_cast<VisibleTileEvent *>(notifierEvent.get());

		if (tile != 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			
			m_mementos.erase(tile->getIndex());
			m_mementosToFill.erase(tile->getIndex());			
		}
	}	

protected:

	PYXIcosIndex findNeededTile()
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
					return *itToFill;
				}
			}
		}
		return PYXIcosIndex();
	}

	void fillerThreadFunction()
	{
		while (!m_stop)
		{
			//try to find a needed tile to fill
			PYXIcosIndex index(findNeededTile());
			if (!index.isNull())
			{
				try
				{
					//fetch old memento
					PYXPointer<PYXObject> oldMemento;
					{
						boost::recursive_mutex::scoped_lock lock(m_mutex);

						MementoMap::iterator it = m_mementos.find(index);
						if (it != m_mementos.end())
						{
							oldMemento = it->second;
						}						
					}
					//we found one - create it.
					PYXPointer<PYXObject> result = baseCreateMemento(index,oldMemento);					

					{
						//mark that we have a new memento
						boost::recursive_mutex::scoped_lock lock(m_mutex);

						//check if we still need this tile...
						if (m_mementos.find(index) != m_mementos.end())
						{
							m_mementos[index] = result;
							m_mementosToFill.erase(index);
						}
					}
				}
				catch(...)
				{
					TRACE_ERROR("Failed to create memento for tile " << index);
				}
			}
			else
			{
				//wait until we notify about a need tile
				boost::mutex::scoped_lock lock(m_threadMutex);
				m_hasTilesToFillCondition.wait(lock);
			}
		}
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Visible Tile Memento with Priorities Filler thread implementation 
///////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
class VisibleTilesWithFillerThread: public VisibleTilesWithFillerThreadBase
{
public:
	VisibleTilesWithFillerThread(const PYXPointer<VisibleStore> & dataStore) : VisibleTilesWithFillerThreadBase(dataStore)
	{
	}

	PYXPointer<T> get(const PYXPointer<VisibleStore::VisibleKey> & key)
	{
		return get(key->getIndex());
	}

	PYXPointer<T> get(const PYXIcosIndex & index)
	{
		return dynamic_cast<T*>(baseGet(index));
	}

protected:
	//!check if the current memento is still uptodate 
	virtual bool isUpToDate(const PYXIcosIndex & index,const PYXPointer<T> & memento)
	{
		return true;
	}

	//! called by the fillter thread
	virtual PYXPointer<T> createMemento(const PYXIcosIndex & index,const PYXPointer<T> & oldMemento) = 0;

protected:
	virtual bool baseIsUpToDate(const PYXIcosIndex & index,const PYXPointer<PYXObject> & memento)
	{
		return isUpToDate(index,dynamic_cast<T*>(memento.get()));
	}

	virtual PYXPointer<PYXObject> baseCreateMemento(const PYXIcosIndex & index,const PYXPointer<PYXObject> & oldMemento)
	{
		return createMemento(index,dynamic_cast<T*>(oldMemento.get()));
	}
};

//namespace Memento
}
//namespace Storage
}