#ifndef PYXIS__UTILITY__NUMERIC_HISTOGRAM_H
#define PYXIS__UTILITY__NUMERIC_HISTOGRAM_H
/******************************************************************************
numberic_histogram.h

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "value.h"
#include "range.h"
#include "wire_buffer.h"

///////////////////////////////////////////////////////////////////////////////
// NumericHistogram
///////////////////////////////////////////////////////////////////////////////

template<typename T>
class NumericHistogram
{
public:
	struct LeafBin
	{
		Range<T> range;
		Range<int> count;

		LeafBin()
		{
		}

		LeafBin(const Range<T> & _range,const Range<int> & _count) : range(_range), count(_count)
		{
		}
	};

public:
	int count() const
	{
		return m_rootBin->binCount;
	}

	Range<int> count(const Range<T> & range) const
	{
		return m_rootBin->count(range);
	}

	//return the estimate precential of the a value (result between 0 and 100)
	Range<double> percentile(const T & value) const
	{
		Range<int> result = count(Range<T>(T(),value,knInfinite,knClosed));
		double ratio = 100.0/count();
		return Range<double>::createClosedClosed(result.min * ratio ,result.max * ratio);
	}

	const double & getSum() const
	{
		return m_sum;
	}
	const double & getSumSquare() const
	{
		return m_sumSquare;
	}
	double getAverage() const
	{
		return getSum()/count();
	}
	const Range<T> & getBoundaries() const
	{
		return m_boundaries;
	}

	NumericHistogram() : m_boundaries(T()), m_rootBin(new Bin(Range<T>(T()),0)), m_sum(0), m_sumSquare(0)
	{
	}

	NumericHistogram(const T & value) : m_boundaries(value), m_sum(value), m_sumSquare(value * value)
	{
		m_rootBin.reset(new Bin(Range<T>(value,value,knClosed,knOpen).normalize(),0));
		safeAdd(*m_rootBin,Bin(Range<T>(value),1));
		optimizeBins();
	}

	NumericHistogram(const NumericHistogram<T> & other): m_sum(0), m_sumSquare(0)
	{
		m_boundaries = other.m_boundaries;
		m_rootBin.reset(new Bin(other.m_rootBin->binRange,0));
		add(other);
		optimizeBins();
	}

	template<class InputIterator>
	NumericHistogram(InputIterator first,InputIterator last) : m_boundaries(T()), m_rootBin(new Bin(Range<T>(T()),0)), m_sum(0), m_sumSquare(0)
	{
		std::vector<T> values(first,last);
		std::sort(values.begin(),values.end());

		if (values.size()>0)
		{
			m_boundaries = Range<T>::createClosedClosed(values.front(),values.back());

			for(std::vector<T>::iterator it = values.begin(); it != values.end(); ++it)
			{
				m_sum += *it;
				m_sumSquare += (*it) * (*it);
			}

			auto normalizedBoundaries = Range<T>::createClosedOpen(values.front(),values.back()).normalize();

			m_rootBin.reset(createBins(normalizedBoundaries,values.begin(),values.end()));

			if (m_rootBin->isLeaf())
			{
				std::auto_ptr<Bin> oldRoot(m_rootBin.release());
				m_rootBin.reset(new Bin(normalizedBoundaries,0));
				safeAdd(*m_rootBin,*oldRoot);
			}

			optimizeBins();
		}
	}

	void add(const NumericHistogram<T> & other)
	{
		if (other.count()==0)
			return;
		
		if (count()==0)
		{
			m_boundaries = other.m_boundaries;
			m_rootBin->binRange = other.m_rootBin->binRange;
		}
		else
		{
			//make the rootBin range large enought to contain the other historgram range
			if (!m_rootBin->binRange.contains(other.m_rootBin->binRange))
			{
				m_rootBin->binRange = other.m_rootBin->binRange;
				//m_rootBin->binRange = m_rootBin->binRange.normalizeWith(other.m_rootBin->binRange);
			}

			m_boundaries.min = std::min(m_boundaries.min,other.m_boundaries.min);
			m_boundaries.max = std::max(m_boundaries.max,other.m_boundaries.max);
		}

		safeAdd(*m_rootBin,*(other.m_rootBin->left));
		safeAdd(*m_rootBin,*(other.m_rootBin->right));
		optimizeBins();
		m_sum += other.m_sum;
		m_sumSquare += other.m_sumSquare;
	}

	void limit(int binCount)
	{
		int maxError = (count()/binCount)+1;

		while(m_rootBin->childBinCount > binCount)
		{
			limit(*m_rootBin,maxError);
			maxError++;
		}
	}

	void getLeafBins(std::vector<LeafBin> & result) const
	{
		return getLeafBins(*m_rootBin,0,result);
	}

public:
	bool operator ==(const NumericHistogram & other) const
	{
		return compare(*m_rootBin,*(other.m_rootBin));
	}

public:
	template<class T>
	friend PYXWireBuffer & operator >> (PYXWireBuffer & buffer,NumericHistogram<T> & histogram);

	template<class T>
	friend PYXWireBuffer & operator << (PYXWireBuffer & buffer,const NumericHistogram<T> & histogram);

protected:
	class Bin : ObjectMemoryUsageCounter<Bin>
#ifdef INSTANCE_COUNTING
				, protected InstanceCounter
#endif
    {
	public:
        Range<T> binRange;
        int binCount;
		int childBinCount;
		std::auto_ptr<Bin> left;
		std::auto_ptr<Bin> right;

		Bin(const Range<T> & range, int count) 
			: binRange(range), binCount(count), childBinCount(0)
        {
			if (!binRange.single())
			{
				binRange.minType = knClosed;
				binRange.maxType = knOpen;
			}
        }

		~Bin()
		{
		}

        Range<int> count(const Range<T> & range) const
        {
            //empty bin
            if (binCount == 0)
                return Range<int>(0);

            //cover completely
            if (range.contains(binRange))
                return Range<int>(binCount);

            //not intersecting
            if (!range.intersects(binRange))
                return Range<int>(0);

            //final bin
            if (isLeaf())
				return Range<int>::createClosedClosed(0, binCount);

            //dig into lower cells
            int uncerentiyCount = binCount - left->binCount - right->binCount;

			Range<int> leftCount = left->count(range);
			Range<int> rightCount = right->count(range);

			Range<int> result = Range<int>::createClosedClosed(leftCount.min+rightCount.min,leftCount.max+rightCount.max + uncerentiyCount);
            return result;
        }

        int uncertaintyCount() const
        {
			if (isLeaf())
				return 0;

			return binCount - left->binCount - right->binCount;
        }

        bool isLeaf() const
        {
			return (left.get() == 0 && right.get() == 0);
        }
    };

protected:
	static void save(PYXWireBuffer & buffer, Bin & bin, bool complex)
	{
		int uncertainty = bin.uncertaintyCount();
        bool single = bin.binRange.single();
        bool leaf = bin.isLeaf();
        unsigned char code = 
            (single ? 0x80 : 0) +
            (leaf ? 0x40 : 0) +
            (complex ? 0x20 : 0);

        if (leaf)
        {
            if (bin.binCount < 16)
            {
                code += (unsigned char)bin.binCount;
            }
            else if (bin.binCount / 256 < 15)
            {
                code += (unsigned char) (16 + (bin.binCount/256));
				assert((code & 0x1F) != 31);
            }
            else
            {
                code += 31;
            }
        }
        else
        {
            if (uncertainty < 16)
            {
                code += (unsigned char) uncertainty;
            }
            else if (uncertainty / 256 < 15)
            {
                code += (byte)(16 + (uncertainty / 256));
				assert((code & 0x1F) != 31);
            }
            else
            {
                code += 31;
            }
        }

        buffer << code;
        
		if (leaf)
        {
            if (bin.binCount < 16)
            {

            }
            else if (bin.binCount / 256 < 15)
            {
                buffer << (unsigned char)(bin.binCount % 256);
            }
            else
            {
                buffer << bin.binCount;
            }
        }
        else
        {
            if (uncertainty < 16)
            {

            }
            else if (uncertainty / 256 < 15)
            {
				buffer << (unsigned char)(uncertainty % 256);
            }
            else
            {
				buffer << uncertainty;
            }
        }

        if (leaf)
        {
            if (single)
            {
				buffer << bin.binRange.min;
            }
            else if (complex)
            {
				buffer << bin.binRange.min << bin.binRange.max;
            }
        }
        else
        {
            if (complex)
            {
				buffer << bin.binRange.min << bin.binRange.max;
            }

			save(buffer,*(bin.left),!(bin.left->binRange == bin.binRange.lowerHalf()));
			save(buffer,*(bin.right),!(bin.right->binRange == bin.binRange.higherHalf()));
        }
	}

	static void load(PYXWireBuffer & buffer, Bin & bin,const Range<T> & expectedRange)
	{
		unsigned char code;

		buffer >> code;
        bool single = (code & 0x80) != 0;
        bool leaf = (code & 0x40) != 0;
        bool complex = (code & 0x20) != 0;

        int count = code & 0x1F;

        if (count == 31)
        {
			buffer >> count;
        }
        else if ((count & 0x10)!=0)
        {
			unsigned char extra;
			buffer >> extra;
            count = extra + (code & 0x0F)*256;
        }

        if (single)
        {
			T value;
			buffer >> value;
			bin.binRange = Range<T>(value);
			bin.binCount = count;
        }
        else if (complex)
        {
			T min,max;
			buffer >> min >> max;
			bin.binRange = Range<T>(min,max,knClosed,knOpen);
			bin.binCount = count;
        }
        else
        {
			bin.binRange = expectedRange;
			bin.binCount = count;
        }

        if (!leaf)
        {
			bin.left.reset(new Bin(Range<T>(),0));
			load(buffer,*(bin.left),bin.binRange.lowerHalf());

			bin.right.reset(new Bin(Range<T>(),0));
			load(buffer,*(bin.right),bin.binRange.higherHalf());

			//add the children count to current bin count the un
            bin.binCount += bin.left->binCount + bin.right->binCount;
            bin.childBinCount = 2 + bin.left->childBinCount + bin.right->childBinCount;
        }
	}
protected:
	std::auto_ptr<Bin> m_rootBin;
	Range<T> m_boundaries;
	double m_sum;
	double m_sumSquare;
	
protected:
	bool compare(const Bin & bin,const Bin & other) const
	{
		if ( bin.binCount == other.binCount && bin.binCount == 0)
		{
			return true;
		}

		if (bin.binRange == other.binRange && bin.binCount == other.binCount && bin.childBinCount == other.childBinCount && bin.isLeaf()== other.isLeaf() )
		{
			if (bin.isLeaf())
			{
				return true;
			}
			else 
			{
				return compare(*(bin.left),*(other.left)) && compare(*(bin.right),*(other.right));
			}
		}
		return false;
	}


protected:
	Bin * createBins(const Range<T> & range,typename std::vector<T>::const_iterator first,typename std::vector<T>::const_iterator last)
	{
		std::auto_ptr<Bin> bin(new Bin(range,0));

		//empty bin
		if (first == last)
		{
			return bin.release();
		}

		//single value
		std::vector<T>::const_iterator back = last;
		--back;
		if (*first == *back)
		{
			bin->binRange = Range<T>(*first);
			bin->binCount = last-first;
			return bin.release();
		}

		//complex case - split into to child bin.
		T middle = range.middle();
		int leftCount = 0;
		int rightCount = 0;

		//find middle position
		std::vector<T>::const_iterator middleIt;
		for(middleIt=first; middleIt != last && *middleIt < middle; middleIt++);

		bin->left.reset(createBins(range.lowerHalf(),first,middleIt));
		bin->right.reset(createBins(range.higherHalf(),middleIt,last));

		bin->binCount = bin->left->binCount + bin->right->binCount;
		bin->childBinCount = 2 + bin->left->childBinCount + bin->right->childBinCount;

		assert(bin->binRange.contains(bin->left->binRange));
		assert(bin->binRange.contains(bin->right->binRange));
		assert(bin->binCount == last-first);

		return bin.release();
	}

	void optimizeBins()
	{
		if (!m_rootBin->isLeaf())
		{
			optimizeBin(*(m_rootBin->left));
			optimizeBin(*(m_rootBin->right));
			m_rootBin->childBinCount = 2 + m_rootBin->left->childBinCount + m_rootBin->right->childBinCount;
		}
	}

	void optimizeBin(Bin & bin)
	{
		//nothing to optimize
		if (bin.isLeaf())
			return;

		Range<T> lastRange = bin.binRange;
		int lastCount = bin.binCount;
		int lastChildBinCount = bin.childBinCount;

		//there is uncertiny involve - can't optimize.
		if (bin.uncertaintyCount() != 0)
		{
			optimizeBin(*(bin.left));
			optimizeBin(*(bin.right));
		}
		else
		{
			if (bin.left->binCount == 0)
			{
				bin.binCount = bin.right->binCount;
				bin.binRange = bin.right->binRange;
				bin.childBinCount = bin.right->childBinCount;
				bin.left.reset(bin.right->left.release());
				bin.right.reset(bin.right->right.release());
				optimizeBin(bin);
			}
			else if (bin.right->binCount == 0)
			{
				bin.binCount = bin.left->binCount;
				bin.binRange = bin.left->binRange;
				bin.childBinCount = bin.left->childBinCount;
				bin.right.reset(bin.left->right.release());
				bin.left.reset(bin.left->left.release());
				optimizeBin(bin);
			}
			else
			{
				optimizeBin(*(bin.left));
				optimizeBin(*(bin.right));
			}
		}

		//update child cell count.
		if(!bin.isLeaf())
		{
			bin.childBinCount = 2 + bin.left->childBinCount + bin.right->childBinCount;
		}

		assert(lastCount == bin.binCount);
		assert(lastRange.contains(bin.binRange));
		assert(lastChildBinCount >= bin.childBinCount);
	}

	void safeAdd(Bin & rootBin,const Bin & other)
	{
		if (rootBin.binRange == other.binRange)
        {
            if (other.isLeaf())
            {
                rootBin.binCount += other.binCount;
            }
            else
            {
                rootBin.binCount += other.uncertaintyCount();
            }

            if (other.left.get() != 0)
            {
                safeAdd(rootBin, *(other.left));
            }
            if (other.right.get() != 0)
            {
                safeAdd(rootBin, *(other.right));
            }
        }
        else if (rootBin.binRange.contains(other.binRange))
        {
            rootBin.binCount += other.binCount;

			T middle = rootBin.binRange.middle();
            bool addToLeft = other.binRange.min < middle;

            if (addToLeft)
            {
                if (rootBin.left.get() == 0)
                {
                    rootBin.left.reset(new Bin(other.binRange, 0));
                    safeAdd(*(rootBin.left), other);
                }
                else if (rootBin.left->binRange.contains(other.binRange))
                {
                    safeAdd(*(rootBin.left), other);
                }
                else
                {
					std::auto_ptr<Bin> oldBin(rootBin.left.release());
					rootBin.left.reset(new Bin(rootBin.binRange.lowerHalf(),0));
                    safeAdd(*(rootBin.left), *oldBin);
                    safeAdd(*(rootBin.left), other);
                }
				if (rootBin.right.get() == 0)
				{
					rootBin.right.reset(new Bin(rootBin.binRange.higherHalf(), 0));
				}
            }
            else
            {
				if (rootBin.right.get() == 0)
                {
                    rootBin.right.reset(new Bin(other.binRange, 0));
                    safeAdd(*(rootBin.right), other);
                }
                else if (rootBin.right->binRange.contains(other.binRange))
                {
                    safeAdd(*(rootBin.right), other);
                }
                else
                {
					std::auto_ptr<Bin> oldBin(rootBin.right.release());
					rootBin.right.reset(new Bin(rootBin.binRange.higherHalf(),0));
                    safeAdd(*(rootBin.right), *oldBin);
                    safeAdd(*(rootBin.right), other);
                }
				if (rootBin.left.get() == 0)
				{
					rootBin.left.reset(new Bin(rootBin.binRange.lowerHalf(), 0));
				}
            }
        }
        else
        {
            PYXTHROW(PYXException,"We should never been here.");
        }

		if (!rootBin.isLeaf())
		{
			rootBin.childBinCount = 2 + rootBin.left->childBinCount + rootBin.right->childBinCount;
		}
	}

	void limit(Bin & bin,int maxError)
	{
		if (bin.isLeaf())
			return;

		//collapse the bin if it's count is less the maxError
		if (bin.binCount < maxError)
		{
			bin.left.reset();
			bin.right.reset();
			bin.childBinCount = 0;
		}
		else 
		{
			//lets the child limit them self if they can..
			limit(*(bin.left),maxError);
			limit(*(bin.right),maxError);

			//recount child bins
			bin.childBinCount = 2 + bin.left->childBinCount + bin.right->childBinCount;
		}
	}

	void getLeafBins(Bin & bin,int parentUncertinityCount,std::vector<LeafBin> & result) const
	{
		if (bin.isLeaf())
		{
			result.push_back(LeafBin(bin.binRange,Range<int>::createClosedClosed(bin.binCount,bin.binCount+parentUncertinityCount)));
		}
		else
		{
			int totalUncertinityCount = bin.uncertaintyCount() + parentUncertinityCount;
			getLeafBins(*(bin.left),totalUncertinityCount,result);
			getLeafBins(*(bin.right),totalUncertinityCount,result);
		}
	}


};

template<typename T>
PYXWireBuffer & operator >> (PYXWireBuffer & buffer,NumericHistogram<T> & histogram)
{
	buffer >> histogram.m_boundaries.min >> histogram.m_boundaries.max;
	histogram.m_boundaries.minType = knClosed;
	histogram.m_boundaries.minType = knClosed;
	buffer >> histogram.m_sum >> histogram.m_sumSquare;
	histogram.m_rootBin.reset(new NumericHistogram<T>::Bin(Range<T>(),0));
	NumericHistogram<T>::load(buffer,*histogram.m_rootBin,histogram.m_boundaries);
	return buffer;
}

template<typename T>
PYXWireBuffer & operator << (PYXWireBuffer & buffer,const NumericHistogram<T> & histogram)
{
	buffer << histogram.m_boundaries.min << histogram.m_boundaries.max;
	buffer << histogram.m_sum << histogram.m_sumSquare;

	NumericHistogram<T>::save(buffer,*histogram.m_rootBin,true);
	return buffer;
}

#endif // guard
