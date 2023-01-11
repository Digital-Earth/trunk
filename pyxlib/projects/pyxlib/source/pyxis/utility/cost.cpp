/******************************************************************************
cost.cpp

begin		: 2010-10-14
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/cost.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>


//! Tester class
Tester< PYXCost > gTester;


PYXCost & PYXCost::operator=(const PYXCost & cost)
{
	m_average = cost.m_average;
	m_deviation = cost.m_deviation;
	return *this;
}

bool PYXCost::operator==(const PYXCost & cost) const
{
	return MathUtils::equal(m_average,cost.m_average) && MathUtils::equal(m_deviation,cost.m_deviation);
}

bool PYXCost::operator!=(const PYXCost & cost) const
{
	return !MathUtils::equal(m_average,cost.m_average) || !MathUtils::equal(m_deviation,cost.m_deviation);
}

bool PYXCost::operator<(const PYXCost & cost) const
{
	return m_average < cost.m_average;
}

bool PYXCost::operator<=(const PYXCost & cost) const
{
	return m_average <= cost.m_average;
}

bool PYXCost::operator>(const PYXCost & cost) const
{
	return m_average > cost.m_average;
}

bool PYXCost::operator>=(const PYXCost & cost) const
{
	return m_average >= cost.m_average;
}

PYXCost & PYXCost::operator+=(double cost)
{
	m_average += cost;
	return *this;
}

PYXCost & PYXCost::operator+=(const PYXCost & cost)
{
	m_average += cost.m_average;
	m_deviation = sqrt(m_deviation*m_deviation + cost.m_deviation*cost.m_deviation);
	return *this;
}

PYXCost & PYXCost::operator-=(const PYXCost & cost)
{
	m_average -= cost.m_average;
	m_deviation = sqrt(m_deviation*m_deviation + cost.m_deviation*cost.m_deviation);
	return *this;
}

PYXCost & PYXCost::operator*=(double factor)
{
	m_average*=factor;
	m_deviation*=abs(factor);
	return *this;
}

PYXCost & PYXCost::operator/=(double factor)
{
	m_average/=factor;
	m_deviation/=abs(factor);
	return *this;
}

PYXCost PYXCost::operator+(double cost) const
{
	return PYXCost(m_average+cost,m_deviation);	
}

PYXCost PYXCost::operator+(const PYXCost & cost) const
{
	return PYXCost(m_average+cost.m_average,sqrt(m_deviation*m_deviation + cost.m_deviation*cost.m_deviation));	
}

PYXCost PYXCost::operator-(const PYXCost & cost) const
{
	return PYXCost(m_average-cost.m_average,sqrt(m_deviation*m_deviation + cost.m_deviation*cost.m_deviation));		
}

PYXCost PYXCost::operator*(double factor) const
{
	return PYXCost(m_average*factor,m_deviation*abs(factor));
}

PYXCost PYXCost::operator/(double factor) const
{
	return PYXCost(m_average/factor,m_deviation/abs(factor));
}

PYXCost PYXCost::mean(const PYXCost & costA,const PYXCost & costB)
{
	return (costA+costB)/2;	
}


//! default 1 second cost
PYXCost PYXCost::knDefaultCost(1.0);

//! default very fast operation (10 milliseconds)
PYXCost PYXCost::knImmediateCost(0.01);

//! default very long network operation (10 ~ 30 seconds)
PYXCost PYXCost::knNetworkCost(20.0,5.0);



void PYXCost::test()
{
	//default values tests
	PYXCost cost(10.0,1.0);

	TEST_ASSERT(cost.getAverageCost() == 10.0);
	TEST_ASSERT(cost.getCostDeviation() == 1.0);
	TEST_ASSERT(cost.getMaximumCost() == 12.0);

	//copy ctor tests
	cost = knNetworkCost;

	TEST_ASSERT(cost.getAverageCost() == knNetworkCost.getAverageCost());
	TEST_ASSERT(cost.getCostDeviation() == knNetworkCost.getCostDeviation());	

	//operators tests
	cost = PYXCost(1.0)*2;
	TEST_ASSERT(cost.getAverageCost() == 2.0);
	cost += 8;
	TEST_ASSERT(cost.getAverageCost() == 10.0);
	cost /= 10;
	TEST_ASSERT(cost.getAverageCost() == 1.0);
}