/******************************************************************************
pyx_numeric_histogram.cpp

begin		: Thursday, November 15, 2012 5:03:23 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/data/impl/numeric_histogram_impl.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/exceptions.h"

// standard includes
#include <cassert>
std::vector<PYXHistogramBin> PYXNumericHistogram::getBins() const
{
	std::vector<NumericHistogram<double>::LeafBin> bins;

	m_histogram.getLeafBins(bins);

	std::vector<PYXHistogramBin> result;
	result.reserve(bins.size());

	Range<double> boundaries = m_histogram.getBoundaries();
	PYXHistogramBin resultBin;
	
	for(auto & bin : bins)
	{
		if (bin.count.max == 0) continue;
		
		//create a PYXValue range that is bounded using the boundaries of the histogram values
		resultBin.range = Range<PYXValue>::createClosedOpen(PYXValue(std::max(bin.range.min,boundaries.min)),PYXValue(std::min(bin.range.max,boundaries.max)));
		resultBin.count = bin.count;

		result.push_back(resultBin);
	}

	return result;
}

void PYXNumericHistogram::add( const PYXHistogram & histogram )
{
	const PYXNumericHistogram * other = dynamic_cast<const PYXNumericHistogram*>(&histogram);

	if (!other)
	{
		PYXTHROW(PYXException,"other histogram is not NumericFeaturesSummaryFieldHistogram");
	}

	m_histogram.add(other->m_histogram);
}



std::vector<PYXCellHistogramBin> PYXNumericCellHistogram::getCellBins() const
{
	std::vector<PYXHistogramBin> bins = m_histogram.getBins();

	std::vector<PYXCellHistogramBin> result(bins.size());

	int i=0;
	for(std::vector<PYXHistogramBin>::iterator it=bins.begin();it!=bins.end();++it,++i)
	{
		result[i].range = it->range;
		result[i].count = it->count;
		result[i].area = Range<double>::createClosedOpen((it->count.min * m_cellArea),(it->count.max * m_cellArea));
	}

	return result;
}

std::vector<PYXHistogramBin> PYXNumericCellHistogram::getNormalizedBins(PYXHistogram::Normalization mode,int binCount) const
{
	return m_histogram.getNormalizedBins(mode,binCount);
}

std::vector<PYXCellHistogramBin> PYXNumericCellHistogram::getCellNormalizedBins(PYXHistogram::Normalization mode,int binCount) const
{
	std::vector<PYXHistogramBin> binResult = getNormalizedBins(mode,binCount);
	std::vector<PYXCellHistogramBin> result;
	result.reserve(binResult.size());

	PYXCellHistogramBin areaBin;

	for(auto & bin : binResult)
	{
		areaBin.range = bin.range;
		areaBin.count = bin.count;
		areaBin.area = Range<double>::createClosedOpen((bin.count.min * m_cellArea),(bin.count.max * m_cellArea));

		result.push_back(areaBin);
	}

	return result;
}

void PYXNumericCellHistogram::add( const PYXHistogram & histogram )
{
	const PYXNumericCellHistogram * other = dynamic_cast<const PYXNumericCellHistogram*>(&histogram);

	if (!other)
	{
		PYXTHROW(PYXException,"other histogram is not PYXNumericCellHistogram");
	}

	m_histogram.add(other->m_histogram);
}

Range<double> PYXNumericCellHistogram::getArea() const
{
	Range<int> count = m_histogram.getFeatureCount();
	return Range<double>::createClosedOpen(count.min*m_cellArea,count.max*m_cellArea);
}

Range<double> PYXNumericCellHistogram::getArea( Range<PYXValue> range ) const
{
	Range<int> count = m_histogram.getFeatureCount(range);
	return Range<double>::createClosedOpen(count.min*m_cellArea,count.max*m_cellArea);
}
