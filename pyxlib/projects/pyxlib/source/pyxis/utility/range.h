#ifndef PYXIS__UTILITY__RANGE_H
#define PYXIS__UTILITY__RANGE_H
/******************************************************************************
range.h

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "value.h"

///////////////////////////////////////////////////////////////////////////////
// Range
///////////////////////////////////////////////////////////////////////////////

enum PYXLIB_DECL RangeBorderType
{
	knInfinite,
	knOpen,
	knClosed
};

template<typename T>
struct Range
{
	T min;
	T max;

	RangeBorderType minType;
	RangeBorderType maxType;

	static Range createClosedOpen(const T & min, const T & max) 
	{
		return Range(min,max,knClosed,knOpen);
	}

	static Range createClosedClosed(const T & min, const T & max) 
	{
		return Range(min,max,knClosed,knClosed);
	}

	Range() : minType(knInfinite), maxType(knInfinite)
	{
	}

	Range(const T & value) : min(value), max(value), minType(knClosed), maxType(knClosed)
	{
	}

	Range(const T & _min,const T & _max, RangeBorderType _minType, RangeBorderType _maxType) : min(_min), max(_max), minType(_minType), maxType(_maxType)
	{
	}

	Range(const Range<T> & other) : min(other.min), max(other.max), minType(other.minType), maxType(other.maxType)
	{
	}

	Range<T> & operator=(const Range<T> & other)
	{
		min = other.min;
		max = other.max;
		minType = other.minType;
		maxType = other.maxType;
		return *this;
	}

	T middle() const
	{
		return min+(max-min)/2;
	}

	bool single() const
	{
		return min == max && minType != knInfinite && maxType != knInfinite;
	}

	bool global() const 
	{
		return minType == knInfinite && maxType == knInfinite;
	}

	bool infinite() const 
	{
		return minType == knInfinite || maxType == knInfinite;
	}

	bool contains(const T & value) const
    {
		switch(minType)
		{
		case knInfinite:
			break;
		case knOpen:
			if (min >= value) 
			{
				return false;
			}
			break;
		case knClosed:
			if (min > value)
			{
				return false;
			}
			break;
		}
        switch(maxType)
		{
		case knInfinite:
			break;
		case knOpen:
			if (max <= value) 
			{
				return false;
			}
			break;
		case knClosed:
			if (max < value)
			{
				return false;
			}
			break;
		}
		return true;
    }

	bool contains(const Range<T> & range) const
    {
		switch (range.minType)
		{
		case knInfinite:
			if (minType != knInfinite)
			{
				return false;
			}
			break;
		case knOpen:
			if (min > range.min)
			{
				return false;
			}
			break;
		case knClosed:
			if (min > range.min)
			{
				return false;
			} 
			if (min == range.min && minType == knOpen)
			{
				return false;
			}
			break;
		}

		switch (range.maxType)
		{
		case knInfinite:
			if (maxType != knInfinite)
			{
				return false;
			}
			break;
		case knOpen:
			if (max < range.max)
			{
				return false;
			}
			break;
		case knClosed:
			if (max < range.max)
			{
				return false;
			} 
			if (max == range.max && maxType == knOpen)
			{
				return false;
			}
			break;
		}

		return true;
    }

    bool intersects(const Range<T> & range) const
    {
		if (range.minType != knInfinite && maxType != knInfinite)
		{
			if (range.min > max)
			{
				return false;
			}
			//boundary case: the given range touches this range
			if (range.min == max) 
			{
				//they intersects only if both ends are closed
				return range.minType == knClosed && maxType == knClosed;					
			}
		}

		if (range.maxType != knInfinite && minType != knInfinite)
		{
			if (range.max < min)
			{
				return false;
			}
			//boundary case: the given range touches this range
			if (range.max == min)
			{
				//they intersects only if both ends are closed
				return range.maxType == knClosed && minType == knClosed;
			}
		}

		return true;
    }

	bool operator ==(const Range<T> & range) const
	{
		return min == range.min && max == range.max && minType == range.minType && maxType == range.maxType;
	}

	bool operator ==(const T & value) const
	{
		return single() && min == value;
	}

	Range<T> lowerHalf() const
	{
		return Range<T>(min,middle(),minType,maxType);
	}

	Range<T> higherHalf() const
	{
		return Range<T>(middle(),max,minType,maxType);
	}

	template<class InputIterator>
	static Range<T> sum(InputIterator first,InputIterator last)
	{
		Range<T> result(0,0,(*first).minType,(*first).maxType);
		for(;first!=last;++first)
		{
			result.min += (*first).min;
			result.max += (*first).max;
		}
		return result;
	}

	template<class InputIterator>
	static Range<T> createFromValues(InputIterator first,InputIterator last)
	{
		bool hasValues = false;
		Range<T> result(0,0,knClosed,knClosed);

		for(;first!=last;++first)
		{
			if (hasValues)
			{
				if ((*first) < result.min)
				{
					result.min = (*first);
				}
				if ((*first) > result.max)
				{
					result.max = (*first);
				}
			}
			else {
				result = Range<T>(*first);
				hasValues = true;
			}
		}
		return result;
	}

	template<class InputIterator>
	static Range<T> merge(InputIterator first,InputIterator last)
	{
		bool hasRanges = false;
		Range<T> result(0,0,knClosed,knClosed);

		for(;first!=last;++first)
		{
			if (hasRanges)
			{
				if ((*first).min < result.min)
				{
					result.min = (*first).min;
					result.minType = (*first).minType;
				}
				if ((*first).max > result.max)
				{
					result.max = (*first).max;
					result.maxType = (*first).maxType;
				}
			}
			else {
				result = *first;
				hasRanges = true;
			}
		}
		return result;
	}

	Range<T> normalize() const
	{
		Range<T> result(-1,1,minType,maxType);

		while (!result.contains(min) || !result.contains(max))
		{
			result.min *= 2;
			result.max *= 2;
		}
		return result;
	}

	Range<T> normalizeWith(const Range<T> & other) const
	{
		Range<T> result(-1,1,minType,maxType);

		while (!result.contains(min) || !result.contains(max) ||
			   !result.contains(other.min) || !result.contains(other.max) )
		{
			result.min *= 2;
			result.max *= 2;
		}
		return result;
	}
};
#ifndef SWIG
PYXLIB_DECL typedef Range< int > RangeInt;
PYXLIB_DECL typedef Range< double > RangeDouble;
PYXLIB_DECL typedef Range< std::string > RangeString;

PYXLIB_DECL typedef Range< PYXValue > PYXValueRange;
#endif

template<>
std::string Range<std::string>::middle() const
{
	std::string result;
	unsigned int length = std::min(min.size(),max.size());

	for(unsigned int i=0;i<length;++i)
	{
		if (min[i] == max[i])
		{
			result += min[i];
		}
		else 
		{
			result += (min[i]+max[i])/2;
			return result;
		}
	}
	if (min.size()>max.size())
	{
		result += (min[max.size()]+255)/2;
	}
	else if (max.size()>min.size())
	{
		result += (max[min.size()]+255)/2;
	}
	return result;
}

template<>
Range<std::string> Range<std::string>::normalize() const
{
	Range<std::string> result("","",minType,maxType);
	PYXTHROW(PYXException,"not implmenented");
}

template<>
Range<std::string> Range<std::string>::normalizeWith(const Range<std::string> & other) const
{
	Range<std::string> result("","",minType,maxType);
	PYXTHROW(PYXException,"not implmenented");
}

template<>
PYXValue Range<PYXValue>::middle() const
{
	assert(min.isNumeric());
	
	PYXValue result(min);
	result.setDouble(min.getDouble()/2+max.getDouble()/2);
	return result;
}

template<>
Range<PYXValue>::Range(const PYXValue & value) : min(value),max(value)
{
	assert(value.getArraySize()==1);
}

template<>
Range<PYXValue>::Range(const PYXValue & _min,const PYXValue & _max,RangeBorderType _minType,RangeBorderType _maxType)
	: min(_min), max(_max), minType(_minType), maxType(_maxType)
{
	assert(_min.getArraySize()==1);
	assert(_max.getArraySize()==1);
	assert(_min.getType() == _max.getType());

	// numbers and strings sort differently e.g. {9, 10}
	// for numbers 9 < 10 and for strings "10" < "9"
	// isNumeric() returns true for a string of digits, make sure
	// numbers are ordered as numbers and strings are ordered as strings
	assert(	(_min.isString() && _min.getString() <= _max.getString()) ||
			(_min.isNumeric() && _min.getDouble() <= _max.getDouble())	);
}

template<>
bool Range<PYXValue>::contains(const PYXValue & value) const
{
    if (single())
    {
        return min == value;
    }

	if (max.isNumeric())
	{
		RangeDouble range(min.getDouble(),max.getDouble(),minType,maxType);
		return range.contains(value.getDouble());
	}
	else 
	{
		RangeString range(min.getString(),max.getString(),minType,maxType);
		return range.contains(value.getString());
	}
}

template<>
bool Range<PYXValue>::contains(const Range<PYXValue> & range) const
{
    if (single())
    {
        return range.single() && min == range.min;
    }
    else if (range.single())
    {
        return contains(range.min);
    }
    else 
    {
		if (max.isNumeric())
		{
			return min.getDouble() <= range.min.getDouble() && max.getDouble() >= range.max.getDouble();
		}
		else 
		{
			return min.getString() <= range.min.getString() && max.getString() >= range.max.getString();
		}
    }
}

template<>
bool Range<PYXValue>::intersects(const Range<PYXValue> & range) const
{
    if (single())
    {
        return range.contains(min);
    }
    else if (range.single())
    {
        return contains(range.min);
    }
    else
    {
        return min.getDouble() < range.max.getDouble() && max.getDouble() > range.min.getDouble();
    }
}

template<>
Range<PYXValue> Range<PYXValue>::normalize() const
{
	Range<PYXValue> result(PYXValue(-1.0),PYXValue(1.0),minType,maxType);

	while (!result.contains(min) || !result.contains(max))
	{
		result.min.setDouble(2*result.min.getDouble());
		result.max .setDouble(2*result.max .getDouble());
	}
	return result;
}

template<>
Range<PYXValue> Range<PYXValue>::normalizeWith(const Range<PYXValue> & other) const
{
	Range<PYXValue> result(PYXValue(-1.0),PYXValue(1.0),minType,maxType);

	while (!result.contains(min) || !result.contains(max) ||
		   !result.contains(other.min) || !result.contains(other.max) )
	{
		result.min.setDouble(2*result.min.getDouble());
		result.max .setDouble(2*result.max .getDouble());
	}
	return result;
}


#endif // guard
