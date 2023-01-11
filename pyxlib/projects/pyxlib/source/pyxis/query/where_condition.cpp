/******************************************************************************
where_condition.cpp

begin		: 2015-03-17
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE 
#include "Stdafx.h"
#include "pyxis/query/where_condition.h"
#include "pyxis/data/value_tile.h"

#include "boost/bind.hpp"

///////////////////////////////////////////////////////////
// PYXWhereCondition
///////////////////////////////////////////////////////////

PYXPointer<PYXCoverageWhereCondition> PYXWhereCondition::coverageHasValues(const boost::intrusive_ptr<ICoverage> & coverage)
{
	return PYXCoverageWhereCondition::create(coverage);
}
PYXPointer<PYXFeaturesWhereCondition> PYXWhereCondition::featuresHasValues(const boost::intrusive_ptr<IFeatureCollection> & features)
{
	return PYXFeaturesWhereCondition::create(features);
}

PYXPointer<PYXGeometryWhereCondition> PYXWhereCondition::geometry(const PYXPointer<PYXGeometry> & geometry) 
{
	return PYXGeometryWhereCondition::create(geometry);
}

///////////////////////////////////////////////////////////
// filter functions
///////////////////////////////////////////////////////////

/* always true */
bool trueFilter(PYXValue value) 
{
	return true;
}

/* range filter function */
bool valueRangeFilter(PYXValue value,PYXValue min,PYXValue max) 
{
	return value.compare(min) >= 0 && value.compare(max) <= 0;
}

///////////////////////////////////////////////////////////
// PYXCoverageWhereCondition
///////////////////////////////////////////////////////////

PYXCoverageWhereCondition::PYXCoverageWhereCondition(const boost::intrusive_ptr<ICoverage> & coverage)
	: m_coverage(coverage)
{
	m_filter = trueFilter;
}

PYXPointer<PYXCoverageWhereCondition> PYXCoverageWhereCondition::range(const PYXValue & min,const PYXValue & max) 
{
	auto cond = create(m_coverage);
	cond->m_filter = boost::bind(valueRangeFilter,_1,min,max);
	return cond;
}


bool PYXCoverageWhereCondition::match(const PYXIcosIndex & index)
{
	return m_filter(m_coverage->getCoverageValue(index));
}

PYXPointer<PYXTileCollection> PYXCoverageWhereCondition::match(const PYXTile & tile) 
{
	auto coverageTile = m_coverage->getCoverageTile(tile);	
	auto tileCollection = PYXTileCollection::create();
	
	auto it = coverageTile->getIterator();
	PYXValue value = coverageTile->getTypeCompatibleValue(0);
	
	for(int i=0;!it->end();it->next(),++i)
	{
		if (coverageTile->getValue(i,0,&value)) 
		{
			if (m_filter(value)) 
			{
				tileCollection->addTile(it->getIndex(),it->getIndex().getResolution());
			}
		}
	}

	return tileCollection;
}


///////////////////////////////////////////////////////////
// PYXFeaturesWhereCondition
///////////////////////////////////////////////////////////

PYXFeaturesWhereCondition::PYXFeaturesWhereCondition(const boost::intrusive_ptr<IFeatureCollection> & features)
	: m_features(features), m_fieldIndex(-1)
{
	m_filter = trueFilter;
}

bool PYXFeaturesWhereCondition::match(const PYXIcosIndex & index)
{
	auto cell = PYXCell(index);
	auto iterator = m_features->getIterator(cell);

	while(!iterator->end()) {
		auto feature = iterator->getFeature();
		//find all intersecting features
		if (feature->getGeometry()->intersects(cell)) {

			if (m_fieldIndex == -1) 
			{
				//no value filtering
				return true;
			}
			//check value filtering
			else if (m_filter(feature->getFieldValue(m_fieldIndex))) 
			{
				return true;
			}			
		}
		iterator->next();
	}
	return false;
}

PYXPointer<PYXTileCollection> PYXFeaturesWhereCondition::match(const PYXTile & tile)
{
	auto result = PYXTileCollection::create();

	for(auto iterator = m_features->getIterator(tile); !iterator->end(); iterator->next()) 
	{
		auto feature = iterator->getFeature();

		//find all intersecting features
		if (feature->getGeometry()->intersects(tile)) {

			if (m_fieldIndex != -1 && !m_filter(feature->getFieldValue(m_fieldIndex))) 
			{
				//skip feature if m_filter return false
				continue;
			}
			
			//find matching cells for that feature
			auto intersection = feature->getGeometry()->intersection(tile);
			
			if (!intersection->isEmpty())
			{
				//add matching cells to results array
				auto tempTileCollection = PYXTileCollection::create();
				intersection->copyTo(tempTileCollection.get(),tile.getCellResolution());
				result->addGeometry(*tempTileCollection);
			}
		}
	}

	return result;
}

PYXPointer<PYXFeaturesWhereCondition> PYXFeaturesWhereCondition::field(const std::string & field)
{
	auto cond = create(m_features);
	cond->m_fieldIndex = m_features->getFeatureDefinition()->getFieldIndex(field);
	cond->m_filter = m_filter;
	return cond;
}

PYXPointer<PYXFeaturesWhereCondition> PYXFeaturesWhereCondition::range(const PYXValue & min,const PYXValue & max)
{
	auto cond = create(m_features);
	cond->m_fieldIndex = m_fieldIndex;
	cond->m_filter = boost::bind(valueRangeFilter,_1,min,max);
	return cond;
}


///////////////////////////////////////////////////////////
// PYXGeometryWhereCondition
///////////////////////////////////////////////////////////

PYXGeometryWhereCondition::PYXGeometryWhereCondition(const PYXPointer<PYXGeometry> & geometry) : m_geometry(geometry)
{
}

bool PYXGeometryWhereCondition::match(const PYXIcosIndex & index)
{
	return m_geometry->intersects(PYXCell(index));
}

PYXPointer<PYXTileCollection> PYXGeometryWhereCondition::match(const PYXTile & tile)
{
	auto result = PYXTileCollection::create();
	auto intersection = m_geometry->intersection(tile);
	intersection->copyTo(result.get(),tile.getCellResolution());
	return result;
}