/******************************************************************************
feature_iterator_with_prefetch.cpp

begin		: 2011-01-25
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/data/feature_iterator_with_prefetch.h"

////////////////////////////////////////////////////////////////////////////////
// FeatureIteratorWithPrefetch
////////////////////////////////////////////////////////////////////////////////


FeatureIteratorWithPrefetch::FeatureIteratorWithPrefetch(const PYXPointer<FeatureIterator> & iterator) 
	:	m_iterator(iterator),
		m_end(iterator->end()),
		m_featuresRead(0),
		m_featuresPassed(0)
{
	if (!m_end)
	{
		m_feature = iterator->getFeature();
		m_featuresRead++;
		m_featuresPassed++;

		m_fetchingThread = PYXNEW(FetchingThread);
		m_fetchingThread->m_thread = boost::thread(boost::bind(&FeatureIteratorWithPrefetch::fetchFeaturesThreadFunc,this));
	}
}

FeatureIteratorWithPrefetch::~FeatureIteratorWithPrefetch()
{
	if (m_end)
	{
		if (m_featuresRead != m_featuresPassed)
		{
			TRACE_ERROR("Not all features were passed to client (" << m_featuresRead << " read and only " << m_featuresPassed << " was passed)");
		}
	}

	m_end = true;

	if (m_fetchingThread)
	{
		m_fetchingThread->m_needFeatures.notify_one();
		m_fetchingThread->m_thread.join();
	}
}

bool FeatureIteratorWithPrefetch::end() const
{
	if (m_fetchingThread)
	{
		boost::mutex::scoped_lock lock(m_fetchingThread->m_mutex);
		return !m_feature;
	}
	else
	{
		return true;
	}
}

void FeatureIteratorWithPrefetch::next()
{
	if (m_fetchingThread)
	{
		boost::mutex::scoped_lock lock(m_fetchingThread->m_mutex);

		while (!m_end && m_fetchingThread->m_prefetchedFeatures.size() == 0)
		{
			m_fetchingThread->m_needFeatures.notify_one();
			m_fetchingThread->m_hasFeatures.wait(lock);
		}

		if (m_fetchingThread->m_prefetchedFeatures.size() > 0)
		{
			m_feature = m_fetchingThread->m_prefetchedFeatures.front();
			m_fetchingThread->m_prefetchedFeatures.pop_front();
			m_featuresPassed++;
		}
		else
		{
			m_feature.reset();
		}
	}

	m_fetchingThread->m_needFeatures.notify_one();
}

boost::intrusive_ptr<IFeature> FeatureIteratorWithPrefetch::getFeature() const 
{
	return m_feature;
}

void FeatureIteratorWithPrefetch::fetchFeaturesThreadFunc()
{
	try
	{
		while (! m_end )
		{	
			m_iterator->next();

			if (m_iterator->end())
			{
				m_end = true;
				m_fetchingThread->m_hasFeatures.notify_one();
			}
			else
			{
				boost::mutex::scoped_lock lock(m_fetchingThread->m_mutex);
				m_featuresRead++;
				
				m_fetchingThread->m_prefetchedFeatures.push_back(m_iterator->getFeature());
				
				m_fetchingThread->m_hasFeatures.notify_one();

				//don't fetch to much if not yet needed...
				if (m_fetchingThread->m_prefetchedFeatures.size()>10)
				{
					m_fetchingThread->m_needFeatures.wait(lock);
				}
			}
		}
	}
	catch(...)
	{
		TRACE_ERROR("fetchFeaturesThreadFunc has died. this should never happen");
	}
}