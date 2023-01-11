/******************************************************************************
string_histogram.cpp

begin		: May 08, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "string_histogram.h"
#include "tester.h"
#include "exceptions.h"
#include "bit_utils.h"

#include "boost/bind.hpp"

// standard includes
#include <cassert>

//! The unit test class
Tester<StringHistogram> gTester;

std::string randomWord()
{
	int length = 2 + rand() % 14;

	std::string result;

	for(int i=0;i<length;++i)
	{
		result += (char)(rand() % ('z'-'a')) + 'a';
	}
	return result;
}

void StringHistogram::test()
{
	{
		StringHistogram hist;

		hist.add("hello");
		hist.add("world");
		hist.add("wow");
		hist.add("hello");

		TEST_ASSERT(hist.count() == 4);
		TEST_ASSERT(hist.count(RangeString("hello")) == 2);
		TEST_ASSERT(hist.count(RangeString("hello world")) == 0);
		TEST_ASSERT(hist.count(RangeString::createClosedOpen("w","ww")) == 2);
	}

	{
		const int WORDCOUNT = 5000;
		StringHistogram hist;

		srand(10);
		std::vector<std::string> words;
		for(int i =0;i<WORDCOUNT;++i)
		{
			std::string ;
			words.push_back(randomWord());
			hist.add(words.back());
		}

		TEST_ASSERT(hist.count() == WORDCOUNT);

		for(unsigned int i=0;i<words.size();i+=200)
		{
			for(unsigned int j=i;j<words.size();j+=rand()%100+10)
			{
				std::string min = words[i];
				std::string max = words[j];
				if (min>max)
				{
					min = words[j];
					max = words[i];
				}

				int realCount = 0;
				for(unsigned int w=0;w<words.size();w++)
				{
					if (min == max)
					{
						if (words[w] == min)
						{
							realCount++;
						}
					}
					else 
					{
						if (min <= words[w] && words[w] < max)
						{
							realCount++;
						}
					}
				}

				RangeInt result = hist.count(RangeString::createClosedOpen(min,max));
				TEST_ASSERT(result == realCount);
			}
		}

		hist.limit(2000);
		TEST_ASSERT(hist.count() == WORDCOUNT);

		for(unsigned int i=0;i<words.size();i+=200)
		{
			for(unsigned int j=i;j<words.size();j+=rand()%100+10)
			{
				std::string min = words[i];
				std::string max = words[j];
				if (min>max)
				{
					min = words[j];
					max = words[i];
				}

				int realCount = 0;
				for(unsigned int w=0;w<words.size();w++)
				{
					if (min == max)
					{
						if (words[w] == min)
						{
							realCount++;
						}
					}
					else 
					{
						if (min <= words[w] && words[w] <max)
						{
							realCount++;
						}
					}
				}

				RangeInt result = hist.count(RangeString::createClosedOpen(min,max));
				TEST_ASSERT(result.min <= realCount && realCount <= result.max);
			}
		}

		PYXStringWireBuffer buffer;

		buffer << hist;

		StringHistogram hist2;

		buffer.setPos(0);

		buffer >> hist2;

		TEST_ASSERT(hist == hist2);
	}

	//TODO: test save and load
}

///////////////////////////////////////////////////////////////////////////////
// StringHistogram
///////////////////////////////////////////////////////////////////////////////

StringHistogram::Bin::Bin() : type(knBinLeaf), binCount(0), totalCount(0)
{
	memset(children,0,sizeof(children));
}

StringHistogram::Bin::Bin(const std::string & buffer,int count) : type(knBinLeaf), binCount(0), totalCount(0)
{
	memset(children,0,sizeof(children));
	Bin * end = this;
	for(unsigned int i=0;i<buffer.size();++i)
	{
		end->type = knBinWholeByte;
		end->value = buffer[i];
		end->totalCount = count;
		end->children[0] = new Bin();
		end = end->children[0];
	}
	end->type = knBinLeaf;
	end->totalCount = count;
	end->binCount = count;
}

StringHistogram::Bin::~Bin()
{
	clearChildren();
}

int StringHistogram::Bin::countChildrenTotal() const
{
	int result = 0;

	for(int i=0;i<MAX_CHILDREN;++i)
	{
		if (children[i])
		{
			result += children[i]->totalCount;
		}
	}
	return result;
}

 int StringHistogram::Bin::uncertaintyCount() const
{
	return totalCount - binCount - countChildrenTotal();
}

bool StringHistogram::Bin::hasChildren() const 
{
	for(int i=0;i<MAX_CHILDREN;++i)
	{
		if (children[i])
		{
			return true;
		}
	}
	return false;
}

unsigned char StringHistogram::Bin::children4Bit() const
{
	unsigned char result = 0;
	if (children[0]) result |= BitUtils::knBit1;
	if (children[1]) result |= BitUtils::knBit2;
	if (children[2]) result |= BitUtils::knBit3;
	if (children[3]) result |= BitUtils::knBit4;
	return result;
}

int StringHistogram::Bin::countNodes() const
{
	int result = 1;

	for(int i=0;i<MAX_CHILDREN;++i)
	{
		if (children[i])
		{
			result += children[i]->countNodes();
		}
	}

	return result;
}

void StringHistogram::Bin::clearChildren(bool doDelete)
{
	if (doDelete)
	{
		for(int i=0;i<MAX_CHILDREN;++i)
		{
			if (children[i])
			{
				delete children[i];
				children[i] = 0;
			}
		}
	}
	else {
		memset(children,0,sizeof(children));
	}
}

int StringHistogram::Bin::getChild(const std::string & buffer, unsigned int digit) const
{
	if (digit < buffer.size()*CHAR_SIZE_IN_BITS)
	{
		//fetch byte [digit/8] from the string.
		//then fetch only 2 bits from it. where digit % 8 == 0 is the MSB digits.
		return (buffer[digit/CHAR_SIZE_IN_BITS]>>(6-(digit%CHAR_SIZE_IN_BITS))) & 0x03;
	}
	return -1;
}

RangeInt StringHistogram::Bin::count(const RangeString & range,unsigned int digit,bool minBound,bool maxBound) const
{
	int min = minBound?getChild(range.min, digit):-1;
    int max = maxBound?getChild(range.max, digit):4;

    RangeInt result(0);

    if (min==-1 && max==4)
    {
        return RangeInt(totalCount);
    }

    if (max!=-1)
    {
        result.max += uncertaintyCount();
    }

    if (min==-1 && (max!=-1 || range.single()))
    {
        result.min += binCount;
        result.max += binCount;
    }

    if (min==-1 && max==-1)
        return result;

    switch (type)
    {
        case knBinLeaf:
            return result;

        case knBinWholeByte:
            if (min == -1)
                minBound = false;

            if (max == 4)
                maxBound = false;

            if ((!minBound || range.min[digit/8] < value) && (!maxBound || range.max[digit/8] > value))
            {
                result.min += children[0]->totalCount;
                result.max += children[0]->totalCount;
                return result;
            }
            else if (minBound && range.min[digit / 8] > value || maxBound && range.max[digit/8] < value)
            {
                return result;
            }
            else 
            {
                RangeInt count = children[0]->count(range, digit + 8,
                                              minBound && range.min[digit/8] == value,
                                              maxBound && range.max[digit/8] == value);
                result.min += count.min;
                result.max += count.max;
                return result;
            }


        case knBinSwitch:

			for (int i = std::max(min, 0); i <= std::min(max, 3); ++i)
            {
                if (children[i] != NULL)
                {
                    RangeInt count = children[i]->count(range, digit + 2, i == min, i == max);
                    result.min += count.min;
                    result.max += count.max;
                }
            }
            return result;

        default:
			PYXTHROW(PYXException,"unknown bin type");
    }
}

RangeInt StringHistogram::Bin::count( const RangeString & range ) const
{
	return count(range,0,range.minType!=knInfinite,range.maxType!=knInfinite);
}

void StringHistogram::Bin::add(Bin & other, int digit)
{
    binCount += other.binCount;
    totalCount += other.totalCount;

    if (other.type == knBinLeaf)
        return;

    if (type == knBinLeaf)
    {
        type = other.type;
        value = other.value;
        clearChildren();
    }

	if (type == knBinWholeByte && other.type == knBinWholeByte && value != other.value)
	{
		convertBinToSwitchBin();
		other.convertBinToSwitchBin();
	}

    if (type != other.type)
    {
        if (type == knBinWholeByte)
        {
            convertBinToSwitchBin();
        }
        else
        {
            other.convertBinToSwitchBin();
        }
    }

    switch (type)
    {
        case knBinWholeByte:
            if (other.children[0] != NULL)
            {
                if (children[0] == NULL)
                {
                    children[0] = new Bin();
                }
                children[0]->add(*(other.children[0]), digit + 8);
            }
            break;

        case knBinSwitch:
            for (int i = 0; i < 4; i++)
            {
                if (other.children[i] != NULL)
                {
                    if (children[i] == NULL)
                    {
						children[i] = new Bin();

                        if ((digit + 2)%8 != 0)
                        {
							children[i]->type = knBinSwitch;
                        }
                    }
                    children[i]->add(*(other.children[i]), digit + 2);
                }
            }
            break;
    }
}

void StringHistogram::Bin::add( Bin & other )
{
	add(other,0);
}

void StringHistogram::Bin::convertBinToSwitchBin()
{
	type = knBinSwitch;
    Bin * oldChild = children[0];

    int childCount = countChildrenTotal();
    clearChildren(false);

    Bin * bin = this;
    for (int i = 0; i < 4; i++)
    {
        int childIndex = (value >> (6 - i*2)) & 0x03;

        if (i < 3)
        {
            bin = bin->children[childIndex] = new Bin();

			bin->type = knBinSwitch;
			bin->totalCount = childCount;
        }
        else
        {
            bin->children[childIndex] = oldChild;
        }
    }
}

void StringHistogram::Bin::limitBins(int binLimit, int digit)
{
	//we are not allowed to have child nodes...
    if (binLimit <= 1)
    {
        type = knBinLeaf;
        clearChildren();
        return;
    }

    switch(type)
    {
        case knBinLeaf:
            return;

        case knBinWholeByte:
            if (children[0] != NULL)
            {
                children[0]->limitBins(binLimit-1, digit+8);
            }
            break;

        case knBinSwitch:
			int nodesCount = countNodes();

			//we are ok
            if (binLimit > nodesCount-1)
                return;

			//no children...
            if (nodesCount == 1)
            {
                return;
            }

			std::vector<int> childrenOrder;

			findChildrenOrder(childrenOrder);

            //first try
            int countLeft = countChildrenTotal();
			int nodesLeft = binLimit - 1;

			for(unsigned int i=0; i<childrenOrder.size(); ++i)
			{
				int childIndex = childrenOrder[i];
				Bin * child = children[childIndex];
				int childPlannedNodesCount = nodesLeft*child->totalCount/ countLeft;
				child->limitBins(childPlannedNodesCount,digit+2);
				int childNewTotalNodes = child->countNodes();
				nodesLeft -= childNewTotalNodes;
				countLeft -= child->totalCount;
				if (countLeft == 0)
				{
					countLeft = 1;
				}
			}

			//update count...
			nodesCount = countNodes();

			//second type
            if (nodesCount-binLimit > 0)
            {
				findChildrenOrder(childrenOrder);

                for(unsigned int i=0; i<childrenOrder.size(); ++i)
				{
					int childIndex = childrenOrder[i];
					Bin * child = children[childIndex];

                    if (child->totalCount < totalCount / binLimit)
                    {
                        delete children[childIndex];
						children[childIndex] = 0;
                    }
                    else
                    {
                        child->type = knBinLeaf;
                        child->clearChildren();
                    }

					nodesCount = countNodes();

                    if (nodesCount <= binLimit)
                        break;
                }
            }


            break;
    }
}

void StringHistogram::Bin::limitBins( int binLimit )
{
	limitBins(binLimit,0);
}

bool StringHistogram::Bin::childCompare(int a,int b) const
{
	return children[a]->countNodes() < children[b]->countNodes();
}

void StringHistogram::Bin::findChildrenOrder(std::vector<int> & childrenOrder) const
{			
	childrenOrder.clear();

	for(int i=0;i<MAX_CHILDREN;++i)
	{
		if (children[i])
		{
			childrenOrder.push_back(i);
		}
	}

	std::sort(childrenOrder.begin(),childrenOrder.end(),boost::bind(&StringHistogram::Bin::childCompare,this,_1,_2));
}

StringHistogram::StringHistogram() : m_rootBin(new Bin()), m_boundaries(), m_maxStringSize(DEFAULT_MAX_STRING_SIZE)
{
}

StringHistogram::StringHistogram(const std::string & value) : m_boundaries(value), m_maxStringSize(DEFAULT_MAX_STRING_SIZE)
{
	m_rootBin.reset(new Bin(value));
}

StringHistogram::StringHistogram(const StringHistogram & other)
{
	m_boundaries = other.m_boundaries;
	m_rootBin.reset(new Bin());
	m_maxStringSize = other.m_maxStringSize;
	add(other);
}

	
void StringHistogram::add(const StringHistogram & other)
{
	if (other.count()==0)
		return;

	if (count() == 0)
    {
        m_boundaries = other.m_boundaries;
    }
    else
    {
		if (other.m_boundaries.min < m_boundaries.min)
		{
			m_boundaries.min = other.m_boundaries.min;
		}
        if (other.m_boundaries.max > m_boundaries.max)
		{
			m_boundaries.max = other.m_boundaries.max;
		}
    }

	m_rootBin->add(*other.m_rootBin);
}

void StringHistogram::add(const std::string & buffer)
{
	if (buffer.size() > m_maxStringSize)
	{
		//trim the string and call add again with only the prefix of the string
		add(std::string(buffer.begin(), buffer.begin() + m_maxStringSize));
	}

	if (count() == 0)
    {
        m_boundaries = RangeString(buffer);
    }
    else
    {
		if (buffer < m_boundaries.min)
		{
			m_boundaries.min = buffer;
		}
        if (buffer > m_boundaries.max)
		{
			m_boundaries.max = buffer;
		}
    }

	m_rootBin->add(Bin(buffer));
}

void StringHistogram::getLeafBins(Bin & bin,const std::string & prefix,unsigned char lastByte,int digit,int uncertaintyCountFromParent, std::vector<LeafBin> & result) const
{		
	if (!bin.hasChildren()) 
	{
		if (bin.binCount > 0 || digit%8 == 0)
		{
			result.push_back(LeafBin(RangeString(prefix),RangeInt::createClosedClosed(bin.totalCount,bin.totalCount+uncertaintyCountFromParent)));
		}
		else if (bin.totalCount > 0)
		{
			result.push_back(LeafBin(RangeString::createClosedOpen(prefix+(char)lastByte,prefix+(char)(lastByte + (0xFF>>(digit%8)))),RangeInt::createClosedClosed(bin.totalCount,bin.totalCount+uncertaintyCountFromParent)));
		}
		return;
	}
	else if (bin.binCount > 0) 
	{
		result.push_back(LeafBin(RangeString(prefix),RangeInt::createClosedClosed(bin.binCount,bin.binCount)));
	}

	int uncertainty = uncertaintyCountFromParent + bin.uncertaintyCount();
	//else have children...
    for(int i=0;i<4;++i)
	{
		Bin * child = bin.children[i];

		if (child == NULL)
		{
			continue;
		}

        if (bin.type == knBinWholeByte)
        {
            getLeafBins(*child, prefix + (char)bin.value, 0 , digit + 8, uncertainty, result);
        }
        else
        {
            unsigned char currentByte = (unsigned char ) (lastByte + (i << (6 - digit%8)));
            if ((digit+2)%8==0)
            {
                getLeafBins(*child, prefix + (char)currentByte, 0, digit + 2, uncertainty, result);
            }
            else
            {
                getLeafBins(*child, prefix, currentByte, digit + 2, uncertainty, result);
            }
        }
    }
}

void StringHistogram::getLeafBins( std::vector<LeafBin> & result ) const
{
	return getLeafBins(*m_rootBin,std::string(),0,0,0,result);
}

bool StringHistogram::compare(const Bin & bin,const Bin & other) const
{
	if (bin.binCount != other.binCount ||
		bin.totalCount != other.totalCount ||
		bin.type != other.type)
	{
		return false;
	}

	if (bin.type==knBinWholeByte && bin.value != other.value)
	{
		return false;
	}

	for(int i=0;i<4;++i)
	{
		if (bin.children[i] == NULL)
		{
			if (other.children[i] != NULL)
			{
				return false;
			}
		}
		else {
			if (other.children[i] == NULL)
			{
				return false;
			}
			if (!compare(*bin.children[i],*other.children[i]))
			{
				return false;
			}
		}
	}
	return true;
}

void StringHistogram::save(PYXWireBuffer & buffer, Bin & bin)
{
	//TODO: write some explaintion on the format I'm using. what each bit means....
	switch (bin.type)
    {
        case knBinSwitch:
            {
                int uncertainty = bin.uncertaintyCount();

				unsigned char header = (BitUtils::knBit8 |
                                      BitUtils::knBit6 |
                                      ((bin.binCount > 0) ? BitUtils::knBit7 : 0) |
                                      ((uncertainty > 0) ? BitUtils::knBit5 : 0) |
                                      bin.children4Bit());
                buffer << header;

                if (bin.binCount > 0)
                {
                    buffer << bin.binCount;
                }

                if (uncertainty > 0)
                {
                    buffer << uncertainty;
                }

				for(int i=0;i<4;++i)
				{
					if (bin.children[i])
					{
						save(buffer,*bin.children[i]);
					}
				}
            }
            break;
        case knBinWholeByte:
            {

                int uncertainty = bin.uncertaintyCount();
				std::vector<unsigned char> chain;
				chain.push_back(bin.value);
                int count = bin.binCount;
				unsigned char header = BitUtils::knBit8 |
                                      ((count > 0) ? BitUtils::knBit7 : 0) |
                                      ((uncertainty > 0) ? BitUtils::knBit5 : 0);

				Bin * child = &bin;

                while (chain.size() < 15 && 
					child->children[0] != NULL && 
					child->children[0]->type == knBinWholeByte && 
					child->children[0]->binCount == 0 && 
					child->children[0]->uncertaintyCount() == 0)
                {
                    child = child->children[0];
                    chain.push_back(child->value);
                }

                header += (unsigned char)chain.size();

                buffer << header;
                if (count > 0)
                {
                    buffer << count;
                }
                if (uncertainty > 0)
                {
                    buffer << uncertainty;
                }

				for(unsigned int i=0;i<chain.size();++i)
				{
					buffer << chain[i];
				}
                save(buffer,*(child->children[0]));
            }
            break;
        case knBinLeaf:
            {
                int uncertainty = bin.uncertaintyCount();

				unsigned char header = uncertainty > 0 ? BitUtils::knBit7 : 0;

                bool addBinCount = false;
                if (bin.binCount < 63)
                {
                    header += (unsigned char ) bin.binCount;
                }
                else
                {
                    addBinCount = true;
                    header += 63;
                }

                buffer << header;
                if (addBinCount)
				{
                    buffer << bin.binCount;
				}
                if (uncertainty > 0)
				{
                    buffer << uncertainty;
				}
            }
            break;
        default:
            PYXTHROW(PYXException,"we should never get here");
    }
}

void StringHistogram::load(PYXWireBuffer & buffer, Bin & bin)
{
	unsigned char header;

	buffer >> header;

	if ((header & BitUtils::knBit8)==0)
    {
        //leaf
		bin.type = knBinLeaf;
		bool hasUncertainty = (header & BitUtils::knBit7) != 0;
        int count = header & 63;

		if (count == 63)
		{
			buffer >> bin.binCount;
		}
		else 
		{
			bin.binCount = count;
		}

        if (hasUncertainty)
        {
            buffer >> bin.totalCount;
        }
        else
        {
            bin.totalCount = 0;
        }

		bin.totalCount += bin.binCount;
    }
	else if ((header & BitUtils::knBit6)==0)
    {
        //wholebyte
        bin.type = knBinWholeByte;

		bool hasCount = (header & BitUtils::knBit7) != 0;
        bool hasUncertainty = (header & BitUtils::knBit5) != 0;
        unsigned char length = (header & 15);

        if (hasCount)
        {
            buffer >> bin.binCount;
        }
        if (hasUncertainty)
        {
            buffer >> bin.totalCount;
        }

        //read all whole byte chain...
        Bin * child = &bin;

		std::vector<Bin*> childrenStack;
        for(int i=0;i<length-1;++i)
        {
            buffer >> child->value;
            child->children[0] = new Bin();
            childrenStack.push_back(child);
            child = child->children[0];
			child->type = knBinWholeByte;
        }
        //the last byte...
        buffer >> child->value;

        //read the next node and add it as a child.
        child->children[0] = new Bin();
		load(buffer,*(child->children[0]));

        //add the total count from the child to this node
        child->totalCount += child->children[0]->totalCount;
        while (!childrenStack.empty())
        {
            child = childrenStack.back();
			childrenStack.pop_back();
            child->totalCount += child->children[0]->totalCount;
        }
        bin.totalCount += bin.binCount;
    }
    else
    {
        //switch
        bin.type = knBinSwitch;
		bool hasCount = (header & BitUtils::knBit7) != 0;
		bool hasUncertainty = (header & BitUtils::knBit5) != 0;

        if (hasCount)
        {
            buffer >> bin.binCount;
        }
        if (hasUncertainty)
        {
            buffer >> bin.totalCount;
        }

        for(int i=0;i<4;++i)
        {
            if ((header & (1 << i))!=0)
            {
                bin.children[i] = new Bin();
				load(buffer,*(bin.children[i]));
				bin.totalCount += bin.children[i]->totalCount;
            }
        }

        bin.totalCount += bin.binCount;
    }
}

bool StringHistogram::operator==( const StringHistogram & other ) const
{
	return compare(*m_rootBin,*(other.m_rootBin));
}

void StringHistogram::limit( int binCount )
{
	m_rootBin->limitBins(binCount);
}

RangeInt StringHistogram::count( const RangeString & range ) const
{
	return m_rootBin->count(range);
}

int StringHistogram::count() const
{
	return m_rootBin->totalCount;
}

Range<double> StringHistogram::percentile( const std::string & value ) const
{
	Range<int> result = count(Range<std::string>(std::string(),value,knInfinite,knClosed));
	double ratio = 100.0/count();
	return Range<double>::createClosedClosed(result.min * ratio ,result.max * ratio);
}

PYXWireBuffer & operator >> (PYXWireBuffer & buffer,StringHistogram & histogram)
{
	buffer >> histogram.m_boundaries.min >> histogram.m_boundaries.max;
	histogram.m_boundaries.minType = knClosed;
	histogram.m_boundaries.maxType = knClosed;
	histogram.m_rootBin.reset(new StringHistogram::Bin());
	StringHistogram::load(buffer,*histogram.m_rootBin);
	return buffer;
}


PYXWireBuffer & operator << (PYXWireBuffer & buffer,const StringHistogram & histogram)
{
	buffer << histogram.m_boundaries.min << histogram.m_boundaries.max;
	StringHistogram::save(buffer,*histogram.m_rootBin);
	return buffer;
}