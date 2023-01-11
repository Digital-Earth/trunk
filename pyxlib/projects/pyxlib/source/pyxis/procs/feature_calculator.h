#ifndef PYXIS__PROCS__FEATURE_CALCULATOR_H
#define PYXIS__PROCS__FEATURE_CALCULATOR_H
/******************************************************************************
feature_calculator.h

begin      : 07/04/2008 9:50:47 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/data/feature.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/value.h"

//! Perform a calculation on a feature.
struct PYXLIB_DECL IFeatureCalculator : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//virtual PYXValue::eType STDMETHODCALLTYPE getReturnType() const = 0;

	virtual PYXValue STDMETHODCALLTYPE calculateValue(boost::intrusive_ptr<IFeature> spFeature,int fieldIndex) const = 0;

	virtual boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE calculateValues(boost::intrusive_ptr<IFeature> spFeature) const = 0;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getOutputDefinition() const = 0;
};

#endif // guard
