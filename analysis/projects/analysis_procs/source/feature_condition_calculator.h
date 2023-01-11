#ifndef FEATURE_CONDITION_CALCULATOR_H
#define FEATURE_CONDITION_CALCULATOR_H
/******************************************************************************
feature condition calculator.h

begin		: 2013-4-22
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"
#include "pyxis/procs/feature_calculator.h"
#include "boost/assign.hpp"
#include "pyxis/utility/range.h"


enum Operation
{
	NO_CONDITION = 10,
	GREATER_THAN_OR_EQUAL = -2,
	GREATER_THAN = -1,
	LESS_THAN = 1,
	LESS_THAN_OR_EQUAL = 2,
	EQUALS = 0,
	BETWEEN = 5,
	NOT_EQUAL = -5
};

class MODULE_ANALYSIS_PROCS_DECL FeatureConditionCalculator : IFeatureCalculator, public ProcessImpl<FeatureConditionCalculator>
{
	PYXCOM_DECLARE_CLASS();

public:
	FeatureConditionCalculator(): m_compareTo(0), m_operation(Operation::NO_CONDITION), m_fieldIndex(0)
	{
		
	}
	//! Destructor
	~FeatureConditionCalculator(){}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCalculator)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeatureCalculator*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeatureCalculator*>(this);
	}

	virtual std::string STDMETHODCALLTYPE FeatureConditionCalculator::getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl()
	{
		m_outputDefinition = PYXTableDefinition::create();
		return IProcess::knInitialized;
	}

public:

	static void test();

public:

	virtual PYXValue STDMETHODCALLTYPE calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const;

	virtual boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getOutputDefinition() const;

private:
	PYXValue m_compareTo;
	PYXValue m_compareToTop;
	Operation m_operation;
	int m_fieldIndex;
	static const std::map<Operation, char const*> s_operationName ;
	PYXPointer<PYXTableDefinition> m_outputDefinition;

private:
	bool isTrue( Range<PYXValue> value ) const;
};

#endif // guard
