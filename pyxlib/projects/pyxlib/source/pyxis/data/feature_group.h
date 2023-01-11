#ifndef PYXIS__DATA__FEATURE_GROUP_H
#define PYXIS__DATA__FEATURE_GROUP_H
/******************************************************************************
feature_group.h

begin		: 2011-12-15
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/feature.h"
#include "pyxis/data/histogram.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/range.h"

// standard includes

/*!
A group of features which is itself a feature.
A group of features can be treaded as simple IFeatureCollection to enumerate all features inside a group.
However, group of features can contain groups of features as well. creating an spatial hierarchy view of all its features.
To enumrate sub-groups inside a featureGroup, you can use getGroupIterator that returns next level of the heirarchy (can be GroupFeatures and simple Features)
*/
//! A feature group.
struct PYXLIB_DECL IFeatureGroup : public IFeatureCollection
{
	PYXCOM_DECLARE_INTERFACE();

public:
	//get the feature count inside the group.
	virtual Range<int> STDMETHODCALLTYPE getFeaturesCount() const = 0;

	//get if the group has finer details
	virtual bool STDMETHODCALLTYPE moreDetailsAvailable() const = 0;

	//get histogram of values for a given field
	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(int fieldIndex) const = 0;

	//get histogram of values for a given field with a given gemoetry
	virtual PYXPointer<PYXHistogram> STDMETHODCALLTYPE getFieldHistogram(const PYXGeometry& geometry, int fieldIndex) const = 0;

	//get a sub group by its group id
	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroup(const std::string & groupId) const = 0;

	//get a sub group by its group id
	virtual boost::intrusive_ptr<IFeatureGroup> STDMETHODCALLTYPE getFeatureGroupForFeature(const std::string & featureId) const = 0;

	//get the next level of details iterator. can return IFeature or IFeatureGroup
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator() const = 0;

	//get the next level of details iterator that intersecnts the given geometry
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getGroupIterator(const PYXGeometry& geometry) const = 0;	
};

#endif // guard
