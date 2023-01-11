/******************************************************************************
aggregate_coverage_calculator.cpp

begin      : 10/03/2012 4:42:06 PM 
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "aggregate_coverage_calculator.h"
#include "pyxis/data/constant_record.h"

#include "pyxis/derm/snyder_projection.h"

#include "pyxis/geometry/vector_geometry2.h"

#include <boost/bind.hpp>
#include "pyxis/utility/value_math.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ConstantFieldCalculator
////////////////////////////////////////////////////////////////////////////////////////////////////////


// {481612EA-3018-42ee-9B56-32601A3051BE}
PYXCOM_DEFINE_CLSID(AggregateCoverageCalculator,
0x481612ea, 0x3018, 0x42ee, 0x9b, 0x56, 0x32, 0x60, 0x1a, 0x30, 0x51, 0xbe);


PYXCOM_CLASS_INTERFACES(AggregateCoverageCalculator, IProcess::iid, IFeatureCalculator::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(AggregateCoverageCalculator, "Aggregate Coverage", "Different aggregation of coverage data into a feature group.", "Analysis/Features/Calculator",
					IFeatureCalculator::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "A coverage data to perform a aggregation on.")
IPROCESS_SPEC_END



int AggregateCoverageCalculator::findResolution(const PYXPointer<PYXGeometry> & geometry,int initialResolution,double maxPartialPerecnt) const
{
	int resolution = initialResolution;

	PYXPointer<PYXTileCollection> coverGeometry = PYXTileCollection::create();
	geometry->copyTo(coverGeometry.get(),resolution);

	while (resolution+11<PYXMath::knMaxAbsResolution)
	{
		long completeCount = 0;
		long partialCount = 0;

		for(PYXPointer<PYXIterator> iterator = coverGeometry->getIterator();!iterator->end();iterator->next())
		{
			PYXTile tile(iterator->getIndex(),resolution+11);

			std::vector<PYXInnerTile> innerTiles = PYXInnerTile::createInnerTiles(tile);

			for(std::vector<PYXInnerTile>::iterator it = innerTiles.begin(); it != innerTiles.end(); ++it)
			{
				PYXPointer<PYXInnerTileIntersectionIterator> geomIt = geometry->getInnerTileIterator(*it);

				while(!geomIt->end())
				{
					switch(geomIt->getIntersection())
					{
					case knIntersectionComplete:
						completeCount += geomIt->getTile().getCellCount();
						break;

					case knIntersectionPartial:
						partialCount += geomIt->getTile().getCellCount();
						break;
					}
					geomIt->next();
				}
			}
		}

		long totalCells = partialCount + completeCount;
		if (completeCount == 0)
		{
			if (partialCount>1000)
			{
				return resolution;
			}
		}
		else
		{

			double percent = ((double)(partialCount)) / totalCells;

			TRACE_INFO("total cells = " << totalCells << ", partial cells = " << partialCount << " (" << percent*100 << "%)");

			if (percent < maxPartialPerecnt)
			{
				return resolution;
			}
		}

		//start new count...
		resolution++;
		coverGeometry = PYXTileCollection::create();
		geometry->copyTo(coverGeometry.get(),resolution);
	}

	return resolution;
}

PYXValue STDMETHODCALLTYPE AggregateCoverageCalculator::calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const
{
	std::auto_ptr<ICoverageAggregationOperator> aggregator = m_operatorFactory->create();

	PYXPointer<PYXGeometry> geometry = spFeature->getGeometry();

	//find a good resolution to perform aggregation on...
	int wantedResultion = m_cellResolution;

	if (wantedResultion <= 0)
	{
		//make it a resolution that would be around 1~6 depth-11 tiles.
		PYXBoundingCircle circle = geometry->getBoundingCircle();
		if (circle.getRadius() != 0)
		{
			wantedResultion = std::min(PYXMath::knMaxAbsResolution,PYXBoundingCircle::estimateResolutionFromRadius(circle.getRadius())+11);
			wantedResultion = findResolution(geometry,std::max(2,wantedResultion-11),0.05)+11;
		}
		else
		{
			wantedResultion = m_coverage->getGeometry()->getCellResolution();
		}
	}

	//found containing cells on a single low resolution
	PYXPointer<PYXTileCollection> coverGeometry = PYXTileCollection::create();
	geometry->copyTo(coverGeometry.get(),std::max(2,wantedResultion-11));

	//NOTE: this could lead to 20% error if polygon has many partial cells
	for(PYXPointer<PYXIterator> iterator = coverGeometry->getIterator();!iterator->end();iterator->next())
	{
		PYXTile tile(iterator->getIndex(),wantedResultion);

		std::vector<PYXInnerTile> innerTiles = PYXInnerTile::createInnerTiles(tile);

		PYXPointer<PYXValueTile> valueTile = m_coverage->getFieldTile(tile.getRootIndex(),tile.getCellResolution(),m_inputFieldIndex);

		if (!valueTile)
		{
			//we didn't got a real tile...
			continue;
		}

		for(std::vector<PYXInnerTile>::iterator it = innerTiles.begin(); it != innerTiles.end(); ++it)
		{
			PYXPointer<PYXInnerTileIntersectionIterator> geomIt = geometry->getInnerTileIterator(*it);

			aggregator->aggregate(valueTile,geomIt);
		}
	}

	return aggregator->getResult();
}

PYXValue generateValue(const AggregateCoverageCalculator & counter,boost::intrusive_ptr<IFeature> spFeature, int index)
{
	return counter.calculateValue(spFeature,index);
}

boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE AggregateCoverageCalculator::calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const
{
	return new ConstantRecord(m_outputDefinition,boost::bind(generateValue,boost::ref(*this),spFeature,_1));
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE AggregateCoverageCalculator::getOutputDefinition() const
{
	return m_outputDefinition;
}

std::string STDMETHODCALLTYPE AggregateCoverageCalculator::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"

		"<xs:simpleType name=\"OperatorType\">"
		"<xs:restriction base=\"xs:string\">"
			"<xs:enumeration value=\"Covered Area\" />"
			"<xs:enumeration value=\"Minimum\" />"
			"<xs:enumeration value=\"Maximum\" />"
			"<xs:enumeration value=\"Sum\" />"
			"<xs:enumeration value=\"AreaDensitySum\" />"
			"<xs:enumeration value=\"Average\" />"
		"</xs:restriction>"
		"</xs:simpleType>"
		
		"<xs:element name=\"AggregateCoverageCalculator\">"
		"<xs:complexType>"
		"<xs:sequence>"

		"<xs:element name=\"InputFieldIndex\" type=\"xs:int\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Input Field Index</friendlyName>"
		"<description>The Coverage Field index to use</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"Operator\" type=\"OperatorType\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Aggregation Operator</friendlyName>"
		"<description>Type of the aggregation.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"OutputPropertyName\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Output Property Name</friendlyName>"
		"<description>Name of the field for the const attribute.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"CellResolution\" type=\"xs:int\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Cell Resolution</friendlyName>"
		"<description>cell resolution to use (-1 - automaticly selected based on feature geometry).</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";


	return strXSD;
}

std::map< std::string, std::string > STDMETHODCALLTYPE AggregateCoverageCalculator::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["OutputPropertyName"] = StringUtils::toString(m_OutputAttributeName);
	mapAttr["InputFieldIndex"] = StringUtils::toString(m_inputFieldIndex);
	mapAttr["Operator"] = StringUtils::toString(m_operatorName);
	mapAttr["CellResolution"] = StringUtils::toString(m_cellResolution);

	return mapAttr;
}

void STDMETHODCALLTYPE AggregateCoverageCalculator::setAttributes( std::map< std::string, std::string > const & mapAttr )
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"InputFieldIndex",int,m_inputFieldIndex);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Operator",m_operatorName);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"OutputPropertyName",m_OutputAttributeName);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"CellResolution",int,m_cellResolution);

	if ("Covered Area" == m_operatorName) 
	{
		m_inputFieldIndex = 0;
		m_operatorFactory.reset(new CoverageAggregationOperatorFactory<CoverageCoverOperator>());
	}
	else if ("Minimum" == m_operatorName) 
	{
		m_operatorFactory.reset( new CoverageAggregationOperatorFactory<CoverageMinOperator>());
	}
	else if ("Maximum" == m_operatorName) 
	{
		m_operatorFactory.reset( new CoverageAggregationOperatorFactory<CoverageMaxOperator>());
	}
	else if ("Sum" == m_operatorName) 
	{
		m_operatorFactory.reset( new CoverageAggregationOperatorFactory<CoverageSumOperator>());
	}
	else if ("Average" == m_operatorName) 
	{
		m_operatorFactory.reset( new CoverageAggregationOperatorFactory<CoverageAverageOperator>());
	}
	else if ("AreaDensitySum" == m_operatorName) 
	{
		m_operatorFactory.reset( new CoverageAggregationOperatorFactory<CoverageAreaDensitySumOperator>());
	}	
	else
	{
		m_operatorFactory.reset( new CoverageAggregationOperatorFactory<CoverageCoverOperator>());
		m_operatorName = "Covered Area";
		m_inputFieldIndex = 0;
	}
}

IProcess::eInitStatus AggregateCoverageCalculator::initImpl()
{
	m_outputDefinition = PYXTableDefinition::create();
	PYXPointer<PYXTableDefinition> definition= m_operatorFactory->create()->getOutputDefinition();
	if (""==m_OutputAttributeName)
	{
		m_OutputAttributeName=definition->getFieldDefinition(0).getName();
	}
	m_outputDefinition->addFieldDefinition(m_OutputAttributeName, PYXFieldDefinition::knContextNormal, definition->getFieldDefinition(0).getType(), definition->getFieldCount() );

	m_coverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();

	if (!m_coverage)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input is not ICoverage");
		return knFailedToInit;
	}

	if (m_inputFieldIndex < 0 || m_inputFieldIndex >= m_coverage->getCoverageDefinition()->getFieldCount())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input field index is out of range");
		return knFailedToInit;
	}

	return knInitialized;
}

AggregateCoverageCalculator::AggregateCoverageCalculator()
{
	m_inputFieldIndex = 0;
	m_operatorName = "Covered Area";
	m_OutputAttributeName = "";
	m_cellResolution = -1;
}


//////////////////////////////////////////////////////////////////////////
//ICoverageAggregationOperator
//////////////////////////////////////////////////////////////////////////

void ICoverageAggregationOperator::aggregate(const PYXPointer<PYXValueTile> & tile, const PYXPointer<PYXInnerTileIntersectionIterator> & neededCells)
{
	const PYXIcosIndex & root = tile->getTile().getRootIndex();
	int resolution = tile->getTile().getCellResolution();
	int cellCount = PYXIcosMath::getCellCount(root,resolution);

	double m_posZeroCellSize = SnyderProjection::getInstance()->calcCellAreaOnReferenceSphere(PYXIcosMath::calcIndexFromOffset(root,resolution,0)); //could be pentagon...
	double m_cellSize = SnyderProjection::getInstance()->calcCellAreaOnReferenceSphere(PYXIcosMath::calcIndexFromOffset(root,resolution,1)); //must be hexagon.

	PYXValue value;

	for(;!neededCells->end();neededCells->next())
	{
		//check if we need this tile...
		PYXInnerTileIntersection intersection = neededCells->getIntersection();
		if(intersection == knIntersectionNone)
		{
			continue;
		}

		//count cover cells
		PYXIcosIndex index = neededCells->getTile().asTile().getRootIndex();

		if(index.getResolution() >= resolution)
		{
			index.setResolution(resolution);
			int nPos = PYXIcosMath::calcCellPosition(root, index);
			if(nPos < cellCount)
			{
				value = tile->getValue(nPos,0);
				aggregate(index,nPos==0?m_posZeroCellSize:m_cellSize,value,intersection);
			}
		}
		else if (index.getResolution() < resolution)
		{
			int tileCellCount = PYXIcosMath::getCellCount(index,resolution);

			index.setResolution(resolution);
			int nPos = PYXIcosMath::calcCellPosition(root, index);

			for(int offset = 0;offset < tileCellCount; ++offset)
			{
				int pos = nPos + offset;
				if(pos < cellCount)
				{
					value = tile->getValue(pos,0);
					aggregate(index,pos==0?m_posZeroCellSize:m_cellSize,value,intersection);
				}
			}

		}
	}
}

//////////////////////////////////////////////////////////////////////////
//CoverageCoverOperator
//////////////////////////////////////////////////////////////////////////

void CoverageCoverOperator::aggregate( const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection )
{
	if (!value.isNull())
	{
		if (cell.isHexagon())
		{
			cellSize = cellArea;
		}
		if (intersection == knIntersectionPartial)
		{
			partialCount++;
		}
		addCount++;
		m_areaInSqaureMetter += cellArea;
	}
}

PYXValue CoverageCoverOperator::getResult() const
{
	TRACE_INFO((double)m_areaInSqaureMetter << " area calculated using sum on " << addCount << " cells (" << partialCount << " partial) == " << (addCount*cellSize) << " using multiply. error is " << (100.0*partialCount/addCount) << "% min area = " << ((addCount-partialCount)*cellSize) );
	return PYXValue(m_areaInSqaureMetter);
}

PYXPointer<PYXTableDefinition> CoverageCoverOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Area", PYXFieldDefinition::knContextArea , PYXValue::knDouble, 1 );
	}
	return m_outputDefinition;
}


//////////////////////////////////////////////////////////////////////////
//CoverageMinOperator
//////////////////////////////////////////////////////////////////////////
void CoverageMinOperator::aggregate( const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection )
{
	if (!value.isNull())
	{
		if (m_foundValue)
		{
			m_value = std::min(m_value,value.getDouble());
		}
		else
		{
			m_value = value.getDouble();
			m_foundValue = true;
		}
	}
}

PYXPointer<PYXTableDefinition> CoverageMinOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Min", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );

	}
	return m_outputDefinition;
}

//////////////////////////////////////////////////////////////////////////
//CoverageMaxOperator
//////////////////////////////////////////////////////////////////////////
void CoverageMaxOperator::aggregate( const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection )
{
	if (!value.isNull())
	{
		if (m_foundValue)
		{
			m_value = std::max(m_value,value.getDouble());
		}
		else
		{
			m_value = value.getDouble();
			m_foundValue = true;
		}
	}	
}


PYXPointer<PYXTableDefinition> CoverageMaxOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Max", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );
	}
	return m_outputDefinition;
}

//////////////////////////////////////////////////////////////////////////
//CoverageSumOperator
//////////////////////////////////////////////////////////////////////////
void CoverageSumOperator::aggregate( const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection )
{
	if (!value.isNull())
	{
		m_value += value.getDouble();
	}
}

PYXPointer<PYXTableDefinition> CoverageSumOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Sum", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );
	}
	return m_outputDefinition;
}


//////////////////////////////////////////////////////////////////////////
//CoverageSumOperator
//////////////////////////////////////////////////////////////////////////
void CoverageAreaDensitySumOperator::aggregate( const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection )
{
	if (!value.isNull())
	{
		m_value += value.getDouble() * cellArea;
	}
}

PYXPointer<PYXTableDefinition> CoverageAreaDensitySumOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Sum", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );
	}
	return m_outputDefinition;
}


//////////////////////////////////////////////////////////////////////////
//CoverageAverageOperator
//////////////////////////////////////////////////////////////////////////
void CoverageAverageOperator::aggregate(  const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection )
{
	if (!value.isNull())
	{
		m_sum += value.getDouble();
		m_count ++;
	}	
}

PYXValue CoverageAverageOperator::getResult() const
{
	if (m_count>0)
	{
		return PYXValue(m_sum/m_count);
	}
	else
	{
		return PYXValue();
	}
}

PYXPointer<PYXTableDefinition> CoverageAverageOperator::getOutputDefinition()
{
	if(!m_outputDefinition)
	{
		m_outputDefinition= PYXTableDefinition::create();
		m_outputDefinition->addFieldDefinition("Average", PYXFieldDefinition::knContextNormal, PYXValue::knDouble, 1 );
	}
	return m_outputDefinition;
}
