#ifndef PYXIS__PROCS__COVERAGE_HISTOGRAM_CALCULATOR_H
#define PYXIS__PROCS__COVERAGE_HISTOGRAM_CALCULATOR_H
/******************************************************************************
coverage_histogram_calculator.h

begin		: Wednesday, October 31, 2012 8:49:41 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
/******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/data/feature.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/value.h"
#include "pyxis/data/histogram.h"

//! Perform a calculation on a feature.
struct PYXLIB_DECL ICoverageHistogramCalculator : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual PYXPointer<PYXCellHistogram> STDMETHODCALLTYPE getHistogram(int fieldIndex,PYXPointer<IFeature> feature) = 0;
};

#endif // guard