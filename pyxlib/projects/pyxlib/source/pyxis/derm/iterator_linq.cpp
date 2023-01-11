/******************************************************************************
iterator_linq.cpp

begin		: 2013-01-09
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE 
#include "Stdafx.h"
#include "pyxis/derm/iterator_linq.h"
#include "pyxis/derm/iterator_linq.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/derm/neighbour_iterator.h"

#include "boost/bind.hpp"

//////////////////////////////////////////////////////////////////////////
// PYXGeometrySafeIterator
//////////////////////////////////////////////////////////////////////////

class PYXGeometrySafeIterator : public PYXIterator
{
public:
	static PYXPointer<PYXGeometrySafeIterator> create(const PYXPointer<PYXGeometry> & geometry)
	{
		return PYXNEW(PYXGeometrySafeIterator,geometry);
	}

	PYXGeometrySafeIterator(const PYXPointer<PYXGeometry> & geometry) : m_geometry(geometry)
	{
		m_iterator = m_geometry->getIterator();
	}

	virtual bool end() const { return m_iterator->end(); }
	virtual void next() { m_iterator->next(); }
	virtual const PYXIcosIndex& getIndex() const { return m_iterator->getIndex(); }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_iterator->getFieldValue(nFieldIndex); }

private:
	PYXPointer<PYXGeometry> m_geometry;
	PYXPointer<PYXIterator> m_iterator;
};

//////////////////////////////////////////////////////////////////////////
// PYXValueTileSafeIterator
//////////////////////////////////////////////////////////////////////////

class PYXValueTileSafeIterator : public PYXIterator
{
public:
	static PYXPointer<PYXValueTileSafeIterator> create(const PYXPointer<PYXValueTile> & valueTile)
	{
		return PYXNEW(PYXValueTileSafeIterator,valueTile);
	}

	PYXValueTileSafeIterator(const PYXPointer<PYXValueTile> & valueTile) : m_valueTile(valueTile),m_iterator(valueTile->getIterator())
	{
	}

	virtual bool end() const { return m_iterator->end(); }
	virtual void next() { m_iterator->next(); }
	virtual const PYXIcosIndex& getIndex() const { return m_iterator->getIndex(); }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_iterator->getFieldValue(nFieldIndex); }

private:
	PYXPointer<PYXValueTile> m_valueTile;
	PYXPointer<PYXIterator> m_iterator;
};

//////////////////////////////////////////////////////////////////////////
// PYXListIterator
//////////////////////////////////////////////////////////////////////////

class PYXListIterator : public PYXIterator
{
public:
	static PYXPointer<PYXListIterator> create(const std::list<std::pair<PYXIcosIndex,PYXValue>> & list)
	{
		return PYXNEW(PYXListIterator,list);
	}

	PYXListIterator(const std::list<std::pair<PYXIcosIndex,PYXValue>> & list) : m_list(list)
	{
		m_iterator = m_list.begin();
	}

	virtual bool end() const { return m_iterator == m_list.end(); }
	virtual void next() { ++m_iterator; }
	virtual const PYXIcosIndex& getIndex() const { return m_iterator->first; }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_iterator->second; }

private:
	std::list<std::pair<PYXIcosIndex,PYXValue>> m_list;
	std::list<std::pair<PYXIcosIndex,PYXValue>>::iterator m_iterator;
};

//////////////////////////////////////////////////////////////////////////
// PYXValueGetterIterator
//////////////////////////////////////////////////////////////////////////

class PYXValueGetterIterator : public PYXIterator
{
public:
	static PYXPointer<PYXValueGetterIterator> create(const boost::intrusive_ptr<ICoverage> & coverage,const PYXPointer<PYXIterator> & iterator)
	{
		return PYXNEW(PYXValueGetterIterator,coverage,iterator);
	}

	PYXValueGetterIterator(const boost::intrusive_ptr<ICoverage> & coverage,const PYXPointer<PYXIterator> & iterator) : m_coverage(coverage),m_iterator(iterator)
	{
	}

	virtual bool end() const { return m_iterator->end(); }
	virtual void next() { m_iterator->next(); }
	virtual const PYXIcosIndex& getIndex() const { return m_iterator->getIndex(); }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_coverage->getCoverageValue(m_iterator->getIndex(),nFieldIndex); }

private:
	boost::intrusive_ptr<ICoverage> m_coverage;
	PYXPointer<PYXIterator> m_iterator;
};

//////////////////////////////////////////////////////////////////////////
// PYXApplyBeforeFunctionIterator
//////////////////////////////////////////////////////////////////////////

class PYXApplyBeforeFunctionIterator : public PYXIterator
{
public:
	static PYXPointer<PYXApplyBeforeFunctionIterator> create(const PYXPointer<PYXIterator> & iterator,const boost::function<void(const PYXIcosIndex &)> & applyFunction)
	{
		return PYXNEW(PYXApplyBeforeFunctionIterator,iterator,applyFunction);
	}

	PYXApplyBeforeFunctionIterator(const PYXPointer<PYXIterator> & iterator,const boost::function<void(const PYXIcosIndex &)> & applyFunction) : m_iterator(iterator),m_applyFunction(applyFunction)
	{
		if (!m_iterator->end())
		{
			m_applyFunction(m_iterator->getIndex());
		}
	}

	virtual bool end() const { return m_iterator->end(); }
	virtual void next()
	{ 
		m_iterator->next(); 
		if (!m_iterator->end())
		{
			m_applyFunction(m_iterator->getIndex());
		}
	}
	virtual const PYXIcosIndex& getIndex() const { return m_iterator->getIndex(); }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_iterator->getFieldValue(nFieldIndex); }

private:
	boost::function<void(const PYXIcosIndex &)> m_applyFunction;
	PYXPointer<PYXIterator> m_iterator;
};

//////////////////////////////////////////////////////////////////////////
// PYXTakeIterator
//////////////////////////////////////////////////////////////////////////

class PYXTakeIterator : public PYXIterator
{
public:
	static PYXPointer<PYXTakeIterator> create(const PYXPointer<PYXIterator> & iterator,int amount)
	{
		return PYXNEW(PYXTakeIterator,iterator,amount);
	}

	PYXTakeIterator(const PYXPointer<PYXIterator> & iterator,int amount) : m_iterator(iterator), m_amount(amount), m_current(0)
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

	virtual const PYXIcosIndex& getIndex() const { return m_iterator->getIndex(); }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_iterator->getFieldValue(nFieldIndex); }

private:
	PYXPointer<PYXIterator> m_iterator;
	int m_amount;
	int m_current;
};

//////////////////////////////////////////////////////////////////////////
// PYXSkipIterator
//////////////////////////////////////////////////////////////////////////

class PYXSkipIterator : public PYXIterator
{
public:
	static PYXPointer<PYXSkipIterator> create(const PYXPointer<PYXIterator> & iterator,int amount)
	{
		return PYXNEW(PYXSkipIterator,iterator,amount);
	}

	PYXSkipIterator(const PYXPointer<PYXIterator> & iterator,int amount) : m_iterator(iterator)
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
		m_iterator->next();		
	}

	virtual const PYXIcosIndex& getIndex() const { return m_iterator->getIndex(); }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_iterator->getFieldValue(nFieldIndex); }

private:
	PYXPointer<PYXIterator> m_iterator;	
};

//////////////////////////////////////////////////////////////////////////
// PYXFilterByIndexIterator
//////////////////////////////////////////////////////////////////////////

class PYXFilterByIndexIterator : public PYXIterator
{
public:
	static PYXPointer<PYXFilterByIndexIterator> create(const PYXPointer<PYXIterator> & iterator,const boost::function< bool (const PYXIcosIndex &) > & filterFunction)
	{
		return PYXNEW(PYXFilterByIndexIterator,iterator,filterFunction);
	}

	PYXFilterByIndexIterator(const PYXPointer<PYXIterator> & iterator,const boost::function< bool (const PYXIcosIndex &) > & filterFunction) : m_iterator(iterator), m_filter(filterFunction)
	{
		findNextMatch();
	}

public:
	virtual bool end() const 
	{
		return m_iterator->end();
	}

	virtual void next()
	{
		m_iterator->next();
		findNextMatch();
	}

	virtual const PYXIcosIndex& getIndex() const { return m_iterator->getIndex(); }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_iterator->getFieldValue(nFieldIndex); }
private:
	void findNextMatch()
	{
		while (!m_iterator->end() && !m_filter(m_iterator->getIndex()))
		{
			m_iterator->next();
		}
	}

private:
	PYXPointer<PYXIterator> m_iterator;
	boost::function< bool (const PYXIcosIndex &) > m_filter;
};

//////////////////////////////////////////////////////////////////////////
// PYXFilterByValueIterator
//////////////////////////////////////////////////////////////////////////

class PYXFilterByValueIterator : public PYXIterator
{
public:
	static PYXPointer<PYXFilterByValueIterator > create(const PYXPointer<PYXIterator> & iterator,const boost::function< bool (const PYXValue &) > & filterFunction,int nFieldIndex = 0)
	{
		return PYXNEW(PYXFilterByValueIterator ,iterator,filterFunction);
	}

	PYXFilterByValueIterator(const PYXPointer<PYXIterator> & iterator,const boost::function< bool (const PYXValue &) > & filterFunction,int nFieldIndex = 0) : m_iterator(iterator), m_filter(filterFunction), m_nFieldIndex(nFieldIndex)
	{
		findNextMatch();
	}

public:
	virtual bool end() const 
	{
		return m_iterator->end();
	}

	virtual void next()
	{
		m_iterator->next();
		findNextMatch();
	}

	virtual const PYXIcosIndex& getIndex() const { return m_iterator->getIndex(); }
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const { return m_iterator->getFieldValue(nFieldIndex); }

private:
	void findNextMatch()
	{
		while (!m_iterator->end() && !m_filter(m_iterator->getFieldValue(m_nFieldIndex)))
		{
			m_iterator->next();
		}
	}

private:
	int m_nFieldIndex;
	PYXPointer<PYXIterator> m_iterator;
	boost::function< bool (const PYXValue &) > m_filter;
};

//////////////////////////////////////////////////////////////////////////
// PYXSelectManyIterator
//////////////////////////////////////////////////////////////////////////

class PYXSelectManyIterator : public PYXIterator
{
public:
	static PYXPointer<PYXSelectManyIterator> create(PYXPointer<PYXIterator> iterator,const boost::function< PYXPointer<PYXIterator> (const PYXIcosIndex &) > & populateFunction)
	{
		return PYXNEW(PYXSelectManyIterator,iterator,populateFunction);
	}

	PYXSelectManyIterator(PYXPointer<PYXIterator> iterator,const boost::function< PYXPointer<PYXIterator> (const PYXIcosIndex &) > & populateFunction) : m_iterator(iterator), m_populateFunction(populateFunction)
	{
		if (!m_iterator->end())
		{
			m_itemIterator = m_populateFunction(m_iterator->getIndex());
		}
		//pop iterator until we get non empty iterator
		while((!m_itemIterator || m_itemIterator->end()) && !m_iterator->end() )
		{
			m_itemIterator.reset();
			m_iterator->next();
			if (!m_iterator->end())
			{
				m_itemIterator = m_populateFunction(m_iterator->getIndex());
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
				m_itemIterator = m_populateFunction(m_iterator->getIndex());
			}
		}
	}

	virtual const PYXIcosIndex& getIndex() const
	{
		return m_itemIterator ? m_itemIterator->getIndex() : PYXIterator::getNullIndex();
	}	
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const
	{
		return m_itemIterator ? m_itemIterator->getFieldValue(nFieldIndex ) : PYXValue();
	}

private:
	PYXPointer<PYXIterator> m_iterator;
	boost::function< PYXPointer<PYXIterator> (const PYXIcosIndex &) > m_populateFunction;
	PYXPointer<PYXIterator> m_itemIterator;
};

//////////////////////////////////////////////////////////////////////////
// FloodFillIterator
//////////////////////////////////////////////////////////////////////////

class FloodFillIterator : public PYXIterator
{
public:
	static PYXPointer<FloodFillIterator> create(PYXPointer<PYXIterator> iterator,const boost::function< bool (const PYXIcosIndex &,const PYXIcosIndex &) > & floodFunction)
	{
		return PYXNEW(FloodFillIterator,iterator,floodFunction);
	}

	FloodFillIterator(PYXPointer<PYXIterator> iterator,const boost::function< bool (const PYXIcosIndex &,const PYXIcosIndex &) > & floodFunction) : m_iterator(iterator), m_floodFunction(floodFunction)
	{
		m_visitedCells = PYXTileCollection::create();
		m_candidatedCells = PYXTileCollection::create();
		fetchNewSourceIndex();
	}

public:
	virtual bool end() const
	{
		return m_candidates.empty() && m_iterator->end();
	}

	virtual void next()
	{
		if (end())
		{
			return;
		}

		//move to next candidate
		if(!m_candidates.empty())
		{
			m_candidates.erase(m_candidates.begin());
		}

		if (m_candidates.empty())
		{
			//search for a new source...
			fetchNewSourceIndex();
		}
		else
		{
			//we need to add all matching neighbors as candidates for the new current index
			addCandidates();
		}
	}

	virtual const PYXIcosIndex& getIndex() const
	{
		return *(m_candidates.begin());
	}
	virtual PYXValue getFieldValue(int nFieldIndex = 0) const
	{
		return PYXValue();
	}

private:
	void fetchNewSourceIndex()
	{
		while (!m_iterator->end())
		{
			const PYXIcosIndex & index = m_iterator->getIndex();
			if (!m_candidatedCells->intersects(index))
			{
				m_candidates.push_back(index);
				m_candidatedCells->addTile(index,index.getResolution());
				m_iterator->next();

				addCandidates();
				return;
			}
			m_iterator->next();
		}
	}

	void addCandidates()
	{
		const PYXIcosIndex & currentIndex = *(m_candidates.begin());
		//we found a new cell to emit.
		m_visitedCells->addTile(currentIndex,currentIndex.getResolution());

		//add all candidates...
		for(PYXNeighbourIterator it(currentIndex);!it.end();it.next())
		{
			const PYXIcosIndex & index = it.getIndex();
			if (!m_candidatedCells->intersects(index) && m_floodFunction(currentIndex,index))
			{
				m_candidates.push_back(index);
				m_candidatedCells->addTile(index,index.getResolution());
			}
		}
	}

private:
	PYXPointer<PYXIterator> m_iterator;
	PYXPointer<PYXTileCollection> m_visitedCells;
	PYXPointer<PYXTileCollection> m_candidatedCells;
	std::list<PYXIcosIndex> m_candidates;
	boost::function< bool (const PYXIcosIndex &,const PYXIcosIndex &) > m_floodFunction;
};

//////////////////////////////////////////////////////////////////////////
// _value lambda helper
//////////////////////////////////////////////////////////////////////////

struct _value_type::details
{
	static bool valueNull(const PYXValue & a)
	{
		return a.isNull();
	}

	static bool valueNotNull(const PYXValue & a)
	{
		return !a.isNull();
	}

	static bool valueNumeric(const PYXValue & a)
	{
		return a.isNumeric();
	}

	static bool valueLess(const PYXValue & a,const PYXValue & b)
	{
		return a.compare(b)<0;
	}
	static bool valueLessEqual(const PYXValue & a,const PYXValue & b)
	{
		return a.compare(b)<=0;
	}
	static bool valueGreater(const PYXValue & a,const PYXValue & b)
	{
		return a.compare(b)>0;
	}
	static bool valueGreaterEqual(const PYXValue & a,const PYXValue & b)
	{
		return a.compare(b)>=0;
	}
	static bool valueEqual(const PYXValue & a,const PYXValue & b)
	{
		return a.compare(b)==0;
	}
	static bool valueNotEqual(const PYXValue & a,const PYXValue & b)
	{
		return a.compare(b)!=0;
	}

	static PYXValue getCoverageValue(const boost::intrusive_ptr<ICoverage> & coverage,const PYXIcosIndex & index,int nFieldIndex)
	{
		return coverage->getCoverageValue(index,nFieldIndex);
	}

	static bool localMaximumOfCoverage( const boost::intrusive_ptr<ICoverage> & coverage,int nFieldIndex,const PYXIcosIndex & index )
	{
		PYXValue val = coverage->getCoverageValue(index,nFieldIndex);
		if (val.isNull())
		{
			return false;
		}
		return PYXIteratorLinq::fromNeighboursWithoutSelf(index).select(coverage).all(_value <= val);
		//return fromNeighboursWithoutSelf(index).select(coverage).all(boost::bind(PYXIteratorLinq::valueGreaterEqual,val,_1));
	}

	static bool localMinimumOfCoverage( const boost::intrusive_ptr<ICoverage> & coverage,int nFieldIndex,const PYXIcosIndex & index )
	{
		PYXValue val = coverage->getCoverageValue(index,nFieldIndex);
		if (val.isNull())
		{
			return false;
		}

		return PYXIteratorLinq::fromNeighboursWithoutSelf(index).select(coverage).all(_value >= val );
		//return fromNeighboursWithoutSelf(index).select(coverage).all(boost::bind(PYXIteratorLinq::valueLessEqual,val,_1));
	}
};

boost::function<PYXValue(const PYXIcosIndex	& )> _value_type::of(const boost::intrusive_ptr<ICoverage> & coverage,int nFieldIndex /*= 0*/)
{
	return boost::bind(details::getCoverageValue,coverage,_1,nFieldIndex);
}

boost::function<bool(const PYXValue&)> _value_type::isNumeric()
{
	return boost::bind(details::valueNumeric,_1);
}

boost::function<bool(const PYXValue&)> _value_type::isNotNull()
{
	return boost::bind(details::valueNotNull,_1);
}

boost::function<bool(const PYXValue&)> _value_type::isNull()
{
	return boost::bind(details::valueNull,_1);
}

boost::function<bool(const PYXIcosIndex &)> _value_type::localMaximumOf(const boost::intrusive_ptr<ICoverage> & coverage,int nFieldIndex)
{
	return boost::bind(details::localMaximumOfCoverage,coverage,nFieldIndex,_1);
}
boost::function<bool(const PYXIcosIndex &)> _value_type::localMinimumOf(const boost::intrusive_ptr<ICoverage> & coverage,int nFieldIndex)
{
	return boost::bind(details::localMinimumOfCoverage,coverage,nFieldIndex,_1);
}

boost::function<bool(const PYXIcosIndex &)> _value_type::equal(const boost::function<PYXValue(const PYXIcosIndex &)> & a,const boost::function<PYXValue(const PYXIcosIndex &)> & b)
{
	return boost::bind(a,_1) == boost::bind(b,_1);
}

boost::function<bool(const PYXIcosIndex &)> _value_type::notEqual(const boost::function<PYXValue(const PYXIcosIndex &)> & a,const boost::function<PYXValue(const PYXIcosIndex &)> & b)
{
	return boost::bind(a,_1) != boost::bind(b,_1);
}

boost::function<bool(const PYXValue&)> operator == (const _value_type & a,const PYXValue & b)
{
	return boost::bind(_value_type::details::valueEqual,_1,b);
}

boost::function<bool(const PYXValue&)> operator == (const PYXValue & a,const _value_type & b)
{
	return boost::bind(_value_type::details::valueEqual,a,_1);
}

boost::function<bool(const PYXValue&)> operator != (const _value_type & a,const PYXValue & b)
{
	return boost::bind(_value_type::details::valueNotEqual,_1,b);
}

boost::function<bool(const PYXValue&)> operator != (const PYXValue & a,const _value_type & b)
{
	return boost::bind(_value_type::details::valueNotEqual,a,_1);
}

boost::function<bool(const PYXValue&)> operator < (const _value_type & a,const PYXValue & b)
{
	//TODO[shatzi]: consider replace with std::less/std::less_equal/etc... operators..
	//return boost::bind(std::less<PYXValue>(),_1,b);
	return boost::bind(_value_type::details::valueLess,_1,b);
}

boost::function<bool(const PYXValue&)> operator < (const PYXValue & a, const _value_type & b)
{
	return boost::bind(_value_type::details::valueLess,a,_1);
}

boost::function<bool(const PYXValue&)> operator <= (const _value_type & a,const PYXValue & b)
{
	return boost::bind(_value_type::details::valueLessEqual,_1,b);
}

boost::function<bool(const PYXValue&)> operator <= (const PYXValue & a, const _value_type & b)
{
	return boost::bind(_value_type::details::valueLessEqual,a,_1);
}

boost::function<bool(const PYXValue&)> operator > (const _value_type & a,const PYXValue & b)
{
	return boost::bind(_value_type::details::valueGreater,_1,b);
}

boost::function<bool(const PYXValue&)> operator > (const PYXValue & a, const _value_type & b)
{
	return boost::bind(_value_type::details::valueGreater,a,_1);
}

boost::function<bool(const PYXValue&)> operator >= (const _value_type & a,const PYXValue & b)
{
	return boost::bind(_value_type::details::valueGreaterEqual,_1,b);
}

boost::function<bool(const PYXValue&)> operator >= (const PYXValue & a, const _value_type & b)
{
	return boost::bind(_value_type::details::valueGreaterEqual,a,_1);
}

//////////////////////////////////////////////////////////////////////////
// PYXIteratorLinq class
//////////////////////////////////////////////////////////////////////////

PYXIteratorLinq PYXIteratorLinq::notNullOnly( int nFieldIndex /*= 0*/ )
{
	return PYXFilterByValueIterator::create(m_iterator,_value.isNotNull(),nFieldIndex);
}

PYXIteratorLinq PYXIteratorLinq::nullOnly( int nFieldIndex /*= 0*/ )
{
	return PYXFilterByValueIterator::create(m_iterator,_value.isNull(),nFieldIndex);
}

PYXIteratorLinq PYXIteratorLinq::filter( const boost::function< bool (const PYXIcosIndex &) > & filterFunc ) const
{
	return PYXFilterByIndexIterator::create(m_iterator,filterFunc);
}

PYXIteratorLinq PYXIteratorLinq::filter( const boost::function< bool (const PYXValue &) > & filterFunc, int nFieldIndex) const
{
	return PYXFilterByValueIterator::create(m_iterator,filterFunc,nFieldIndex);
}

PYXIteratorLinq PYXIteratorLinq::orderBy(boost::function<PYXValue(const PYXIcosIndex & )> orderFunction) const
{	
	std::list<std::pair<PYXIcosIndex,PYXValue>> indexAndValues;

	while(!m_iterator->end())
	{
		indexAndValues.push_back(std::make_pair(m_iterator->getIndex(),orderFunction(m_iterator->getIndex())));
		m_iterator->next();
	}

	indexAndValues.sort([&](const std::pair<PYXIcosIndex,PYXValue> & a,const std::pair<PYXIcosIndex,PYXValue> & b){ return a.second<b.second; });
	
	return PYXIteratorLinq::fromList(indexAndValues);
}

PYXIteratorLinq PYXIteratorLinq::floodFill( const boost::function< bool (const PYXIcosIndex &,const PYXIcosIndex &) > & canFloodFunction )
{
	return FloodFillIterator::create(m_iterator,canFloodFunction);
}

PYXIteratorLinq PYXIteratorLinq::findShortestPath(	const PYXIcosIndex & source,
													const boost::function<bool(const PYXIcosIndex &)> & isTargetFunction ,
													const boost::function<double(const PYXIcosIndex &,const PYXIcosIndex &)> & costFunction,
													const boost::function<double(double,double)> & costAggregationFunction )
{
	struct State { 
		PYXIcosIndex source;
		double cost;
		std::multimap<double,PYXIcosIndex>::iterator candiateLocation;

		State() {} State(const PYXIcosIndex & index,double aCost) : source(index), cost(aCost) {}
	};
	std::map<PYXIcosIndex,State> states;
	std::multimap<double,PYXIcosIndex> candiadtes;
	candiadtes.insert(std::make_pair(0,source));
	states[source] = State(source,0);
	states[source].candiateLocation = candiadtes.begin();

	PYXIcosIndex target;

	while(!candiadtes.empty())
	{
		PYXIcosIndex current = candiadtes.begin()->second;
		candiadtes.erase(candiadtes.begin());

		if (isTargetFunction(current))
		{
			target = current;
			break;
		}

		State & currentState = states[current];

		for(PYXPointer<PYXIterator> it = fromNeighboursWithoutSelf(current);!it->end();it->next())
		{
			const PYXIcosIndex & neighbour = it->getIndex();
			double moveCost = costFunction(current,neighbour);
			double totalCost = costAggregationFunction(currentState.cost,moveCost);
			if (states.find(neighbour)==states.end())
			{
				states[neighbour] = State(current,totalCost);
				states[neighbour].candiateLocation = candiadtes.insert(std::make_pair(totalCost,neighbour));
			}
			else if (states[neighbour].cost > totalCost)
			{
				candiadtes.erase(states[neighbour].candiateLocation);
				states[neighbour] = State(current,totalCost);
				states[neighbour].candiateLocation = candiadtes.insert(std::make_pair(totalCost,neighbour));
			}
		}
	}

	if (target.isNull())
	{
		return PYXEmptyIterator::create();
	}

	std::list<std::pair<PYXIcosIndex,PYXValue>> way;

	while(target != source)
	{
		const State & state = states[target];
		way.push_front(std::make_pair(target,PYXValue(state.cost)));
		target = state.source;
	}

	way.push_front(std::make_pair(source,PYXValue(0.0)));

	return fromList(way);
}

PYXPointer<PYXGeometry> PYXIteratorLinq::toGeometry()
{
	PYXPointer<PYXTileCollection> tileCollection = PYXTileCollection::create();

	while(!m_iterator->end())
	{
		const PYXIcosIndex & index = m_iterator->getIndex();
		tileCollection->addTile(index ,index.getResolution());
		m_iterator->next();
	}
	return tileCollection;
}

void PYXIteratorLinq::pushBackToVector( std::vector<PYXIcosIndex> & indices )
{
	while(!m_iterator->end())
	{
		indices.push_back(m_iterator->getIndex());
		m_iterator->next();
	}
}

void PYXIteratorLinq::pushBackToList( std::list<PYXIcosIndex> & indices )
{
	while(!m_iterator->end())
	{
		indices.push_back(m_iterator->getIndex());
		m_iterator->next();
	}
}

PYXValue PYXIteratorLinq::average( int nFieldIndex /*= 0*/ )
{
	double sum = 0.0;
	int count = 0;
	if (m_iterator->end())
	{
		return PYXValue();
	}
	for(/*nothing*/;!m_iterator->end();m_iterator->next())
	{
		sum += m_iterator->getFieldValue(nFieldIndex).getDouble();
		count++;
	}
	return PYXValue(sum/count);
}

PYXValue PYXIteratorLinq::count()
{
	int count = 0;
	for(/*nothing*/;!m_iterator->end();m_iterator->next())
	{
		count++;
	}
	return PYXValue(count);
}

PYXValue PYXIteratorLinq::min( int nFieldIndex /*= 0*/ )
{
	if (m_iterator->end())
	{
		return PYXValue();
	}
	PYXValue minValue = m_iterator->getFieldValue(nFieldIndex);
	m_iterator->next();

	for(/*nothing*/;!m_iterator->end();m_iterator->next())
	{
		PYXValue newValue = m_iterator->getFieldValue(nFieldIndex);
		if (newValue.compare(minValue) < 0)
		{
			minValue = newValue;
		}
	}
	return minValue;
}

PYXIcosIndex PYXIteratorLinq::minValueIndex( int nFieldIndex /*= 0*/ )
{
	if (m_iterator->end())
	{
		return PYXIcosIndex();
	}
	PYXValue minValue = m_iterator->getFieldValue(nFieldIndex);
	PYXIcosIndex minIndex = m_iterator->getIndex();
	m_iterator->next();

	for(/*nothing*/;!m_iterator->end();m_iterator->next())
	{
		PYXValue newValue = m_iterator->getFieldValue(nFieldIndex);
		if (newValue.compare(minValue) < 0)
		{
			minValue = newValue;
			minIndex = m_iterator->getIndex();
		}
	}
	return minIndex;
}

PYXValue PYXIteratorLinq::max( int nFieldIndex /*= 0*/ )
{
	if (m_iterator->end())
	{
		return PYXValue();
	}
	PYXValue maxValue = m_iterator->getFieldValue(nFieldIndex);
	m_iterator->next();

	for(/*nothing*/;!m_iterator->end();m_iterator->next())
	{
		PYXValue newValue = m_iterator->getFieldValue(nFieldIndex);
		if (maxValue.compare(newValue) < 0)
		{
			maxValue = newValue;
		}
	}
	return maxValue;
}

PYXIcosIndex PYXIteratorLinq::maxValueIndex( int nFieldIndex /*= 0*/ )
{
	if (m_iterator->end())
	{
		return PYXIcosIndex();
	}
	PYXIcosIndex maxIndex = m_iterator->getIndex();
	PYXValue maxValue = m_iterator->getFieldValue(nFieldIndex);
	m_iterator->next();

	for(/*nothing*/;!m_iterator->end();m_iterator->next())
	{
		PYXValue newValue = m_iterator->getFieldValue(nFieldIndex);
		if (maxValue.compare(newValue) < 0)
		{
			maxValue = newValue;
			maxIndex = m_iterator->getIndex();
		}
	}
	return maxIndex;
}

bool PYXIteratorLinq::any( const boost::function<bool(const PYXValue & value)> & testFunc,int nFieldIndex /*= 0*/ )
{
	while(!m_iterator->end())
	{
		if (testFunc(m_iterator->getFieldValue(nFieldIndex)))
		{
			return true;
		}
		m_iterator->next();
	}
	return false;
}

bool PYXIteratorLinq::all( const boost::function<bool(const PYXValue & value)> & testFunc,int nFieldIndex /*= 0*/ )
{
	while(!m_iterator->end())
	{
		if (!testFunc(m_iterator->getFieldValue(nFieldIndex)))
		{
			return false;
		}
		m_iterator->next();
	}
	return true;
}

PYXIteratorLinq PYXIteratorLinq::beforeEachItem( const boost::function< void (const PYXIcosIndex &) > & applyFunction )
{
	return PYXApplyBeforeFunctionIterator::create(m_iterator,applyFunction);
}

PYXIteratorLinq PYXIteratorLinq::selectMany( const boost::function< PYXPointer<PYXIterator> (const PYXIcosIndex &) > & populateFunction )
{
	return PYXSelectManyIterator::create(m_iterator,populateFunction);
}

PYXIteratorLinq PYXIteratorLinq::select( const boost::intrusive_ptr<ICoverage> & coverage ) const
{
	return PYXValueGetterIterator::create(coverage,m_iterator);
}

PYXIteratorLinq PYXIteratorLinq::single() const
{
	return PYXTakeIterator::create(m_iterator,1);
}


PYXIteratorLinq PYXIteratorLinq::take( int amount ) const
{
	return PYXTakeIterator::create(m_iterator,amount);
}

PYXIteratorLinq PYXIteratorLinq::skip( int amount ) const
{
	return PYXSkipIterator::create(m_iterator,amount);
}

PYXIteratorLinq PYXIteratorLinq::fromCoverageTile( const boost::intrusive_ptr<ICoverage> & coverage, const PYXTile & tile )
{
	PYXPointer<PYXValueTile> valueTile = coverage->getCoverageTile(tile);
	if (valueTile)
	{
		return PYXValueTileSafeIterator::create(valueTile);
	}
	return PYXEmptyIterator::create();
}

PYXIteratorLinq PYXIteratorLinq::fromSelfAndNeighbours( const PYXIcosIndex & index )
{
	return PYXNeighbourIterator::create(index);
}

PYXIteratorLinq PYXIteratorLinq::fromNeighboursWithoutSelf( const PYXIcosIndex & index )
{
	PYXPointer<PYXIterator> iterator = PYXNeighbourIterator::create(index);
	iterator->next();//skip self
	return iterator;
}

PYXIteratorLinq PYXIteratorLinq::fromGeometry( const PYXPointer<PYXGeometry> & geometry )
{
	return PYXGeometrySafeIterator::create(geometry);
}

PYXIteratorLinq PYXIteratorLinq::fromGeometry( const PYXGeometry & geometry )
{
	return PYXGeometrySafeIterator::create(geometry.clone());
}

PYXIteratorLinq PYXIteratorLinq::fromList( const std::list<std::pair<PYXIcosIndex,PYXValue>> & indexAndValues )
{
	return PYXListIterator::create(indexAndValues);
}

PYXIteratorLinq PYXIteratorLinq::fromIndex( const PYXIcosIndex & index )
{
	return PYXSingleIterator::create(index);
}

PYXIteratorLinq::PYXIteratorLinq( const PYXPointer<PYXIterator> & iterator )
{
	m_iterator = iterator;
}

PYXIcosIndex PYXIteratorLinq::firstIndex()
{
	if (m_iterator->end())
	{
		return PYXIcosIndex();
	}
	return m_iterator->getIndex();
}

PYXValue PYXIteratorLinq::firstValue(int nFiledIndex)
{
	if (m_iterator->end())
	{
		return PYXValue();
	}
	return m_iterator->getFieldValue(nFiledIndex);
}

//////////////////////////////////////////////////////////////////////////
// PYXTemporaryCoverage
//////////////////////////////////////////////////////////////////////////

PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE PYXTemporaryCoverage::getCoverageDefinition() const
{
	return m_inputCoverage->getCoverageDefinition();
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE PYXTemporaryCoverage::getCoverageDefinition()
{
	return m_inputCoverage->getCoverageDefinition();
}

PYXValue STDMETHODCALLTYPE PYXTemporaryCoverage::getCoverageValue( const PYXIcosIndex& index, int nFieldIndex /*= 0 */ ) const
{
	PYXTile tile = getTileForIndex(index);

	bool hasValue = false;
	PYXValue val = getCoverageTile(tile)->getValue(index,nFieldIndex,&hasValue);

	if (hasValue)
	{
		return val;
	}
	else
	{
		return PYXValue();
	}
}

PYXPointer<PYXValueTile> STDMETHODCALLTYPE PYXTemporaryCoverage::getFieldTile( const PYXIcosIndex& index, int nRes, int nFieldIndex /*= 0 */ ) const
{
	PYXTile tile(index,nRes);
	TilesMap::const_iterator it = m_modifications.find(tile);
	if (it != m_modifications.end())
	{
		const PYXPointer<PYXValueTile> & valueTile = it->second;

		if (nFieldIndex >= valueTile->getNumberOfDataChannels())
		{
			PYXTHROW(PYXException,"nFieldIndex is out of range. requesting #" << nFieldIndex << " field, but tile contains only "<< valueTile->getNumberOfDataChannels() << "fields");
		}
		if (valueTile->getNumberOfDataChannels() == 1)
		{
			return valueTile;
		}
		return valueTile->cloneFieldTile(nFieldIndex);
	}
	return m_inputCoverage->getFieldTile(index,nRes,nFieldIndex);
}

PYXCost STDMETHODCALLTYPE PYXTemporaryCoverage::getFieldTileCost( const PYXIcosIndex& index, int nRes, int nFieldIndex /*= 0 */ ) const
{
	PYXTile tile(index,nRes);
	TilesMap::const_iterator it = m_modifications.find(tile);
	if (it != m_modifications.end())
	{
		return PYXCost::knImmediateCost;
	}
	return m_inputCoverage->getFieldTileCost(index,nRes,nFieldIndex);
}

PYXPointer<PYXValueTile> STDMETHODCALLTYPE PYXTemporaryCoverage::getCoverageTile( const PYXTile& tile ) const
{
	if (!m_lastTile || m_lastTile->getTile() != tile)
	{
		TilesMap::const_iterator it = m_modifications.find(tile);
		if (it != m_modifications.end())
		{
			m_lastTile = it->second;
		}
		else
		{
			PYXPointer<PYXValueTile> valueTile = m_inputCoverage->getCoverageTile(tile);
			if (!valueTile )
			{
				//create empty tile
				m_modifications[tile] = PYXValueTile::create(tile,getCoverageDefinition());
			}
			else
			{
				m_modifications[tile] = valueTile->clone();
			}
			m_lastTile = m_modifications[tile];
		}
	}
	return m_lastTile;
}

PYXCost STDMETHODCALLTYPE PYXTemporaryCoverage::getTileCost( const PYXTile& tile ) const
{
	TilesMap::const_iterator it = m_modifications.find(tile);
	if (it != m_modifications.end())
	{
		return PYXCost::knImmediateCost;
	}
	return m_inputCoverage->getTileCost(tile);
}

void STDMETHODCALLTYPE PYXTemporaryCoverage::setCoverageValue( const PYXValue& value, const PYXIcosIndex& index, int nFieldIndex /*= 0 */ )
{
	PYXTile tile = getTileForIndex(index);
	getCoverageTile(tile)->setValue(index,nFieldIndex,value);
}

void STDMETHODCALLTYPE PYXTemporaryCoverage::setCoverageTile( PYXPointer<PYXValueTile> spValueTile )
{
	m_modifications[spValueTile->getTile()] = spValueTile;
}

PYXTile PYXTemporaryCoverage::getTileForIndex( const PYXIcosIndex & index ) const
{
	int cellResolution = index.getResolution();
	PYXIcosIndex rootIndex = index;
	if (cellResolution-11>=2)
	{
		rootIndex.setResolution(cellResolution-11);
	}
	else
	{
		rootIndex.setResolution(2);
	}
	return PYXTile(rootIndex,cellResolution);
}

//////////////////////////////////////////////////////////////////////////////////
// Testing
//////////////////////////////////////////////////////////////////////////////////

class PYXIteratorLinqTester	
{
public:
	static PYXPointer<PYXIterator> selectManyForSomeIncides(const PYXIcosIndex & index)
	{
		if (!index.hasVertexChildren())
		{
			return PYXEmptyIterator::create();
		}
		return PYXIteratorLinq::fromNeighboursWithoutSelf(index);
	}

	static void test()
	{
		std::list<std::pair<PYXIcosIndex,PYXValue>> indicesAndValues;

		indicesAndValues.push_back(std::make_pair(PYXIcosIndex("A-000000"),PYXValue(1)));
		indicesAndValues.push_back(std::make_pair(PYXIcosIndex("A-000001"),PYXValue(8)));
		indicesAndValues.push_back(std::make_pair(PYXIcosIndex("A-000005"),PYXValue(4)));
		indicesAndValues.push_back(std::make_pair(PYXIcosIndex("B-000020"),PYXValue(10)));
		indicesAndValues.push_back(std::make_pair(PYXIcosIndex("A-000050"),PYXValue(7)));
		indicesAndValues.push_back(std::make_pair(PYXIcosIndex("A-000030"),PYXValue(10)));
		indicesAndValues.push_back(std::make_pair(PYXIcosIndex("A-000100"),PYXValue(5)));
		indicesAndValues.push_back(std::make_pair(PYXIcosIndex("A-000101"),PYXValue(9)));

		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).max().getInt(),10);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).min().getInt(),1);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).maxValueIndex(),PYXIcosIndex("B-000020"));
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).minValueIndex(),PYXIcosIndex("A-000000"));

		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).filter(_value < PYXValue(10.0)).max().getInt(),9);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).filter(_value < PYXValue(10.0)).count().getInt(),6);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).filter(PYXValue("2") > _value).count().getInt(),3); //do string comparison...!!!! so, "2" > "10", but "2" < "5"...

		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).skip(2).firstIndex(),PYXIcosIndex("A-000005"));
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).skip(2).firstValue(),PYXValue(4));
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).skip(5).take(10).count().getInt(),3);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).skip(5).single().count().getInt(),1);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).skip(5).single().firstIndex(),PYXIcosIndex("A-000030"));
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).skip(5).single().skip(10).count().getInt(),0);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).take(0).count().getInt(),0);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).take(5).count().getInt(),5);

		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).take(5).average().getInt(),6);

		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).selectMany(boost::bind(PYXIteratorLinq::fromNeighboursWithoutSelf,_1)).count().getInt(),6*8);
		TEST_ASSERT_EQUAL(PYXIteratorLinq::fromList(indicesAndValues).selectMany(boost::bind(PYXIteratorLinqTester::selectManyForSomeIncides,_1)).count().getInt(),6*5);
	}
};
Tester<PYXIteratorLinqTester> g_PYXIteratorLinqTester;