#pragma once
/******************************************************************************
visible_store.h

begin		: 2010-05-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model_api.h"

#include "pyxis/derm/index.h"
#include "pyxis/procs/viewpoint.h"
#include "pyxis/utility/object.h"

#include <boost/thread/recursive_mutex.hpp>

#include <map>
#include <vector>
#include <set>


/*!
 *
 * VisibleStore is a class that cache visual data, per tile and/or per pipeline
 * VisibleStore is not responsible to store the data itself. but to manage the allocation and deallocation of the data it self.
 * The Memento design pattern is used to store the data. while the VisibleStore is used to gain keys to the memento class.
 *
 * To gain a key for a memento from the VisibleStore do the following:
 *  * gain PipelineKey - VisibleStore[pointer<IProcess>] or VisibleStore[ProcRef] - get a key that point to a pipeline (and all visible tiles associated with it).
 *  * gain TileKey - VisibleStore[PYXIcosIndex] - get a key that point to a tile (and all visible pipelines associated with it).
 *  * gain PipelineTileKey - VisibleStore[ProcRef][PYXIcosIndex] - gain key for a specific pipeline and a tile.
 * 
 * The lifetime of data inside VisibleStore is same as the visible tiles. the VisibleStore only keep keys objects and not data itself
 * To update the ViewpointProcess use the validate(IProcess) method that would create and remove needed pipelines.
 * To update the visible tiles, use setVisibleTiles(vector<PYXIcosIndex>) to refresh the visible list
 *
 * to determine if a key is visible use Storage::VisisbleStore::VisibleKey::isVisible(), that returns true if the tile is visible
 *
 * To store data inside the VisibleStore, use the Storage::Memento classes with the VisibleStore.
 * the Memento classes are template classes that attach to the specific notifiers inside the VisibleStore.
 * This mechanism is used to notify the Memento class to release any unneeded mementos.
 *
 * You can use the following mementos templates:
 *  * Memento object per pipeline - good for general data about pipelines
 *      Storage::Memento::VisiblePipelines<T>
 *  * Memento object per tile - good for elevation data for each tile
 *      Storage::Memento::VisibleTiles<T>
 *      Storage::Memento::CachedTiles<T> - and a cache size is passed on ctor
 *		Storage::Memento::VisibleTilesWithFillerThread<T> - if the creation process is slow, use this memento that have background thread to create mementos
 *  * Memento object per pipeline per tile - good for loading state for each tile
 *      Storage::Memento::VisiblePipelineTiles<T>
 *      Storage::Memento::CachedPipelineTiles<T> - and a cache size is passed on ctor
 *		Storage::Memento::VisiblePipelineTilesWithFillerThread<T> - if the creation process is slow, use this memento that have background thread to create mementos
 * 
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Visual Data Store declaration
///////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Storage
{
	namespace Memento
	{
	}

class VisibleStore;

class VisiblePipelineEvent : public NotifierEvent
{
protected:
	PYXPointer<VisibleStore> m_dataStore;
	ProcRef m_procRef;

public:
	VisiblePipelineEvent(PYXPointer<VisibleStore> dataStore,const ProcRef & procRef) : m_dataStore(dataStore), m_procRef(procRef)
	{
	}

	const ProcRef & getProcRef() const { return m_procRef; }
	PYXPointer<VisibleStore> getDataStore() { return m_dataStore; }
};

class VisibleTileEvent : public NotifierEvent
{
protected:
	PYXPointer<VisibleStore> m_dataStore;
	PYXIcosIndex m_index;

public:
	VisibleTileEvent(PYXPointer<VisibleStore> dataStore,const PYXIcosIndex & index) : m_dataStore(dataStore), m_index(index)
	{
	}

	const PYXIcosIndex & getIndex() const { return m_index; }
	PYXPointer<VisibleStore> getDataStore() { return m_dataStore; }
};

//! VisibleStore - the main class that keep track of the visual pipelines and vthe visual tiles
class VisibleStore : public PYXObject
{	
public:

	//! VisibleKey - represnt a specific visible tile for a specific pipeline.
	class VisibleKey : public PYXObject, public std::pair<PYXIcosIndex,ProcRef>
	{
	protected:
		PYXPointer<VisibleStore> m_dataStore;

	public:
		VisibleKey(PYXPointer<VisibleStore> dataStore,const PYXIcosIndex & index,const ProcRef & procRef) 
			: std::pair<PYXIcosIndex,ProcRef>(index,procRef),
			  m_dataStore(dataStore)
		{
		}

		VisibleKey(const VisibleKey & other) : m_dataStore(other.m_dataStore), std::pair<PYXIcosIndex,ProcRef>(other.first,other.second)
		{
		}

		VisibleKey & operator =(const VisibleKey & other)
		{
			m_dataStore = other.m_dataStore;
			first = other.first;
			second = other.second;
		}

		const PYXIcosIndex & getIndex() const { return first; }
		const ProcRef & getProcRef() const { return second; }
		PYXPointer<VisibleStore> getDataStore() { return m_dataStore; }
		
		bool isVisible() const { return m_dataStore->hasKey(*this); }
	};

	typedef std::vector<PYXPointer<VisibleKey>> VisibleKeysVector;

	//! VisibleTile - represent a visible tile inside the visible store. 
	class VisibleTile : public PYXObject
	{
	protected:
		PYXPointer<VisibleStore> m_dataStore;
		PYXIcosIndex m_index;

	public:
		VisibleTile(PYXPointer<VisibleStore> dataStore,const PYXIcosIndex & index) 
			: m_index(index),
			  m_dataStore(dataStore)
		{
		}
	
		const PYXIcosIndex & getIndex() const { return m_index; }
		PYXPointer<VisibleStore> getDataStore() { return m_dataStore; }
		VisibleKeysVector getKeys();
	};

	//! VisiblePipeline - represent a visible pipeline inside the visible store. 
	class VisiblePipeline : public PYXObject
	{
	protected:
		PYXPointer<VisibleStore> m_dataStore;
		ProcRef m_procRef;

	public:
		VisiblePipeline(PYXPointer<VisibleStore> dataStore,const ProcRef & procRef) 
			: m_procRef(procRef),
			  m_dataStore(dataStore)
		{
		}

		const ProcRef & getProcRef() const { return m_procRef; }
		PYXPointer<VisibleStore> getDataStore() { return m_dataStore; }
		VisibleKeysVector getKeys();
	};

protected:
	mutable boost::recursive_mutex m_mutex;

	boost::intrusive_ptr<IViewPoint> m_viewPointProcess;
	ProcRef m_viewPointProcessProcRef;

	//Quick access set
	std::set<ProcRef> m_pipelinesSet;
	std::set<PYXIcosIndex> m_visibleTilesSet;

	//Ordered visible tiles for fast returning all-* functions
	std::vector<ProcRef> m_pipelines;	
	std::vector<PYXIcosIndex> m_visibleTiles;

	Notifier m_tileAddedNotifier;
	Notifier m_tileRemovedNotifier;
	
	Notifier m_pipelineAddedNotifier;	
	Notifier m_pipelineRemovedNotifier;	

public:
	//! access the visible store to receive a visible tile
	PYXPointer<VisibleTile> operator[](const PYXIcosIndex & index);

	//! access the visible store to receive a visible pipeline
	PYXPointer<VisiblePipeline> operator[](const ProcRef & procRef);

	//! access the visible store to receive a visible tile of a visible pipeline
	PYXPointer<VisibleKey> getKey(const PYXIcosIndex & index,const ProcRef & procRef);

	//! query if a key is still visible.
	bool hasKey(const VisibleKey & key) const;
	bool hasKey(const PYXIcosIndex & index,const ProcRef & procRef) const;

	//! set the viewPointProcess. updates the visible pipelines inside the visible store and notify needed events
	void setViewPointProcess(boost::intrusive_ptr<IViewPoint> m_viewPointProcess);

	//! get the viewPointProcess of the visible store
	boost::intrusive_ptr<IViewPoint> getViewPointProcess() const;

	//! set the list (ordered list) of visible tiles. notify needed events
	void setVisibleTiles(const std::vector<PYXIcosIndex> & visibleTiles); 

	//! clear the visual store
	void clearAllData();

	//! get list of all visible pipelines
	std::vector<ProcRef> getPipelines() const;

	//! get list of all visible tiles
	std::vector<PYXIcosIndex> getVisibleTiles() const;

	//! get notifier that notify when a new tile become visible
	Notifier & getTileAddedNotifier() { return m_tileAddedNotifier;}
	//! get notifier that notify when a new tile become hidden
	Notifier & getTileRemovedNotifier() { return m_tileRemovedNotifier;}
	
	//! get notifier that notify when a new pipeline become visible
	Notifier & getPipelineAddedNotifier() { return m_pipelineAddedNotifier;}

	//! get notifier that notify when a new pipeline become hidden
	Notifier & getPipelineRemovedNotifier() { return m_pipelineRemovedNotifier;}

public:
	static PYXPointer<VisibleStore> create() 
	{
		return PYXNEW(VisibleStore);
	}

	VisibleStore();

	virtual ~VisibleStore();
	
};

};