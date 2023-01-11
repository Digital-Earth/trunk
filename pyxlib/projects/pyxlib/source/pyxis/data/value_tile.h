#ifndef PYXIS__DATA__VALUE_TILE_H
#define PYXIS__DATA__VALUE_TILE_H
/******************************************************************************
value_tile.h

begin		: 2006-05-10
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/utility/cache_status.h"
#include "pyxis/utility/value.h"

// boost includes
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

// forward declarations
class CacheStatus;
class MemoryToken;
class PYXValueTable;
class PYXTableDefinition;

//! Returned by the calcStatistics method
class PYXValueTileStatistics
{
public:

	//! Number of cells in the tile
	int nCells;

	//! Number of non-null values in the tile
	int nValues;

	//! Minimum value in the tile
	PYXValue minValue;

	//! Maximum value in the tile
	PYXValue maxValue;

	//! Average value of non-null numeric values in the tile
	PYXValue avgValue;
};

/*!
PYXValueTile is a container for all the data values for one PYXIS tile.  It wraps
a PYXValueTable which has one row per cell, ordered in the usual PYXIS exhaustive
iteration sequence.  This fact enables random access to cells by integer offset
as well as by PYXIS indices.
*/
//! PYXValueTile is a container for all the data values for one PYXIS tile.
class PYXLIB_DECL PYXValueTile : public PYXObject
{
	friend PYXLIB_DECL std::ostream& operator <<(	std::ostream& out,
													PYXValueTile& pvt	);
	friend PYXLIB_DECL std::istream& operator >>(	std::istream& in,
													PYXValueTile& pvt	);

public:

	//! Unit test method
	static void test();

	//! Create a value tile by reading it from a stream.
	static PYXPointer<PYXValueTile> create(std::istream& in)
	{
		PYXPointer<PYXValueTile> newTile = PYXNEW(PYXValueTile);
		newTile->deserialize(in);
		return newTile;
	}
	
	//! Creator
	static PYXPointer<PYXValueTile> create(
		const PYXIcosIndex& index,
		int nRes,
		PYXPointer<const PYXTableDefinition> spDefn)
	{
		return PYXNEW(PYXValueTile, index, nRes, spDefn);
	}

	static PYXPointer<PYXValueTile> create(
		const PYXValueTile & source)
	{
		return PYXNEW(PYXValueTile, source);
	}

	PYXValueTile(const PYXIcosIndex& index, int nRes, PYXPointer<const PYXTableDefinition> spDefn);		

	//! Creator
	static PYXPointer<PYXValueTile> create(
		const PYXTile& tile, 
		PYXPointer<const PYXTableDefinition> spDefn)
	{
		return PYXNEW(PYXValueTile, tile, spDefn);
	}

	PYXValueTile(const PYXTile& tile, PYXPointer<const PYXTableDefinition> spDefn);		

	//! Creator
	static PYXPointer<PYXValueTile> create(
		const PYXTile& tile,
		const std::vector<PYXValue::eType>& vecTypes)
	{
		return PYXNEW(PYXValueTile, tile, vecTypes);
	}

	//! Simple constructor: use when all data channels are non-array type
	PYXValueTile(	const PYXTile& tile,
					const std::vector<PYXValue::eType>& vecTypes	);

	//! Creator
	static PYXPointer<PYXValueTile> create(
		const PYXTile& tile,
		const std::vector<PYXValue::eType>& vecTypes,
		const std::vector<int>& vecCounts)
	{
		return PYXNEW(PYXValueTile, tile, vecTypes, vecCounts);
	}

	//! General constructor: use when one or more data channels have array type
	PYXValueTile(	const PYXTile& tile,
					const std::vector<PYXValue::eType>& vecTypes,
					const std::vector<int>& vecCounts	);

	//! Copy constructor
	PYXValueTile(const PYXValueTile&);

	//! Copy constructor that only copies one of the contained field tiles.
	PYXValueTile::PYXValueTile(const PYXValueTile& orig, int nFieldIndex);

	//! Destructor
	virtual ~PYXValueTile();

	//! Verify that the coverage definition matches a value tile.
	bool isValueTileCompatible(PYXPointer<const PYXTableDefinition> spDefn) const;

	//! get a PYXValue that is uninitialized, but of the correct type to hold this field's data.
	inline PYXValue getTypeCompatibleValue(const int nChannelIndex) const
		{return PYXValue::create(getDataChannelType(nChannelIndex), 0, getDataChannelCount(nChannelIndex), 0);}

	//! Serialize to stream
	void serialize(std::ostream& out);

	//! De-serialize from stream
	void deserialize(std::istream& in);

	//! Get total memory allocated
	inline int getHeapBytes() const;

	//! Return the timer object for the tile
	CacheStatus* getCacheStatus() {return &m_cacheStatus;}

	//! Get number of data channels (columns)
	inline int getNumberOfDataChannels() const;

	//! Get number of cells (rows)
	inline int getNumberOfCells() const;

	//! Get the type for a specific data channel
	inline const PYXValue::eType getDataChannelType(const int nChannelIndex) const;

	//! Get the array count for a data channel
	inline const int getDataChannelCount(const int nChannelIndex) const;

	//! Get value for given cell index and channel
	PYXValue getValue(	const PYXIcosIndex& cellIndex,
						const int nChannelIndex,
						bool* pbInitialized=0	) const;

	//! Get value for given cell index and channel, returns false for null value
	bool getValue(	const PYXIcosIndex& cellIndex,
					const int nChannelIndex,
					PYXValue* pValue) const;

	//! Get value for given integer cell offset and channel
	PYXValue getValue(	const int nOffset,
						const int nChannelIndex,
						bool* pbInitialized=0	) const;

	//! Get value for given integer cell offset and channel, returns false for null value
	bool getValue(	const int nOffset,
					const int nChannelIndex,
					PYXValue* pValue) const;

	//! Set value for given cell index and channel
	void setValue(	const PYXIcosIndex& cellIndex,
					const int nChannelIndex,
					const PYXValue& value	);

	//! Set value for given integer cell offset and channel
	void setValue(	const int nOffset,
					const int nChannelIndex,
					const PYXValue& value	);

	//! Get the dirty flag
	bool isDirty() {return m_bDirty;}

	//! Get the complete flag
	bool isComplete() { return m_bComplete; }

	//! Set the complete flag.
	void setIsComplete(bool isComplete) { m_bComplete = isComplete; }

	//! Get the tile geometry.
	const PYXTile& getTile() {return m_tile;}

	//! Set the tile geometry.
	/*! Set the tile geometry to a new tile.
	WARNING: This routine needs to be used with caution.  The contents
	of the tile are not reinitialized and thus will be applied to the 
	new exhaustive iteration order of the new tile.  This routine is currently
	used in achieving high performance from the constant coverage where all values
	being returned are the same.  One should check that the tile has enough cells
	allocated to represent the new tile before calling this function.
	*/
	void setTile(const PYXTile& tile) 
	{
		assert(tile.getCellCount() <= getNumberOfCells());
		m_tile = tile;
	}

	//! Get an iterator to the individual cells in the geometry
	PYXPointer<PYXIterator> getIterator() const
	{
		return m_tile.getIterator();
	}

	//! Return a copy of the field tile that is at the given index.
	PYXPointer<PYXValueTile> cloneFieldTile(int nFieldIndex) const;

	//! Return a copy of the field tile that is at the given index.
	PYXPointer<PYXValueTile> clone() const;

	//! Returns a new value tile from the desired field that is one resolution higher than this tile.
	/*! Creates a new tile from the desired channel that has the same root index 
	as this tile, but has a depth that is one greater.  The data of the returned 
	tile is interpreted from the data in this tile.  Cells with three parent cells 
	are returned as the average value of the three parents. A cell with one parent 
	(a centroid) is calculated as a weighted average of it's parent * 1 and it's 
	neighbors at 1/6th (or 1/5 in the case of a pentagon).  And then normalized by 
	dividing by two.
	*/
	PYXPointer<PYXValueTile> zoomIn(int nFieldIndex);

	//! Calculate some statistics on the tile.
	PYXValueTileStatistics calcStatistics() const;

	//! Static helper method to see if a serialized tile file is complete by reading the header.
	static bool isTileFileComplete (const std::string& tileFilename);

	//! Static helper method to load a pyxtile from a file
	static PYXPointer<PYXValueTile> createFromFile(const std::string& tileFilename);

	static PYXPointer<PYXValueTile> createFromBase64String(const std::string& base64);

	std::string toBase64String();

protected:
	//! Default constructor creates an empty value tile
	PYXValueTile();

	//! Disable copy assignment
	void operator =(const PYXValueTile&);

private:
	//! Generate a file name to read the zoom in look up table from.
	boost::filesystem::path PYXValueTile::getZoomInFilename();

	//! Generate the lookup table to get from this tile to a tile one res higher.
	void generateZoomInLUT(PYXTile targetTile, int* pOffsets);

	//! Table of values, one PYXIS index per row
	std::auto_ptr<PYXValueTable> m_spValueTable;

	//! Number of table rows, for quick comparisons
	int m_nTableRows;

	//! This tile's geometry
	PYXTile m_tile;

	//! Timer object for the tile
	CacheStatus m_cacheStatus;

	//! Initially false, set by setValue(), cleared by serialize()
	bool m_bDirty;

	//! Keep track of if this tile is complete or not.
	bool m_bComplete;
};

//! Stream output operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, PYXValueTile& pvt);

//! Stream input operator.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXValueTile& pvt);

#endif // guard
