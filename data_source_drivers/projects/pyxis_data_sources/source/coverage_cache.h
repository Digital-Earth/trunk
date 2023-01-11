#ifndef coverage_cache_H
#define coverage_cache_H
/******************************************************************************
coverage_cache.h

begin		: 2006-05-31
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "default_coverage.h"
#include "module_pyxis_coverages.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/cache.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/cache_map.h"
#include "pyxis/utility/file_utils.h"


/*!
A Process which caches requested data points using a single PYXDefaultCoverage.

By default, the PYXDefaultCoverage objects are non-persistent.  Call
setCachePersistence(true) to force persistent caching, e.g. when your objective
is to create complete, valid PYXIS data sets on disk.

Usage order for optional functions: setCacheDir(), setCacheCellResolution(),
setCacheTileDepth() and setCachePersistence() should be called before setInput().
*/
//! PYXCoverageCache: cache using a single PYXDefaultCoverage.
class MODULE_PYXIS_COVERAGES_DECL PYXCoverageCache : public ProcessImpl<PYXCoverageCache>,
	public ICoverage, public ICache
{
	PYXCOM_DECLARE_CLASS();

public: //PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
		IUNKNOWN_QI_CASE(ICache)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(PYXCoverageCache, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the interface that is output.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	//! Get the interface that is output.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual IProcess::eInitStatus STDMETHODCALLTYPE initProc(bool bRecursive = false);

	//! Handler for any input changing its data.  
	virtual void handleInputDataChanged( PYXPointer<NotifierEvent> eventData);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL_WITHOUT_HINTS();

	virtual void STDMETHODCALLTYPE geometryHint(PYXPointer<PYXGeometry> spGeom);

	virtual void STDMETHODCALLTYPE endGeometryHint(PYXPointer<PYXGeometry> spGeom);

public: // ICoverage

	//! Get the coverage definition of our input parameter.
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		if (m_spCache == 0)
		{
			PYXTHROW (PYXException, "Process can not be used until InitProc() has been called (getCoverageDefinition).");
		}
		return m_spCache->getCoverageDefinition();
	}				  

	//! Get the coverage definition of our input parameter.
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		if (m_spCache == 0)
		{
			PYXTHROW (PYXException, "Process can not be used until InitProc() has been called (getCoverageDefinition).");
		}
		return m_spCache->getCoverageDefinition();
	}

	//! Get (and cache) coverage value at specified PYXIS index.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
		int nFieldIndex = 0) const;

	//! Get coverage values for an entire tile.
	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
		int nRes,
		int nFieldIndex = 0) const;

	//! Get estimated cost for generating a tile for a single field
	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(const PYXIcosIndex& index,
		int nRes,
		int nFieldIndex = 0	) const;

	//! Get coverage values for an entire tile.
	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE
		getCoverageTile(const PYXTile& tile) const;

	//! Get estimated cost for generating an entire tile 
	virtual PYXCost STDMETHODCALLTYPE getTileCost(const PYXTile& tile) const;

	//! Proactively put a value into the cache (input untouched).
	virtual void STDMETHODCALLTYPE setCoverageValue(const PYXValue& nValue,
		const PYXIcosIndex& index,
		int nFieldIndex=0);

	//! Set coverage values for an entire tile.
	virtual void STDMETHODCALLTYPE setCoverageTile(PYXPointer<PYXValueTile> spValueTile);

public: //ICache

	//! Set persistence: default is true.
	virtual void STDMETHODCALLTYPE setCachePersistence(bool bPersistent) {m_bPersistent = bPersistent;}

	//! Get tile depth.
	virtual const int STDMETHODCALLTYPE getCacheTileDepth() {return m_nTileDepth;}

	virtual const void STDMETHODCALLTYPE initCacheDir() const;

	virtual const std::string STDMETHODCALLTYPE getCacheDir() const {return m_strCacheDir;}

	virtual void STDMETHODCALLTYPE setCacheDir(const std::string& strDir) 
	{
		// this operation is only meaningful if the contained DefaultCoverage
		// has not been opened.
		if (m_spCache != 0 && m_spCache->getName().length() > 0)
		{
			PYXTHROW (PYXException, "Changing the directory of an open cache is not allowed.");
		}

		m_strCacheDir = strDir;
	}

	//! Set tile depth for the PYXDefaultCoverage (optional).
	virtual void STDMETHODCALLTYPE setCacheTileDepth(const int nDepth) {m_nTileDepth = nDepth;}

	//! Open an existing cache for reading and writing.
	virtual bool STDMETHODCALLTYPE openReadWrite(const std::string& strDir)
	{
		if (m_spCache == 0)
		{
			PYXTHROW (PYXException, "Can not open a cache that has not been initialized.");
		}
		return getCache()->openReadWrite(strDir);
	}

	//! Sets the maximum tiles the cache can hold before starts kicking them out.
	void STDMETHODCALLTYPE setCacheMaxTileCount(const int nMaxTileCount)
	{
		getCache()->setCacheMaxTileCount(nMaxTileCount);
	}

	//! Create and open a cache for reading and writing.
	virtual bool STDMETHODCALLTYPE openReadWrite(const std::string& strDir,
		const PYXTableDefinition& defn,
		const std::vector<PYXValue>& vecValues,
		const PYXTableDefinition& coverageDefn,
		int nCellResolution,
		int nTileResolution = 0)
	{
		return getCache()->openReadWrite(strDir, defn, vecValues, coverageDefn, nCellResolution, nTileResolution);
	}

	//! Open a cache to be read from only.
	virtual bool STDMETHODCALLTYPE openReadOnly(const std::string& strDir)
	{
		if (m_spCache == 0)
		{
			PYXTHROW (PYXException, "Can not open a cache that has not been initialized.");
		}
		return getCache()->openReadOnly(strDir);
	}

	//! Get the tile depth of the cache.
	virtual const int STDMETHODCALLTYPE getCacheTileDepth() const { return m_nTileDepth;}

	//! Close the cache.
	virtual void STDMETHODCALLTYPE close() 
	{
		// Only close if we have one created.
		if (m_spCache != 0)
		{
			m_spCache->close();
		}
	}

	//! Set the cell resolution to cache at.
	virtual void STDMETHODCALLTYPE setCacheCellResolution(const int nCellResolution)
	{
		assert(nCellResolution > 0 && "Cell resolution cannot be less then zero.");
		m_nCellResolution = nCellResolution;
	}

	//! Get the cell resolution of the cache.
	virtual const int STDMETHODCALLTYPE getCacheCellResolution()
	{	
		return m_nCellResolution;
	}

	//! Force all tiles to be persisted immediately (mostly for testing)
	virtual void STDMETHODCALLTYPE persistAllTiles()
	{
		getCache()->persistAllTiles();
	}

	//! Return the storage file for the given tile.
	virtual const std::string STDMETHODCALLTYPE toFileName(const PYXTile &tile) const
	{
		if (m_spCache == 0)
		{
			PYXTHROW (PYXException, "Can not convert to filename with a cache that is not open.");
		}

		return FileUtils::pathToString(m_spCache->toFileName(tile));
	}

	/*!
	When in greedy mode the cache will always fetch and cache a tile sized piece of 
	data when a single value is requested.
	*/
	virtual void STDMETHODCALLTYPE setGreedyCache(bool bGreedy)
	{
		m_bIsGreedy = bGreedy;
	}

	/*! 
	Insert a tile file into the cache.  This should be a properly named
	file, in the naming convention of the DefaultCoverage, and it should already
	be in the correct directory.
	*/
	virtual void STDMETHODCALLTYPE addTileFile(const std::string& strFileName, PYXPointer<PYXTile> spTile,
		ProcessDataChangedEvent::ChangeTrigger trigger)
	{
		if (m_spCache == 0)
		{
			PYXTHROW (PYXException, "Can not add tiles to a cache that is not open.");
		}
		m_spCache->addTileFile(strFileName);
		this->onDataChanged(spTile,trigger);
	}

	//! Force a tile of data to be retrieved and stored on disk in the cache directory.
	virtual bool STDMETHODCALLTYPE forceCoverageTile(PYXTile tile);

	//! Event which gets fired when a Tile of data is needed.
	virtual Notifier& STDMETHODCALLTYPE getNeedATileNotifier()
	{
		return m_needATileNotifier;
	}

	//! Event which gets fired when a cache changes in a way that woud effect
	//! the PyxNet data stream.
	virtual Notifier& STDMETHODCALLTYPE getCacheChangedNotifier()
	{
		return m_cacheChangedNotifier;
	}

public: 

	//! Constants
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXCoverageCache();

	//! Destructor.
	~PYXCoverageCache() {;}

private:
	//! Set the input coverage (ownership shared with caller).
	virtual void setInput(boost::intrusive_ptr<ICoverage> spInput);

	/*!
	Sets the coverage definition, to be the definition that is passed in.

	\param spDef The coverage definition of the cache.
	*/
	void setCoverageDefinition(PYXPointer<PYXTableDefinition> spDef)
	{
		if (!spDef)
		{
			PYXTHROW(PYXException, "Cannot set the coverage definition to be null.");
		}
		m_spCache->setCoverageDefinition(spDef);
	}

protected:

	//! Disable copy constructor
	PYXCoverageCache(const PYXCoverageCache&);

	//! Disable copy assignment
	void operator =(const PYXCoverageCache&);

private:

	//! Restores the value tile from the blob provider
	PYXPointer <PYXValueTile> streamFromBlob ( const PYXTile & tile) const;

	//! Stores the value tile on the blob provider
	bool streamToBlob ( PYXPointer <PYXValueTile> tile) const;

	//! Gets the input as an ICoverage.
	boost::intrusive_ptr<const ICoverage> getInput() const;

	//! Gets the cache. If one doesn't exists it is created.
	PYXPointer<PYXDefaultCoverage> getCache() const
	{
		if (m_spCache)
		{
			return m_spCache;
		}

		return createCache();
	}

	//! Check to see if our input process is initialized and ready to go.
	bool cacheInputIsOK() const;

	//! Check to see if we have a local data supply.
	bool cacheHasDataSupply() const;

	//! store the cacheHasDataSupply result
	bool m_cacheHasDataSupplyResult;	

	//! Pointer to the underlying cache object as a Cache.
	mutable PYXPointer<PYXDefaultCoverage> m_spCache;

	//! Pointer to the input coverage (that is to be cached).
	boost::intrusive_ptr<ICoverage> m_spCov;

	//! Create the actual cache data source.
	PYXPointer<PYXDefaultCoverage> createCache() const;

	//! Desired cache directory
	mutable std::string m_strCacheDir;

	//! True if the cache is to be persistent; false if it's temporary.
	bool m_bPersistent;

	//! Flag to determine if full tiles should always be requested
	bool m_bIsGreedy;

	//! Desired cache tile depth
	mutable int m_nTileDepth;

	//! Cache cell resolution.
	mutable int m_nCellResolution;

	//! The need a tile notifier.
	mutable Notifier m_needATileNotifier;

	//! The cache was changed notifier.
	mutable Notifier m_cacheChangedNotifier;

	mutable CacheMap<PYXTile,bool> m_tileHasValuesCache;

	//! mutex to guard the getCoverageValue and getCoverageTile
	mutable boost::recursive_mutex m_getCoverageMutex;
};

#endif	// Endif
