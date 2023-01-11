/******************************************************************************
pyx_string_histogram.cpp

begin		: Thursday, November 15, 2012 5:03:23 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"

#include "pyxis/data/impl/string_histogram_impl.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/bit_utils.h"

#include "boost/bind.hpp"

// standard includes
#include <cassert>

std::vector<PYXHistogramBin> PYXStringHistogram::getBins() const
{
	std::vector<StringHistogram::LeafBin> bins;
	m_histogram.getLeafBins(bins);

	std::vector<PYXHistogramBin> result(bins.size());

	int i=0;
	for(std::vector<StringHistogram::LeafBin>::const_iterator it=bins.begin();it!=bins.end();++it,++i)
	{
		result[i].range = Range<PYXValue>(PYXValue(it->range.min),PYXValue(it->range.max),it->range.minType,it->range.maxType);
		result[i].count = it->count;
	}

	return result;
}

void PYXStringHistogram::add( const PYXHistogram & histogram )
{
	const PYXStringHistogram * other = dynamic_cast<const PYXStringHistogram*>(&histogram);

	if (!other)
	{
		PYXTHROW(PYXException,"other histogram is not StringFeaturesSummaryFieldHistogram");
	}

	m_histogram.add(other->m_histogram);
}

void PYXStringHistogram::add( const PYXValue & value )
{
	m_histogram.add(value.getString());
}

PYXStringHistogram::PYXStringHistogram() : m_histogram()
{

}

PYXStringHistogram::PYXStringHistogram( const StringHistogram & other ) : m_histogram(other)
{

}

PYXPointer<PYXStringHistogram > PYXStringHistogram::create()
{
	return PYXNEW(PYXStringHistogram);
}

PYXPointer<PYXStringHistogram > PYXStringHistogram::create( const StringHistogram & other )
{
	return PYXNEW(PYXStringHistogram,other);
}
