#ifndef PYXIS__QUERY__WHERE_CONDITION_H
#define PYXIS__QUERY__WHERE_CONDITION_H
/******************************************************************************
where_condition.h

begin		: 2015-03-17
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/coverage.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/utility/range.h"

#include "boost/function.hpp"

class PYXCoverageWhereCondition;
class PYXFeaturesWhereCondition;
class PYXGeometryWhereCondition;


//! Abstract base for classes for a where condition
/*!
Abstract base for classes for a where condition
*/
class PYXLIB_DECL PYXWhereCondition : public PYXObject
{
public:
	//! Check condition on a given index
	virtual bool match(const PYXIcosIndex & index) = 0;

	//! Check condition on a given tile, returns all cells matching condition
	virtual PYXPointer<PYXTileCollection> match(const PYXTile & tile) = 0;

public:

	//! Create condition using a coverage
	static PYXPointer<PYXCoverageWhereCondition> coverageHasValues(const boost::intrusive_ptr<ICoverage> & coverage);

	//! Create condition using a feature collections
	static PYXPointer<PYXFeaturesWhereCondition> featuresHasValues(const boost::intrusive_ptr<IFeatureCollection> & features);

	//! Create condition using a geometry
	static PYXPointer<PYXGeometryWhereCondition> geometry(const PYXPointer<PYXGeometry> & geometry);
};


//! Where Condition for coverage
class PYXLIB_DECL PYXCoverageWhereCondition : public PYXWhereCondition 
{
	//PYXWhereCondition
public:
	virtual bool match(const PYXIcosIndex & index);
	virtual PYXPointer<PYXTileCollection> match(const PYXTile & tile);

private:
	PYXCoverageWhereCondition(const boost::intrusive_ptr<ICoverage> & coverage);

public:
	//! create a coverage condition that return all cells where coverage has a value
	static PYXPointer<PYXCoverageWhereCondition> create(const boost::intrusive_ptr<ICoverage> & coverage) 
	{
		return PYXNEW(PYXCoverageWhereCondition,coverage);
	}

	//! create a coverage condition that return all cells where coverage has a value inside the given range
	PYXPointer<PYXCoverageWhereCondition> range(const PYXValue & min,const PYXValue & max);

private:
	boost::intrusive_ptr<ICoverage> m_coverage;
	boost::function<bool(PYXValue)> m_filter;
};

//! Where Condition for features
class PYXLIB_DECL PYXFeaturesWhereCondition : public PYXWhereCondition 
{
	//PYXWhereCondition
public:
	virtual bool match(const PYXIcosIndex & index);
	virtual PYXPointer<PYXTileCollection> match(const PYXTile & tile);

private:
	PYXFeaturesWhereCondition(const boost::intrusive_ptr<IFeatureCollection> & features);

public:
	//! create a coverage condition that return all cells where features exists
	static PYXPointer<PYXFeaturesWhereCondition> create(const boost::intrusive_ptr<IFeatureCollection> & features) 
	{
		return PYXNEW(PYXFeaturesWhereCondition,features);
	}

	//! create a coverage condition that return all cells where features has a value for a given field
	PYXPointer<PYXFeaturesWhereCondition> field(const std::string & field);

	//! create a coverage condition that return all cells where features has a value inside the given range
	PYXPointer<PYXFeaturesWhereCondition> range(const PYXValue & min,const PYXValue & max);

private:	
	boost::intrusive_ptr<IFeatureCollection> m_features;
	int m_fieldIndex;
	boost::function<bool(PYXValue)> m_filter;
};


//! Where Condition for geometry
class PYXLIB_DECL PYXGeometryWhereCondition : public PYXWhereCondition 
{
	//PYXWhereCondition
public:
	virtual bool match(const PYXIcosIndex & index);
	virtual PYXPointer<PYXTileCollection> match(const PYXTile & tile);

private:
	PYXGeometryWhereCondition(const PYXPointer<PYXGeometry> & geometry);

public:
	//! create a coverage condition that return all cells that intersects the given geometry
	static PYXPointer<PYXGeometryWhereCondition> create(const PYXPointer<PYXGeometry> & geometry) 
	{
		return PYXNEW(PYXGeometryWhereCondition,geometry);
	}

private:
	PYXPointer<PYXGeometry> m_geometry;
};



#endif // guard
