#ifndef PYXIS__DATA__FEATURE_ITERATOR_WITH_PREFETCH_H
#define PYXIS__DATA__FEATURE_ITERATOR_WITH_PREFETCH_H
/******************************************************************************
feature_iterator_with_prefetch.h

begin		: 2011-01-25
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/feature_collection.h"
#include "pyxis/utility/object.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>

/*
! Helper class to speed up the feature iterator.

This feature Iterator uses a given iterator as an input. it create a backgroud thread to pre featch feature before the client ask them.
This way, if the machine have 2 cpus, one thread can be busy of generating the needed features while the main thread is busy processing them.
*/
class PYXLIB_DECL FeatureIteratorWithPrefetch : public FeatureIterator
{
private:
	PYXPointer<FeatureIterator>		m_iterator;
	boost::intrusive_ptr<IFeature>	m_feature;
	bool	m_end;	
	int		m_featuresRead;
	int		m_featuresPassed;

	class FetchingThread : public PYXObject
	{
	public:
		std::list<boost::intrusive_ptr<IFeature>> m_prefetchedFeatures;
		
		boost::thread				m_thread;
		mutable boost::mutex		m_mutex;

		boost::condition_variable	m_hasFeatures;
		boost::condition_variable	m_needFeatures;
	};

	PYXPointer<FetchingThread> m_fetchingThread;

public:

	static PYXPointer<FeatureIteratorWithPrefetch> create(const PYXPointer<FeatureIterator> & iterator)
	{
		return PYXNEW(FeatureIteratorWithPrefetch,iterator);
	}

	FeatureIteratorWithPrefetch(const PYXPointer<FeatureIterator> & iterator);

	virtual ~FeatureIteratorWithPrefetch();

	virtual bool end() const;

	virtual void next();

	virtual boost::intrusive_ptr<IFeature> getFeature() const;

protected:
	void fetchFeaturesThreadFunc();
};

#endif // guard
