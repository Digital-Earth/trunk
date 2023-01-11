#ifndef TILE_AGGREGATOR_H
#define TILE_AGGREGATOR_H
/******************************************************************************
tile_aggregator.h

begin		: 2017-03-03
copyright	: (C) 2017 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxis/derm/index.h"
#include "pyxis/region/region.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/data/coverage.h"
#include "pyxis/utility/color_palette.h"
#include "pyxis/utility/thread_pool.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/region/circle_region.h"
#include <boost/math/tools/config.hpp>
#include "value_tile.h"


class BooleanAggregator
{
public:
	bool value;

public:
	BooleanAggregator() : value(false)
	{
		
	}

	BooleanAggregator(bool val) : value(val)
	{
		
	}

	BooleanAggregator & operator += (const BooleanAggregator & other)
	{
		value |= other.value;
		return *this;
	}
};


template <class T>
class SumAndCountAggregator
{
public:
	int count;
	T sum;

public:
	SumAndCountAggregator() : count(0), sum(0)
	{

	}

	SumAndCountAggregator(T value) : count(1), sum(value)
	{

	}

	SumAndCountAggregator(int aCount, T value) : count(aCount), sum(value)
	{

	}

	SumAndCountAggregator(const PYXPointer<IFeature> & feature, int fieldIndex ) : count(1)
	{
		sum = feature->getFieldValue(fieldIndex).getDouble();
	}

	SumAndCountAggregator(const PYXPointer<IFeatureGroup> & group, int fieldIndex ) 
	{
		auto histogram = group->getFieldHistogram(fieldIndex);

		sum = histogram->getSum().getDouble();
		count = histogram->getFeatureCount().middle();
	}

	SumAndCountAggregator & operator += (const SumAndCountAggregator & other)
	{
		count += other.count;
		sum += other.sum;
		return *this;
	}
};

template <class T>
class MaxAggregator
{
public:
	bool hasValue;
	T value;

public:
	MaxAggregator() : hasValue(false)
	{

	}

	MaxAggregator(T v) : hasValue(true), value(v)
	{

	}

	MaxAggregator(const PYXPointer<IFeature> & feature, int fieldIndex ) : hasValue(true)
	{
		value = feature->getFieldValue(fieldIndex).getDouble();
	}

	MaxAggregator(const PYXPointer<IFeatureGroup> & group, int fieldIndex )  : hasValue(true)
	{
		auto histogram = group->getFieldHistogram(fieldIndex);

		value = histogram->getBoundaries().max.getDouble();
	}

	MaxAggregator & operator += (const MaxAggregator & other)
	{
		if (other.hasValue)
		{
			if (!hasValue || value < other.value)
			{
				value = other.value;
				hasValue = true;
			}
		}
		return *this;
	}
};

template <class T>
class MinAggregator
{
public:
	bool hasValue;
	T value;	

public:
	MinAggregator() : hasValue(false)
	{

	}

	MinAggregator(T v) : hasValue(true), value(v)
	{

	}

	MinAggregator(const PYXPointer<IFeature> & feature, int fieldIndex ) : hasValue(true)
	{
		value = feature->getFieldValue(fieldIndex).getDouble();
	}

	MinAggregator(const PYXPointer<IFeatureGroup> & group, int fieldIndex )  : hasValue(true)
	{
		auto histogram = group->getFieldHistogram(fieldIndex);

		value = histogram->getBoundaries().min.getDouble();
	}

	MinAggregator & operator += (const MinAggregator & other)
	{
		if (other.hasValue)
		{
			if (!hasValue || value > other.value)
			{
				value = other.value;
				hasValue = true;
			}
		}
		return *this;
	}
};

template <class Aggregator>
class FieldAggregator
{
};

template <class Aggregator>
class TransformFieldAggregator
{
};

class PYXValueTransform
{
private:
	PYXValue m_transformType;
	std::map<PYXValue,PYXValue> m_transform;
	bool m_exactMatch;

public:
	PYXValueTransform(const std::map<PYXValue,PYXValue> & transform, bool exactMatch) : m_transform(transform), m_exactMatch(exactMatch)
	{
		for(auto value : transform)
		{
			if (!value.first.isNull())
			{
				m_transformType = value.first;
				break;
			}
		}
	}

	PYXValue transform(const PYXValue & valIn) const
	{
		if (!valIn.isNull())
		{
			auto safeVal = valIn.cast(m_transformType.getType());

			auto equal_or_after = m_transform.lower_bound(safeVal);

			if (equal_or_after == m_transform.end())
			{
				return PYXValue();
			}

			if (safeVal == equal_or_after->first)
			{
				return equal_or_after->second;
			}

			if (!m_exactMatch)
			{
				if (equal_or_after != m_transform.begin())
				{
					auto previous = std::prev(equal_or_after);

					return previous->second;
				}
			}
		}
		return PYXValue();
	}

	//transform iterator location using an histogram
	Range<int> countFeatures(const std::map<PYXValue,PYXValue>::const_iterator & it, const PYXPointer<PYXHistogram> & histogram) const
	{
		auto next = std::next(it);
		if (it->second.isNull())
		{
			return 0;
		}
			
		if (m_exactMatch)
		{
			return histogram->getFeatureCount(it->first);
		} 
		
		if (next != m_transform.end())
		{
			return histogram->getFeatureCount(PYXValueRange(it->first,next->first,knClosed,knOpen));	
		}
		
		if (it->first < histogram->getBoundaries().max)
		{
			return histogram->getFeatureCount(PYXValueRange(it->first,histogram->getBoundaries().max,knClosed,knClosed));
		}

		return 0;
	}

	const std::map<PYXValue,PYXValue> & getTransformValues() const
	{
		return m_transform;
	}
};


template <>
class FieldAggregator< SumAndCountAggregator<double> > 
{
private:
	int m_fieldIndex;

public:
	FieldAggregator (int fieldIndex) : m_fieldIndex(fieldIndex)
	{

	}

	SumAndCountAggregator<double> operator()(const boost::intrusive_ptr<IFeature> & feature) const
	{
		return SumAndCountAggregator<double>(feature->getFieldValue(m_fieldIndex).getDouble());	
	}

	SumAndCountAggregator<double> operator()(const boost::intrusive_ptr<IFeatureGroup> & group) const
	{
		auto histogram = group->getFieldHistogram(m_fieldIndex);

		auto sum = histogram->getSum().getDouble();
		auto count = histogram->getFeatureCount().middle();

		return SumAndCountAggregator<double>(count,sum);
	}
};

template <>
class TransformFieldAggregator< SumAndCountAggregator<double> >
{
private:
	PYXValueTransform m_transform;
	int m_fieldIndex;

public:


	TransformFieldAggregator(int fieldIndex, const std::map<PYXValue,PYXValue> & transform, bool extactMatch) : m_transform(transform,extactMatch), m_fieldIndex(fieldIndex)
	{
	}

	SumAndCountAggregator<double> operator()(const boost::intrusive_ptr<IFeature> & feature) const
	{
		auto result = m_transform.transform(feature->getFieldValue(m_fieldIndex));

		if (result.isNull())
		{
			return SumAndCountAggregator<double>();
		}

		return SumAndCountAggregator<double>(result.getDouble());	
	}

	SumAndCountAggregator<double> operator()(const boost::intrusive_ptr<IFeatureGroup> & group) const
	{
		auto histogram = group->getFieldHistogram(m_fieldIndex);

		double sum = 0;
		int count = 0;

		auto transform = m_transform.getTransformValues();

		for(auto it = transform.begin(); it != transform.end(); ++it)
		{
			auto matchFeaturesCount = m_transform.countFeatures(it,histogram);
			
			if (matchFeaturesCount.max > 0 && !it->second.isNull())
			{
				auto amount = matchFeaturesCount.max;
				count += amount;
				sum += amount * it->second.getDouble();
			}
		}

		return SumAndCountAggregator<double>(count,sum);
	}
};

template <>
class FieldAggregator< MinAggregator<double> > 
{
private:
	int m_fieldIndex;

public:
	FieldAggregator (int fieldIndex) : m_fieldIndex(fieldIndex)
	{

	}

	MinAggregator<double> operator()(const boost::intrusive_ptr<IFeature> & feature) const
	{
		return MinAggregator<double>(feature->getFieldValue(m_fieldIndex).getDouble());	
	}

	MinAggregator<double> operator()(const boost::intrusive_ptr<IFeatureGroup> & group) const
	{
		auto histogram = group->getFieldHistogram(m_fieldIndex);

		auto value = histogram->getBoundaries().min.getDouble();

		return MinAggregator<double>(value);
	}
};

template <>
class TransformFieldAggregator< MinAggregator<double> > 
{
private:
	PYXValueTransform m_transform;
	int m_fieldIndex;

public:

	TransformFieldAggregator(int fieldIndex, std::map<PYXValue,PYXValue> transform, bool extactMatch) : m_transform(transform,extactMatch), m_fieldIndex(fieldIndex)
	{

	}

	MinAggregator<double> operator()(const boost::intrusive_ptr<IFeature> & feature) const
	{
		auto result = m_transform.transform(feature->getFieldValue(m_fieldIndex));

		if (result.isNull())
		{
			return MinAggregator<double>();
		}

		return MinAggregator<double>(result.getDouble());	
	}

	MinAggregator<double> operator()(const boost::intrusive_ptr<IFeatureGroup> & group) const
	{
		auto histogram = group->getFieldHistogram(m_fieldIndex);

		MinAggregator<double> result;

		auto transform = m_transform.getTransformValues();

		for(auto it = transform.begin(); it != transform.end(); ++it)
		{
			auto matchFeaturesCount = m_transform.countFeatures(it,histogram);

			if (matchFeaturesCount.max > 0 && ! it->second.isNull())
			{
				result += MinAggregator<double>(it->second.getDouble());
			}
		}

		return result;
	}
};


template <>
class FieldAggregator< MaxAggregator<double> > 
{
private:
	int m_fieldIndex;

public:
	FieldAggregator (int fieldIndex) : m_fieldIndex(fieldIndex)
	{

	}

	MaxAggregator<double> operator()(const boost::intrusive_ptr<IFeature> & feature) const
	{
		return MaxAggregator<double>(feature->getFieldValue(m_fieldIndex).getDouble());	
	}

	MaxAggregator<double> operator()(const boost::intrusive_ptr<IFeatureGroup> & group) const
	{
		auto histogram = group->getFieldHistogram(m_fieldIndex);

		auto value = histogram->getBoundaries().max.getDouble();

		return MaxAggregator<double>(value);
	}
};

template <>
class TransformFieldAggregator< MaxAggregator<double> >
{
private:
	PYXValueTransform m_transform;
	int m_fieldIndex;

public:

	TransformFieldAggregator(int fieldIndex, std::map<PYXValue,PYXValue> transform, bool extactMatch) : m_transform(transform,extactMatch), m_fieldIndex(fieldIndex)
	{

	}

	MaxAggregator<double> operator()(const boost::intrusive_ptr<IFeature> & feature) const
	{
		auto result = m_transform.transform(feature->getFieldValue(m_fieldIndex));

		if (result.isNull())
		{
			return MaxAggregator<double>();
		}

		return MaxAggregator<double>(result.getDouble());	
	}

	MaxAggregator<double> operator()(const boost::intrusive_ptr<IFeatureGroup> & group) const
	{
		auto histogram = group->getFieldHistogram(m_fieldIndex);

		MaxAggregator<double> result;

		auto transform = m_transform.getTransformValues();

		for(auto it = transform.begin(); it != transform.end(); ++it)
		{
			auto matchFeaturesCount = m_transform.countFeatures(it,histogram);

			if (matchFeaturesCount.max > 0 && ! it->second.isNull())
			{
				result += MaxAggregator<double>(it->second.getDouble());
			}
		}

		return result;
	}
};


template<class Aggregator>
class TileAggregator
{
protected:
	PYXTile m_tile;
	std::vector<PYXInnerTile> m_innerTiles;
	int m_cellCount;
	double m_cellRadius;
	double m_errorThreshold;
	boost::recursive_mutex m_cellsMutex[16];
	boost::scoped_array<Aggregator> m_cells;

protected:
	void visitCell(int nPos, const Aggregator & newValue)
	{
		boost::recursive_mutex::scoped_lock lock(m_cellsMutex[nPos % 16]);

		m_cells[nPos] += newValue;
	}

	void visitCells(int nPos, int nLength, const Aggregator & newValue)
	{
		int max = std::min(16,nLength);

		for(int i=0;i<max;++i)
		{
			//using 16 locks to minimize locking delays
			boost::recursive_mutex::scoped_lock lock(m_cellsMutex[(nPos+i) % 16]);

			for(int pos = nPos+i;pos<nPos+nLength;pos+=16)
			{
				m_cells[pos] += newValue;
			}
		}
	}

public:
	TileAggregator(PYXTile tile) : m_tile(tile)
	{
		//things to speed up visiting
		m_cellCount = m_tile.getCellCount();
		m_cellRadius = PYXIcosMath::UnitSphere::calcCellCircumRadius(m_tile.getCellResolution());
		m_errorThreshold = m_cellRadius / 5;
		m_innerTiles = PYXInnerTile::createInnerTiles(m_tile);

		//todo - we might want to move this to static function to avoid throw on ctor 
		try
		{
			m_cells.reset(new Aggregator[m_cellCount]);
		}
		CATCH_AND_RETHROW("Failed to alloc cells state");
	}

	virtual ~TileAggregator()
	{	
	}

public:
	void visit(const PYXPointer<PYXVectorRegion> & region, const Aggregator & newValue)
	{
		int resolution = m_tile.getCellResolution();
		auto rootIndex = m_tile.getRootIndex();
		PYXBoundingCircle regionCircle = region->getBoundingCircle();
		if (regionCircle.getRadius() < PYXIcosMath::UnitSphere::calcCellCircumRadius(resolution ))
		{
			//this is more or less a single cell - just rasterize it.

			PYXIcosIndex index;
			SnyderProjection::getInstance()->nativeToPYXIS(SphereMath::xyzll(regionCircle.getCenter()),&index,resolution);

			if (index.isDescendantOf(rootIndex))
			{
				int nPos = PYXIcosMath::calcCellPosition(rootIndex, index);
				visitCell(nPos, newValue);
			}
			return;
		}


		for(int nPos = 0; nPos<m_cellCount; nPos++)
		{
			PYXIcosIndex index = PYXIcosMath::calcIndexFromOffset(rootIndex,resolution,nPos);
			PYXCoord3DDouble location;
			SnyderProjection::getInstance()->pyxisToXYZ(index,&location);

			double distance = 0;

			distance = region->getDistanceToBorder(location,m_errorThreshold);

			bool isBorder = distance < m_cellRadius;

			while(!isBorder && index.hasVertexChildren() && index.getResolution() > resolution )
			{
				index.decrementResolution();

				isBorder = distance < PYXIcosMath::UnitSphere::calcTileCircumRadius(index) + m_cellRadius; //radius of the boundary
			}

			if (index.getResolution() == resolution)
			{
				if (isBorder)
				{
					visitCell(nPos, newValue);
				}
				else if (region->isPointContained(location,m_errorThreshold))
				{
					visitCell(nPos, newValue);
				}
			}
			else
			{
				if (isBorder)
				{
					index.incrementResolution();
				}

				//amount of cells in our tile
				int tileCellCount = PYXIcosMath::getCellCount(index,resolution);

				if (region->isPointContained(location,m_errorThreshold))
				{
					visitCells(nPos, tileCellCount, newValue);
				}

				nPos += tileCellCount-1;
			}
		}
	}

	void visit(const PYXVectorGeometry & geometry, const Aggregator & newValue)
	{
		auto unionRegion = dynamic_cast<PYXCollectionVectorRegion*>(geometry.getRegion().get());

		if (unionRegion != nullptr)
		{
			for(int i=0;i<unionRegion->getRegionCount();i++)
			{
				visit(unionRegion->getRegion(i),newValue);
			}
		}
		else
		{
			visit(geometry.getRegion(),newValue);
		}
	}

	void visit(const PYXVectorGeometry2 & geometry, const Aggregator & newValue)
	{
		auto resolution = m_tile.getCellResolution();
		auto rootIndex = m_tile.getRootIndex();

		for(auto & innerTile : m_innerTiles)
		{
			for(PYXPointer<PYXInnerTileIntersectionIterator> iterator = geometry.getInnerTileIterator(innerTile);
				!iterator->end();
				iterator->next())
			{
				PYXInnerTileIntersection intersection = iterator->getIntersection();
				if(intersection != knIntersectionNone)
				{
					PYXIcosIndex index = iterator->getTile().asTile().getRootIndex();
					if(index.getResolution() == resolution)
					{
						int nPos = PYXIcosMath::calcCellPosition(rootIndex, index);
						if(nPos < m_cellCount)
						{
							visitCell(nPos,newValue);
						}
						else
						{
							assert(0 && "Should never get here");
						}
					}
					else if (index.getResolution() < resolution)
					{
						int tileCellCount = PYXIcosMath::getCellCount(index,resolution);

						index.setResolution(resolution);
						int nPos = PYXIcosMath::calcCellPosition(rootIndex, index);

						visitCells(nPos,tileCellCount,newValue);
					}
				}
			}
		}
	}

	void visitWithIntersection(const PYXGeometry & geometry, const Aggregator & newValue)
	{
		//do deep intersection with tile
		PYXPointer<PYXGeometry> intersectionGeom = geometry.intersection(m_tile);	

		if (intersectionGeom->isEmpty())
		{
			//nothing to visit
			return;
		}

		//single tile at high resolution edge case
		auto intersectionTile = dynamic_cast<PYXTile*>(intersectionGeom.get());
		if (intersectionTile != nullptr && intersectionTile->getRootIndex().getResolution() > m_tile.getCellResolution())
		{
			PYXIcosIndex i(intersectionTile->getRootIndex());
			i.setResolution(m_tile.getCellResolution());
			intersectionGeom = PYXTile::create(i,m_tile.getCellResolution());
		}

		//make sure the result is at the right resolution
		intersectionGeom->setCellResolution(m_tile.getCellResolution());

		if (m_tile.getDepth() == 0)
		{
			if (!intersectionGeom->isEmpty())
			{
				visitCell(0,newValue);
			}
		}
		else
		{
			auto tileCollection = dynamic_cast<PYXTileCollection*>(intersectionGeom.get());

			if (tileCollection != nullptr)
			{
				//fast iteration over tiles
				PYXPointer<PYXTileCollectionIterator> spIt = tileCollection->getTileIterator();
				for (; !spIt->end(); spIt->next())
				{
					auto tile = spIt->getTile();

					int nPos = PYXIcosMath::calcCellPosition(m_tile.getRootIndex(), tile->getRootIndex());
					visitCells(nPos,tile->getCellCount(),newValue);
				}
			}
			else 
			{
				//slow iteration over cells
				PYXPointer<PYXIterator> spIt = intersectionGeom->getIterator();
				for (; !spIt->end(); spIt->next())
				{
					const PYXIcosIndex& index2 = spIt->getIndex();

					int nPos = PYXIcosMath::calcCellPosition(m_tile.getRootIndex(), index2);

					visitCell(nPos,newValue);
				}
			}
		}
	}

public:
	Aggregator* getCells() const
	{
		return m_cells.get();
	}

	int getCellCount() const
	{
		return m_cellCount;
	}

	const Aggregator & getCell(int nPos) const
	{
		return m_cells[nPos];
	}

	const PYXTile & getTile() const
	{
		return m_tile;
	}

	void visit(const PYXGeometry & geometry, const Aggregator & newValue)
	{
		try
		{
			auto vectorGeometry = dynamic_cast<const PYXVectorGeometry *>(&geometry);

			if (vectorGeometry)
			{
				visit(*vectorGeometry,newValue);
				return;
			}

			auto * vectorGeometry2 = dynamic_cast<const PYXVectorGeometry2 *>(&geometry);
			if(vectorGeometry2)
			{
				visit(*vectorGeometry2,newValue);
				return;
			}

			visitWithIntersection(geometry,newValue);
		}
		catch(PYXException & __pyx_exp__) 
		{ 
			PYXTHROW(PYXException,"failed to visit geometry " << " due to error : " << __pyx_exp__.getFullErrorString()); 
		} 
		catch(std::exception & __std_exp__) 
		{ 
			PYXTHROW(PYXException,"failed to visit geometry " << " due to error : " << __std_exp__.what()); 
		} 
		catch(...) 
		{ 
			PYXTHROW(PYXException,"failed to visit geometry " << " due to error : unknown" ); \
		} 
	}
};

class TileFeaturesVisitor
{
protected:
	PYXTile m_tile;
	PYXPointer<PYXVectorGeometry> m_tileVectorGeom;
	PYXTaskGroup m_rasterTasks;
	bool m_useTasks;

protected:
	//return true if visiter should visit subGroups as well.
	virtual bool onGroup(const PYXPointer<IFeatureGroup> & group) = 0;

	virtual void onFeature(const PYXPointer<IFeature> & feature) = 0;

	void previsitGroup(PYXPointer<IFeatureGroup> & group)
	{
		if (onGroup(group))
		{
			//visit all sub groups
			visitGroup(group);
		}
	}

	void visitGroup(PYXPointer<IFeatureGroup> & group)
	{
		if (group->getFeaturesCount().max == 0)
		{
			return;
		}

		std::vector< boost::intrusive_ptr<IFeature> > subFeaturesToRasterize;
		std::vector< boost::intrusive_ptr<IFeatureGroup> > subGroupToRasterize;

		try
		{
			//split between groups and features.
			for (PYXPointer<FeatureIterator> spFit = group->getGroupIterator(*m_tileVectorGeom); !spFit->end(); spFit->next())
			{
				boost::intrusive_ptr<IFeature> spF = spFit->getFeature();

				boost::intrusive_ptr<IFeatureGroup> subGroup = spF->QueryInterface<IFeatureGroup>();

				if (subGroup)
				{
					subGroupToRasterize.push_back(subGroup);
				}
				else
				{
					subFeaturesToRasterize.push_back(spF);
				}
			}
		}
		CATCH_AND_RETHROW("Failed to iterate over feature in group " << group->getID());

		try
		{
			//rasterize groups...
			for(auto & subGroup : subGroupToRasterize)
			{
				if (m_useTasks)
				{
					m_rasterTasks.addTask(boost::bind(&TileFeaturesVisitor::previsitGroup,this,subGroup));
				}
				else
				{
					previsitGroup(subGroup);
				}
			}	
		}
		CATCH_AND_RETHROW("Failed to start subgroup tasks for group " << group->getID());

		try
		{
			//put all features at the back - this should improve the speed and memory consumption as the thead pool do tasks in lifo order.
			//so, we would like to get read of all the features ASAP.
			for(auto & feature : subFeaturesToRasterize)
			{
				if (m_useTasks)
				{
					m_rasterTasks.addTask(boost::bind(&TileFeaturesVisitor::onFeature,this,feature));
				} 
				else
				{
					onFeature(feature);
				}
			}
		}
		CATCH_AND_RETHROW("Failed to start features tasks for group " << group->getID());	
	}

public: 
	TileFeaturesVisitor(const PYXTile & tile) : m_tile(tile), m_useTasks(true)
	{
		m_tileVectorGeom = PYXVectorGeometry::create(PYXCircleRegion::create(m_tile.getRootIndex(),true),m_tile.getCellResolution());
	}

	virtual ~TileFeaturesVisitor()
	{

	}

	void useTasks(bool use)
	{
		m_useTasks = use;
	}

	void visit(const PYXPointer<IFeatureCollection> & features)
	{
		PYXPointer<IFeatureGroup> group = features->QueryInterface<IFeatureGroup>();

		if (group)
		{
			visitGroup(group);
		}
		else 
		{
			PYXPointer<FeatureIterator> spFit = features->getIterator(m_tile);

			for (; !spFit->end(); spFit->next())
			{
				boost::intrusive_ptr<IFeature> spF = spFit->getFeature();

				if (m_useTasks)
				{
					m_rasterTasks.addTask(boost::bind(&TileFeaturesVisitor::onFeature,this,spF));
				}
				else
				{
					onFeature(spF);
				}
			}
		}

		if (m_useTasks)
		{
			m_rasterTasks.joinAll();
		}
	}
};

template<class Aggregator,class AggregatorFactory>
class TileFeaturesFieldAggregator : public TileFeaturesVisitor
{
protected:
	AggregatorFactory & m_factory;
	TileAggregator<Aggregator> & m_tileAggregator;

	bool onGroup(const PYXPointer<IFeatureGroup> & group) 
	{
		auto geometry = group->getGeometry();

		if (geometry->getCellResolution() - 3 < m_tile.getCellResolution())
		{
			return true;
		}

		m_tileAggregator.visit(*geometry, m_factory(group));

		return false;
	}

	void onFeature(const PYXPointer<IFeature> & feature)
	{
		m_tileAggregator.visit(*feature->getGeometry(), m_factory(feature));
	}

public:
	TileFeaturesFieldAggregator(TileAggregator<Aggregator> & tileAggregator, AggregatorFactory & factory) : TileFeaturesVisitor(tileAggregator.getTile()), 
		m_factory(factory), m_tileAggregator(tileAggregator)
	{	
	}

	virtual ~TileFeaturesFieldAggregator()
	{

	}
};


enum TileGeometryAggregatorOperation
{
	knAdd,
	knIntersection,
	knSubstraction
};

class PYXLIB_DECL TileGeometryAggregator
{
private:
	PYXTile m_tile;
	TileAggregator<BooleanAggregator> m_tileAgg;
	TileAggregator<BooleanAggregator> m_tileAggResult;
	int m_trueCells;

public:
	TileGeometryAggregator(const PYXTile & tile);

	virtual ~TileGeometryAggregator();

protected:
	void updateResult(TileGeometryAggregatorOperation operation);
	
public:
	TileGeometryAggregator & add(const PYXPointer<PYXGeometry> & geometry);

	TileGeometryAggregator & intersect(const PYXPointer<PYXGeometry> & geometry);
	TileGeometryAggregator & intersect(const boost::intrusive_ptr<IFeatureCollection> & features);
	TileGeometryAggregator & intersect(const boost::intrusive_ptr<ICoverage> & coverage);

	TileGeometryAggregator & substract(const PYXPointer<PYXGeometry> & geometry);
	TileGeometryAggregator & substract(const boost::intrusive_ptr<IFeatureCollection> & features);
	TileGeometryAggregator & substract(const boost::intrusive_ptr<ICoverage> & coverage);

	int getFoundCellCount() const;
	PYXPointer<PYXTileCollection> asTileCollection() const;

};

#endif // guard
