/******************************************************************************
feature_iterator_linq.cpp

begin		: 2013-01-09
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE 
#include "Stdafx.h"
#include "pyxis/data/feature_iterator_linq.h"

#include "boost/bind.hpp"

class PYXSingleFeatureAsIterator : public FeatureIterator
{
public:
	static PYXPointer<PYXSingleFeatureAsIterator> create(const boost::intrusive_ptr<IFeature> & feature)
	{
		return PYXNEW(PYXSingleFeatureAsIterator,feature);
	}

	PYXSingleFeatureAsIterator(const boost::intrusive_ptr<IFeature> & feature) : m_feature(feature), m_end(false)
	{
	}

public:
	virtual bool end() const 
	{
		return m_end;
	}

	virtual void next()
	{
		m_end = true;
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return m_feature;
	}

private:
	boost::intrusive_ptr<IFeature> m_feature;
	bool m_end;	
};

class PYXTakeFeatureIterator : public FeatureIterator
{
public:
	static PYXPointer<PYXTakeFeatureIterator> create(PYXPointer<FeatureIterator> iterator,int amount)
	{
		return PYXNEW(PYXTakeFeatureIterator,iterator,amount);
	}

	PYXTakeFeatureIterator(PYXPointer<FeatureIterator> iterator,int amount) : m_iterator(iterator), m_amount(amount), m_current(0)
	{
	}

public:
	virtual bool end() const 
	{
		return m_iterator->end() || m_current == m_amount;
	}

	virtual void next()
	{
		if (!end())
		{
			m_iterator->next();
			++m_current;
		}
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return m_iterator->getFeature();
	}

private:
	PYXPointer<FeatureIterator> m_iterator;
	int m_amount;
	int m_current;
};

class PYXSkipFeatureIterator : public FeatureIterator
{
public:
	static PYXPointer<PYXSkipFeatureIterator> create(PYXPointer<FeatureIterator> iterator,int amount)
	{
		return PYXNEW(PYXSkipFeatureIterator,iterator,amount);
	}

	PYXSkipFeatureIterator(PYXPointer<FeatureIterator> iterator,int amount) : m_iterator(iterator)
	{
		for(int i=0;i<amount && !m_iterator->end();i++)
		{
			m_iterator->next();
		}
	}

public:
	virtual bool end() const 
	{
		return m_iterator->end();
	}

	virtual void next()
	{
		if (!end())
		{
			m_iterator->next();
		}
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return m_iterator->getFeature();
	}

private:
	PYXPointer<FeatureIterator> m_iterator;
};

class PYXFilterFeatureIterator : public FeatureIterator
{
public:
	static PYXPointer<PYXFilterFeatureIterator> create(PYXPointer<FeatureIterator> iterator,boost::function< bool (PYXPointer<IFeature>) > filterFunction)
	{
		return PYXNEW(PYXFilterFeatureIterator,iterator,filterFunction);
	}

	PYXFilterFeatureIterator(PYXPointer<FeatureIterator> iterator,boost::function< bool (PYXPointer<IFeature>) > filterFunction) : m_iterator(iterator), m_filter(filterFunction)
	{
		while (!m_iterator->end() && !m_filter(m_iterator->getFeature()))
		{
			m_iterator->next();
		}
	}

public:
	virtual bool end() const 
	{
		return m_iterator->end();
	}

	virtual void next()
	{
		m_iterator->next();
		while (!m_iterator->end() && !m_filter(m_iterator->getFeature()))
		{
			m_iterator->next();
		}
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return m_iterator->getFeature();
	}

private:
	PYXPointer<FeatureIterator> m_iterator;
	boost::function< bool (PYXPointer<IFeature>) > m_filter;
};


class PYXSelectManyFeatureIterator : public FeatureIterator
{
public:
	static PYXPointer<PYXSelectManyFeatureIterator> create(PYXPointer<FeatureIterator> iterator,boost::function< PYXPointer<FeatureIterator> (const boost::intrusive_ptr<IFeature> &) > populateFunction)
	{
		return PYXNEW(PYXSelectManyFeatureIterator,iterator,populateFunction);
	}

	PYXSelectManyFeatureIterator(PYXPointer<FeatureIterator> iterator,boost::function< PYXPointer<FeatureIterator> (const boost::intrusive_ptr<IFeature> &) > populateFunction) : m_iterator(iterator), m_populateFunction(populateFunction)
	{
		if (!m_iterator->end())
		{
			m_itemIterator = m_populateFunction(m_iterator->getFeature());
		}
		//pop iterator until we get non empty iterator
		while((!m_itemIterator || m_itemIterator->end()) && !m_iterator->end() )
		{
			m_itemIterator.reset();
			m_iterator->next();
			if (!m_iterator->end())
			{
				m_itemIterator = m_populateFunction(m_iterator->getFeature());
			}
		}
	}

public:
	virtual bool end() const 
	{
		return (!m_itemIterator || m_itemIterator->end()) && m_iterator->end();
	}

	virtual void next()
	{
		if (end())
		{
			return;
		}

		if (m_itemIterator && !m_itemIterator->end())
		{
			m_itemIterator->next();
		}

		//pop iterator until we get non empty iterator
		while((!m_itemIterator || m_itemIterator->end()) && !m_iterator->end() )
		{
			m_itemIterator.reset();
			m_iterator->next();
			if (!m_iterator->end())
			{
				m_itemIterator = m_populateFunction(m_iterator->getFeature());
			}
		}
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return m_itemIterator ? m_itemIterator->getFeature() : NULL;
	}

private:
	PYXPointer<FeatureIterator> m_iterator;
	boost::function< PYXPointer<FeatureIterator> (const boost::intrusive_ptr<IFeature> &) > m_populateFunction;
	PYXPointer<FeatureIterator> m_itemIterator;
};

class PYXOrderByFeatureIterator : public FeatureIterator
{
public:
	static PYXPointer<PYXOrderByFeatureIterator> create(
		PYXPointer<FeatureIterator> iterator,
		boost::function< bool (PYXPointer<IFeature>,PYXPointer<IFeature>) > orderFunction)
	{
		return PYXNEW(PYXOrderByFeatureIterator,iterator,orderFunction);
	}

	PYXOrderByFeatureIterator(
		PYXPointer<FeatureIterator> iterator,
		boost::function< bool (PYXPointer<IFeature>,PYXPointer<IFeature>) > orderFunction) : m_orderFunction(orderFunction)
	{
		while(!iterator->end())
		{
			m_orderedFeatures.push_back(iterator->getFeature());
			iterator->next();
		}

		m_orderedFeatures.sort(orderFunction);
		m_feature = m_orderedFeatures.begin();
	}

public:
	virtual bool end() const 
	{
		return m_feature == m_orderedFeatures.end();
	}

	virtual void next()
	{
		++m_feature;
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return *m_feature;
	}

private:
	std::list<boost::intrusive_ptr<IFeature>> m_orderedFeatures;
	std::list<boost::intrusive_ptr<IFeature>>::iterator m_feature;
	boost::function< bool (PYXPointer<IFeature>,PYXPointer<IFeature>) > m_orderFunction;
};


class PYXSmoothTripFeatureIterator : public FeatureIterator
{
public:
	static PYXPointer<PYXSmoothTripFeatureIterator> create(PYXPointer<FeatureIterator> iterator,PYXCoord3DDouble startLocation)
	{
		return PYXNEW(PYXSmoothTripFeatureIterator,iterator,startLocation);
	}

	PYXSmoothTripFeatureIterator(PYXPointer<FeatureIterator> iterator,PYXCoord3DDouble startLocation)
	{
		std::list<boost::intrusive_ptr<IFeature>> features;
		while(!iterator->end())
		{
			features.push_back(iterator->getFeature());
			iterator->next();
		}

		PYXCoord3DDouble location = startLocation;
		while(!features.empty())
		{
			m_orderedFeatures.push_back(popNearestFeature(features,location));
		}

		m_feature = m_orderedFeatures.begin();
	}

private:
	boost::intrusive_ptr<IFeature> popNearestFeature(std::list<boost::intrusive_ptr<IFeature>> & features,PYXCoord3DDouble & location)
	{
		//find nearest feature
		std::list<boost::intrusive_ptr<IFeature>>::iterator iterator = features.begin();
		std::list<boost::intrusive_ptr<IFeature>>::iterator selected = features.begin();
		double selectedDistance = SphereMath::distanceBetween(location,(*selected)->getGeometry()->getBoundingCircle().getCenter());
		++iterator;
		while(iterator != features.end())
		{
			double distance = SphereMath::distanceBetween(location,(*iterator)->getGeometry()->getBoundingCircle().getCenter());

			if (distance < selectedDistance)
			{
				selectedDistance = distance;
				selected = iterator;
			}
			++iterator;
		}

		boost::intrusive_ptr<IFeature> feature = *selected;

		//pop the feature
		features.erase(selected);
		//update the location to the selected feature - so next time we would select nearest feature to the feature we just selected
		location = feature->getGeometry()->getBoundingCircle().getCenter();

		return feature;
	}

public:
	virtual bool end() const 
	{
		return m_feature == m_orderedFeatures.end();
	}

	virtual void next()
	{
		++m_feature;
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return *m_feature;
	}

private:
	std::list<boost::intrusive_ptr<IFeature>> m_orderedFeatures;
	std::list<boost::intrusive_ptr<IFeature>>::iterator m_feature;	
};

PYXFeatureIteratorLinq::PYXFeatureIteratorLinq(const boost::intrusive_ptr<IFeature> & feature)
{
	m_iterator = PYXSingleFeatureAsIterator::create(feature);
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::single() const
{
	return take(1);
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::take(int amount) const
{
	return PYXFeatureIteratorLinq(PYXTakeFeatureIterator::create(m_iterator,amount));
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::skip(int amount) const
{
	return PYXFeatureIteratorLinq(PYXSkipFeatureIterator::create(m_iterator,amount));
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::orderBy(boost::function< bool (PYXPointer<IFeature>,PYXPointer<IFeature>) > orderFunction) const
{
	return PYXFeatureIteratorLinq(PYXOrderByFeatureIterator::create(m_iterator,orderFunction));
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::filter(boost::function< bool (PYXPointer<IFeature>) > filterFunction) const
{
	return PYXFeatureIteratorLinq(PYXFilterFeatureIterator::create(m_iterator,filterFunction));
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::filter(const PYXGeometry & geometry) const
{
	return filter(geometry.clone());
}

bool geometryFilter(PYXPointer<IFeature> feature,PYXPointer<PYXGeometry> geometry)
{
	return feature->getGeometry()->intersects(*geometry);
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::filter(const PYXPointer<PYXGeometry> & geometry) const
{
	return filter(boost::bind(geometryFilter,_1,geometry));
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::selectMany(boost::function< PYXPointer<FeatureIterator> (const boost::intrusive_ptr<IFeature> &) > populateFunction) const
{
	return PYXFeatureIteratorLinq(PYXSelectManyFeatureIterator::create(m_iterator,populateFunction));
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::expandGroupToGeometry( PYXPointer<IFeature> feature, const PYXPointer<PYXGeometry> & geometry )
{
	auto featureGroup = feature->QueryInterface<IFeatureGroup>();
	if (featureGroup)
	{
		int groupResolution = featureGroup->getGeometry()->getCellResolution();
 		int geometryResolution = geometry->getCellResolution();
		// the resolution of the root group is the native resolution of the geometry following condition will skip the first group:
		if( groupResolution > geometryResolution + 3)
		{
			groupResolution = PYXBoundingCircle::estimateResolutionFromRadius(featureGroup->getGeometry()->getBoundingCircle().getRadius());
		}

		// if the required resolution is less than groups resolution return the group's geometry.
		if( groupResolution < geometryResolution + 3)
		{
			return PYXFeatureIteratorLinq(featureGroup->getGroupIterator(*geometry)).selectMany(boost::bind(&PYXFeatureIteratorLinq::expandGroupToGeometry,_1, geometry));
		}
	}
	return PYXFeatureIteratorLinq(feature);
	
}

PYXFeatureIteratorLinq PYXFeatureIteratorLinq::orderForTrip(PYXCoord3DDouble startLocation) const
{
	return PYXFeatureIteratorLinq(PYXSmoothTripFeatureIterator::create(m_iterator,startLocation));
}