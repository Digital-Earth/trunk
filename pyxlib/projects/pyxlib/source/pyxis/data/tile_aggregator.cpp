/******************************************************************************
tile_aggregator.cpp

begin		: 2017-03-03
copyright	: (C) 2017 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/data/tile_aggregator.h"


class SimpleTileFeaturesVisitor : public TileFeaturesVisitor
{
protected:
	TileAggregator<BooleanAggregator> & m_tileAggregator;

	bool onGroup(const PYXPointer<IFeatureGroup> & group) override
	{
		auto geometry = group->getGeometry();

		if (geometry->getCellResolution() - 3 < m_tile.getCellResolution())
		{
			return true;
		}

		m_tileAggregator.visit(*geometry, BooleanAggregator(true));

		return false;
	}

	void onFeature(const PYXPointer<IFeature> & feature) override
	{
		m_tileAggregator.visit(*feature->getGeometry(), BooleanAggregator(true));
	}

public:
	SimpleTileFeaturesVisitor(TileAggregator<BooleanAggregator> & tileAggregator) : TileFeaturesVisitor(tileAggregator.getTile()), m_tileAggregator(tileAggregator)
	{	
	}

	virtual ~SimpleTileFeaturesVisitor()
	{

	}
};


TileGeometryAggregator::TileGeometryAggregator(const PYXTile & tile) : m_tile(tile), m_tileAgg(tile), m_tileAggResult(tile), m_trueCells(0)
{
}


TileGeometryAggregator::~TileGeometryAggregator()
{	
}

void TileGeometryAggregator::updateResult(TileGeometryAggregatorOperation operation)
{
	auto cells = m_tileAgg.getCells();
	auto resultCells = m_tileAggResult.getCells();
	auto count = m_tile.getCellCount();

	switch(operation)
	{
	case knAdd:
		for(auto i = 0; i < count; i++)
		{
			if (cells[i].value && !resultCells[i].value)
			{
				resultCells[i].value = true;
				m_trueCells++;
			}
			cells[i].value = false;
		}
		break;
	case knIntersection:
		for(auto i = 0; i < count; i++)
		{
			if (!cells[i].value && resultCells[i].value)
			{
				resultCells[i].value = false;
				m_trueCells--;
			}
			cells[i].value = false;
		}
		break;
	case knSubstraction:
		for(auto i = 0; i < count; i++)
		{
			if (cells[i].value && resultCells[i].value)
			{
				resultCells[i].value = false;
				m_trueCells--;
			}
			cells[i].value = false;
		}
		break;
	}
}


TileGeometryAggregator & TileGeometryAggregator::add(const PYXPointer<PYXGeometry> & geometry)
{
	m_tileAgg.visit(*geometry, BooleanAggregator(true));
	updateResult(knAdd);
	return *this;
}

TileGeometryAggregator & TileGeometryAggregator::intersect(const PYXPointer<PYXGeometry> & geometry)
{
	m_tileAgg.visit(*geometry, BooleanAggregator(true));
	updateResult(knIntersection);
	return *this;
}

TileGeometryAggregator & TileGeometryAggregator::intersect(const boost::intrusive_ptr<IFeatureCollection> & features)
{
	SimpleTileFeaturesVisitor visitor(m_tileAgg);
	visitor.visit(features);
	updateResult(knIntersection);
	return *this;
}

TileGeometryAggregator & TileGeometryAggregator::intersect(const boost::intrusive_ptr<ICoverage> & coverage)
{
	auto valueTile = coverage->getCoverageTile(m_tile);

	auto cells = m_tileAgg.getCells();
	auto count = m_tile.getCellCount();

	PYXValue value = valueTile->getTypeCompatibleValue(0);
	for(auto i = 0; i < count; i++)
	{
		cells[i].value = valueTile->getValue(i,0,&value);
	}

	updateResult(knIntersection);
	return *this;
}

TileGeometryAggregator & TileGeometryAggregator::substract(const PYXPointer<PYXGeometry> & geometry)
{
	m_tileAgg.visit(*geometry, BooleanAggregator(true));
	updateResult(knSubstraction);
	return *this;
}

TileGeometryAggregator & TileGeometryAggregator::substract(const boost::intrusive_ptr<IFeatureCollection> & features)
{
	SimpleTileFeaturesVisitor visitor(m_tileAgg);
	visitor.visit(features);
	updateResult(knSubstraction);
	return *this;
}

TileGeometryAggregator & TileGeometryAggregator::substract(const boost::intrusive_ptr<ICoverage> & coverage)
{
	auto valueTile = coverage->getCoverageTile(m_tile);

	auto cells = m_tileAgg.getCells();
	auto count = m_tile.getCellCount();

	PYXValue value = valueTile->getTypeCompatibleValue(0);
	for(auto i = 0; i < count; i++)
	{
		cells[i].value = valueTile->getValue(i,0,&value);
	}

	updateResult(knSubstraction);
	return *this;
}

int TileGeometryAggregator::getFoundCellCount() const
{
	return m_trueCells;
}

PYXPointer<PYXTileCollection> TileGeometryAggregator::asTileCollection() const
{
	PYXPointer<PYXTileCollection> result = PYXTileCollection::create();
	result->setCellResolution(m_tile.getCellResolution());

	auto cells = m_tileAggResult.getCells();
	
	auto i = 0;
	for(auto iter = m_tile.getIterator();!iter->end();iter->next(),i++)
	{
		if (cells[i].value)
		{
			result->addTile(iter->getIndex(),m_tile.getCellResolution());
		}
	}

	return result;
}
