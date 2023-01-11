#ifndef PYXIS__GEOMETRY__TILE_COLLECTION_H
#define PYXIS__GEOMETRY__TILE_COLLECTION_H
/******************************************************************************
tile_collection.h

begin		: 2004-11-16
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/tile_set.h"
#include "pyxis/geometry/geometry_collection.h"
#include "pyxis/geometry/geometry_iterator.h"
#include "pyxis/geometry/tile.h"

// standard includes
#include <vector>

// forward declarations
class PYXTileCollectionIterator;

/*!
PYXTileCollection provides a container as well as some useful query functions 
on a group of PYXTile objects that have the same cell resolution. 
*/
//! A collection of tiles with the same cell resolution.
class PYXLIB_DECL PYXTileCollection : public PYXGeometryCollection
{
public:

	//! An iterator for iterating through the root indices.
	class Iterator : public PYXAbstractIterator {

		//! The tile set being iterated.
		const PYXTileSet& m_tileSet;

		//! The current root index within the tile collection.
		std::auto_ptr<PYXTileSet::Iterator> m_apIterator;

	private:

		//! Disabled.
		Iterator(const Iterator&);

		//! Disabled.
		void operator =(const Iterator&);

	public:

		/*!
		Construct the iterator.

		\param	tileCollection	The tile collection to iterate over. Altering
								this collection during iteration will cause
								undefined behavior in the iterator.
		*/
		explicit Iterator(const PYXTileCollection& tileCollection);

		//! Move to the next item.
		void next();

		/*!
		See if we have covered all the items.

		\return	true if all items have been covered, otherwise false.
		*/
		bool end() const;

		/*!
		Start the iteration at the beginning of the collection.
		*/
		void reset();

		//! Return the current item.  If past end, return the last item.
		const PYXIcosIndex& operator *() const;
	};

	//! Test method
	static void test();

	//! Creator
	static PYXPointer<PYXTileCollection> create(bool bAutoAggregate = true)
	{
		return PYXNEW(PYXTileCollection, bAutoAggregate);
	}

	//! Copy creator
	static PYXPointer<PYXTileCollection> create(const PYXTileCollection& rhs)
	{
		return PYXNEW(PYXTileCollection, rhs);
	}

	//! Deserialization creator
	static PYXPointer<PYXTileCollection> create(std::basic_istream< char>& in)
	{
		return PYXNEW(PYXTileCollection, in);
	}

	//! copy from a geometry
	static PYXPointer<PYXTileCollection> create(const PYXGeometry & geometry)
	{
		PYXPointer<PYXTileCollection> result = PYXTileCollection::create();
		geometry.copyTo(result.get());
		return result;
	}

	//! Constructor
	explicit PYXTileCollection(bool bAutoAggregate = true);

	// Copy constructor
	PYXTileCollection(const PYXTileCollection& source);

	// Copy constructor
	PYXTileCollection(const PYXTileCollection& source, bool bAutoAggregate);

	//! Deserialization constructor
	explicit PYXTileCollection(std::basic_istream< char>& in);

	//! Destructor
	virtual ~PYXTileCollection();

	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;

	//! Copy assignment.
	PYXTileCollection& operator =(const PYXTileCollection& tileCollection);

	//! Serialize.
	virtual void serialize(std::basic_ostream< char>& out) const;

	//! Deserialize.
	virtual void deserialize(std::basic_istream< char>& in);

	/*!
	Is the geometry empty.

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	//! Determine if a collection has any tiles
	virtual bool isEmpty() const;

	//! Clear all entries in the collection.
	virtual void clear();

	//! Get the PYXIS resolution of cells in the geometry.
	virtual int getCellResolution() const;

	/*!
	Set the PYXIS resolution of cells in the geometry. This operation does not 
	retain area. It is a simple truncation or append operation.

	\param	nCellResolution	The cell resolution.
	*/
	//! Set a new resolution for the tile collection.
	virtual void setCellResolution(int nCellResolution);

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTileCollection& collection) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXTile& tile) const;

	//! Get the intersection of this geometry and the specified geometry.
	PYXPointer<PYXGeometry> intersection(const PYXCell& cell) const;

	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXTileCollection& collection) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXTile& tile) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXCell& cell) const;

	//! Return a boolean indication of intersection.
	bool intersects(const PYXIcosIndex& cell) const;


	//! Return whether this geometry completely contains the specified geometry.
	virtual bool contains(const PYXGeometry& geometry) const;

	//! Return whether this geometry completely contains the specified geometry.
	bool contains(const PYXTileCollection& collection) const;

	/*!
	Get the disjunction (union) of this geometry and the specified geometry as a new
	geometry.

	\param	geometry		The geometry to unite with this one.

	\return	The disjunction (union) of geometries.
	*/
	virtual PYXPointer<PYXGeometry> disjunction(const PYXGeometry& geometry) const;

	/*!
	Get the disjunction (union) of this tile collection and the specified tile collection as a new
	geometry.

	\param	tileCollection	The tile collection to unite with this geometry.

	\return	The disjunction (union) of geometries.
	*/
	virtual PYXPointer<PYXGeometry> disjunction(const PYXTileCollection& tileCollection) const;

	/*!
	Get the number of geometries in this collection.

	\return	The number of geometries in this geometry.
	*/
	//! Return the number of geometries in the collection.
	virtual int getGeometryCount() const;
	
	//! Get an iterator to the geometries in this collection.
	virtual PYXPointer<PYXGeometryIterator> getGeometryIterator() const;

	//! Get an iterator to the tiles in this collection.
	virtual PYXPointer<PYXTileCollectionIterator> getTileIterator() const;

	virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const;

	/*!
	\param	pVecIndex	The container to hold the returned indices.
	*/
	//! Calculate a series of PYXIS indices around a geometry.
	virtual void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const;

	//! Copies a representation of this geometry into a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copies a representation of this geometry into a tile collection at the specified resolution.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;

	virtual PYXBoundingCircle PYXTileCollection::getBoundingCircle() const;

public:

	//! Add a tile to the collection.
	void addTile(PYXPointer<PYXTile> spTile);

	//! Add a tile to the collection.
	void addTile(const PYXIcosIndex& index, int nCellResolution);

	//! Add a geometry to the collection.
	void addTile(const PYXGeometry& geometry);
	
	//! Add the contents of a tile collection to this one.
	void addGeometry(const PYXTileCollection& tc);

	//! Returns whether tiles are automatically aggregated.
	bool getAutoAggregate() const;

	//! Helper method to expose equality to the C# world.
	bool isEqual(const PYXTileCollection& to) { return *(m_spTileSet) == *(to.m_spTileSet); }	

	//! adjust the resolution to limit the number of cells in the geometry
	void limitCellsCountTo(int maxCellsCount);

	//! get number of cells inside the tile collection.
	long getCellCount() const;

	//! get area on earth in square meters.
	double getAreaOnReferenceShpere() const;

private:

	//! A tile set.
	//! TODO: Eventually rewire everything to use this, and remove other fields.
	std::auto_ptr<PYXTileSet> m_spTileSet;

	//! Flag to enable or disable automatic aggregation of tiles.
	bool m_bAutoAggregate;

private:

	friend PYXLIB_DECL bool operator ==(	const PYXTileCollection& lhs,
											const PYXTileCollection& rhs	);

	friend class TileCollectionInnerTileIntersectionIterator;
};

/*!
This class is designed to iterate over the contents of a PYXTileCollection. The 
iterator depends on the tile collection that was used to create it. If the tile
collection is altered during the iteration process the results will be 
undefined.
*/
//! Iterates through a collection of tiles.
class PYXLIB_DECL PYXTileCollectionIterator : public PYXGeometryIterator
{
public:

	/*!
	Create a new instance of a PYXTileCollectionIterator. The iterator is
	returned in a managed pointer. The iterator is not built to handle
	changes in the originating tile collection. If the originating tile 
	collection is changed the behavior is undefined.

	\param	spTileCollection	The tile collection to iterate over. Altering
								this collection during iteration will cause
								undefined behavior in the iterator.

	\return A new instance of a tile collection iterator.
	*/
	//! Create a managed pointer to a PYXTileCollectionIterator.
	static PYXPointer<PYXTileCollectionIterator> create(
		const PYXTileCollection & tileCollection	)
	{
		return PYXNEW(PYXTileCollectionIterator, tileCollection);
	}

	//! The constructor for the iterator
	explicit PYXTileCollectionIterator(const PYXTileCollection & tileCollection);

	//! Destructor
	virtual ~PYXTileCollectionIterator() {}

	//!	Reset the current geometry back to the first geometry.
	virtual void reset();

	//! Move to the next geometry.
	virtual void next();

	//! Determine if the iterator has reached the end of the collection
	virtual bool end() const;

	//! Return the geometry of the current tile for the iterator
	virtual PYXPointer<PYXGeometry> getGeometry() const;

	//! Return the current tile for the iterator
	virtual PYXPointer<PYXTile> getTile() const;

private:

	//! The tile collection the iterator is associated with.
	const PYXTileCollection & m_collection;

	//! The resolution to use for iteration.
	int m_resolution;

	//! The current position within the cell collection.
	PYXTileCollection::Iterator m_itCurrent;
};

//! Allows PYXIS tiles to be written to streams.
PYXLIB_DECL std::basic_ostream< char>& operator <<(
	std::basic_ostream< char>& out,
	const PYXTileCollection& pyxTileCollection	);

//! Allows PYXIS tiles to be read from streams.
PYXLIB_DECL std::basic_istream< char>& operator >>(
	std::basic_istream< char>& input, 
	PYXTileCollection& pyxTileCollection	);


#endif 
