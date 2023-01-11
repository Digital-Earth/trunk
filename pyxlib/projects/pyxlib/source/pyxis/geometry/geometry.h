#ifndef PYXIS__GEOMETRY__GEOMETRY_H
#define PYXIS__GEOMETRY__GEOMETRY_H
/******************************************************************************
geometry.h

begin		: 2004-10-18
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/bounding_shape.h"
#include "pyxis/utility/rect_2d.h"

// standard includes
#include <vector>

// local forward declarations
class PYXIcosIndex;
class PYXIterator;
class PYXInnerTile;
class PYXInnerTileIntersectionIterator;
class PYXTileCollection;
class ICoordConverter;

/*!
PYXGeometry is the abstract base for all classes that represent a collection
of cells at the same resolution.
*/
//! Abstract base class representing a collection of cells at the same resolution.
class PYXLIB_DECL PYXGeometry : public PYXObject
{
public:

	/*!
	Create a copy of the geometry.
	
	\return	A copy of the geometry.
	*/
	virtual PYXPointer<PYXGeometry> clone() const = 0;

	/*!
	Is the geometry a collection.

	\return	true if the geometry is a collection, otherwise false.
	*/
	virtual bool isCollection() const { return false; }

	/*!
	Is the geometry empty.

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	virtual bool isEmpty() const = 0;

	/*!
	Get the PYXIS resolution of cells in the geometry.

	\return	The PYXIS resolution or -1 if the geometry is empty.
	*/
	virtual int getCellResolution() const = 0;

	/*!
	Set the PYXIS resolution of cells in the geometry.

	\param	nCellResolution	The cell resolution.
	*/
	virtual void setCellResolution(int nCellResolution) = 0;

	/*!
	Get the intersection of this geometry and the specified geometry as a new
	geometry.

	\param	geometry		The geometry to intersect with this one.
	\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

	\return	The intersection of geometries.
	*/
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	/*! 
	Determine if any intersection between this geometry and the specified 
	geometry exists.

	\param	geometry		The geometry to test for intersection with this one.
	\param	bCommutative	Indicates whether the operation is to be commutative (true) or one-way (false).

	\return true if there is any intersection, otherwise false.
	*/
	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const;


	/*! 
	Determine this geometry contians the specified geometry.

	\param	geometry		The geometry to test for intersection with this one.
	
	\return true if this geometry contains the given geometry, otherwise false.
	*/
	//! Return a boolean indication of contains.
	virtual bool contains(const PYXGeometry& geometry) const;

	/*!
	Get the disjunction (union) of this geometry and the specified geometry as a new
	geometry.

	\param	geometry		The geometry to unite with this one.

	\return	The disjunction (union) of geometries.
	*/
	virtual PYXPointer<PYXGeometry> disjunction(const PYXGeometry& geometry) const;

	/*!
	Get an iterator to the individual cells in the geometry.

	\return	The iterator (ownership transferred)
	*/
	virtual PYXPointer<PYXIterator> getIterator() const = 0;

	/*!
	Get an iterator to the individual cells in the geometry.

	\return	The iterator (ownership transferred)
	*/
	virtual PYXPointer<PYXInnerTileIntersectionIterator> getInnerTileIterator(const PYXInnerTile & tile) const;

	/*!
	Calculate a series of PYXIS indices around a geometry.

	This is useful for operations like displaying the geometry.
	(It's not necessarily a formal boundary or envelope.)

	\param	pVecIndex	The container to hold the returned indices.
						(must not be null)
	*/
	virtual void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const = 0;

	/*!
	Copies a representation of this geometry into a tile collection.

	\param	pTileCollection	The destination tile collection (must not be null).
	*/
	virtual void copyTo(PYXTileCollection* pTileCollection) const = 0;

	/*!
	Copies a representation of this geometry into a tile collection at the specified resolution.

	\param	pTileCollection		The destination tile collection (must not be null).
	\param	nTargetResolution	The target resolution.
	*/
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const = 0;

	/*!
	\param	coordConverter		A pointer to a coordinate convertor to use.  The client must
								ensure that the coordinate convertor lives for the duration of
								this function call.  The caller maintains ownership of the
								coordinate convertor.

	\param pRect1				A pointer to a PYXRect2DDouble class that will recieve the 
								result.
	\param pRect2				A pointer to a PYXRect2DDouble class that will recieve the 
								optional second part of the result if the geometry spans the
								day/date line.

	IMPORTANT LIMITATION: The first implementation of this only works for coodinate systems that go 
	from -180 to 180 and -90 to 90.
	*/
	//! Get the bounding box (or two if we span the day/date line) for this geometry.
	virtual void getBoundingRects(const ICoordConverter* coordConvertor,
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2) const;

	//Get bounding Circle around the geometry
	virtual PYXBoundingCircle getBoundingCircle() const = 0;

};

/*!
PYXEmptyGeometry represents a simple empty geometry. Most geometry classes can
be empty but this class provides a simple way to provide an empty geometry when
one is required.
*/
//! A simple empty geometry.
class PYXLIB_DECL PYXEmptyGeometry : public PYXGeometry
{
public:

	//! Test method
	static void test();

	//! Creator.
	static PYXPointer<PYXEmptyGeometry> create()
	{
		return PYXNEW(PYXEmptyGeometry);
	}

	//! Copy creator.
	static PYXPointer<PYXEmptyGeometry> create(const PYXEmptyGeometry& rhs)
	{
		return PYXNEW(PYXEmptyGeometry, rhs);
	}

	//! Constructor.
	PYXEmptyGeometry() {}

	//! Destructor
	virtual ~PYXEmptyGeometry() {}

	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;

	//! Is the geometry empty.
	virtual bool isEmpty() const;

	//! Get the PYXIS resolution of cells in the geometry.
	virtual int getCellResolution() const;

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const; 

	//! Get an iterator to the individual cells in the geometry.
	virtual PYXPointer<PYXIterator> getIterator() const;

	/*!
	\param	pVecIndex	The container to hold the returned indices.
	*/
	//! Calculate a series of PYXIS indices around a geometry.
	virtual inline void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const;

	//! Copies a representation of this geometry into a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copies a representation of this geometry into a tile collection at the specified resolution.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;

	//Get bounding Circle around the geometry
	virtual PYXBoundingCircle getBoundingCircle() const;
};

/*!
PYXGlobalGeometry represents a geometry that covers the entire globe.
*/
//! A geometry that represents the entire globe.
class PYXLIB_DECL PYXGlobalGeometry : public PYXGeometry
{
public:

	//! Test method
	static void test();

	//! Creator
	static PYXPointer<PYXGlobalGeometry> create(int nResolution)
	{
		return PYXNEW(PYXGlobalGeometry, nResolution);
	}

	//! Copy creator
	static PYXPointer<PYXGlobalGeometry> create(const PYXGlobalGeometry& rhs)
	{
		return PYXNEW(PYXGlobalGeometry, rhs);
	}

	//! Deserialization creator
	static PYXPointer<PYXGlobalGeometry> create(std::basic_istream< char>& in)
	{
		return PYXNEW(PYXGlobalGeometry, in);
	}

	//! Constructor initializes member variables
	explicit PYXGlobalGeometry(int nResolution) : m_nCellResolution(nResolution) {}

	//! Deserialization constructor
	explicit PYXGlobalGeometry(std::basic_istream< char>& in);

	//! Destructor
	virtual ~PYXGlobalGeometry() {}

	//! Create a copy of the geometry.
	virtual PYXPointer<PYXGeometry> clone() const;

	//! Serialize.
	virtual void serialize(std::basic_ostream< char>& out) const;

	//! Is the geometry empty.
	virtual bool isEmpty() const;

	//! Get the PYXIS resolution of cells in the geometry; can be less than 0 for empty.
	virtual int getCellResolution() const;

	//! Set the PYXIS resolution of cells in the geometry.
	virtual void setCellResolution(int nCellResolution);

	//! Get the intersection of this geometry and the specified geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const; 

	//! Get an iterator to the individual cells in the geometry.
	virtual PYXPointer<PYXIterator> getIterator() const;

	/*!
	\param	pVecIndex	The container to hold the returned indices.
	*/
	//! Calculate a series of PYXIS indices around a geometry.
	virtual inline void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const;

	//! Copies a representation of this geometry into a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copies a representation of this geometry into a tile collection at the specified resolution.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;

	//Get bounding Circle around the geometry
	virtual PYXBoundingCircle getBoundingCircle() const;

private:

	//! The cell resolution
	int m_nCellResolution;
};

//! The equality operator.
inline
PYXLIB_DECL bool operator ==(	const PYXGlobalGeometry& lhs,
								const PYXGlobalGeometry& rhs	)
{
	return (lhs.getCellResolution() == rhs.getCellResolution());
}

//! The equality operator for the general case.
PYXLIB_DECL bool operator ==(const PYXGeometry& lhs, const PYXGeometry& rhs);

#endif // guard
