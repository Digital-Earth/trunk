#ifndef TILE_CACHE_H
#define TILE_CACHE_H
/******************************************************************************
tile_cache.h

begin		: 2005-11-17
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"
#include "pyxis/data/exceptions.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/utility/cache_status.h"
#include "pyxis/utility/memory_manager.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/trace.h"

// boost includes

// standard includes
#include <list>

// forward declarations

/*!
Compare iterators based on the access time of the associated tile.

\param it1	The first iterator compare
\param it2	The second iterator

\return true if the first iterator tile is older than the second iterator tile.
*/
//! Comparison for sorting cached tile iterators by time
template <class T> 
bool timeCompare(const T& it1, const T& it2)
{
	return	it1->second->getCacheStatus()->getAccessedTime() < 
			it2->second->getCacheStatus()->getAccessedTime();
}

/*!
Compare iterators based on the number of references and then access time.
If the first has fewer references the method will return true. If the
two objects have the same reference count the the method will only 
return true if the first element is older.

\param it1	The first iterator to compare 
\param it2	The second iterator

\return true if the first iterator tile has fewer references than the second.
*/
//! Comparison for sorting cached tile iterators by time
template <class T> 
bool deleteCompare(const T& it1, const T& it2)
{
	//! first we order by refcount. but if they are equal...
	if (it1->second->getRefCount() == it2->second->getRefCount())
	{
		//we check the last accessedTime
		return	it1->second->getCacheStatus()->getAccessedTime() < 
				it2->second->getCacheStatus()->getAccessedTime();	
	}

	return it1->second->getRefCount() < it2->second->getRefCount();	
}

/*!
This class is used exclusively in test code to exercise the cache.
*/
//! This class is used to test the functionality of the generic tile cache
class PYXLIB_DECL TestTile : public PYXObject
{
public: 

	//! Definition for a list of pointers to a test tile
	typedef std::list< PYXPointer<TestTile> > TestTilePtrList;

	//! Create a new instance of a test tile by copying an existing tile.
	static PYXPointer<TestTile> create(const PYXTile& tile)
	{
		return PYXNEW(TestTile, tile);
	}

	//! Create a new instance of a test tile.
	static PYXPointer<TestTile> create()
	{
		return PYXNEW(TestTile);
	}

	//! constructor
	explicit TestTile(const PYXTile& tile) :
		m_tile(tile) 
	{
		m_cacheStatus.initialize();
	}

	//! Destructor
	~TestTile() {;}

	//! Return the tile geometry
	const PYXTile& getTile() {return m_tile;}

	//! Return the timer object
	CacheStatus* getCacheStatus() {return &m_cacheStatus;}

protected:

	//! Disable default constructor
	explicit TestTile()
	{
		m_cacheStatus.initialize();
	}

private:

	//! The tile geometry
	PYXTile m_tile;
	
	//! The time tracking object for the tile
	CacheStatus m_cacheStatus;
};

//! Events that originate from a tile cache.
template<class T> class  PYXTileCacheEvent : public NotifierEvent
{
public:
	
	//! The types of different PYXDataDisplayTileEvents
	enum eEventType
	{
		knDeletingTile,		//!< Tile is about to be deleted from the cache.
		knDeleteAllTiles	//!< The entire cache is about to be cleared.
	};

	//! Create a new instance of a tile event for a specific tile.
	static PYXPointer< PYXTileCacheEvent<T> > create(PYXPointer<T> spTile)
	{
		return PYXNEW(PYXTileCacheEvent, spTile);
	}

	//! Destructor.
	virtual ~PYXTileCacheEvent() {;}

	//! Set the specific event type.
	void setEventType(eEventType nType) {m_nType = nType;}

	//! Get the specific event type.
	eEventType getEventType() {return m_nType;}

	//! Get the tile source of the event message.
	PYXPointer<T> getTile() {return m_spTile;}

protected:

	//! Default constructor sets the type to knDeletingTile.
	PYXTileCacheEvent(PYXPointer<T> spTile):
		m_nType(knDeletingTile)
	{
		m_spTile = spTile;
	}

	//! Disable copy constructor
	PYXTileCacheEvent(const PYXTileCacheEvent&);

	//! Disable copy assignment
	void operator =(const PYXTileCacheEvent&);

private:

	//! The type of event being transmitted.
	eEventType m_nType;

	//! The source tile for the event.
	PYXPointer<T> m_spTile;
};

/*!
This cache allows the storage of any tile based, reference counted object.  The
cache holds and manages objects with large memory footprints that are keyed on 
geometry.  In order to create an implementation of a tile cache the tiles being 
stored must provide access to a CacheStatus object through a getCacheStatus() 
call.  

This class implements a thread safe interface to all public methods.

At the current time stored objects must also be reference counted.
*/
//! Templated and memory managed storage for tile based memory objects.
template<class T> class PYXTileCache 
: 
public PYXObject, 
public MemoryConsumer, 
public Notifier
{
public:

	//! Definition for a list of pointers to a type of tile
	typedef std::list< PYXPointer<T> > TilePtrList;

	//! Constants for the tile cache
	enum eConstants
	{
		//! Minimum size of tile cache
		knMinSize = 50,

		//! Maximum size of tile cache
		knMaxSize = 10000,

		//! Default size of tile cache
		knDefaultCacheSize = 150,

		//! Defaut number of tiles to release at one time
		knDefaultFreeBlockSize = 30,

		//! Defaut number of tiles to keep when requesting to free memory
		knDefaultMinSize = 10
	};

	//! Test method for automated testing.
	static void test();

	// Create a new instance of a tile cache of a specific type.
	static PYXPointer< PYXTileCache<T> > create()
	{
		return PYXNEW(PYXTileCache<T>);
	}

	//! Default constructor.
	PYXTileCache() :
		m_nMaxTileCount(knDefaultCacheSize),
		Notifier("Tile Cache") {;}

	//! Destructor.
	virtual ~PYXTileCache() {;}

	/*! 
	Add a tile to the tile cache

	\param spTile	The PYXTile to add to the cache
	*/
	//! Add a display tile to the cache
	void add(PYXPointer<T> spTile)
	{
		assert(spTile && "Invalid argument.");

		// verify the cache has the capacity to store another tile
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (m_nMaxTileCount <= static_cast<int>(m_mapTiles.size()))
		{
			// free a block of tiles
			if (!clearTiles(knDefaultFreeBlockSize))
			{
				// only throw if we are at the max count.
				if (m_nMaxTileCount <= static_cast<int>(m_mapTiles.size()))
				{
					PYXTHROW(	PYXTileCacheException,
								"Unable to free space in cache."	);
				}
				TRACE_ERROR("Unable to remove a full block of tiles from the tile cache during the add operation");
			}
		}

		// add the tile to the cache
		m_mapTiles.insert(std::make_pair(spTile->getTile(), spTile));
	}

	//! Return the number of tiles in the cache
	int size() const
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return static_cast<int>(m_mapTiles.size());
	}

	//! Return the maximum number of tiles the cache can hold
	int getMaxTileCount() const
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return m_nMaxTileCount;
	}

	/*!
	Set the maximum number of tiles the cache can hold.  If the new limit is set
	to less than the current size of the cache excess tiles will be deleted.  If 
	sufficient tiles can not be released the new value will not be set.

	\param nNewTileCount	The new size for the cache (must be in range).

	\return true if the new size of the cache was set.
	*/
	//! Set the maximum number of tiles the cache can hold
	bool setMaxTileCount(int nNewTileCount)
	{
		assert(	knMinSize <= nNewTileCount && nNewTileCount <= knMaxSize &&
				"Value out of range."	);

		boost::recursive_mutex::scoped_lock lock(m_mutex);

		if (nNewTileCount <= static_cast<int>(m_mapTiles.size()))
		{
			if (!clearTiles(	static_cast<int>(m_mapTiles.size()) - 
								nNewTileCount + 1	))
			{
				// the operation did not succeed
				TRACE_INFO(	"Unable to set max tile count to '" 
							<< nNewTileCount << "'."	);
				return false;
			}
		}

		m_nMaxTileCount = nNewTileCount;
		return true;
	}

	/*!
	Remove all tiles from the cache.  Observers will be notified with
	a PYXTileCacheEvent of type knDeleteAllTiles before anything is actually 
	removed.
	*/
	//! Remove all tiles from the cache.
	void clearAllTiles() 
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		PYXPointer< PYXTileCacheEvent<T> > spEvent = 
			PYXTileCacheEvent<T>::create(PYXPointer<T>());
		spEvent->setEventType(PYXTileCacheEvent<T>::knDeleteAllTiles);
		notify(spEvent);
		m_mapTiles.clear();
	}

	/*!
	Find and return the first tile in the cache that matches the given tile geometry.
	The cache will not return any tiles that have an expired cache state.

	\param tile	The tile geometry to look for

	\return	A smart pointer to the tile if one is found, else empty smart pointer.
	*/
	//! Retrieve a matching tile.
	virtual PYXPointer<T> getTile(const PYXTile& tile)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		// find the range of tiles that match the geometry
		std::pair<TileMap::iterator, TileMap::iterator> range = 
			m_mapTiles.equal_range(tile);
		TileMap::iterator itTile = range.first;
		for (; itTile != range.second; ++itTile)
		{
			if (!itTile->second->getCacheStatus()->hasExpired())
			{
				// first non-expired instance found: update accessed status and return
				itTile->second->getCacheStatus()->setAccessed();
				return itTile->second;
			}
			else
			{
				// remove the expired tile from the cache
				TileMap::iterator itDelete = itTile;
				
				PYXPointer< PYXTileCacheEvent<T> > spEvent = 
					PYXTileCacheEvent<T>::create(itTile->second);
				assert(	spEvent->getEventType() == PYXTileCacheEvent<T>::knDeletingTile &&
						"Default event type has changed.");
				notify(spEvent);

				m_mapTiles.erase(itDelete);
			}
		}

		// if we find nothing, return empty smart pointer
		return PYXPointer<T>();
	}

	/*! 
	Find and return all tiles in the cache that match the passed tile geometry.
	The cache will not return any tiles that have an expired time state.

	\param tile			The tile geometry to compare against
	\param pTileList	The list to fill with tile pointers
	\param bFastFind	When true, no cache cleanup of expired tiles will be done. 

	\return The number of tiles that are added to the list.
	*/
	//! Return the tiles that match the passed geometry
	int getTiles(const PYXTile& tile, TilePtrList* pTileList, bool bFastFind = false)
	{
		assert(pTileList != 0 && "NULL Tile list");
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		// find the range of tiles that match the geometry
		int nTileCount = 0;
		std::pair<TileMap::iterator, TileMap::iterator> range = 
			m_mapTiles.equal_range(tile);
		TileMap::iterator itTile = range.first;
		while (itTile != range.second)
		{
			if (!itTile->second->getCacheStatus()->hasExpired())
			{
				// add the tile to the set and increment the count
				itTile->second->getCacheStatus()->setAccessed();
				pTileList->push_back(itTile->second);
				++nTileCount;
				++itTile;
			}
			else if (!bFastFind)
			{
				// remove the expired tile from the cache
				TileMap::iterator itDelete = itTile;
				++itTile;

				PYXPointer< PYXTileCacheEvent<T> > spEvent = 
					PYXTileCacheEvent<T>::create(itDelete->second);
				assert(	spEvent->getEventType() == PYXTileCacheEvent<T>::knDeletingTile &&
						"Default event type has changed.");
				notify(spEvent);

				m_mapTiles.erase(itDelete);
			}
		}
		return nTileCount;
	}

	/*!
	The cache will free some of its memory resources at the request of the
	memory manager.
	*/
	//! Called by the memory manager to free up memory.
	virtual void freeMemory()
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		TRACE_MEMORY(	"freeMemory in PYXTileCache<> .  Cache size is '" <<
						static_cast<int>(m_mapTiles.size()) << "'."	);

		unsigned int blocksAmount = (unsigned int)std::max(std::max((int)m_mapTiles.size()-knDefaultFreeBlockSize,(int)m_mapTiles.size()/2),(int)knDefaultMinSize);
		if (blocksAmount < m_mapTiles.size())
		{
			clearTiles(m_mapTiles.size() - blocksAmount);
			TRACE_MEMORY(	"Cache size after clearing tiles is '" << 
							static_cast<int>(m_mapTiles.size()) << 
							"' after clearing tiles."	);
		}
	}

	//! Definition of a multi map of intrusive pointers keyed on tile geometry
	typedef std::multimap< PYXTile, PYXPointer<T> > TileMap;
 
	/*!
	This method provides direct access to the internal storage map used by 
	the cache.  Any use of this method must be aware of threading issues
	related to direct access.

	\return	A pointer to the internal tile storage container.
	*/
	//! Return a pointer to the storage map (ownership retained)
	TileMap* getCache() {return &m_mapTiles;}

	//! Return a reference to the mutex object
	boost::recursive_mutex& getMutex() {return m_mutex;}

	/*!
	Remove nBlockSize tiles. A tile is eligable for removal when its reference 
	count equal to 1. In other words, any tile that is only held in the cache
	is eligable for deletion. The least recently used of the eligable tiles are
	removed until the removal request is filled. This method is not thread safe 
	on its own.  It is assumed that any caller of this method will perform this 
	action within a critical section.

	\param nBlockSize		The number of tiles to attempt to free

	\return true if the requested number of tiles could be removed otherwise 
			false.
	*/
	//! Remove a number of tiles from the cache
	bool clearTiles(int nBlockSize)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		//TRACE_INFO("Clearing '" << nBlockSize << "' tiles from cache of size '" << m_mapTiles.size() << "'.");

		// remove all of the expired tiles
		TileMap::iterator itTiles = m_mapTiles.begin();
		std::vector< TileMap::iterator > vecIterators;
		for (; itTiles != m_mapTiles.end(); ++itTiles)
		{
			if (itTiles->second->getCacheStatus()->hasExpired())
			{
				PYXPointer< PYXTileCacheEvent<T> > spEvent = 
					PYXTileCacheEvent<T>::create(itTiles->second);

				notify(spEvent);
				m_mapTiles.erase(itTiles);
				--nBlockSize;
				continue;
			}

			// add all non expired tiles to a vector for potential deletion
			vecIterators.push_back(itTiles);
		}

		if (nBlockSize <= 0)
		{
			// the expired tiles filled the delete request
			return true;
		}

		// partition the nBlockSize oldest elements
		int nNumToDelete = std::min(	static_cast<int>(vecIterators.size()),
										nBlockSize	);
		std::nth_element(	vecIterators.begin(), 
							vecIterators.begin() + nNumToDelete, 
							vecIterators.end(), 
							deleteCompare< TileMap::iterator >	);

		// delete nNumToDelete elements from the map
		std::vector< TileMap::iterator >::iterator itVec = vecIterators.begin();
		for (; itVec != vecIterators.begin() + nNumToDelete; ++itVec)
		{
			PYXPointer< PYXTileCacheEvent<T> > spEvent = 
				PYXTileCacheEvent<T>::create((*itVec)->second);
			notify(spEvent);
			m_mapTiles.erase(*itVec);
		}

		if (nNumToDelete < nBlockSize)
		{
			// Not all of the tiles could be removed, trace a message
			TRACE_INFO(	"Could only remove '" << 
							nNumToDelete << 
							"' of '" <<
							nBlockSize <<
							"' tiles from the display tile cache."	);
			return false;
		}
		return true;
	}

protected:

private:

	//! The container that holds the display tiles
	TileMap m_mapTiles;

	//! The maximum number of tiles that can be held in the container
	int m_nMaxTileCount;

	//! The mutex used to lock the class
	mutable boost::recursive_mutex m_mutex;
};

#endif
