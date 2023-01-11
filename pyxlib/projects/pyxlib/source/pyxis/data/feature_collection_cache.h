#ifndef PYXIS__DATA__FEATURE_COLLECTION_CACHE_H
#define PYXIS__DATA__FEATURE_COLLECTION_CACHE_H

#include "pyxis/data/feature_collection.h"

#include <pyxis/grid/dodecahedral/tree.hpp>

#include <boost/bimap.hpp>
#include <boost/intrusive_ptr.hpp>

/* TODO:
-	Handle mutating feature collection: when pulling key from gazetteer,
	double-check the feature region to make sure it is the same first
-	Test
*/
// A wrapper for a feature collection that adds fast spatial querying.
class PYXLIB_DECL FeatureCollectionCache : public PYXObject
{
	typedef Pyxis::Grid::Dodecahedral::Tree Tree;
	typedef Pyxis::Grid::Gazetteer< Tree, size_t > Gazetteer;

	class KeyFeatureIDTranslator
	{
		typedef boost::bimap< size_t, std::string > KeyFeatureIDBimap;
		KeyFeatureIDBimap keyFeatureIDBimap;
	public:
		size_t getKey(std::string const & featureID)
		{
			KeyFeatureIDBimap::right_map::const_iterator iterator = this->keyFeatureIDBimap.right.find(featureID);
			if (iterator == this->keyFeatureIDBimap.right.end())
			{
				size_t key = this->keyFeatureIDBimap.size();
				this->keyFeatureIDBimap.insert(KeyFeatureIDBimap::value_type(key, featureID));
				return key;
			}
			return iterator->second;
		}
		std::string & getFeatureID(std::string & featureID, size_t key) const
		{
			KeyFeatureIDBimap::left_map::const_iterator iterator = this->keyFeatureIDBimap.left.find(key);
			assert(iterator != this->keyFeatureIDBimap.left.end() && "We shouldn't be looking for a key we haven't inserted yet.");
			return (featureID = iterator->second);
		}
	};

	boost::intrusive_ptr< IFeatureCollection const > m_featureCollection;

	Gazetteer m_gazetteer;

	// Translates between keys and feature IDs.
	KeyFeatureIDTranslator m_keyFeatureIDTranslator;

public:

	typedef Pyxis::Grid::Geometry< Tree > Geometry;

	//! An iterator of features that intersect a given geometry.
	class PYXLIB_DECL IntersectionIterator : public FeatureIterator
	{
		PYXPointer< FeatureCollectionCache > m_featureCollectionCache;

		// The feature iterator.
		PYXPointer< FeatureIterator > m_featureIterator;

		// The tree geometry that results must intersect.
		boost::intrusive_ptr< Geometry const > m_geometry;

		// The keys visited so far.
		Pyxis::Set< size_t > m_visited;

		// The range of keys in the gazetteer.
		Gazetteer::Keys m_keys;

		// Inserts the base class's current feature (if there is one) into
		// the gazetteer.  If it doesn't intersect the geometry
		// or it has already been visited, advance to the next one and repeat.
		void insert();

	public:

		//! Constructs the iterator dynamically.
		/*! It is unsafe to modify the geometry during the lifetime of this object. */
		static PYXPointer< IntersectionIterator > STDMETHODCALLTYPE create(
			PYXPointer< FeatureCollectionCache > featureCollectionCache,
			boost::intrusive_ptr< Geometry const > geometry);

		//! Constructs the iterator.
		/*! It is unsafe to modify the geometry during the lifetime of this object. */
		explicit STDMETHODCALLTYPE IntersectionIterator(
			PYXPointer< FeatureCollectionCache > featureCollectionCache,
			boost::intrusive_ptr< Geometry const > geometry);

	public: // FeatureIterator

		//! Move to the next feature.
		void next();

		//! The end condition test.
		bool end() const;

		//! Get the current feature.  Guaranteed to be non-null if called correctly.
		boost::intrusive_ptr< IFeature > getFeature() const;
	};

	static PYXPointer< FeatureCollectionCache > STDMETHODCALLTYPE create(
		boost::intrusive_ptr< IFeatureCollection const > featureCollection);

	// Constructs a feature set that wraps the given feature collection.
	// The feature collection doesn't have to be fully constructed; this class
	// just keeps a reference.
	explicit STDMETHODCALLTYPE FeatureCollectionCache(
		boost::intrusive_ptr< IFeatureCollection const > featureCollection);

	// Gets the feature with the given feature ID, if any, that 
	// intersects the given geometry.
	// FeatureType must fulfill the Region concept.
	boost::intrusive_ptr< IFeature > STDMETHODCALLTYPE getFeature(
		std::string const & featureID, 
		Geometry const & intersectee);
};

#endif // guard
