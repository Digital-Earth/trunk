#ifndef PYXIS__QUERY__WHERE_H
#define PYXIS__QUERY__WHERE_H
/******************************************************************************
where.h

begin		: 2015-03-17
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/query/where_condition.h"

//! Where query
/*!
Where query
*/
class PYXLIB_DECL PYXWhere : public PYXObject
{
private:
	PYXWhere(const PYXWhere & other);
	PYXWhere();

public:
	//! Create a query with no conditions
	static PYXPointer<PYXWhere> create();

	//! Create a query with a single conditions
	static PYXPointer<PYXWhere> create(const PYXPointer<PYXWhereCondition> & condition);

	//! returns a new where query that intersect the current query with a the given geometry
	PYXPointer<PYXWhere> intersect(const PYXPointer<PYXGeometry> & geometry);

	//! returns a new where query that intersect the current query with a the given condition
	PYXPointer<PYXWhere> intersect(const PYXPointer<PYXWhereCondition> & condition);

	//! returns a new where query that subtract the given geometry from the current query
	PYXPointer<PYXWhere> subtract(const PYXPointer<PYXGeometry> & geometry);

	//! returns a new where query that subtract the given condition from the current query
	PYXPointer<PYXWhere> subtract(const PYXPointer<PYXWhereCondition> & condition);

	//! returns a new where query that disjunct (union) the current query with a the given geometry
	PYXPointer<PYXWhere> disjunct(const PYXPointer<PYXGeometry> & geometry);

	//! returns a new where query that disjunct (union) the current query with a the given condition
	PYXPointer<PYXWhere> disjunct(const PYXPointer<PYXWhereCondition> & condition);

public:
	//! return all matching cells for the current query on a given tile
	PYXPointer<PYXGeometry> on(const PYXTile & tile);

	//! return all matching cells for the current query on a given geometry
	PYXPointer<PYXGeometry> on(const PYXPointer<PYXGeometry> & geometry);

	//! return all matching cells for the current query on a given geometry running on the given resolution
	PYXPointer<PYXGeometry> on(const PYXPointer<PYXGeometry> & geometry, int resolution);

private:
	enum Operation
	{
		knIntersection,
		knDisjunction,
		knSubtraction
	};

	struct ConditionAndOperation 
	{
		PYXPointer<PYXWhereCondition> condition;
		Operation operation;
	};

	PYXPointer<PYXWhere> addCondition(const PYXPointer<PYXWhereCondition> & condition, Operation operation);

	typedef std::vector< ConditionAndOperation > ConditionVector;
	ConditionVector m_conditions;	
};

#endif // guard
