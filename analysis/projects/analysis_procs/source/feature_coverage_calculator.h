#ifndef FEATURE_COVERAGE_CALCULATOR_H
#define FEATURE_COVERAGE_CALCULATOR_H
/******************************************************************************
feature_coverage_calculator.h

begin		: 2008-03-28
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/feature_calculator.h"

/*!
Calculates the percentage of a feature's geometry that is covered by a particular
value in a coverage.
*/
//! Feature coverage calculator process.
class MODULE_ANALYSIS_PROCS_DECL FeatureCoverageCalculator : public ProcessImpl<FeatureCoverageCalculator>, public IFeatureCalculator
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeatureCoverageCalculator();

	//! Destructor
	~FeatureCoverageCalculator();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCalculator)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IDataProcessor

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

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

// FeatureCalculator
public:

	//! The calculated ratio is always returned as a double
	PYXValue::eType STDMETHODCALLTYPE getReturnType() const
	{
		return PYXValue::knDouble;
	}

	//! Perform the calculation
	virtual PYXValue STDMETHODCALLTYPE calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const;

	virtual boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getOutputDefinition() const;

private:

	boost::intrusive_ptr<ICoverage> m_spCov;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
};

#endif // guard
