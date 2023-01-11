/******************************************************************************
coverage_histogram_calculator.cpp

begin		: Wednesday, October 31, 2012 8:49:41 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "coverage_histogram_calculator.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/derm/iterator.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/data/constant_record.h"

// standard includes
#include <algorithm>
#include <cassert>
#include "pyxis/derm/snyder_projection.h"

// {5636F858-F2CA-44c9-8E15-F00936FDDE75}
 PYXCOM_DEFINE_CLSID(CoverageHistogramCalculator,
					0x5636f858, 0xf2ca, 0x44c9, 0x8e, 0x15, 0xf0, 0x9, 0x36, 0xfd, 0xde, 0x75);

PYXCOM_CLASS_INTERFACES(CoverageHistogramCalculator, ICoverageHistogramCalculator::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(CoverageHistogramCalculator, "Create Coverage Histogram", "Creates histogram of a coverage inside a given feature", "Analysis/Coverages/Statistics",
					ICoverageHistogramCalculator::iid, IProcess::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "The coverage that will be used for histogram calculations")
IPROCESS_SPEC_END

CoverageHistogramCalculator::CoverageHistogramCalculator()
{
}

CoverageHistogramCalculator::~CoverageHistogramCalculator()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////
std::string STDMETHODCALLTYPE CoverageHistogramCalculator::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"

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
std::map< std::string, std::string > STDMETHODCALLTYPE CoverageHistogramCalculator::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["CellResolution"] = StringUtils::toString(m_cellResolution);
	return mapAttr;
}

void STDMETHODCALLTYPE CoverageHistogramCalculator::setAttributes( std::map< std::string, std::string > const & mapAttr )
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"CellResolution",int,m_cellResolution);
}
IProcess::eInitStatus CoverageHistogramCalculator::initImpl()
{
	m_spCoverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();

	if (!m_spCoverage)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input is not ICoverage");
		return knFailedToInit;
	}
	m_cellResolution = -1;
	return knInitialized;
}


int CoverageHistogramCalculator::findResolution(const PYXPointer<PYXGeometry> & geometry,int initialResolution,double maxPartialPerecnt) const
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

PYXPointer<PYXCellHistogram> STDMETHODCALLTYPE CoverageHistogramCalculator::getHistogram( int fieldIndex,PYXPointer<IFeature> spFeature )
{

	PYXPointer<PYXGeometry> geometry = spFeature->getGeometry();

	if(m_cellResolution < 0)
	{
		//find a good resolution to perform aggregation on...
		m_cellResolution = m_spCoverage->getGeometry()->getCellResolution();
		//make it a resolution that would be around 1~6 depth-11 tiles.
		PYXBoundingCircle circle = geometry->getBoundingCircle();
		if (circle.getRadius() != 0)
		{
			m_cellResolution = std::min(PYXMath::knMaxAbsResolution,PYXBoundingCircle::estimateResolutionFromRadius(circle.getRadius())+11);
			m_cellResolution = findResolution(geometry,std::max(2,m_cellResolution-11),0.05)+11;
		}

		m_histogram = PYXNumericCellHistogram::create(m_numericHistogram,m_cellResolution);
	}

	//found containing cells on a single low resolution
	PYXPointer<PYXTileCollection> coverGeometry = PYXTileCollection::create();
	geometry->copyTo(coverGeometry.get(),std::max(2,m_cellResolution-11));

	//NOTE: this could lead to 20% error if polygon has many partial cells
	for(PYXPointer<PYXIterator> iterator = coverGeometry->getIterator();!iterator->end();iterator->next())
	{
		PYXTile tile(iterator->getIndex(),m_cellResolution);

		std::vector<PYXInnerTile> innerTiles = PYXInnerTile::createInnerTiles(tile);

		PYXPointer<PYXValueTile> valueTile = m_spCoverage->getFieldTile(tile.getRootIndex(),tile.getCellResolution(),fieldIndex);

		for(std::vector<PYXInnerTile>::iterator it = innerTiles.begin(); it != innerTiles.end(); ++it)
		{
			PYXPointer<PYXInnerTileIntersectionIterator> geomIt = geometry->getInnerTileIterator(*it);
			if(valueTile && geomIt)
			{
				aggregate(valueTile,geomIt);
			}
		}
	}

	return m_histogram;
}

void CoverageHistogramCalculator::aggregate(const PYXPointer<PYXValueTile> & tile, const PYXPointer<PYXInnerTileIntersectionIterator> & neededCells)
{
	const PYXIcosIndex & root = tile->getTile().getRootIndex();
	int resolution = tile->getTile().getCellResolution();
	int cellCount = PYXIcosMath::getCellCount(root,resolution);

	double m_posZeroCellSize = SnyderProjection::getInstance()->calcCellAreaOnReferenceSphere(PYXIcosMath::calcIndexFromOffset(root,resolution,0)); //could be pentagon...
	double m_cellSize = SnyderProjection::getInstance()->calcCellAreaOnReferenceSphere(PYXIcosMath::calcIndexFromOffset(root,resolution,1)); //must be hexagon.

	PYXValue value = tile->getTypeCompatibleValue(0);

	std::vector<double> valuesToAdd;
	valuesToAdd.reserve(1000);

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
				if (tile->getValue(nPos, 0, &value))
				{
					valuesToAdd.push_back(value.getDouble(0));	
				}
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
					if (tile->getValue(pos, 0, &value))
					{
						valuesToAdd.push_back(value.getDouble(0));	
					}
				}
			}
		}

		if (valuesToAdd.size() >= 1000)
		{
			m_histogram->add(*PYXNumericCellHistogram::create(NumericHistogram<double>(valuesToAdd.begin(),valuesToAdd.end()),m_cellResolution));
			valuesToAdd.clear();
		}
	}

	if (!valuesToAdd.empty())
	{
		m_histogram->add(*PYXNumericCellHistogram::create(NumericHistogram<double>(valuesToAdd.begin(),valuesToAdd.end()),m_cellResolution));
	}
}



