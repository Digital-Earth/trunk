#ifndef FEATURE_PROCESSING_PROCS__AGGREGATE_COVERAGE_CALCULATOR_H
#define FEATURE_PROCESSING_PROCS__AGGREGATE_COVERAGE_CALCULATOR_H
/******************************************************************************
aggregate_coverage_calculator.h

begin      : 10/03/2012 4:42:06 PM 
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/value.h"
#include "pyxis/procs/feature_calculator.h"


class ICoverageAggregationOperator
{
public:
	virtual void aggregate(const PYXPointer<PYXValueTile> & tile, const PYXPointer<PYXInnerTileIntersectionIterator> & neededCells);
	virtual void aggregate(const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection) = 0;
	virtual PYXValue getResult() const = 0;
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition() = 0;
};


class ICoverageAggregationOperatorFactory
{
public: 
	virtual std::auto_ptr<ICoverageAggregationOperator> create() const = 0;
};


template <class T> 
class CoverageAggregationOperatorFactory : public ICoverageAggregationOperatorFactory
{
public: 
	virtual std::auto_ptr<ICoverageAggregationOperator> create() const
	{
		return std::auto_ptr<ICoverageAggregationOperator>(new T());
	}
};


class MODULE_FEATURE_PROCESSING_PROCS_DECL AggregateCoverageCalculator :  public ProcessImpl<AggregateCoverageCalculator>, IFeatureCalculator 
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCalculator)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: //IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr< PYXCOM_IUnknown const > STDMETHODCALLTYPE getOutput() const
	{
		return static_cast< IFeatureCalculator const * >(this);
	}

	//! Get the output of this process.
	virtual boost::intrusive_ptr< PYXCOM_IUnknown > STDMETHODCALLTYPE getOutput()
	{
		return static_cast< IFeatureCalculator * >(this);
	}

	//! Get the schema to describe how the attributes should be edited.
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	/*!
	Get the attributes associated with  this process. 

	\return a map of standard string - standard string containing the attributes to be serialized.
	*/
	virtual std::map< std::string, std::string > STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes of this process. 
	virtual void STDMETHODCALLTYPE setAttributes(std::map< std::string, std::string > const & mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IFeatureCalculator

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getOutputDefinition() const;

	virtual boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const;

	virtual PYXValue STDMETHODCALLTYPE calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const;


public: //AggregateCoverageCalculator

	AggregateCoverageCalculator();

private:
	void aggregateFeatures(const PYXVectorGeometry2 & geometry,ICoverageAggregationOperator & aggregator) const;
	void aggregateFeatures(const PYXTileCollection & geometry,ICoverageAggregationOperator & aggregator) const;

	int findResolution(const PYXPointer<PYXGeometry> & geometry,int initialResolution,double maxPartialPerecnt) const;

// Members
private:

	std::string m_OutputAttributeName;
	int m_inputFieldIndex;
	std::string m_operatorName;
	int m_cellResolution;

	boost::shared_ptr<ICoverageAggregationOperatorFactory> m_operatorFactory;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
	boost::intrusive_ptr<ICoverage> m_coverage;
};


class HighPrecisionSummary
{
public:
	HighPrecisionSummary(double statingSum) : sum(statingSum), c(0)
	{
	}

	HighPrecisionSummary() : sum(0), c(0)
	{
	}

	void add(double value)
	{
		//NOTE: algorithm can be found at : http://en.wikipedia.org/wiki/Kahan_summation_algorithm
		double next = value - c; //remove the carry
		double nextSum = sum + next; //add the big part of the value
		c = (nextSum - sum) - next; //find the new carry;
		sum = nextSum; //this is the new sum, next time we will use the carry.
	}

	double getSum() const
	{
		return sum - c;
	}

	HighPrecisionSummary & operator+=(double value)
	{
		add(value);
		return *this;
	}

	operator double() const { return getSum(); }

private:
	double sum; //total
	double c; //carry
};


class CoverageCoverOperator : public ICoverageAggregationOperator
{
public:
	CoverageCoverOperator() : m_areaInSqaureMetter(0), addCount(0), partialCount(0)
	{
	}
	virtual void aggregate(const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection);
	virtual PYXValue getResult() const;
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	HighPrecisionSummary m_areaInSqaureMetter;
	int addCount;
	int partialCount;
	double cellSize;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

class CoverageMinOperator : public ICoverageAggregationOperator
{
public:

	CoverageMinOperator() : m_value(0), m_foundValue(false)
	{
	}

	virtual void aggregate(const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection);
	virtual PYXValue getResult() const
	{
		if (m_foundValue)
		{
			return PYXValue(m_value);
		}
		else
		{
			return PYXValue();
		}
	}
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();

private:

	bool m_foundValue;
	double m_value;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

class CoverageMaxOperator : public ICoverageAggregationOperator
{
public:

	CoverageMaxOperator() : m_value(0), m_foundValue(false)
	{
	}

	virtual void aggregate(const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection);
	virtual PYXValue getResult() const
	{
		if (m_foundValue)
		{
			return PYXValue(m_value);
		}
		else
		{
			return PYXValue();
		}
	}
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	bool m_foundValue;
	double m_value;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};


class CoverageSumOperator : public ICoverageAggregationOperator
{
public:

	CoverageSumOperator() : m_value(0)
	{
	}

	virtual void aggregate(const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection);
	virtual PYXValue getResult() const
	{
		return PYXValue(m_value);
	}
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	HighPrecisionSummary m_value;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

class CoverageAreaDensitySumOperator : public ICoverageAggregationOperator
{
public:

	CoverageAreaDensitySumOperator() : m_value(0)
	{
	}

	virtual void aggregate(const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection);
	virtual PYXValue getResult() const
	{
		return PYXValue(m_value);
	}
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	HighPrecisionSummary m_value;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};


class CoverageAverageOperator : public ICoverageAggregationOperator
{
public:
	CoverageAverageOperator() : m_sum(0), m_count(0)
	{
	}
	virtual void aggregate(const PYXIcosIndex & cell, double cellArea, const PYXValue & value, PYXInnerTileIntersection intersection);
	virtual PYXValue getResult() const;
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	HighPrecisionSummary m_sum;
	int    m_count;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

#endif
