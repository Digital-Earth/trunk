#ifndef PYXIS__SAMPLING__XY_BOUNDS_GEOMETRY_H
#define PYXIS__SAMPLING__XY_BOUNDS_GEOMETRY_H
/******************************************************************************
xy_bounds_geometry.h

begin		: 2005-08-12
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/utility/rect_2d.h"
#include "pyxis/region/xy_bounds_region.h"
#include "pyxis/geometry/vector_geometry.h"

// boost includes
#include <boost/intrusive_ptr.hpp>

// forward declarations
class PYXCell;
class ICoordConverter;
class PYXMultiCell;
class PYXTile;

/*!
*/
//! Concrete class representing bounds in native coordinate system.
class PYXLIB_DECL PYXXYBoundsGeometry : public PYXVectorGeometry
{
public:
	/*! the minimum number of geometries (tiles) that will be used
	to represent the XY "square" geometry.  This number was arrived at 
	experimentally and is a trade off between accuracy of representing the
	square, and calculation time.  The greater the accuracy of the 
	representation the more time we take here to calculate it, but
	less time is used by the clients of the resulting geometry.  Also,
	the more accurate the representation, the more memory we require to store
	the geometry.
	*/
	static const int knMinimumGeometryCount = 10;

	//! Test method
	static void test();

	//! Create method
	static PYXPointer<PYXXYBoundsGeometry> create(
		const PYXRect2DDouble& bounds,
		const ICoordConverter& coordConverter,
		int nResolution	)
	{
		return PYXNEW(PYXXYBoundsGeometry, bounds, coordConverter, nResolution);
	}

	//! Copy creator.
	static PYXPointer<PYXXYBoundsGeometry> create(const PYXXYBoundsGeometry& rhs)
	{
		return PYXNEW(PYXXYBoundsGeometry, rhs);
	}

	//! Deserialization creator
	static PYXPointer<PYXXYBoundsGeometry> create(std::basic_istream< char>& in)
	{
		return PYXNEW(PYXXYBoundsGeometry, in);
	}

	/*!
	\param bounds The bounds (will be copied).
	\param coordConverter The coordinate converter (will be cloned).
	\param nResolution The resolution (will be copied).
	*/
	//! Constructor
	PYXXYBoundsGeometry(	const PYXRect2DDouble& bounds,
							const ICoordConverter& coordConverter,
							int nResolution	);

	//! Expand the bounds to include a point.
	void expand(const PYXCoord2DDouble& pt);

	//! Deserialization constructor
	explicit PYXXYBoundsGeometry(std::basic_istream< char>& in);

	//! Clone.
	virtual PYXPointer<PYXGeometry> clone() const;

	//! Serialize.
	virtual void serialize(std::basic_ostream< char>& out) const;

	//! Deserialize.
	virtual void deserialize(std::basic_istream< char>& in);

	/*!
	Is the geometry a collection.

	\return	true if the geometry is a collection, otherwise false.
	*/
	virtual bool isCollection() { return false; }

	/*!
	Is the geometry empty.

	\return	true if the geometry is empty (no cells) or false otherwise.
	*/
	virtual bool isEmpty() const { return false; }

	/*!
	Get the bounds of this geometry.
	\return	The bounds in native coordinates.
	*/
	const PYXRect2DDouble& getBounds() const;

	/*!
	Get the coord converter of this geometry.
	\return	The coord converter (ownership retained).
	*/
	const boost::intrusive_ptr<ICoordConverter> & getCoordConverter() const;

	//! Get the bounding box for this geometry.
	virtual void getBoundingRects(const ICoordConverter* coordConvertor,
		PYXRect2DDouble* pRect1,
		PYXRect2DDouble* pRect2) const;

private:

	//! Copy constructor.
	PYXXYBoundsGeometry(const PYXXYBoundsGeometry& other);

	//! Copy assignment not implemented.
	PYXXYBoundsGeometry& operator=(const PYXXYBoundsGeometry&);

};

#endif	// guard
