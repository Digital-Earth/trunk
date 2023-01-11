#ifndef PYXIS__GEOMETRY__MULTI_GEOMETRY_H
#define PYXIS__GEOMETRY__MULTI_GEOMETRY_H
/******************************************************************************
multi_geometry.h

begin		: 2006-04-28
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/derm/iterator.h"

#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/geometry_collection.h"
#include "pyxis/geometry/geometry_serializer.h"
#include "pyxis/geometry/tile_collection.h"

#include "pyxis/utility/stl_utils.h"

// standard includes

// forward declarations
class PYXCurve;
class PYXPolygon;

template<class T>
class
#if !defined(MSVC_BUG_99611) // Remove this when Microsoft's fix is available.
PYXLIB_DECL
#endif
/*!
PYXMultiGeometry is a template class that holds a sorted collection of like
complex geometries.
*/
//! A sorted collection of geometries.
PYXMultiGeometry : public PYXGeometry
{
public:

	//! Test method
	static void test() {}

	//! Creator.
	static PYXPointer< PYXMultiGeometry<T> > create()
	{
		return PYXNEW(PYXMultiGeometry);
	}

	//! Copy creator.
	static PYXPointer< PYXMultiGeometry<T> > create(const PYXMultiGeometry<T>& rhs)
	{
		return PYXNEW(PYXMultiGeometry, rhs);
	}

	//! Deserialization creator.
	static PYXPointer< PYXMultiGeometry<T> > create(std::basic_istream< char>& in)
	{
		return PYXNEW(PYXMultiGeometry, in);
	}

	//! Constructor
	PYXMultiGeometry() {}

	//! Deserialization Constructor.
	PYXMultiGeometry(std::basic_istream< char>& in)
	{
		deserialize(in);
	}

	//! Destructor
	virtual ~PYXMultiGeometry() {}

	//! Test if two geometries are equal
	bool operator ==(const PYXMultiGeometry<T>& rhs) const
	{
		if (m_setGeometry.size() != rhs.m_setGeometry.size())
		{
			return false;
		}

		GeometrySet::const_iterator itLeft = m_setGeometry.begin();
		GeometrySet::const_iterator itRight = rhs.m_setGeometry.begin();
		for (; itLeft != m_setGeometry.end(); ++itLeft, ++itRight)
		{
			if (**itLeft != **itRight)
			{
				return false;
			}
		}

		return true;
	}

	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const
	{
		return create(*this);
	}

	/*!
	Is the geometry a collection.

	\return	true if the geometry is a collection, otherwise false.
	*/
	virtual bool isCollection() {return true;}

	/*!
	Is the geometry empty.

	\return	true if the geometry is empty or false otherwise.
	*/
	//! Determine if a collection is empty.
	virtual bool isEmpty() const {return m_setGeometry.empty();}

	//! Clear all entries in the collection.
	virtual void clear() {m_setGeometry.clear();}

	//! Get the PYXIS resolution of cells in the geometry.
	virtual int getCellResolution() const
	{
		int nCellResolution = -1;

		if (!m_setGeometry.empty())
		{
			nCellResolution = (*m_setGeometry.begin())->getCellResolution();
		}

		return nCellResolution;
	}

	/*!
	Set the PYXIS resolution of cells in the geometry.

	\param	nCellResolution	The cell resolution.
	*/
	virtual void setCellResolution(int nCellResolution)
	{
		for (	GeometrySet::const_iterator it(m_setGeometry.begin());
				it != m_setGeometry.end();
				++it	)
		{
			(*it)->setCellResolution(nCellResolution);
		}
	}

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const
	{
		// There are type issues here. For now, use a tile collection.
		PYXPointer<PYXTileCollection> spTC = PYXTileCollection::create();

		for (	GeometrySet::const_iterator it(m_setGeometry.begin());
				it != m_setGeometry.end();
				++it	)
		{
			PYXPointer<PYXGeometry> spInt = (*it)->intersection(geometry);
			if (const PYXEmptyGeometry* p = dynamic_cast<const PYXEmptyGeometry*>(spInt.get()))
			{
				continue;
			}
			else
			{
				// Copy through a tile collection.
				PYXTileCollection tc;
				spInt->copyTo(&tc);
				for (PYXPointer<PYXTileCollectionIterator> spIt = tc.getTileIterator();
					!spIt->end(); spIt->next())
				{
					spTC->addTile(spIt->getTile());
				}
			}
		}

		if (spTC->isEmpty())
		{
			return PYXEmptyGeometry::create();
		}

		return spTC;
	}

	//! Does this geometry intersect with the specified geometry?
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const
	{
		for (	GeometrySet::const_iterator it(m_setGeometry.begin());
				it != m_setGeometry.end();
				++it	)
		{
			assert(0 != *it);
			if ((*it)->intersects(geometry))
			{
				return true;
			}
		}

		return false;
	}

	/*!
	Get an iterator to the individual cells in the geometry.

	\return	The iterator (ownership transferred)
	*/
	virtual PYXPointer<PYXIterator> getIterator() const
	{
		return PYXMultiGeometryIterator::create(	m_setGeometry.begin(),
													m_setGeometry.end()	);
	}

	/*!
	Calculate a series of PYXIS indices around a geometry.

	This is useful for operations like displaying the geometry.
	(It's not necessarily a formal boundary or envelope.)

	\param	pVecIndex	The container to hold the returned indices.
						(must not be null)
	*/
	virtual void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const
	{
		assert(false && "Not implemented.");
	}

	virtual PYXBoundingCircle getBoundingCircle() const
	{
		assert(false && "Not yet implemented.");
		return PYXBoundingCircle();
	}

	/*!
	Copies a representation of this geometry into a tile collection.

	\param	pTileCollection	The destination tile collection (must not be null).
	*/
	//! Copy the multi geometry into a tile collection
	virtual void copyTo(PYXTileCollection* pTileCollection) const
	{
		copyTo(pTileCollection, getCellResolution());
	}

	/*!
	Copies a representation of this geometry into a tile collection at the specified resolution.

	\param	pTileCollection		The destination tile collection (must not be null).
	\param	nTargetResolution	The target resolution.
	*/
	//! Copy the multi geometry into a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const
	{
		assert(pTileCollection != 0);

		// start with a fresh geometry
		pTileCollection->clear();

		for (	GeometrySet::const_iterator it(m_setGeometry.begin());
				it != m_setGeometry.end();
				++it	)
		{
			PYXTileCollection tileCollection;
			(*it)->copyTo(&tileCollection, nTargetResolution);
			pTileCollection->addGeometry(tileCollection);
		}
	}

	//! Add a geometry to the collection.
	void addGeometry(PYXPointer<T> spGeometry)
	{
		if (!isEmpty())
		{
			assert(	(getCellResolution() == spGeometry->getCellResolution()) &&
					"Cell resolutions must match."	);
		}

		m_setGeometry.push_back(spGeometry);
	}

	/*!
	Serialize the multi-geometry to a stream.

	\param	out	The stream to serialize to.
	*/
	//! Serialize this geometry to a stream.
	virtual void serialize(std::basic_ostream< char>& out) const
	{
		out << m_setGeometry.size();

		for (GeometrySet::const_iterator it(m_setGeometry.begin());
			it != m_setGeometry.end();
			++it)
		{
			PYXPointer<PYXGeometry> spGeometry = boost::dynamic_pointer_cast<PYXGeometry>(*it);
			PYXGeometry* geometry = spGeometry.get();
			PYXGeometrySerializer::serialize(*geometry, out);
		}
	}

	/*!
	Deserialize the multi-geometry from a stream.

	\param	out	The stream to deserialize from.
	*/
	//! Deserialize the multi-geometry from a stream.
	virtual void deserialize(std::basic_istream< char>& in)
	{
		int nGeometryCount = 0;
		
		in >> nGeometryCount;		
		m_setGeometry.clear();

		for(int i = 0; i < nGeometryCount; ++i)
		{
			m_setGeometry.push_back(boost::dynamic_pointer_cast<T>(
				PYXGeometrySerializer::deserialize(in)));
		}
	}

private:

	//! Copy constructor
	PYXMultiGeometry(const PYXMultiGeometry& rhs) : PYXGeometry(rhs)
	{
		*this = rhs;
	}

	//! Copy assignment
	PYXMultiGeometry& operator =(const PYXMultiGeometry& rhs)
	{
		if (this != &rhs)
		{
			m_setGeometry.clear();

			for (	GeometrySet::const_iterator it(rhs.m_setGeometry.begin());
					it != rhs.m_setGeometry.end();
					++it	)
			{
				PYXPointer<PYXGeometry> spNewGeom = it->get()->clone();

				m_setGeometry.push_back(boost::dynamic_pointer_cast<T>(spNewGeom));
			}

			PYXGeometry::operator =(rhs);
		}

		return *this;
	}

	//! Typedef for a set of geometries
	typedef std::vector<PYXPointer<T>> GeometrySet;

	/*!
	PYXMultiGeometryIterator performs an iteration over the indices in each of
	the geometries it contains.
	*/
	//! Iterates over the indices in the contained geometries
	class PYXMultiGeometryIterator : public PYXIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<PYXMultiGeometryIterator> create(
			typename GeometrySet::const_iterator itBegin,
			typename GeometrySet::const_iterator itEnd	)
		{
			return PYXNEW(PYXMultiGeometryIterator, itBegin, itEnd);
		}

		/*!
		Constructor initializes member variables.

		\param	itBegin	Iterator to the beginning of the geometry collection
		\param	itEnd	Iterator to the end of the geometry collection
		*/
		PYXMultiGeometryIterator(	typename GeometrySet::const_iterator itBegin,
									typename GeometrySet::const_iterator itEnd	) :
			m_itGeometry(itBegin),
			m_itGeometryEnd(itEnd)
		{
			next();
		}

		/*!
		Destructor cleans up memory.
		*/
		virtual ~PYXMultiGeometryIterator() {}

		/*!
		Move to the next cell.
		*/
		virtual void next()
		{
			while (!end())
			{
				if (!m_spCellIt)
				{
					m_spCellIt = (*m_itGeometry)->getIterator();
				}
				else
				{
					m_spCellIt->next();
				}

				if (!m_spCellIt->end())
				{
					break;
				}

				m_spCellIt = 0;
				++m_itGeometry;
			}
		}

		/*!
		See if we have covered all the geometries.

		\return	true if all geometries have been covered, otherwise false.
		*/
		virtual bool end() const
		{
			return (m_itGeometry == m_itGeometryEnd);
		}

		/*!
		Get the current index.

		\return	The index.
		*/
		virtual const PYXIcosIndex& getIndex() const
		{
			if (end())
			{
				return getNullIndex();
			}

			return m_spCellIt->getIndex();
		}

	private:

		//! Iterator to the current geometry in the collection
		typename GeometrySet::const_iterator m_itGeometry;

		//! Points one past the end of the geometries in the collection
		typename GeometrySet::const_iterator m_itGeometryEnd;

		//! Iterator to the cells in the current geometry
		PYXPointer<PYXIterator> m_spCellIt;
	};

private:

	//! The list of geometries
	GeometrySet m_setGeometry;
};

//! Typedef for a collection of curves
PYXLIB_DECL typedef PYXMultiGeometry<PYXCurve> PYXMultiCurve;

//! Typedef for a collection of polygons - use PYXCurve until PYXPolygon is implemented
PYXLIB_DECL typedef PYXMultiGeometry<PYXPolygon> PYXMultiPolygon;

#endif // guard
