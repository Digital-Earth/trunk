#ifndef PYXIS__DATA_SOURCE__FEATURE_ITERATOR_H
#define PYXIS__DATA_SOURCE__FEATURE_ITERATOR_H
/******************************************************************************
feature_iterator.h

begin		: 2004-11-04
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/feature.h"
#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/object.h"

//! Abstract base for classes that iterate over PYXIS features.
/*!
The PYXFeatureIterator iterates over a collection of features.
*/
class PYXLIB_DECL PYXFeatureIterator : public PYXObject, public PYXAbstractIterator
{
public:

	//! Destructor
	virtual ~PYXFeatureIterator() {}

	/*!
	Get the current PYXIS feature.

	\return	The feature.
	*/
	virtual PYXPointer<const PYXFeature> getFeature() const = 0;
};

//! Iterator that returns no features. (i.e. end() is true immediately)
class PYXLIB_DECL PYXEmptyFeatureIterator : public PYXFeatureIterator
{
public:

	//! Dynamic creator
	static PYXPointer<PYXEmptyFeatureIterator> create()
	{
		return PYXNEW(PYXEmptyFeatureIterator);
	}

	//! Constructor
	PYXEmptyFeatureIterator() {}

	//! Destructor
	virtual ~PYXEmptyFeatureIterator() {}

	/*!
	Move to the next feature.
	*/
	virtual void next() {}

	/*!
	See if we have covered all the features.

	\return	true if all features have been covered, otherwise false.
	*/
	virtual bool end() const {return true;}

	/*!
	Get the current feature.

	\return	An empty shared pointer.
	*/
	virtual PYXPointer<const PYXFeature> getFeature() const;
};

//! Iterator that returns a single feature
class PYXLIB_DECL PYXSingleFeatureIterator : public PYXFeatureIterator
{
public:

	/*!
	Constructor initializes member variables.

	\param spFeature	The feature.
	*/
	PYXSingleFeatureIterator(PYXPointer<const PYXFeature> spFeature) : m_spFeature(spFeature) {}

	//! Destructor
	virtual ~PYXSingleFeatureIterator() {}

	/*!
	Move to the next feature.
	*/
	virtual void next() {m_spFeature = 0;}

	/*!
	See if we have covered all the features.

	\return	true if all features have been covered, otherwise false.
	*/
	virtual bool end() const {return (m_spFeature == 0);}

	/*!
	Get the current feature.

	\return	Shared pointer to current feature or empty shared pointer if at end.
	*/
	virtual PYXPointer<const PYXFeature> getFeature() const {return m_spFeature;}

private:

	//! The single feature
	PYXPointer<const PYXFeature> m_spFeature;
};

#endif // guard
