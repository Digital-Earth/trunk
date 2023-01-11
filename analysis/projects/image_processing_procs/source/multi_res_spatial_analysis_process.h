#ifndef MULTI_RES_SPATIAL_ANALYSIS_PROCESS
#define MULTI_RES_SPATIAL_ANALYSIS_PROCESS
/******************************************************************************
multi_res_analysis_process.h

begin		: 2006-09-05
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "module_image_processing_procs.h"

// pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"


/*!
MultiResSpatialAnalysisProcess, is a filter which implements a
multi resolutional spatial analysis algorithm. This algorithm
is implemented as a series of zoom in, and zoom out filters
interconnected to separate high, low, medium frequencies
from a coverage. This filter takes a single channel coverage as
input and turns it into n channels creating a multi channel
datasource. Each channel subdivides the frequency data of the image.
Low channels will contain high frequency data from the image while
higher channels contain lower frequncy data from the image. The higher
the channel the lower the frequncy data.
Each channel of this coverage is at a different spectrum
resolution. The changes in resolution are accomplished through
a series of daisy chained, zoom in and zoom out filters,
along with cache filters at appropriate places to increase
performance.
*/
//!  Creates a MultiResolutional data source, of n channels.
class MODULE_IMAGE_PROCESSING_PROCS_DECL MultiResSpatialAnalysisProcess : public ProcessImpl<MultiResSpatialAnalysisProcess>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public: //PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( MultiResSpatialAnalysisProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the interface this process outputs.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	//! Get the interface this process outputs.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		return m_spCovDefn;
	}

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		return m_spCovDefn;
	}

	//! Get the attributes associated with this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes associated with this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	//! Get a coverage value at the specified index.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;
public: // MultiResSpatialAnalysisProcess
	
	//! Constants
	static const std::string kstrSteps;

	//! Unit test method
	static void test();

	//! Default constructor.
	MultiResSpatialAnalysisProcess(): m_nNumberOfSteps(0){;}

protected:
	//! Destructor.
	virtual ~MultiResSpatialAnalysisProcess(){;}
	
protected: //Coverage base.

	//! Create the geometry for the coverage.
	virtual void createGeometry() const;

	//! Disable copy constructor
	MultiResSpatialAnalysisProcess(const MultiResSpatialAnalysisProcess&);

	//! Disable copy assignment
	void operator =(const MultiResSpatialAnalysisProcess&);

private:

	//! Helper method to create meta data to describe the coverage.
	void createMetaData();

	//! The blur filter which performs filtering.
	boost::intrusive_ptr<ICoverage> m_spBlurProc;
	
	//! Number of steps to zoom in/out on.
	int m_nNumberOfSteps;

	//! The input coverage.
	boost::intrusive_ptr<ICoverage> m_spInputCoverage;

};

#endif	// end guard
