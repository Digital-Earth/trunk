#ifndef PYXIS__UTILITY__STRING_HISTOGRAM_H
#define PYXIS__UTILITY__STRING_HISTOGRAM_H
/******************************************************************************
string_histogram.h

begin		: May 08, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "value.h"
#include "range.h"
#include "wire_buffer.h"

///////////////////////////////////////////////////////////////////////////////
// StringHistogram
///////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL StringHistogram
{
public:
	//! Test method.
	static void test();

	static const unsigned int DEFAULT_MAX_STRING_SIZE = 100;

public:
	struct LeafBin
	{
		RangeString range;
		RangeInt count;

		LeafBin()
		{
		}

		LeafBin(const RangeString & _range,const RangeInt & _count) : range(_range), count(_count)
		{
		}
	};

protected:
	enum BinType
    {
		knBinLeaf,
        knBinWholeByte,
        knBinSwitch,
    };

	class Bin : ObjectMemoryUsageCounter<Bin>
#ifdef INSTANCE_COUNTING
				, protected InstanceCounter
#endif
    {
	public:
		static const int MAX_CHILDREN = 4;
		static const int CHAR_SIZE_IN_BITS = 8;

		unsigned char value;
        BinType type; 

        int binCount;
        int totalCount;

        Bin* children[MAX_CHILDREN];

		Bin();
		Bin(const std::string & buffer,int count=1);
		virtual ~Bin();


		int countChildrenTotal() const;
        int uncertaintyCount() const;
		bool hasChildren() const;

		unsigned char children4Bit() const;
		int countNodes() const;

		void clearChildren(bool doDelete = true);

		void add(Bin & other);

        RangeInt count(const RangeString & range) const;

		void limitBins(int binLimit);

	private:
		int getChild(const std::string & buffer, unsigned int digit) const;
		
		RangeInt count(const RangeString & range,unsigned int digit,bool minBound,bool maxBound) const;
        void add(Bin & other, int digit);

		void convertBinToSwitchBin();
		void limitBins(int binLimit, int digit);
		bool childCompare(int a,int b) const;
		void findChildrenOrder(std::vector<int> & childrenOrder) const;
    };

protected:
	std::auto_ptr<Bin> m_rootBin;
	RangeString m_boundaries;
	unsigned int m_maxStringSize;

public:
	int count() const;

	RangeInt count(const RangeString & range) const;

	//return the estimate precentile of the value (result between 0 and 100)
	Range<double> percentile(const std::string & value) const;

	const RangeString & getBoundaries() const
	{
		return m_boundaries;
	}

	StringHistogram();
	StringHistogram(const std::string & value);
	StringHistogram(const StringHistogram & other);

	template<class InputIterator>
	StringHistogram(InputIterator first,InputIterator last) : m_boundaries(), m_rootBin(new Bin()), m_maxStringSize(DEFAULT_MAX_STRING_SIZE)
	{
		while(first != last)
		{
			add(*first);
			first++;
		}
	}

	void add(const StringHistogram & other);
	void add(const std::string & buffer);

	void limit(int binCount);

	void getLeafBins(std::vector<LeafBin> & result) const;

protected:
	void getLeafBins(Bin & bin,const std::string & prefix,unsigned char lastByte,int digit,int uncertaintyCountFromParent, std::vector<LeafBin> & result) const;

public:
	bool operator ==(const StringHistogram & other) const;

protected:
	bool compare(const Bin & bin,const Bin & other) const;

public:
	friend PYXLIB_DECL PYXWireBuffer & operator >> (PYXWireBuffer & buffer,StringHistogram & histogram);
	friend PYXLIB_DECL PYXWireBuffer & operator << (PYXWireBuffer & buffer,const StringHistogram & histogram);

protected:
	static void save(PYXWireBuffer & buffer, Bin & bin);
	static void load(PYXWireBuffer & buffer, Bin & bin);
};

PYXLIB_DECL PYXWireBuffer & operator >> (PYXWireBuffer & buffer,StringHistogram & histogram);
PYXLIB_DECL PYXWireBuffer & operator << (PYXWireBuffer & buffer,const StringHistogram & histogram);

#endif // guard
