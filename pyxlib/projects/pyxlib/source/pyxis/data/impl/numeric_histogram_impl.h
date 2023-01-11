#ifndef PYXIS__DATA__IMPL_NUMERIC_HISTOGRAM_H
#define PYXIS__DATA__IMPL_NUMERIC_HISTOGRAM_H
/******************************************************************************
numeric_histogram.h

begin		: Thursday, November 15, 2012 5:03:23 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/range.h"
#include "pyxis/utility/wire_buffer.h"
#include "pyxis/data/histogram.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/numeric_histogram.h"

///////////////////////////////////////////////////////////////////////////////
// Pyx Numeric Histogram
///////////////////////////////////////////////////////////////////////////////
class PYXLIB_DECL PYXNumericHistogram : public PYXHistogram
{
protected:
	NumericHistogram<double> m_histogram;

public:
	static PYXPointer<PYXNumericHistogram> create(const NumericHistogram<double> & other)
	{
		return PYXNEW(PYXNumericHistogram,other);
	}

	PYXNumericHistogram(const NumericHistogram<double> & other) : m_histogram(other)
	{
	}

public:
	virtual Range<int> getFeatureCount() const
	{
		return Range<int>(m_histogram.count());
	}

	virtual Range<int> getFeatureCount(Range<PYXValue> range) const 
	{
		return m_histogram.count(Range<double>::createClosedOpen(range.min.getDouble(),range.max.getDouble()));
	}

	// get summation of the field values (not supported for not numeric types)
	virtual PYXValue getSum() const 
	{
		return PYXValue(m_histogram.getSum());
	}

	// get average of the field values (not supported for not numeric types)
	virtual PYXValue getAverage() const 
	{
		return PYXValue(m_histogram.getAverage());
	}

	// get summation of the field values square (not supported for not numeric types)
	virtual PYXValue getSumSquare() const 
	{
		return PYXValue(m_histogram.getSumSquare());
	}

	virtual Range<PYXValue> getBoundaries() const
	{
		Range<double> boundary(m_histogram.getBoundaries());

		return Range<PYXValue>::createClosedClosed(PYXValue(boundary.min),PYXValue(boundary.max));
	}

	virtual std::vector<PYXHistogramBin> getBins() const;

	virtual void add(const PYXValue & value)
	{
		m_histogram.add(NumericHistogram<double>(value.getDouble()));
	}

	virtual void add(const PYXHistogram & histogram);

	NumericHistogram<double> & getDoubleHistogram() { return m_histogram; }

	const NumericHistogram<double> & getDoubleHistogram() const { return m_histogram; }
};


class PYXLIB_DECL PYXNumericCellHistogram : public PYXCellHistogram
{
protected:
	PYXNumericHistogram m_histogram;
	double m_cellArea;
	int m_cellResolution;

public:
	static PYXPointer<PYXNumericCellHistogram> create(const NumericHistogram<double> & other, int wantedResolution)
	{
		return PYXNEW(PYXNumericCellHistogram,other,wantedResolution);
	}

	PYXNumericCellHistogram(const NumericHistogram<double> & other,int wantedResolution) : m_histogram(other), m_cellResolution(wantedResolution)
	{
		PYXIcosIndex index = PYXIcosIndex("A-0");
		index.setResolution(wantedResolution);
		m_cellArea = SnyderProjection::getInstance()->calcCellAreaOnReferenceSphere(index);
	}

public:
	virtual Range<int> getFeatureCount() const
	{
		return m_histogram.getFeatureCount();
	}

	virtual Range<int> getFeatureCount(Range<PYXValue> range) const 
	{
		return m_histogram.getFeatureCount(range);
	}

	// get summation of the field values (not supported for not numeric types)
	virtual PYXValue getSum() const 
	{
		return m_histogram.getSum();
	}

	// get average of the field values (not supported for not numeric types)
	virtual PYXValue getAverage() const 
	{
		return m_histogram.getAverage();
	}

	// get summation of the field values square (not supported for not numeric types)
	virtual PYXValue getSumSquare() const 
	{
		return m_histogram.getSumSquare();
	}

	virtual Range<PYXValue> getBoundaries() const
	{
		return m_histogram.getBoundaries();
	}

	virtual std::vector<PYXHistogramBin> getBins() const
	{
		return m_histogram.getBins();
	}

	virtual std::vector<PYXHistogramBin> getNormalizedBins(Normalization mode,int binCount) const;

	virtual void add(const PYXValue & value)
	{
		m_histogram.add(value);
	}

	virtual void add(const PYXHistogram & histogram);

	NumericHistogram<double> & getDoubleHistogram() { return m_histogram.getDoubleHistogram(); }

	const NumericHistogram<double> & getDoubleHistogram() const { return m_histogram.getDoubleHistogram(); }

	virtual std::vector<PYXCellHistogramBin> getCellBins() const;

	virtual std::vector<PYXCellHistogramBin> getCellNormalizedBins(Normalization mode,int binCount) const;

	virtual int getCellResolution() const {return m_cellResolution;}

	virtual Range<double> getArea() const;

	virtual Range<double> getArea( Range<PYXValue> range ) const;

};


#endif // guard
