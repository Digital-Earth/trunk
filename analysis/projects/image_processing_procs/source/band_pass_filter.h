#ifndef BAND_PASS_FILTER_H
#define BAND_PASS_FILTER_H
/******************************************************************************
band_pass_filter.h

begin		: 05/04/2008 9:02:17 PM
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_image_processing_procs.h"

#include "pyxis/data/coverage.h"
#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/data_processor.h"
#include "pyxis/procs/string.h"

/*!
The user sets the data limits and the appropriate operatons in the attributes of the 
process. Data from the input coverage that is within the user specified range is 
allowed to pass through the filter. Data that is outside of the range is returned 
as null.
*/
//! Filter out all data that is not within the specified band of values.
class MODULE_IMAGE_PROCESSING_PROCS_DECL BandPassFilter : public ProcessImpl<BandPassFilter>, public CoverageBase
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

public:

	enum Op
	{
		knNoop, // must be first
		knLess,
		knLessEqual,
		knEqualTo,
		knNotEqualTo,
		knGreater,
		knGreaterEqual
	};

public:

	//! Constructor
	BandPassFilter();

protected:
	//! Destructor
	virtual ~BandPassFilter();

public: // ICoverage

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		return m_spCov->getCoverageDefinition();
	}

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		return m_spCov->getCoverageDefinition();
	}

	//! Get a coverage value at particular index.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex = 0	) const;

	virtual void createGeometry() const;

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

private:

	boost::intrusive_ptr<ICoverage> m_spCov;

	Op m_op1;
	Op m_op2;

	PYXValue m_v1;
	PYXValue m_v2;

	mutable bool m_bDirty;
	boost::intrusive_ptr<IString> m_spOutput;
};

#endif // guard
