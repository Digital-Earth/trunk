#ifndef PYXIS__DERM__ITERATOR_LINQ_H
#define PYXIS__DERM__ITERATOR_LINQ_H
/******************************************************************************
iterator_linq.h

begin		: 2013-01-09
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/data/coverage.h"

#include "boost/function.hpp"

//////////////////////////////////////////////////////////////////////////
// _value lambda helper
//////////////////////////////////////////////////////////////////////////

//TODO[shatzi]: consider using boost lambda - need to check performance cost..
//#include <boost/lambda/lambda.hpp>

struct PYXLIB_DECL _value_type
{
private:
	struct details;	

public:
	boost::function<bool(const PYXValue&)> isNull();

	boost::function<bool(const PYXValue&)> isNotNull();

	boost::function<bool(const PYXValue&)> isNumeric();

	boost::function<PYXValue(const PYXIcosIndex	& )> of(const boost::intrusive_ptr<ICoverage> & coverage,int nFieldIndex = 0);
	boost::function<bool(const PYXIcosIndex &)> localMaximumOf(const boost::intrusive_ptr<ICoverage> & coverage,int nFieldIndex);
	boost::function<bool(const PYXIcosIndex &)> localMinimumOf(const boost::intrusive_ptr<ICoverage> & coverage,int nFieldIndex);

	boost::function<bool(const PYXIcosIndex &)> equal(const boost::function<PYXValue(const PYXIcosIndex &)> & a,const boost::function<PYXValue(const PYXIcosIndex &)> & b);
	boost::function<bool(const PYXIcosIndex &)> notEqual(const boost::function<PYXValue(const PYXIcosIndex &)> & a,const boost::function<PYXValue(const PYXIcosIndex &)> & b);

	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator == (const _value_type & a,const PYXValue & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator != (const _value_type & a,const PYXValue & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator < (const _value_type & a,const PYXValue & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator <= (const _value_type & a,const PYXValue & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator > (const _value_type & a,const PYXValue & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator >= (const _value_type & a,const PYXValue & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator == (const PYXValue & a,const _value_type & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator != (const PYXValue & a,const _value_type & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator < (const PYXValue & a, const _value_type & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator <= (const PYXValue & a, const _value_type & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator > (const PYXValue & a, const _value_type & b);
	friend boost::function<bool(const PYXValue&)> PYXLIB_DECL operator >= (const PYXValue & a, const _value_type & b);

};

PYXLIB_DECL _value_type _value;

boost::function<bool(const PYXValue&)> PYXLIB_DECL operator == (const _value_type & a,const PYXValue & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator != (const _value_type & a,const PYXValue & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator < (const _value_type & a,const PYXValue & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator <= (const _value_type & a,const PYXValue & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator > (const _value_type & a,const PYXValue & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator >= (const _value_type & a,const PYXValue & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator == (const PYXValue & a,const _value_type & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator != (const PYXValue & a,const _value_type & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator < (const PYXValue & a, const _value_type & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator <= (const PYXValue & a, const _value_type & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator > (const PYXValue & a, const _value_type & b);
boost::function<bool(const PYXValue&)> PYXLIB_DECL operator >= (const PYXValue & a, const _value_type & b);

//////////////////////////////////////////////////////////////////////////
// PYXIteratorLinq class
//////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXIteratorLinq
{
//ctor
public:
	PYXIteratorLinq(const PYXPointer<PYXIterator> & iterator);

//from ctor
public:
	static PYXIteratorLinq fromIndex(const PYXIcosIndex & index);
	static PYXIteratorLinq fromList(const std::list<std::pair<PYXIcosIndex,PYXValue>> & indexAndValues);
	static PYXIteratorLinq fromGeometry(const PYXGeometry & geometry);
	static PYXIteratorLinq fromGeometry(const PYXPointer<PYXGeometry> & geometry);
	static PYXIteratorLinq fromNeighboursWithoutSelf(const PYXIcosIndex & index);
	static PYXIteratorLinq fromSelfAndNeighbours(const PYXIcosIndex & index);
	static PYXIteratorLinq fromCoverageTile(const boost::intrusive_ptr<ICoverage> & coverage, const PYXTile & tile);

public: //selecting
	PYXIteratorLinq select(const boost::intrusive_ptr<ICoverage> & coverage) const;
	PYXIteratorLinq selectMany(const boost::function< PYXPointer<PYXIterator> (const PYXIcosIndex &) > & populateFunction);
	/*
	PYXIteratorLinq select(boost::function<PYXValue(const PYXValue &)> & selectFunction) const
	PYXIteratorLinq select(boost::function<PYXValue(const PYXIcosIndex &)> & selectFunction) const
	PYXIteratorLinq select(boost::function<PYXValue(const PYXIcosIndex &,const PYXValue &)> & selectFunction) const
	*/

	PYXIteratorLinq single() const;
	PYXIteratorLinq take(int amount) const;
	PYXIteratorLinq skip(int amount) const;

	//! Apply a function before each item is get return from the iterator. useful for trace or to invoke specific functions during while not iterupting the iterations
	PYXIteratorLinq beforeEachItem(const boost::function< void (const PYXIcosIndex &) > & applyFunction);

public: //consume iterator
	bool all(const boost::function<bool(const PYXValue & value)> & testFunc,int nFieldIndex = 0);
	bool any(const boost::function<bool(const PYXValue & value)> & testFunc,int nFieldIndex = 0);

	PYXIcosIndex maxValueIndex(int nFieldIndex = 0);
	PYXValue max(int nFieldIndex = 0);

	PYXIcosIndex minValueIndex(int nFieldIndex = 0);
	PYXValue min(int nFieldIndex = 0);

	PYXValue count();
	PYXValue average(int nFieldIndex = 0);

//transform functions
public:
	template<typename Action>
	void foreach(const Action & func)
	{
		while(!m_iterator->end())
		{
			func(m_iterator->getIndex());
			m_iterator->next();
		}
	}

	void pushBackToList(std::list<PYXIcosIndex> & indices);
	void pushBackToVector(std::vector<PYXIcosIndex> & indices);

	PYXPointer<PYXGeometry> toGeometry();

	PYXIcosIndex firstIndex();
	PYXValue firstValue(int nFiledIndex = 0);


	PYXIteratorLinq orderBy(boost::function<PYXValue(const PYXIcosIndex	& )> orderFunction) const;
	
//shortest path finding
public:	
	//! find the shortest path from a source cell to any matching target cell (by using isTargetFunction) using the given cost function.
	static PYXIteratorLinq findShortestPath(
		const PYXIcosIndex & source,
		const boost::function<bool(const PYXIcosIndex &)> & isTargetFunction ,
		const boost::function<double(const PYXIcosIndex &,const PYXIcosIndex &)> & costFunction,
		const boost::function<double(double,double)> & costAggregationFunction);

	PYXIteratorLinq floodFill(const boost::function< bool (const PYXIcosIndex &,const PYXIcosIndex &) > & canFloodFunction );

//filter functions
public:
	PYXIteratorLinq filter(const boost::function< bool (const PYXIcosIndex &) > & filterFunc ) const;
	PYXIteratorLinq filter(const boost::function< bool (const PYXValue &) > & filterFunc, int nFieldIndex = 0 ) const;
	PYXIteratorLinq nullOnly(int nFieldIndex = 0);
	PYXIteratorLinq notNullOnly(int nFieldIndex = 0);

//helper cast back to PYXIterator
public:
	operator const PYXPointer<PYXIterator> & () const
	{
		return m_iterator;
	}

private:
	PYXPointer<PYXIterator> m_iterator;
};


class PYXLIB_DECL PYXTemporaryCoverage : public ICoverage
{
private:
	typedef std::map<PYXTile,PYXPointer<PYXValueTile>> TilesMap;

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(ICoverage)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL();

public:
	IRECORD_IMPL_PROXY(*m_inputCoverage);
	IFEATURE_IMPL_PROXY(*m_inputCoverage);
	IFEATURECOLLECTION_IMPL_PROXY(*m_inputCoverage);

public:
	PYXTemporaryCoverage(const boost::intrusive_ptr<ICoverage> & coverage) : m_inputCoverage(coverage) {}

public:
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition();

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(const PYXIcosIndex& index, int nFieldIndex = 0) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(const PYXIcosIndex& index, int nRes, int nFieldIndex = 0) const;

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(const PYXIcosIndex& index, int nRes, int nFieldIndex = 0) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const;

	virtual PYXCost STDMETHODCALLTYPE getTileCost(const PYXTile& tile) const;

	virtual void STDMETHODCALLTYPE setCoverageValue(const PYXValue& value, const PYXIcosIndex& index, int nFieldIndex = 0);

	virtual void STDMETHODCALLTYPE setCoverageTile(PYXPointer<PYXValueTile> spValueTile);

private:
	PYXTile getTileForIndex(const PYXIcosIndex & index) const;

private:
	boost::intrusive_ptr<ICoverage> m_inputCoverage;

	mutable PYXPointer<PYXValueTile> m_lastTile;
	mutable TilesMap m_modifications;
};

#endif // guard
