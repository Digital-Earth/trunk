/******************************************************************************
icos_tree.cpp

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "icos_tree.h"

#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>


void test()
{
	RangeInt i(0,100,knClosed,knClosed);

	i.contains(10);
	i.contains(100);

	std::vector<RangeInt> v;
	v.push_back(RangeInt::createClosedClosed(0,10));
	v.push_back(RangeInt::createClosedClosed(10,100));
	RangeInt s = RangeInt::sum(v.begin(),v.end());

	RangeInt d = RangeInt::merge(v.begin(),v.end());

	RangeInt norm = i.normalize();


	std::list<double> list;

	NumericHistogram<double> hist(list.begin(),list.end());

	PYXStringWireBuffer buffer;

	buffer << hist;

	buffer.setPos(0);

	buffer >> hist;
}


SpatialHistogram::Bin::Bin() : m_binCount(0), m_binTotalCount(0)
{
	for(int i=0;i<7;i++) 
	{
		m_childBins[i] = 0;
	}
}

SpatialHistogram::Bin::~Bin()
{
	for(int i=0;i<7;i++)
	{
		delete m_childBins[i];
	}
}

SpatialHistogram::Bin * SpatialHistogram::Bin::getSingleChildOrNull()
{
	Bin * result = 0;

	for(int i=0;i<7;i++) 
	{
		if (m_childBins[i] != NULL)
		{
			if (result == NULL)
				result = m_childBins[i];
			else
				return NULL;
		}
	}
	return result;
}

int SpatialHistogram::Bin::removeChildBins()
{
	int result = 0;

	for(int i=0;i<7;i++) 
	{
		if (m_childBins[i] != NULL)
		{
			delete m_childBins[i];
			m_childBins[i] = NULL;
			result++;
		}
	}
	
	return result;
}

SpatialHistogram::SpatialHistogram() : m_rootBins(MAXROOTS),m_binCount(0)
{
}

SpatialHistogram::~SpatialHistogram()
{
	for(int i=0;i<MAXROOTS;++i)
	{
		delete m_rootBins[i];
	}
}

int SpatialHistogram::getRootIndex(const PYXIcosIndex & index) const
{
	if (index.isVertex())
	{
		return index.getPrimaryResolution() - index.knFirstVertex;
	}
	else
	{
		return index.getPrimaryResolution() - index.kcFaceFirstChar + index.knLastVertex - index.knFirstVertex + 1;
	}
}

void SpatialHistogram::addFeature(const PYXIcosIndex & index,const PYXBoundingCircle & sphere)
{
	int rootIndex = getRootIndex(index);
	if (m_rootBins[rootIndex] == NULL)
	{
		m_rootBins[rootIndex] = new Bin();
		m_binCount++;
	}

	addFeature(m_rootBins[rootIndex],index.getSubIndex(),0,sphere);
}

void SpatialHistogram::addFeature(Bin * bin,const PYXIndex & index,int digit,const PYXBoundingCircle & sphere)
{
	if (index.getDigitCount() == digit)
	{
		bin->m_binCount++;
		bin->m_binTotalCount++;
		bin->m_sphere += sphere;
	}
	else
	{
		bin->m_binTotalCount++;
		int childIndex = index.getDigit(digit);
		if (bin->m_childBins[childIndex] == NULL)
		{
			bin->m_childBins[childIndex] = new Bin();
			bin->m_binCount++;
		}
		addFeature(bin->m_childBins[childIndex],index,digit+1,sphere);
		bin->m_sphere += bin->m_childBins[childIndex]->m_sphere;
	}
}

void SpatialHistogram::limitBins(int limit)
{
	std::vector<Bin*> noneSingleNodes;

	for(int i=0;i<MAXROOTS;++i)
	{
		collectNoneSingleNodes(noneSingleNodes,m_rootBins[i]);
	}

	std::sort(noneSingleNodes.begin(),noneSingleNodes.end(),SpatialHistogram::Bin::sizeOfSphere);

	for(int i=0;m_binCount>limit;++i)
	{
		m_binCount -= noneSingleNodes[i]->removeChildBins();
	}
}



void SpatialHistogram::collectNoneSingleNodes(std::vector<Bin*> & noneSingleNodes,Bin * bin)
{
	if (bin == NULL)
		return;

	Bin * singleChild = bin->getSingleChildOrNull();

	if (singleChild != NULL)
	{
		collectNoneSingleNodes(noneSingleNodes,singleChild);
	}
	else 
	{
		noneSingleNodes.push_back(bin);
		for(int i=0;i<7;++i)
		{
			collectNoneSingleNodes(noneSingleNodes,bin->m_childBins[i]);
		}
	}
}

class BoundingCircleVisitor
{
private:
	const PYXBoundingCircle & m_sphere;

public:
	BoundingCircleVisitor(const PYXBoundingCircle & sphere) : m_sphere(sphere)
	{
	}

	PYXRegion::CellIntersectionState operator()(const PYXBoundingCircle & sphere) const
	{
		if (m_sphere.intersects(sphere))
		{
			if (m_sphere.contains(sphere))
			{
				return PYXRegion::knComplete;
			}
			return PYXRegion::knPartial;
		}
		return PYXRegion::knNone;
	}
};

Range<float> SpatialHistogram::getFeaturesCount(const PYXBoundingCircle & sphere)
{
	return getFeaturesCount(BoundingCircleVisitor(sphere));
}


class RegionVisitor
{
private:
	const PYXVectorRegion & m_region;

public:
	RegionVisitor(const PYXVectorRegion & region) : m_region(region)
	{
	}

	PYXRegion::CellIntersectionState operator()(const PYXBoundingCircle & sphere) const
	{
		return m_region.intersects(sphere);
	}
};


Range<float> SpatialHistogram::getFeaturesCount(const PYXVectorRegion & region)
{
	return getFeaturesCount(RegionVisitor(region));
}



/////////////////////////////////////////////////////////////////////////////////////
// test
//////////////////////////////////////////////////////////////////////////////////////


class IcosTreeTests 
{
public:
	static void test()
	{
		CellIntersectionIcosTree tree;

		tree[PYXIcosIndex("A-001010")].state = PYXRegion::knComplete;
		tree[PYXIcosIndex("A-00104")].state = PYXRegion::knComplete;
		tree[PYXIcosIndex("A-00105")].state = PYXRegion::knPartial;

		TEST_ASSERT(tree[PYXIcosIndex("A-001010")].state == PYXRegion::knComplete);
		TEST_ASSERT(tree[PYXIcosIndex("A-00104")].state == PYXRegion::knComplete);
		TEST_ASSERT(tree[PYXIcosIndex("A-00105")].state == PYXRegion::knPartial);

		TEST_ASSERT(tree.getNode(PYXIcosIndex("A-00105")) != NULL);
	}
};

Tester<IcosTreeTests> tester;