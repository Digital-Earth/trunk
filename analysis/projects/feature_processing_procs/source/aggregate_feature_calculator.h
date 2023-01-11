#ifndef FEATURE_PROCESSING_PROCS__AGGREGATE_FEATURE_CALCULATOR_H
#define FEATURE_PROCESSING_PROCS__AGGREGATE_FEATURE_CALCULATOR_H
/******************************************************************************
aggregate_feature_calculator.h

begin      : 08/22/2012 4:42:06 PM 
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/value.h"
#include "pyxis/procs/feature_calculator.h"


class IAggregationOperator
{
public:
	virtual void aggregate(const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex)  = 0;
	virtual void aggregate(const boost::intrusive_ptr<IFeature> & feature, int fieldIndex)  = 0;
	virtual PYXValue getResult() const = 0;
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition() = 0;
};


class IAggregationOperatorFactory
{
public: 
	virtual std::auto_ptr<IAggregationOperator> create() const = 0;
};


template <class T> 
class AggregationOperatorFactory : public IAggregationOperatorFactory
{
public: 
	virtual std::auto_ptr<IAggregationOperator> create() const
	{
		return std::auto_ptr<IAggregationOperator>(new T());
	}
};


class MODULE_FEATURE_PROCESSING_PROCS_DECL AggregateFeatureCalculator :  public ProcessImpl<AggregateFeatureCalculator>, IFeatureCalculator 
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


public: //XYBoundsRegionProc

	AggregateFeatureCalculator();

private:
	void aggregateFeatures(const boost::intrusive_ptr<IFeatureGroup> & group,const PYXVectorGeometry2 & geometry,IAggregationOperator & aggregator) const;
	void aggregateFeatures(const boost::intrusive_ptr<IFeatureGroup> & group,const PYXTileCollection & geometry,IAggregationOperator & aggregator) const;

// Members
private:

	std::string m_OutputAttributeName;
	int m_InputAttributeIndex;
	std::string m_inputName;
	boost::shared_ptr<IAggregationOperatorFactory> m_operatorFactory;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
	boost::intrusive_ptr<IFeatureGroup> m_featureGroup;
	std::string m_operatorName;
};


class CountOperator : public IAggregationOperator
{
public:
	CountOperator() : m_value(0)
	{
	}
	virtual void aggregate(const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex);
	virtual void aggregate(const boost::intrusive_ptr<IFeature> & feature, int fieldIndex);
	virtual PYXValue getResult() const;
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	int m_value;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

class MinOperator : public IAggregationOperator
{
public:

	MinOperator() : m_value(0), m_foundValue(false)
	{
	}
	virtual void aggregate(const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex);
	virtual void aggregate(const boost::intrusive_ptr<IFeature> & feature, int fieldIndex);
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
		return PYXValue(m_value);
	}
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	bool m_foundValue;
	double m_value;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

class MaxOperator : public IAggregationOperator
{
public:

	MaxOperator() : m_value(0), m_foundValue(false)
	{
	}

	virtual void aggregate(const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex);
	virtual void aggregate(const boost::intrusive_ptr<IFeature> & feature, int fieldIndex);
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
		return PYXValue(m_value);
	}
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	bool m_foundValue;
	double m_value;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

class SumOperator : public IAggregationOperator
{
public:

	SumOperator() : m_value(0)
	{
	}

	virtual void aggregate(const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex);
	virtual void aggregate(const boost::intrusive_ptr<IFeature> & feature, int fieldIndex);
	virtual PYXValue getResult() const
	{
		return PYXValue(m_value);
	}
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	double m_value;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};


class AverageOperator : public IAggregationOperator
{
public:
	virtual void aggregate(const boost::intrusive_ptr<IFeatureGroup> & group, int fieldIndex);
	virtual void aggregate(const boost::intrusive_ptr<IFeature> & feature, int fieldIndex);
	virtual PYXValue getResult() const;
	virtual PYXPointer<PYXTableDefinition> getOutputDefinition();
private:
	SumOperator m_sum;
	CountOperator m_count;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

#endif
