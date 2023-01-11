#ifndef CACHE_H
#define CACHE_H
/******************************************************************************
cache.h

begin      : 3/14/2007 1:47:35 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"
#include "pyxis/data/record.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/pyxcom.h"

//! PyxCOM interface to a cache process.
struct PYXLIB_DECL ICache : public PYXCOM_IUnknown 
{
	PYXCOM_DECLARE_INTERFACE();

public:

	/*! Sets default directory to store the cache.  Normally, this is set lazily at
	    the time the cache is created, but sometimes it is needed that this be set ahead
		so that files can be placed into the proper directory to seed the cache.
	*/
	virtual const void STDMETHODCALLTYPE initCacheDir() const = 0;

	//! Set the directory in which to persist the cache.
	virtual void STDMETHODCALLTYPE setCacheDir(const std::string& strDir) = 0;

	//! Get the directory where the cache is persisted.
	virtual const std::string STDMETHODCALLTYPE getCacheDir() const = 0;

	//! Set the tile depth.
	virtual void STDMETHODCALLTYPE setCacheTileDepth(const int nDepth) = 0;

	//! Get the Tile Depth.
	virtual const int STDMETHODCALLTYPE getCacheTileDepth() const = 0;

	//! Set the resolution of cells to cache at. 
	virtual void STDMETHODCALLTYPE setCacheCellResolution(const int nCellResolution) = 0;

	virtual const int STDMETHODCALLTYPE getCacheCellResolution() = 0;

	//! Set whether to persist the cache to disk or not. 
	virtual void STDMETHODCALLTYPE setCachePersistence(bool bPersistent) = 0;

	//! Open an existing data set for reading.
	virtual bool STDMETHODCALLTYPE openReadOnly(const std::string& strDir) = 0;

	//! Open an existing data set for read/write.
	virtual bool STDMETHODCALLTYPE openReadWrite(const std::string& strDir) = 0;

	//! Set the maximum number of tiles the cache can hold.
	virtual void STDMETHODCALLTYPE setCacheMaxTileCount(const int nMaxTiles) = 0;

	//! Create a new data set for writing, or open it if it already exists
	virtual bool  STDMETHODCALLTYPE openReadWrite(
		const std::string& strDir,
		const PYXTableDefinition& defn,
		const std::vector<PYXValue>& vecValues,
		const PYXTableDefinition& coverageDefn,
		int nCellResolution,
		int nTileResolution = 0	) = 0;

	//! Close an open data set
	virtual void STDMETHODCALLTYPE close() = 0;

	//! Force all tiles to be persisted immediately 
	virtual	void STDMETHODCALLTYPE persistAllTiles() = 0;

	//! Return the storage file for the given tile.
	virtual const std::string STDMETHODCALLTYPE toFileName(const PYXTile &tile) const = 0;

	/*!
	When in this mode the cache will request at least a block (in PYXIS terms a
	tile) of data from its input even if only a single value is requested from the 
	process itself. 
	*/
	//! The mode where a cache will always store blocks of data rather than single values
	virtual void STDMETHODCALLTYPE setGreedyCache(bool bGreedy) = 0;

	/*! 
	Insert a tile file into the cache.  This should be a properly named
	file, in the naming convention of the DefaultCoverage, and it should already
	be in the correct directory.
	*/
	virtual void STDMETHODCALLTYPE addTileFile(const std::string& strFileName, PYXPointer<PYXTile> spTile,
											   ProcessDataChangedEvent::ChangeTrigger trigger) = 0;

	/*!
	Force the cache to load a tile from it's local data source only if the local data source is active.
	If a new tile is loaded, then the complete cache is flushed to disk so that the disk version of the
	cache is completely up-to-date.
	Returns true if a new tile is loaded.
	*/
	virtual bool STDMETHODCALLTYPE forceCoverageTile(PYXTile tile) = 0;

	//! an event which gets fired if the cache can not find a tile it needs because it has no input.
	virtual Notifier& STDMETHODCALLTYPE getNeedATileNotifier() = 0;

	//! an event which gets fired if the cache definition changes.
	virtual Notifier& STDMETHODCALLTYPE getCacheChangedNotifier() = 0;
};

//! Management class to hold notifiers for cache objects.
class PYXLIB_DECL CacheManager
{
public:
	//! notifier the fires when a new cache is constructed.
	static Notifier& getCacheCreatedNotifier()
	{
		return m_cacheCreatedNotifier;
	}

private:
	//! The new cache notifier
	static Notifier m_cacheCreatedNotifier;

};

//! Base class for all ICache events.
class PYXLIB_DECL CacheEvent : public NotifierEvent
{
public:

	//! Returns the cache that triggered the event.
	boost::intrusive_ptr<ICache> getCache()
	{
		return m_spCache;
	}

protected:

	CacheEvent(boost::intrusive_ptr<ICache> spCache) :
		m_spCache(spCache)
	{
	}

private:

	//! The process that triggered the event.
	boost::intrusive_ptr<ICache> m_spCache;
};

//! Event class for the Need A Tile Event.
class PYXLIB_DECL CacheNeedsTileEvent : public CacheEvent
{
public:
	//! Creator
	static PYXPointer<CacheNeedsTileEvent> create(
		boost::intrusive_ptr<ICache> spCache, PYXPointer<PYXTile> spTile)
	{
		return PYXNEW(CacheNeedsTileEvent, spCache, spTile);
	}

	//! Returns the cache that triggered the event.
	const PYXPointer<PYXTile> getTile()
	{
		return m_spTile;
	}

	//! Returns whether the download of the tile failed.
	const bool getDownloadFailed()
	{
		return m_bDownloadFailed;
	}

	//! Sets the flag ot indicate whether the download failed.
	void setDownloadFailed(const bool bDownloadFailed)
	{
		m_bDownloadFailed = bDownloadFailed;
	}

protected:

	CacheNeedsTileEvent(boost::intrusive_ptr<ICache> spCache, PYXPointer<PYXTile> spTile) :
		CacheEvent(spCache), m_spTile(spTile)
	{
		m_bDownloadFailed = false;
	}

private:

	//! The tile that is being requested.
	const PYXPointer<PYXTile> m_spTile;

	//! Whether the download of the tile failed.
	bool m_bDownloadFailed;
};

//! Event class for the new cache created event.
class PYXLIB_DECL CacheWithProcessEvent : public CacheEvent
{
public:
	//! Creator
	static PYXPointer<CacheWithProcessEvent> create(
		boost::intrusive_ptr<ICache> spCache, boost::intrusive_ptr<IProcess> spProcess)
	{
		return PYXNEW(CacheWithProcessEvent, spCache, spProcess);
	}

	//! Returns the cache that triggered the event.
	const boost::intrusive_ptr<IProcess> getProcess()
	{
		return m_spProcess;
	}

	//! Returns the handled state of the event (true if the event has been handled).
	const bool getHandled()
	{
		return m_handled;
	}

	//! Sets the handled state of the event.
	void setHandled (bool handled)
	{
		m_handled = handled;
	}

protected:

	CacheWithProcessEvent(boost::intrusive_ptr<ICache> spCache, boost::intrusive_ptr<IProcess> spProcess) :
		CacheEvent(spCache), m_spProcess(spProcess), m_handled(false)
	{
	}

private:

	//! The cache as a process.
	const boost::intrusive_ptr<IProcess> m_spProcess;

	//! Handlers will set this to true if they have handled this notification.
	bool m_handled;
};

#endif //end guard