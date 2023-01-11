#ifndef COVERAGE_HISTOGRAM_CALCULATOR_H
#define COVERAGE_HISTOGRAM_CALCULATOR_H
/******************************************************************************
coverage_histogram_calculator.h

begin		: Wednesday, October 31, 2012 8:49:41 PM
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_feature_processing_procs.h"

#include "pyxis/data/coverage.h"
#include "pyxis/pipe/process.h"
#include "pyxis/data/histogram.h"
#include "pyxis/procs/coverage_histogram_calculator.h"
#include "pyxis/data/impl/numeric_histogram_impl.h"
#include "pyxis/data/impl/string_histogram_impl.h"



/*!
Returns a histogram for the given coverage inside the given feature
*/
//! Create coverage histogram process.
class MODULE_FEATURE_PROCESSING_PROCS_DECL CoverageHistogramCalculator : public ProcessImpl<CoverageHistogramCalculator>, public ICoverageHistogramCalculator
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	CoverageHistogramCalculator();


	//! Destructor
	~CoverageHistogramCalculator();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(ICoverageHistogramCalculator)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IDataProcessor

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverageHistogramCalculator*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverageHistogramCalculator*>(this);
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

// ICoverageHistogramProvider
public:

	virtual PYXPointer<PYXCellHistogram> STDMETHODCALLTYPE getHistogram( int fieldIndex,PYXPointer<IFeature> feature );
	int findResolution(const PYXPointer<PYXGeometry> & geometry,int initialResolution,double maxPartialPerecnt) const;
	void aggregate(const PYXPointer<PYXValueTile> & tile, const PYXPointer<PYXInnerTileIntersectionIterator> & neededCells);

private:

	boost::intrusive_ptr<ICoverage> m_spCoverage;
	boost::intrusive_ptr<IFeature> m_spFeature;
	PYXPointer<PYXCellHistogram> m_histogram;
	NumericHistogram<double> m_numericHistogram;
	int m_cellResolution;
};


#endif // guard
