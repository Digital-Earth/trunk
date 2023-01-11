#ifndef PYXIS__DATA__HISTOGRAM_H
#define PYXIS__DATA__HISTOGRAM_H
/******************************************************************************
histogram.h

begin		: Friday, November 23, 2012 4:33:58 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/feature.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/range.h"

// standard includes

struct PYXLIB_DECL PYXHistogramBin
{
	Range<PYXValue> range;
	Range<int>      count;
};

struct PYXLIB_DECL PYXCellHistogramBin : public PYXHistogramBin
{
	Range<double>   area;
};

class PYXLIB_DECL PYXHistogram : public PYXObject
{
public:
	enum Normalization
	{
		knLinearBins,
		knNormalizedBin
	};

	//creates an histogram of values of the given field collected from the given feature iterator.
	static PYXPointer<PYXHistogram> createFromFeatures(PYXPointer<FeatureIterator> features, int fieldIndex);

public:
	//get the feature count that was used to build the histogram.
	virtual Range<int> getFeatureCount() const = 0;

	//get the feature count of features with matching value that is contained by the given range.
	virtual Range<int> getFeatureCount(Range<PYXValue> range) const = 0;

	// get summation of the field values (unsupported for non-numeric types)
	virtual PYXValue getSum() const = 0;

	// get average of the field values (unsupported for non-numeric types)
	virtual PYXValue getAverage() const = 0;

	// get summation of the field values square (unsupported for non-numeric types)
	virtual PYXValue getSumSquare() const = 0;

	//get the min/max values of the histogram
	virtual Range<PYXValue> getBoundaries() const = 0;

	virtual std::vector<PYXHistogramBin> getBins() const = 0;

	//provide default implementation of normalize the histogram
	virtual std::vector<PYXHistogramBin> getNormalizedBins(Normalization mode,int binCount) const;

	virtual void add(const PYXValue & value) = 0;

	virtual void add(const PYXHistogram & histogram) = 0;
};



class PYXLIB_DECL PYXCellHistogram : public PYXHistogram
{
public:

	virtual int getCellResolution() const = 0;

	virtual Range<double> getArea() const = 0;

	virtual Range<double> getArea(Range<PYXValue> range) const = 0;

	virtual std::vector<PYXCellHistogramBin> getCellBins() const = 0;

	virtual std::vector<PYXCellHistogramBin> getCellNormalizedBins(Normalization mode,int binCount) const = 0;
};


#endif // guard
