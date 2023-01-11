#ifndef PYXIS__UTILITY__COST_H
#define PYXIS__UTILITY__COST_H
/******************************************************************************
cost.h

begin		: 2010-10-14
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

/*!
This class used to estimate the cost of operation.
A Cost is define by a normal distribution function (Mean and Deviation)
*/
//! This class used to estimate the cost of operation.
class PYXLIB_DECL PYXCost
{
public:

	//! Default constructor.
	PYXCost() : m_average(0),m_deviation(0) {}

	//! convince constructor.
	PYXCost(double cost,double deviation = 0) : m_average(cost),m_deviation(deviation) {}

	//! copy constructor.
	PYXCost(const PYXCost & cost) : m_average(cost.m_average),m_deviation(cost.m_deviation) {}

	//! Destructor.
	virtual ~PYXCost() {}

	//Access
public:
	//! Return the average cost in seconds
	double getAverageCost() const { return m_average; }

	//! Return the deviation of cost 
	double getCostDeviation() const { return m_deviation; } 

	//! Return the estimated maximum (95%) cost in seconds
	double getMaximumCost() const { return getAverageCost()+2*getCostDeviation(); }

	//Operators
public:
	PYXCost & operator=(const PYXCost & cost);

	bool operator==(const PYXCost & cost) const;
	bool operator!=(const PYXCost & cost) const;
	bool operator<(const PYXCost & cost) const;
	bool operator<=(const PYXCost & cost) const;
	bool operator>(const PYXCost & cost) const;
	bool operator>=(const PYXCost & cost) const;

	PYXCost & operator+=(double cost);	
	PYXCost & operator+=(const PYXCost & cost);
	PYXCost & operator-=(const PYXCost & cost);
	PYXCost & operator*=(double factor);
	PYXCost & operator/=(double factor);

	PYXCost operator+(double cost) const; 	
	PYXCost operator+(const PYXCost & cost) const;
	PYXCost operator-(const PYXCost & cost) const;
	PYXCost operator*(double factor) const;
	PYXCost operator/(double factor) const;

	//static
public:
	static PYXCost mean(const PYXCost & costA,const PYXCost & costB);

	//! default 1 second cost
	static PYXCost knDefaultCost;

	//! default very fast operation (10 milliseconds)
	static PYXCost knImmediateCost;

	//! default very long network operation (10 ~ 30 seconds)
	static PYXCost knNetworkCost;

private:

	//! The cost average (in seconds)
	double m_average;

	//! The cost average (in seconds)
	double m_deviation;

public:
	//! Unit test
	static void test();
};

#endif // guard
