#ifndef PYXIS__GEOMETRY__GEOMETRY_COLLECTION_H
#define PYXIS__GEOMETRY__GEOMETRY_COLLECTION_H
/******************************************************************************
geometry_collection.h

begin		: 2004-10-18
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/derm/iterator.h"

// forward declarations
class PYXGeometryIterator;

/*!
PYXGeometryCollection is the abstract base for classes that represent
collections of geometries. This is used to represent higher order collections
of geometries. Simple collections of cells do not need to derive from this
class.
*/
//! Abstract base class for collections of geometries.
class PYXLIB_DECL PYXGeometryCollection : public PYXGeometry
{
public:

	//! Destructor
	virtual ~PYXGeometryCollection() {}

	/*!
	Is the geometry a collection.

	\return	true if the geometry is a collection, otherwise false.
	*/
	virtual bool isCollection() const {return true;}

	/*!
	Get an iterator to the individual cells in the geometry.

	\return	The iterator (ownership transferred).
	*/
	virtual PYXPointer<PYXIterator> getIterator() const
	{
		return PYXGeometryCollectionIterator::create(this);
	}

	/*!
	Clear all entries in the collection. After this method is called, isEmpty()
	will return true.
	*/
	virtual void clear() = 0;

	/*!
	Set the PYXIS resolution of cells in the geometry.

	\param	nCellResolution	The cell resolution.
	*/
	virtual inline void setCellResolution(int nCellResolution);

	/*!
	Get the number of geometries contained in this collection.

	\return	The number of geometries in this collection.
	*/
	virtual int getGeometryCount() const = 0;

	/*!
	Get an iterator to the geometries in this collection.

	\return	An iterator to the geometries in this collection (ownership transferred).
	*/
	virtual PYXPointer<PYXGeometryIterator> getGeometryIterator() const = 0;

	//! Get the bounding box for this geometry.
	virtual void getBoundingRects(const ICoordConverter* coordConvertor,
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2) const;

	virtual PYXBoundingCircle getBoundingCircle() const;

private:

	/*!
	PYXGeometryCollectionCellIterator iterates over the cells in the geometries
	that belong to the collection.
	*/
	//! Iterates over the cells of the constituent geometries in a collection.
	class PYXGeometryCollectionIterator : public PYXIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<PYXGeometryCollectionIterator> create(
			const PYXGeometryCollection* pCollection	)
		{
			return PYXNEW(PYXGeometryCollectionIterator, pCollection);
		}

		/*!
		Constructor initializes member variables.

		\param	pCollection	The geometry collection (ownership retained by caller).
		*/
		PYXGeometryCollectionIterator(const PYXGeometryCollection* pCollection) :
			m_spGeometryIt(pCollection->getGeometryIterator()),
			m_spCurrentGeometry(),
			m_spCellIt()
		{
			next();
		}

		/*!
		Destructor cleans up memory.
		*/
		virtual ~PYXGeometryCollectionIterator() {}

		/*!
		Move to the next cell.
		*/
		virtual inline void next();

		/*!
		See if we have covered all the geometries.

		\return	true if all geometries have been covered, otherwise false.
		*/
		virtual inline bool end() const;

		/*!
		Get the current index.

		\return	The index.
		*/
		virtual const PYXIcosIndex& getIndex() const
		{
			return m_spCellIt->getIndex();
		}

	private:

		//! Iterator to the geometries in the collection
		PYXPointer<PYXGeometryIterator> m_spGeometryIt;

		PYXPointer<PYXGeometry> m_spCurrentGeometry;

		//! Iterator to the cells in the current geometry
		PYXPointer<PYXIterator> m_spCellIt;
	};
};

#endif // guard
