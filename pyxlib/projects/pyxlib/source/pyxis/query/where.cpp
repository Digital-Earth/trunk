/******************************************************************************
where.cpp

begin		: 2015-03-17
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE 
#include "Stdafx.h"
#include "pyxis/query/where.h"

////////////////////////////////////////////////////////////
// PYXWhereTileResults
////////////////////////////////////////////////////////////

/*!
PYXWhereTileResults goal is to perform boolean operation on cells
on a given tile. 
It allow intersect,subtract and disjunct conditions.
*/
class PYXWhereTileResults {
public:
	PYXWhereTileResults(const PYXTile & tile)
		: m_tile(tile), m_matchCount(0)
	{
		m_matches.resize(tile.getCellCount(),false);
	}

	void intersect(const PYXPointer<PYXWhereCondition> & condition)
	{
		if (empty()) 
		{
			return;
		}

		auto partial = condition->match(m_tile);

		auto it = m_tile.getIterator();
		for(size_t i = 0; i < m_matches.size(); ++i, it->next()) 
		{
			if(m_matches[i] && !partial->intersects(it->getIndex())) 
			{
				m_matches[i] = false;
				m_matchCount--;
			}
		}
	}

	void disjunct(const PYXPointer<PYXWhereCondition> & condition) 
	{
		auto partial = condition->match(m_tile);

		auto it = m_tile.getIterator();
		for(size_t i = 0; i < m_matches.size(); ++i, it->next()) 
		{
			if(!m_matches[i] && partial->intersects(it->getIndex())) 
			{
				m_matches[i] = true;
				m_matchCount++;
			}
		}
	}

	void subtract(const PYXPointer<PYXWhereCondition> & condition) 
	{
		if (empty()) 
		{
			return;
		}

		auto partial = condition->match(m_tile);

		auto it = m_tile.getIterator();
		for(size_t i = 0; i < m_matches.size(); ++i, it->next()) 
		{
			if(m_matches[i] && partial->intersects(it->getIndex())) 
			{
				m_matches[i] = false;
				m_matchCount--;
			}
		}
	}

	bool empty() const
	{
		return m_matchCount == 0;
	}

	PYXPointer<PYXTileCollection> asTileCollection() 
	{
		auto result = PYXTileCollection::create();

		auto it = m_tile.getIterator();
		for(size_t i = 0; i < m_matches.size(); ++i, it->next()) 
		{
			if(m_matches[i])
			{
				result->addTile(it->getIndex(),m_tile.getCellResolution());
			}
		}
		return result;
	}

private:
	PYXTile m_tile;
	std::vector<bool> m_matches;
	int m_matchCount;
};

////////////////////////////////////////////////////////////
// PYXWhere
////////////////////////////////////////////////////////////

PYXWhere::PYXWhere()
{
}

PYXWhere::PYXWhere(const PYXWhere & other) 
	: m_conditions(other.m_conditions)
{
}


PYXPointer<PYXWhere> PYXWhere::create()
{
	return PYXNEW(PYXWhere);
}

PYXPointer<PYXWhere> PYXWhere::create(const PYXPointer<PYXWhereCondition> & condition)
{
	return create()->disjunct(condition);
}

PYXPointer<PYXWhere> PYXWhere::addCondition(const PYXPointer<PYXWhereCondition> & condition, Operation operation)
{
	if (operation != knDisjunction && m_conditions.empty())
	{	
		PYXTHROW(PYXException,"The query first condition must be disjunction");
	}

	auto query = PYXNEW(PYXWhere,*this);
	ConditionAndOperation conditionAndOperation;
	conditionAndOperation.condition = condition;
	conditionAndOperation.operation = operation;
	query->m_conditions.push_back(conditionAndOperation);
	return query;
}

PYXPointer<PYXWhere> PYXWhere::intersect(const PYXPointer<PYXGeometry> & geometry)
{
	return intersect(PYXWhereCondition::geometry(geometry));
}

PYXPointer<PYXWhere> PYXWhere::subtract(const PYXPointer<PYXGeometry> & geometry)
{
	return subtract(PYXWhereCondition::geometry(geometry));
}

PYXPointer<PYXWhere> PYXWhere::disjunct(const PYXPointer<PYXGeometry> & geometry)
{
	return disjunct(PYXWhereCondition::geometry(geometry));
}

PYXPointer<PYXWhere> PYXWhere::intersect(const PYXPointer<PYXWhereCondition> & condition)
{
	return addCondition(condition,knIntersection);
}

PYXPointer<PYXWhere> PYXWhere::subtract(const PYXPointer<PYXWhereCondition> & condition)
{
	return addCondition(condition,knSubtraction);
}

PYXPointer<PYXWhere> PYXWhere::disjunct(const PYXPointer<PYXWhereCondition> & condition)
{
	return addCondition(condition,knDisjunction);
}

PYXPointer<PYXGeometry> PYXWhere::on(const PYXTile & tile) 
{
	PYXWhereTileResults result(tile);

	//intersect positive intersection 
	for(auto conditionAndOperation : m_conditions) 
	{
		switch(conditionAndOperation.operation) 
		{
		case knIntersection:
			result.intersect(conditionAndOperation.condition);
			break;
		
		case knSubtraction:
			result.subtract(conditionAndOperation.condition);
			break;

		case knDisjunction:
			result.disjunct(conditionAndOperation.condition);
			break;
		}
	}

	return result.asTileCollection();
}


PYXPointer<PYXGeometry> PYXWhere::on(const PYXPointer<PYXGeometry> & geometry)
{
	return on(geometry,geometry->getCellResolution());
}

PYXPointer<PYXGeometry> PYXWhere::on(const PYXPointer<PYXGeometry> & geometry, int resolution)
{
	if (!geometry)
	{
		PYXTHROW(PYXException,"geometry is null");
	}

	//create query result geometry
	auto result = PYXTileCollection::create();

	//add a geometry intersections to the query to make sure results are only inside the given geometry
	auto boundedQuery = this->intersect(geometry);

	//copy the give geometry to a resolution fit to create default depth tiles
	auto coverResolution = std::max(2,resolution-PYXTile::knDefaultTileDepth);
	auto coverGeometry = PYXTileCollection::create();

	try
	{
		//make sure we don't "miss" a tile due to tile-outer size calculation
		geometry->copyTo(coverGeometry.get(), coverResolution + PYXTile::knDefaultTileDepth/2);
		coverGeometry->setCellResolution(coverResolution);
	}
	CATCH_AND_RETHROW("failed to find covering tiles");

	try 
	{
		//go over all tiles in the cover geometry...
		for(auto it = coverGeometry->getIterator(); !it->end(); it->next())
		{
			//run the boundedQuery on a single tile
			auto tile = PYXTile(it->getIndex(),resolution);
			auto tileResult = boundedQuery->on(tile);

			//merge the results
			if (tileResult->isEmpty())
			{
				continue;
			}
			result->addGeometry(*dynamic_cast<PYXTileCollection*>(tileResult.get()));
		}
	}
	CATCH_AND_RETHROW("failed to calculate where query");

	return result;
}