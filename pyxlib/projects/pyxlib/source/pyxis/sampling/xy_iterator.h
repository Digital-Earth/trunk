#ifndef PYXIS__SAMPLING__XY_ITERATOR_H
#define PYXIS__SAMPLING__XY_ITERATOR_H
/******************************************************************************
xy_iterator.h

begin		: 2006-03-13
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/value.h"

// forward declarations


/*!
The PYXXYIterator iterates over points in an xy data source.

\sa PYXXYCoverage
*/
//! Abstract base for classes that iterate over xy coordinates
class PYXXYIterator : public PYXObject, public PYXAbstractIterator
{
public:

	//! Destructor
	virtual ~PYXXYIterator() {}

	//! Simple operator "++myIterator"
	virtual PYXXYIterator& operator++() 
	{
		next();
		return *this;
	}

	/*!
	Get the coordinates for the current point.

	\return	The coordinates for the current point.
	*/
	virtual PYXCoord2DDouble getPoint() const = 0;

	//! Get the value at the current point.
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const {return PYXValue();}

public:

	/*!
	Provide an empty point for convenience.

	\return	The empty point.
	*/
	static inline const PYXCoord2DDouble getEmptyPoint()
	{
		static const PYXCoord2DDouble kEmptyPoint;

		return kEmptyPoint;
	}
};

//! Iterator that returns no points. (i.e. end() is true immediately)
class PYXXYEmptyIterator : public PYXXYIterator
{
public:

	//! Dynamic creator
	static PYXPointer<PYXXYEmptyIterator> create()
	{
		return PYXNEW(PYXXYEmptyIterator);
	}

	//! Constructor
	PYXXYEmptyIterator() {}

	//! Destructor
	virtual ~PYXXYEmptyIterator() {}

	/*!
	Move to the next point.
	*/
	virtual void next() {}

	/*!
	See if we have covered all the points.

	\return	true if all points have been covered, otherwise false.
	*/
	virtual bool end() const {return true;}

	/*!
	Get the coordinates for the current point.

	\return	The coordinates for the current point.
	*/
	virtual PYXCoord2DDouble getPoint() const {return getEmptyPoint();}
};

#endif // guard
