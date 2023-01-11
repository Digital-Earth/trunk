#ifndef PYXIS__DATA__FEATURE_COLLECTION_INDEX_H
#define PYXIS__DATA__FEATURE_COLLECTION_INDEX_H
/******************************************************************************
feature_collection_index.h

begin		: 2013-05-28
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/feature_collection.h"

// standard includes

/*!
A collection of features which can be searched by a given value.
*/
//! A feature collection.
struct PYXLIB_DECL IFeatureCollectionIndex : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//return all features that contain the given value
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXValue & value) const = 0;

	//return all features that contains the given value and intersects with the given geometry
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry,const PYXValue & value) const = 0;

	//return a list of possible values to complete given an input value
	virtual std::vector<PYXValue> STDMETHODCALLTYPE suggest(const PYXValue & value) const = 0;
};

#endif // guard
