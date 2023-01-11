#ifndef PYXIS__DATA__ALL_FEATURE_ITERATOR_H
#define PYXIS__DATA__ALL_FEATURE_ITERATOR_H

#include "pyxis/data/feature_collection.h"

//! An iterator over all features in a collection.
/*!
The feature collection must be alive for the duration of this iterator.

FeatureCollectionType fulfills the following interface:
-	size_t getFeatureCount() const
-	boost::intrusive_ptr< IFeature > getFeature(size_t) const
*/
template < typename FeatureCollectionType, typename FeatureType >
class AllFeatureIterator :
public FeatureIterator
{
	typedef AllFeatureIterator This;

	// The features being iterated.
	FeatureCollectionType const & featureCollection;

	// The offset of the current feature.
	size_t featureOffset;

	// The current feature.
	boost::intrusive_ptr< FeatureType > feature;

public:

	//! Constructs the iterator dynamically.
	static PYXPointer< This > create(
		FeatureCollectionType const & featureCollection)
	{
		return PYXNEW(This, featureCollection);
	}

	//! Constructs the iterator.
	explicit This(
		FeatureCollectionType const & featureCollection) :
	featureCollection(featureCollection),
	featureOffset(0),
	feature(featureCollection.getFeature(featureOffset))
	{
		// It is safe to use virtual calls in the constructor in this way.
		// http://www.parashift.com/c%2B%2B-faq-lite/strange-inheritance.html#faq-23.5
		if (!this->feature && !this->end())
		{
			this->next();
		}
	}

public: // FeatureIterator

	//! The end condition test.
	bool end() const
	{
		return this->featureCollection.getFeatureCount() <= this->featureOffset;
	}

	//! Move to the next feature.
	void next()
	{
		if (!this->end())
		{
			do 
			{
				++this->featureOffset;
				if (this->end())
				{
					break;
				}
				this->feature = this->featureCollection.getFeature(this->featureOffset);
			} while (!this->feature);
		}
	}

	//! Get the current feature.  Guaranteed to be non-null if not at end.
	boost::intrusive_ptr< IFeature > getFeature() const
	{
		return this->feature;
	}
};

#endif // guard
