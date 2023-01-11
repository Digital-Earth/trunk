#ifndef PYXIS__DATA__IMPL_STRING_HISTOGRAM_H
#define PYXIS__DATA__IMPL_STRING_HISTOGRAM_H
/******************************************************************************
string_histogram.h

begin		: Thursday, November 15, 2012 5:03:23 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/range.h"
#include "pyxis/utility/wire_buffer.h"
#include "pyxis/utility/string_histogram.h"
#include "pyxis/data/histogram.h"

///////////////////////////////////////////////////////////////////////////////
// PYX String Histogram
///////////////////////////////////////////////////////////////////////////////
class PYXLIB_DECL PYXStringHistogram : public PYXHistogram
{
private:
	StringHistogram m_histogram;

public:
	static PYXPointer<PYXStringHistogram > create();

	PYXStringHistogram();

	static PYXPointer<PYXStringHistogram > create(const StringHistogram & other);

	PYXStringHistogram(const StringHistogram & other);

public:
	virtual Range<int> getFeatureCount() const {
		return m_histogram.count();
	}

	virtual Range<int> getFeatureCount(Range<PYXValue> range) const 
	{
		return m_histogram.count(RangeString::createClosedOpen(range.min.getString(),range.max.getString()));
	}

	// get summation of the field values (not supported for not numeric types)
	virtual PYXValue getSum() const 
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

	// get average of the field values (not supported for not numeric types)
	virtual PYXValue getAverage() const 
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

	// get summation of the field values square (not supported for not numeric types)
	virtual PYXValue getSumSquare() const 
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}

	virtual Range<PYXValue> getBoundaries() const
	{
		return Range<PYXValue>::createClosedClosed(PYXValue(m_histogram.getBoundaries().min),PYXValue(m_histogram.getBoundaries().max));
	}

	virtual std::vector<PYXHistogramBin> getBins() const;

	virtual void add(const PYXValue & value);

	virtual void add(const PYXHistogram & histogram);

	StringHistogram & getStringHistogram() { return m_histogram; }

	const StringHistogram & getStringHistogram() const { return m_histogram; }
};


#endif // guard
