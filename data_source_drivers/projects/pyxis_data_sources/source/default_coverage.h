#ifndef DEFAULT_COVERAGE_H
#define DEFAULT_COVERAGE_H
/******************************************************************************
pyx_default_coverage.h

begin		: 2006-05-30
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_pyxis_coverages.h"

// pyxlib include
#include "pyxis/data/value_tile.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/tile_cache.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/data/record.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/data/feature.h"

// boost includes

// standard includes
#include <string>

/*!
PYXDefaultCoverage is a PYXIS-specific data format for coverage data sets.  Data
are organized as a set of tiles, implemented using PYXValueTile.  On disk, each
tile is serialized into a separate binary file, and the surrounding folder (which
also contains three XML files for the serialized metadata) represents the data
source as a whole.

NOTE 1: The binary data file format (see PYXValueTile::serialize()) is architecture
dependent.  This is a deliberate compromise in the name of speed.

NOTE 2: This code distinguishes both writability (m_bWritable) and persistence
(m_bPersistent).  Only a writable PYXDefaultCoverage can have its values altered
via setCoverageValue() or setCoverageTile().  Any writable PYXDefaultCoverage may
have to write out ("persist") some of its tiles to disk from time to time, due to
memory limitations; only a persistent PYXDefaultCoverage will always persist all
of its tiles on close.  A PYXDefaultCoverage is persistent by default; use
setPersistence() to change this.
*/
//! Binary PYXIS-format coverage data source.
class MODULE_PYXIS_COVERAGES_DECL PYXDefaultCoverage : public PYXObject, public Observer
{
public:

	//! Unit test method
	static void test();

	//! Constants
	static const std::string kstrScope;
	static const std::string kstrFileExtension;

	IRECORD_IMPL();

	//! Dynamic Creator.
	static PYXPointer<PYXDefaultCoverage> create()
	{
		return PYXNEW(PYXDefaultCoverage);
	}

	//! Constructor
	// TODO: should this be private in the new PYXPointer scheme???
	PYXDefaultCoverage();

private:
	//! Helper function to initialze members after an open.
	void PYXDefaultCoverage::initTypeArrays();

public:
	//! Open an existing data set for reading.
	bool openReadOnly(const std::string& strDir);

	//! Open an existing data set for read/write.
	bool openReadWrite(const std::string& strDir);

	//! Create a new data set for writing, or open it if it already exists
	bool openReadWrite(
		const std::string& strDir,
		const PYXTableDefinition& defn,
		const std::vector<PYXValue>& vecValues,
		const PYXTableDefinition& coverageDefn,
		int nCellResolution,
		int nTileResolution = 0	);

	//! Close an open data set
	void close();

	//! Destructor
	virtual ~PYXDefaultCoverage();
	
	virtual void createGeometry() const
	{
		// start with an empty tile collection
		m_spGeometry = PYXTileCollection::create();
	}
	
	//! Return the class name of this observer class.
	virtual std::string getObserverDescription() const
		{return kstrScope + " " + getName();}

	//! Is this data source writable?
	virtual bool isWritable() const {return m_bWritable;}

	//! Set data source persistent: default is persistent
	void setPersistence(bool bPersistent) {m_bPersistent = bPersistent;}

	//! Get the geometry of the data source (must be non-null).
	virtual PYXPointer<const PYXGeometry> getGeometry() const {return m_spGeometry;}

	//! Get the geometry of the data source (must be non-null).
	virtual PYXPointer<PYXGeometry> getGeometry()  {return m_spGeometry;}

	//! Get the geometry of the data source (must be non-null).
	void setGeometry(PYXPointer<PYXGeometry> spGeometry);

	//! Get the name of the cache.
	const std::string getName() const
	{
		return m_strName;
	}

	//! Return the storage file for the given tile.
	const boost::filesystem::path toFileName(const PYXTile &tile) const;

	//! Set whether the cache is to be an in memory cache or a persitant to disk cache.
	void setCachePersistence(bool persistent)
	{
		m_bPersistent = persistent;
	}

	//! Set the coverage definition of the coverage that is being cached.
	void setCoverageDefinition(PYXPointer<PYXTableDefinition> spDef) const
	{
		m_spCovDefinition = spDef;
	}

	//! Get the coverage definition.
	PYXPointer<const PYXTableDefinition> getCoverageDefinition() const
	{
		return m_spCovDefinition;
	}

	//! Get the coverage definition.
	PYXPointer<PYXTableDefinition> getCoverageDefinition()
	{
		return m_spCovDefinition;
	}

	//! Receive notification of e.g., tile deleted from cache
	void updateObserverImpl(PYXPointer<NotifierEvent> spEvent);

	//! Get the coverage value at the specified PYXIS index, with empty/null distinction.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex,
		bool *pbInitialized	) const;

	//! Get coverage values for an entire tile.
	virtual PYXPointer<PYXValueTile>
		getCoverageTile(const PYXTile& tile) const;

	//! Look up a tile in the memory cache - without trying to retrive it
	PYXPointer<PYXValueTile> getTileFromMemory (const PYXTile& tile) const;

	//! Set the coverage value at the specified PYXIS index.
	virtual void setCoverageValue(
		const PYXValue& value,	
		const PYXIcosIndex& index,
		int nFieldIndex = 0	);

	//! Set coverage values for an entire tile.
	virtual void setCoverageTile(PYXPointer<PYXValueTile> spValueTile);

	//! Set tile cache size limit.
	void setCacheMaxTileCount(const int nTileCount) {m_tileCache.setMaxTileCount(nTileCount);}

	//! Get tile depth.
	const int getDefaultTileDepth() {return m_defaultTileDepth;}

	/*! Set default tile depth.
	    A value of -1 passed in will use a suitable default tile depth.
	*/
	void setDefaultTileDepth(int value) 
	{
		if (value <= 0)
		{
			value = PYXTile::knDefaultTileDepth;
		}
		m_defaultTileDepth = value;
	}

	// TODO: evaluate the need for the cell resolution to be stored here, when it is part of the geometry.
	// we used to need to do this kid of thing because the cell resolution was not stored on disk, because the
	// geometry was not stored on disk.
	//! Get cell resolution.
	const int getCellResolution() {return m_nCellResolution;}

	// TODO:  deal with open coverages here.
	//! Set cell resolution.
	void setCellResolution(int value) {m_nCellResolution = value;}

	//! Force all tiles to be persisted immediately (mostly for testing)
	void persistAllTiles();

	/*! 
	Insert a tile file into the cache.  This should be a properly named
	file, in the naming convention of the DefaultCoverage, and it should already
	be in the correct directory.
	*/
	void addTileFile(const std::string& strFileName);

	//! return a tile that would contain a cell at the given index.
	PYXTile PYXDefaultCoverage::getDefaultTile(const PYXIcosIndex& index) const;

	//! Purges any data stored for the given area.  (Null defaults to everything.)
	void reset( PYXPointer<PYXGeometry> spInvalidatedArea);

protected:

	//! Set the name of the cache.
	void setName(std::string strName)
	{
		m_strName = strName;
	}

	//! Load the data source and coverage definitions from files
	bool loadDefinitions(const std::string& strDir);

	//! Save the data source and coverage definitions to files
	void saveDefinitions(const std::string& strDir);

	//! Load the geometry from saved file.
	void loadGeometry(const std::string& strDir);

	//! Save the geometry to disk.
	void saveGeometry(const std::string& strDir);

	//! Look up a tile in the cache
	PYXPointer<PYXValueTile> getTile (const PYXTile& tile) const;

	//! Called to persist one tile to disk
	void persistTile(PYXValueTile *pTile);

	//! Recover a persisted/deleted tile back from disk into cache
	PYXPointer<PYXValueTile> recoverTile(const PYXTile& tile) const;

	//! Calculate the tile resolution for a given cell index accounting for the minimum resolution.
	int tileResolution (const PYXIcosIndex& index, int depth) const;

	//! Locate a tile that contains the cell for the given index.
    PYXPointer<PYXValueTile> findTileForCell(const PYXIcosIndex& index) const;

private:

	//! The name of the cache. 
	std::string m_strName;

	//! Disable copy constructor
	PYXDefaultCoverage(const PYXDefaultCoverage&);

	//! Coverage meta data of the coverage that is cached.
	mutable PYXPointer<PYXTableDefinition> m_spCovDefinition;

	//! Disable copy assignment
	void operator =(const PYXDefaultCoverage&);

	//! The default depth that tiles will have if we are creating a new tile.
	int m_defaultTileDepth;

	//! Native resolution of data
	int m_nCellResolution;

	//! Types for our data channels
	std::vector<PYXValue::eType> m_vecTypes;

	//! Array counts for our data channels
	std::vector<int> m_vecCounts;

	// Store the geometry for this data set.
	mutable PYXPointer<PYXGeometry> m_spGeometry;

	//! Cache of tile data
	mutable PYXTileCache<PYXValueTile> m_tileCache;

	//! The last tile that was fetched from the cache.
	mutable PYXPointer<PYXValueTile> m_spTileHint;

	//! mutex to guard the TileHint
	mutable boost::recursive_mutex m_tileHintMutex;

	//! True if read/write, false if read-only
	bool m_bWritable;

	//! True if we intend to keep the data, false if it's for temporary use
	bool m_bPersistent;	

	//! The styles associated with the feature.
	std::vector<FeatureStyle> m_vecStyles;
};

#endif	// end guard
