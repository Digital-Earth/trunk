/******************************************************************************
histogram.cpp

begin		: Friday, November 23, 2012 4:33:58 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/data/histogram.h"
#include "pyxis/utility/numeric_histogram.h"
#include "pyxis/utility/string_histogram.h"
#include "pyxis/data/impl/numeric_histogram_impl.h"
#include "pyxis/data/impl/string_histogram_impl.h"

std::vector<PYXHistogramBin> PYXHistogram::getNormalizedBins(PYXHistogram::Normalization mode,int binCount) const
{
	Range<PYXValue> boundaries = getBoundaries();
	if (boundaries.min.isString())
	{
		//we can't make a linear range bins - so we will do the best we can - and that normalized the bins
		mode = PYXHistogram::knNormalizedBin;
	}

	switch(mode)
	{
	case knLinearBins:
		{
			std::vector<PYXHistogramBin> result;
			result.reserve(binCount);
			double min = boundaries.min.getDouble();
			double max =boundaries.max.getDouble();
			double step = (max - min)/binCount;

			PYXHistogramBin linearBin;

			
			for(int i=0;i<binCount;++i)
			{
				auto range = Range<PYXValue>::createClosedOpen(PYXValue(min+step*i),PYXValue(min+step*(i+1)));
				linearBin.count = getFeatureCount(range);
				linearBin.range = Range<PYXValue>::createClosedOpen(PYXValue(range.min),PYXValue(range.max));
				
				result.push_back(linearBin);
			}

			return result;
		}
	case knNormalizedBin:
		{
			int totalCount = getFeatureCount().max;
			int largeBins = 0;
			int countPerBin = std::max(1,totalCount / binCount);

			std::vector<PYXHistogramBin> bins = getBins();			

			//  Issue: large bins
			//  -----------------
			//  think about the following disterbution of values: 100 differnet values. 1 value has 99% of the features.
			//  if we want 20 bins - we would 5% per bin - which result in two bins: one bin with 99% count and another
			//  bin with 1% of count with the rest of the 99 values.
			//  where what we want is one bin with 99% and then another 19 bins with 0.01% count per bin.
			//  I call this 'large bin', and we deal with them by recalculating % per bin without all the large bins.

			//find all 'large bins'... we would just add them as they are.
			for(auto & bin : bins)
			{
				if (bin.count.max > countPerBin*2)
				{
					totalCount -= bin.count.middle();
					largeBins++;
				}
			}

			//recalcaulte countPerBin without the 'large bins'
			countPerBin = std::max(1,totalCount / (binCount-largeBins));

			std::vector<PYXHistogramBin> result;
			result.reserve(binCount);
			
			PYXHistogramBin currentBin;
			currentBin.count = Range<int>(0);
			
			for(auto & bin : bins)
			{				
				if (currentBin.count.max > 0) 
				{
					if (currentBin.count.max + bin.count.max < countPerBin)
					{
						currentBin.count.min += bin.count.min;
						currentBin.count.max += bin.count.max;

						//merge the range
						if (currentBin.range.min.compare(bin.range.min)>0)
						{
							currentBin.range.min = bin.range.min;
						}
						if (currentBin.range.max.compare(bin.range.min)<0)
						{
							currentBin.range.max = bin.range.max;
						}						
						
						continue;
					}
					else
					{
						result.push_back(currentBin);
					}
				}
				//start new bin...
				currentBin.count = bin.count;
				currentBin.range = bin.range;			
			}

			//add the lest over
			if (currentBin.count.max > 0) 
			{
				result.push_back(currentBin);
			}

			return result;
		}
	default:
		PYXTHROW(PYXException,"Unknown normalization mode");
	}	
}

//creates an histogram of values of the given field collected from the given feature iterator.
PYXPointer<PYXHistogram> PYXHistogram::createFromFeatures(PYXPointer<FeatureIterator> features, int fieldIndex)
{
	if (features->end()) 
	{
		return nullptr;
	}

	auto firstFeature = features->getFeature();

	if (!firstFeature) 
	{
		return nullptr;
	}

	if (firstFeature->getDefinition()->getFieldDefinition(fieldIndex).isNumeric())
	{
		std::vector<double> values;

		for (;!features->end();features->next())
		{
			values.push_back(features->getFeature()->getFieldValue(fieldIndex).getDouble());
		}

		return PYXNumericHistogram::create(NumericHistogram<double>(values.begin(),values.end()));
	}
	else 
	{
		std::vector<std::string> values;

		for (;!features->end();features->next())
		{
			values.push_back(features->getFeature()->getFieldValue(fieldIndex).getString());
		}

		return PYXStringHistogram::create(StringHistogram(values.begin(),values.end()));
	}

}