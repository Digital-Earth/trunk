#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/data/feature_collection_cache.h"

// Inserts the base class's current feature (if there is one) into
// the gazetteer.  If it doesn't intersect the geometry
// or it has already been visited, advance to the next one and repeat.
void FeatureCollectionCache::IntersectionIterator::insert()
{
	assert(this->m_keys.getIsEmpty());
	assert(this->m_featureCollectionCache);
	assert(this->m_geometry);

	for (; !this->m_featureIterator->end(); this->m_featureIterator->next())
	{
		// Get the current feature ID from the feature iterator.
		std::string const featureID(this->m_featureIterator->getFeature()->getID());

		// Convert the feature ID to a key.
		size_t const key = this->m_featureCollectionCache->m_keyFeatureIDTranslator.getKey(featureID);

		if (!this->m_visited.find(key))
		{
			this->m_visited.insert(key);

			// Insert the intersection into the gazetteer.
			// Note that once the key range reports that it is empty,
			// it will not "restart" when new items are inserted.
			assert(this->m_featureIterator->getFeature());
			if (this->m_featureCollectionCache->m_gazetteer.insert(
					*this->m_geometry,
					key,
					*this->m_featureIterator->getFeature()->getRegion()))
			{
				return;
			}
		}
	}
}

//! Constructs the iterator dynamically.
/*! It is unsafe to modify the geometry during the lifetime of this object. */
PYXPointer< FeatureCollectionCache::IntersectionIterator >
FeatureCollectionCache::IntersectionIterator::create(
	PYXPointer< FeatureCollectionCache > featureCollectionCache,
	boost::intrusive_ptr< Geometry const > geometry)
{
	return PYXNEW(IntersectionIterator, featureCollectionCache, geometry);
}

//! Constructs the iterator.  The geometry cannot be null.
/*! It is unsafe to modify the geometry during the lifetime of this object. */
FeatureCollectionCache::IntersectionIterator::IntersectionIterator(
	PYXPointer< FeatureCollectionCache > featureCollectionCache,
	boost::intrusive_ptr< Geometry const > geometry) :
m_featureCollectionCache(featureCollectionCache),
m_featureIterator(featureCollectionCache->m_featureCollection->getIterator()),
m_geometry(geometry),
m_visited(),
m_keys(featureCollectionCache->m_gazetteer, *geometry, m_visited)
{
	if (this->m_keys.getIsEmpty())
	{
		this->insert();
	}
}

//! Move to the next feature.
void FeatureCollectionCache::IntersectionIterator::next()
{
	if (this->m_keys.getIsEmpty())
	{
		this->m_featureIterator->next();
	} else
	{
		this->m_keys.popFront();
		if (!this->m_keys.getIsEmpty())
		{
			return;
		}
	}
	this->insert();
}

//! The end condition test.
bool FeatureCollectionCache::IntersectionIterator::end() const
{
	return this->m_keys.getIsEmpty() && this->m_featureIterator->end();
}

//! Get the current feature.  Guaranteed to be non-null if called correctly.
boost::intrusive_ptr< IFeature > FeatureCollectionCache::IntersectionIterator::getFeature() const
{
	assert(this->m_featureCollectionCache);
	assert(this->m_featureCollectionCache->m_featureCollection);

	if (this->m_keys.getIsEmpty())
	{
		return this->m_featureIterator->getFeature();
	}

	// Get the key.
	size_t const key(this->m_keys.getFront());

	// Convert to feature ID.
	std::string featureID;
	this->m_featureCollectionCache->m_keyFeatureIDTranslator.getFeatureID(featureID, key);

	return this->m_featureCollectionCache->m_featureCollection->getFeature(featureID);
}

PYXPointer< FeatureCollectionCache > FeatureCollectionCache::create(
	boost::intrusive_ptr< IFeatureCollection const > featureCollection)
{
	return PYXNEW(FeatureCollectionCache, featureCollection);
}

// Constructs a feature set that wraps the given feature collection.
// The feature collection doesn't have to be fully constructed; this class
// just keeps a reference.
FeatureCollectionCache::FeatureCollectionCache(
	boost::intrusive_ptr< IFeatureCollection const > featureCollection) :
m_featureCollection(featureCollection),
m_gazetteer(),
m_keyFeatureIDTranslator()
{}

// Gets the feature with the given feature ID, if any, that 
// intersects the given geometry.
// FeatureType must fulfill the Region concept.
boost::intrusive_ptr< IFeature > FeatureCollectionCache::getFeature(
	std::string const & featureID, 
	Geometry const & intersectee)
{
	assert(this->m_featureCollection);

	size_t const key(this->m_keyFeatureIDTranslator.getKey(featureID));

	boost::logic::tribool const intersects(
		this->m_gazetteer.find(intersectee, key));
	if (intersects != false)
	{
		boost::intrusive_ptr< IFeature > feature(
			this->m_featureCollection->getFeature(featureID));
		if (intersects || 
			this->m_gazetteer.insert(intersectee, key, *feature->getRegion()))
		{
			return feature;
		}
	}
	return 0;
}
