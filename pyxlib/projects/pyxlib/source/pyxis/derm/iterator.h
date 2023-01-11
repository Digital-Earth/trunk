#ifndef PYXIS__DERM__ITERATOR_H
#define PYXIS__DERM__ITERATOR_H
/******************************************************************************
iterator.h

begin		: 2003-12-17
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/index.h"
#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/object.h"

// forward declarations
class PYXValue;

/*!
The PYXIterator iterates over cells at a given resolution. It provides methods
for getting the cell index and getting and setting the cell's values.
*/
//! Abstract base for classes that iterate over PYXIS cells.
class PYXLIB_DECL PYXIterator : public PYXAbstractIterator, public PYXObject
{
public:

	//! Destructor
	virtual ~PYXIterator() {}

	/*!
	Get the PYXIS icos index for the current cell.

	\return	The PYXIS icos index for the current cell.
	*/
	virtual const PYXIcosIndex& getIndex() const = 0;

	//! Get the value of the current cell.
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const;

	//! Set the value of the current cell.
	virtual void setFieldValue(const PYXValue& value, int nFieldIndex = 0);

	//! Simple operator of the form "++myIterator".
	virtual PYXIterator &operator++() 
	{
		next();
		return *this;
	}

protected:

	/*!
	Provide a null index for convenience.

	\return	The null index.
	*/
	static const PYXIcosIndex& getNullIndex()
	{
		static const PYXIcosIndex kNullIndex;

		return kNullIndex;
	}
};

//! Iterator that returns no indices. (i.e. end() is true immediately)
class PYXLIB_DECL PYXEmptyIterator : public PYXIterator
{
public:

	//! Dynamic creator
	static PYXPointer<PYXEmptyIterator> create()
	{
		return PYXNEW(PYXEmptyIterator);
	}

	//! Constructor
	PYXEmptyIterator() {}

	//! Destructor
	virtual ~PYXEmptyIterator() {}

	/*!
	Move to the next cell.
	*/
	virtual void next() {}

	/*!
	See if we have covered all the cells.

	\return	true if all cells have been covered, otherwise false.
	*/
	virtual bool end() const {return true;}

	/*!
	Get the PYXIS icos index for the current cell.

	\return	The PYXIS icos index for the current cell.
	*/
	virtual const PYXIcosIndex& getIndex() const {return getNullIndex();}
};

//! Iterator that returns a single index.
class PYXLIB_DECL PYXSingleIterator : public PYXIterator
{
public:

	//! Dynamic creator
	static PYXPointer<PYXSingleIterator> create(const PYXIcosIndex& index)
	{
		return PYXNEW(PYXSingleIterator, index);
	}

	//! Constructor
	PYXSingleIterator(const PYXIcosIndex& index) : m_index(index) {}

	//! Destructor
	virtual ~PYXSingleIterator() {}

	/*!
	Move to the next cell.
	*/
	virtual void next() {m_index.reset();}

	/*!
	See if we have covered all the cells.

	\return	true if all cells have been covered, otherwise false.
	*/
	virtual bool end() const {return m_index.isNull();}

	/*!
	Get the PYXIS icos index for the current cell.

	\return	The PYXIS icos index for the current cell.
	*/
	virtual const PYXIcosIndex& getIndex() const {return m_index;}

private:

	//! The single index
	PYXIcosIndex m_index;
};

#endif // guard
