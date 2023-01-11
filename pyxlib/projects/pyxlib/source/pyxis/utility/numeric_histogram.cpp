/******************************************************************************
numeric_histogram.cpp

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "numeric_histogram.h"
#include "tester.h"
#include "exceptions.h"
#include "math_utils.h"

// standard includes
#include <cassert>

class HistogramDoubleTester
{
private:
	typedef NumericHistogram<double> HistogramDouble;

public:
	static void test()
	{
		{
			//test generation of many numbers
			srand(10);

			std::vector<double> values;
			double sum=0;
			double sumSquare=0;

			for(int i=0;i<10000;i++)
			{
				double number = rand()%1000+(rand()%100000)/100000.0;
				values.push_back(number);
				sum += number;
				sumSquare += number * number;
			}

			HistogramDouble hist(values.begin(),values.end());

			hist.limit(1000);

			TEST_ASSERT(MathUtils::equal(hist.getSum()/sum,1.0));
			TEST_ASSERT(MathUtils::equal(hist.getSumSquare()/sumSquare,1.0));

			//test counting of values
			for(int i=0;i<1000;i++)
			{
				Range<double> r(i,i+0.5,knClosed,knOpen);
				int realCount = 0;

				for(unsigned int j=0;j<values.size();++j)
				{
					if (r.contains(values[j]))
					{
						realCount ++;
					}
				}

				Range<int> histCount = hist.count(r);

				TEST_ASSERT(histCount.contains(realCount));
			}

			//test adding histograms
			double number2 = 999;
			HistogramDouble hist2(number2);

			hist.add(hist2);

			TEST_ASSERT(MathUtils::equal(hist.getSum()/(sum+number2),1.0));
			TEST_ASSERT(MathUtils::equal(hist.getSumSquare()/(sumSquare+(number2*number2)),1.0));
		}

		{
			//check boundary case when the boundary is a power of 2.
			std::vector<double> values;
			values.push_back(0);
			values.push_back(256);

			HistogramDouble hist(values.begin(),values.end());

			TEST_ASSERT(MathUtils::equal(hist.getBoundaries().min,0));
			TEST_ASSERT(MathUtils::equal(hist.getBoundaries().max,256));
			TEST_ASSERT(hist.count() == 2);
			TEST_ASSERT(hist.count(RangeDouble(0)) == 1);
			TEST_ASSERT(hist.count(RangeDouble(256)) == 1);
			TEST_ASSERT(hist.count(RangeDouble::createClosedOpen(0,256)) == 1);
		}
	}
};


//! The unit test class
Tester<HistogramDoubleTester> gTester;
