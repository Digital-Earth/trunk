#if !defined(PYXIS__GLOBE__GAZETTEER)
#define PYXIS__GLOBE__GAZETTEER

namespace Pyxis
{
	struct Empty;

	namespace Globe
	{
		// The Key is the type of key in the feature map that this gazetteer wraps,
		// and Value is the type of value in the feature.
		template < typename Key, typename Value > class Gazetteer;
	}
}

#include "pyxis/globe/feature.hpp"
#include "pyxis/globe/raster.hpp"
#include "pyxis/globe/tile.hpp"
#include "pyxis/intrusive_functor_reference.hpp"
#include "pyxis/map.hpp"
#include <boost/thread.hpp>
#include <queue>

template < typename Key, typename Value >
class Pyxis::Globe::Gazetteer :
public virtual Pointee,
boost::noncopyable // TODO (mutex is noncopyable)
{
public:

	typedef boost::intrusive_ptr< Feature< Value > const > Feature;
	typedef MapInterface< Key, Feature > KeyFeatureMap;
	typedef boost::compressed_pair< Key, Feature > KeyFeaturePair;

private:

	// A thread-safe cache.
	// TODO: This doesn't include resolution-spanning logic.
	class Cache :
	boost::noncopyable // Mutex is noncopyable.
	{
		// Cache of intersecting features.
		mutable boost::mutex intersectingFeatureCacheMutex;
		Multimap< Tile const &, Key > intersectingFeatureCache;

		// Cache of non-intersecting features.
		mutable boost::mutex nonIntersectingFeatureCacheMutex;
		Multimap< Tile const &, Key > nonIntersectingFeatureCache;

		void insertIntersection(Raster const & raster, Key key)
		{
			Raster::Tiles tiles(raster);
			if (!tiles.getIsEmpty())
			{
				boost::mutex::scoped_lock lock(intersectingFeatureCacheMutex);

				do
				{
					intersectingFeatureCache.insert(tiles.getFront(), key);
					tiles.popFront();
				} while (!tiles.getIsEmpty());
			}
		}

	public:

		class Test;

		Cache() :
		intersectingFeatureCacheMutex(), intersectingFeatureCache(),
		nonIntersectingFeatureCacheMutex(), nonIntersectingFeatureCache()
		{}

		void insertCompleteIntersection(Raster const & raster, Key key)
		{
			insertIntersection(raster, key);
		}

		void insertPartialIntersection(Raster const & raster, Key key)
		{
			insertIntersection(raster, key);
		}
		
		void insertNonIntersection(Raster const & raster, Key key)
		{
			Raster::Tiles tiles(raster);
			if (!tiles.getIsEmpty())
			{
				boost::mutex::scoped_lock lock(nonIntersectingFeatureCacheMutex);

				do
				{
					nonIntersectingFeatureCache.insert(tiles.getFront(), key);
					tiles.popFront();
				} while (!tiles.getIsEmpty());
			}
		}

		// Visits partial and complete intersections.
		bool visitIntersections(Raster const & raster,
			MutableSetInterface< Key > & results,
			FunctorInterface< bool, Key > & callback) const
		{
			Raster::Tiles tiles(raster);
			if (!tiles.getIsEmpty())
			{
				boost::mutex::scoped_lock lock(intersectingFeatureCacheMutex);

				do
				{
					if (!intersectingFeatureCache.visit(tiles.getFront(), results, callback, true))
					{
						return false;
					}
					tiles.popFront();
				} while (!tiles.getIsEmpty());
			}
			return true;
		}

		// Finds non-intersections.
		MutableSetInterface< Key > & findNonIntersections(Raster const & raster,
			MutableSetInterface< Key > & results) const
		{
			Raster::Tiles tiles(raster);
			if (!tiles.getIsEmpty())
			{
				boost::mutex::scoped_lock lock(nonIntersectingFeatureCacheMutex);

				// Find values mapped to entire first tile, and populate results set directly.
				nonIntersectingFeatureCache.find(tiles.getFront(), results, false);

				// For each remaining tile: intersect results with values mapped to entire tile.
				for (; ; )
				{
					tiles.popFront();
					if (tiles.getIsEmpty()) { break; }
					Set< Key > intersectingValues;
					nonIntersectingFeatureCache.find(tiles.getFront(), intersectingValues, false);
					results.intersect(intersectingValues);
				}
			}
			return results;
		}
	};

	// A query for features that intersect a raster.
	// This class is not required to be re-entrant;
	// each instance is only accessed by one thread.
	class FeatureQuery :
	public FunctorInterface< bool, KeyFeaturePair >
	{
		// Helper for constructor.
		class IntersectionCallback : public FunctorInterface< bool, Key >
		{
			Gazetteer const & gazetteer;
			FunctorInterface< bool, KeyFeaturePair > & callback;

		public:

			explicit IntersectionCallback(
				Gazetteer const & gazetteer,
				FunctorInterface< bool, KeyFeaturePair > & callback) :
			gazetteer(gazetteer), callback(callback)
			{}

			bool operator ()(Key key)
			{
				assert(gazetteer.features);
				return callback(KeyFeaturePair(key, gazetteer.features->getValue(key)));
			}
		};

		Gazetteer const & gazetteer;
		Raster const & raster;
		FunctorInterface< bool, KeyFeaturePair > & callback;
		Set< Key > visitedCachedFeatures;
		size_t visitedFeatureCount;
		bool completed;

	public:

		// Constructs and executes the query.
		// Virtual calls are safe, because this class has no derived classes.
		FeatureQuery(Gazetteer const & gazetteer,
			Raster const & raster,
			FunctorInterface< bool, KeyFeaturePair > & callback) :
		gazetteer(gazetteer),
		raster(raster),
		callback(callback),
		visitedCachedFeatures(),
		visitedFeatureCount(),
		completed(false)
		{
			assert(gazetteer.features);
			if (!gazetteer.features->getIsEmpty())
			{
				// For each cached intersection, add to "visited features" set
				// and do callback.  Return false if the callback returns false.
				IntersectionCallback intersectionCallback(gazetteer, callback);
				if (!gazetteer.cache.visitIntersections(
						raster, visitedCachedFeatures, intersectionCallback))
				{
					return;
				}

				// Get all cached non-intersections that cover the whole raster,
				// and add them to the "visited features" set.
				// This operation must be locked so that an invalidating insertion doesn't
				// happen in the middle.
				gazetteer.cache.findNonIntersections(raster, visitedCachedFeatures);

				// If there are features remaining that haven't been processed,
				// visit them and process.
				size_t featureCount;
				{
					boost::mutex::scoped_lock lock(gazetteer.featureCountMutex);
					featureCount = gazetteer.featureCount;
				}
				assert(!featureCount || visitedCachedFeatures.getCount() <= featureCount);
				if (!featureCount || visitedCachedFeatures.getCount() < featureCount)
				{
					// Visit the features.
					// Since the KeyFeatureMap is const, and we use a thread-safe mutables policy,
					// no lock is required;
					// this allows results to come back from multiple queries in parallel.
					// The callback is defined in this class, and is re-entrant.
					if (!gazetteer.features->visit(*this))
					{
						return;
					}

					// Update the feature count.
					{
						boost::mutex::scoped_lock lock(gazetteer.featureCountMutex);
						assert(!gazetteer.featureCount || gazetteer.featureCount == visitedFeatureCount);
						gazetteer.featureCount = visitedFeatureCount;
					}
				}
			}
			completed = true;
		}

		// Callback for each key/feature pair in the key feature map.
		// This is called only once for each during this query.
		bool operator ()(KeyFeaturePair keyFeaturePair)
		{
			assert(!this->completed);
			assert(this->gazetteer.features);
			assert(!this->gazetteer.features->getIsEmpty());

			++visitedFeatureCount;

			Key key = keyFeaturePair.first();
			if (!visitedCachedFeatures.find(key))
			{
				Raster completeIntersectionRaster(raster.getResolution());
				Raster partialIntersectionRaster(raster.getResolution());
				Raster unknownRaster(raster.getResolution());

				Feature feature = keyFeaturePair.second();
				assert(feature);
				boost::logic::tribool const intersects = feature->getRegion().getIntersection(
					raster,
					&completeIntersectionRaster, &partialIntersectionRaster, &unknownRaster);

				// Determine non-intersecting raster, and insert it into the cache.
				Raster nonIntersectingRaster(raster);
				nonIntersectingRaster.remove(completeIntersectionRaster);
				nonIntersectingRaster.remove(partialIntersectionRaster);
				nonIntersectingRaster.remove(unknownRaster);
				gazetteer.cache.insertNonIntersection(nonIntersectingRaster, key);

				// If it intersects, insert intersecting raster into the cache and do callback.
				// If it returns false, return false.
				if (intersects)
				{
					gazetteer.cache.insertCompleteIntersection(completeIntersectionRaster, key);
					gazetteer.cache.insertPartialIntersection(partialIntersectionRaster, key);
					if (!callback(keyFeaturePair))
					{
						return false;
					}
				}
			}
			return true;
		}

		operator bool() const
		{
			return completed;
		}
	};

	// The set of features.
	// Because the list is immutable, no deep copy is required for copy constructor.
	// Because it is const, no mutex is required due to "thread-safe mutables" policy.
	boost::intrusive_ptr< KeyFeatureMap const > features;

	// The feature count.  If 0, and !features.getIsEmpty(), it has not been computed.
	// We cannot use boost::detail::atomic_count because it doesn't support assignment.
	mutable boost::mutex featureCountMutex;
	mutable size_t featureCount;

	// The intersection cache.
	mutable Cache cache;

public:

	class Test;

	// A range representing the features intersecting a raster.
	class Features : ForwardRangeInterface< KeyFeaturePair >
	{
		// A thread worker that adds results to the queue.
		// This is also used as the gazetteer callback, when a feature is found.
		class Worker :
		boost::noncopyable,
		public FunctorInterface< bool, KeyFeaturePair > // For gazetteer to call.
		{
			Gazetteer const & gazetteer;
			Raster raster;
	 		boost::mutex & queueMutex;
	 		std::queue< KeyFeaturePair > & queue;
			boost::condition_variable & condition; // Notifies on push and finish (one or more).
			boost::detail::atomic_count isFinished;

		public:

			// The objects referred to must be alived for the lifetime of this worker,
			// which is on a different thread.
			// Swaps in the raster.
			explicit Worker(
				Gazetteer const & gazetteer,
				Raster & raster,
				boost::mutex & queueMutex,
				std::queue< KeyFeaturePair > & queue,
				boost::condition_variable & condition) :
			gazetteer(gazetteer),
			raster(raster.getResolution()),
			queueMutex(queueMutex),
			queue(queue),
			condition(condition),
			isFinished(0)
			{
				this->raster.swap(raster);
			}

			void setIsFinished()
			{
				// This may fire more than once, if multiple threads flag finish.
				++isFinished;
		        condition.notify_one(); // There is only ever a maximum of one thread waiting.
			}

			bool getIsFinished() const
			{
				return !!isFinished;
			}

			// Called by the thread constructor.
			void operator ()()
			{
				// Gazetteer::visitFeatures is re-entrant, so no lock is required.
				gazetteer.visitFeatures(raster, *this);

				setIsFinished();
			}

			// Called by the gazetteer, serially, when a feature is found.
			bool operator ()(KeyFeaturePair keyFeaturePair)
			{
				if (isFinished) return false; // Avoid a push if we're already finished.
				{
					boost::mutex::scoped_lock lock(queueMutex);
					queue.push(keyFeaturePair);
				}
		        condition.notify_one(); // There is only ever a maximum of one thread waiting.
				return !isFinished;
			}
		};

 		mutable boost::mutex queueMutex;
 		mutable std::queue< KeyFeaturePair > queue;

		mutable boost::condition_variable condition;
		Worker worker;
		boost::thread thread; // This lifetime of this thread is contained by that of this object.

	public:

		// The gazetteer and the raster must be alive for the lifetime of this object.
		// Swaps in the raster.
		explicit Features(
			Gazetteer const & gazetteer,
			Raster & raster) :
		queueMutex(), queue(),
		condition(),
		worker(gazetteer, raster, queueMutex, queue, condition),
		thread(boost::ref(worker))
		{}

		~Features()
		{
			// Set the finished flag, used by the thread.
			worker.setIsFinished();

			// Wait until the thread finishes.  This guarantees that
			// everything the thread references stays in scope.
			try
			{
				thread.join();
			} catch (boost::thread_interrupted const &)
			{
			} catch (...)
			{
				// Should not throw any other exceptions.
				assert(0);
			}
		}

		bool getIsEmpty() const
		{
			boost::mutex::scoped_lock lock(queueMutex);
			while (queue.empty())
			{
				if (worker.getIsFinished()) return queue.empty();
	            condition.wait(lock);
	        }
	        return false;
		}

		// Asserts non-empty.
		KeyFeaturePair getFront() const
		{
	        boost::mutex::scoped_lock lock(queueMutex);
	        while (queue.empty())
	        {
	        	assert(!worker.getIsFinished() && "The range is empty.");
	            condition.wait(lock);
	        }
	        return queue.front();
		}

		// Asserts non-empty.
		void popFront()
		{
	        boost::mutex::scoped_lock lock(queueMutex);
	        while (queue.empty())
	        {
	        	assert(!worker.getIsFinished() && "The range is empty.");
	            condition.wait(lock);
	        }
	        queue.pop();
		}
	};

	// Constructs the gazetteer.
	// Very lightweight; doesn't do any indexing until required.
	// Takes ownership of the feature map.
	// This feature map may be accessed from multiple threads without a lock,
	// but because it is const, only the mutables must be thread-safe.
	explicit Gazetteer(std::auto_ptr< KeyFeatureMap const > features) :
	features(features.release()),
	featureCountMutex(), featureCount(),
	cache()
	{
		if (!this->features)
		{
			throw std::invalid_argument("The feature map cannot be null.");
		}
	}

	// Visits the features that intersect the given raster.
	// This is re-entrant, so can be called from multiple threads.
	// The callback passed to this function must also be re-entrant.
	bool visitFeatures(Raster const & raster,
		FunctorInterface< bool, KeyFeaturePair > & callback) const
	{
		return FeatureQuery(*this, raster, callback);
	}
};

template < typename Key, typename Value >
class Pyxis::Globe::Gazetteer< Key, Value >::Cache::Test
{
	static bool testInsertCompleteIntersection(Tree::Index const & subtree, Tree::Level depth)
	{
		// TODO
		(void)subtree;
		(void)depth;
		return true;
	}

	static bool testInsertPartialIntersection(Tree::Index const & subtree, Tree::Level depth)
	{
		// TODO
		(void)subtree;
		(void)depth;
		return true;
	}

	static bool testInsertNonIntersection(Tree::Index const & subtree, Tree::Level depth)
	{
		// TODO
		(void)subtree;
		(void)depth;
		return true;
	}

	Tree::Index centerIndex;
	Tree::Index vertexIndex;
	Tree::Level depth;

public:

	explicit Test() :
	centerIndex("10400"), vertexIndex("10401"), depth(5)
	{}

	operator bool() const
	{
		if (!testInsertCompleteIntersection(this->centerIndex, this->depth)) return false;
		if (!testInsertCompleteIntersection(this->vertexIndex, this->depth)) return false;

		if (!testInsertPartialIntersection(this->centerIndex, this->depth)) return false;
		if (!testInsertPartialIntersection(this->vertexIndex, this->depth)) return false;

		if (!testInsertNonIntersection(this->centerIndex, this->depth)) return false;
		if (!testInsertNonIntersection(this->vertexIndex, this->depth)) return false;

		return true;
	}
};

// TODO: Proper unit tests
template < typename Key, typename Value >
class Pyxis::Globe::Gazetteer< Key, Value >::Test
{
public:
	operator bool() const
	{
		if (!(typename Cache::Test()))
		{
			assert(0);
			return false;
		}

		Gazetteer gazetteer(
			std::auto_ptr< KeyFeatureMap const >(new Map< size_t, Feature >()));

		Raster raster(Resolution(5));
		// TODO: Populate raster.

		struct Predicate : FunctorInterface< bool, KeyFeaturePair >
		{
			bool operator ()(KeyFeaturePair argument)
			{
				// TODO: Do stuff.
				return true;
			}
		} callback;

		bool complete = gazetteer.visitFeatures(raster, callback);
		(void)complete;

		return true;
	}
};

#endif
