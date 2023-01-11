#ifndef PYXIS__GEOMETRY__GEOMETRY_ITERATOR_H
#define PYXIS__GEOMETRY__GEOMETRY_ITERATOR_H
/******************************************************************************
geometry_iterator.h

begin		: 2004-11-17
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/object.h"

//! Abstract base for classes that iterate over PYXIS geometries.
/*!
The PYXGeometryIterator iterates over geometries in a collection.
*/
class PYXLIB_DECL PYXGeometryIterator : public PYXObject, public PYXAbstractIterator
{
public:

	//! Destructor
	virtual ~PYXGeometryIterator() {}

	/*!
	Reset the current geometry back to the first geometry.
	*/
	virtual void reset() = 0;

	/*!
	Get the current geometry.

	\return	The current geometry.
	*/
	virtual PYXPointer<PYXGeometry> getGeometry() const = 0;
};

//! Iterator that returns no geometries. (i.e. end() is true immediately)
class PYXLIB_DECL PYXEmptyGeometryIterator : public PYXGeometryIterator
{
public:

	//! Constructor
	PYXEmptyGeometryIterator() {}

	//! Destructor
	virtual ~PYXEmptyGeometryIterator() {}

	/*!
	Reset the current geometry back to the first geometry.
	*/
	virtual void reset() {}

	/*!
	Move to the next geometry.
	*/
	virtual void next() {}

	/*!
	See if we have covered all the geometries.

	\return	true if all geometries have been covered, otherwise false.
	*/
	virtual bool end() const {return true;}

	/*!
	Get the current geometry.

	\return	0
	*/
	virtual PYXPointer<PYXGeometry> getGeometry() const { return PYXPointer<PYXGeometry>(); }
};

//! Iterator that returns a single geometry
class PYXLIB_DECL PYXSingleGeometryIterator : public PYXGeometryIterator
{
public:

	/*!
	Constructor initializes member variables.

	\param spGeometry	The geometry.
	*/
	explicit PYXSingleGeometryIterator(PYXPointer<PYXGeometry> spGeometry) :
		m_spGeometry(spGeometry),
		m_spCurrentGeometry(spGeometry) {}

	//! Destructor
	virtual ~PYXSingleGeometryIterator() {}

	/*!
	Reset the current geometry back to the first geometry.
	*/
	virtual void reset() {m_spCurrentGeometry = m_spGeometry;}

	/*!
	Move to the next feature.
	*/
	virtual void next() {m_spCurrentGeometry.reset();}

	/*!
	See if we have covered all the geometries.

	\return	true if all geometries have been covered, otherwise false.
	*/
	virtual bool end() const {return (0 == m_spCurrentGeometry);}

	/*!
	Get the current geometry.

	\return	The current geometry (ownership retained) or 0 if at end.
	*/
	virtual PYXPointer<PYXGeometry> getGeometry() const {return m_spCurrentGeometry;}

private:

	//! Disabled copy constructor
	PYXSingleGeometryIterator(const PYXSingleGeometryIterator&);

	//! Disabled assignment operator
	PYXSingleGeometryIterator& operator =(const PYXSingleGeometryIterator&);

	//! The single geometry
	PYXPointer<PYXGeometry> m_spGeometry;

	//! The single geometry
	PYXPointer<PYXGeometry> m_spCurrentGeometry;
};

#endif // guard
