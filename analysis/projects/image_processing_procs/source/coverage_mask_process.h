#ifndef COVERAGE_MASK_PROCESS_H
#define COVERAGE_MASK_PROCESS_H
/******************************************************************************
coverage_mask_process.h

begin		: 2006-09-06
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_image_processing_procs.h"

//pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

/*!
Filters an image by returning data from an input based on the state of
the data at the control input.  The process can either return data
for everywhere that the control input has data, or for everywhere that
the control input does not have data, or for everywhere that the control
input has an exact data value.
*/
class MODULE_IMAGE_PROCESSING_PROCS_DECL CoverageMaskProcess  : public ProcessImpl<CoverageMaskProcess>, public CoverageBase
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

	IUNKNOWN_DEFAULT_CAST( CoverageMaskProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	//! Get the output of this process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	//! Get the attributes associated with this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes associated with this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	std::string STDMETHODCALLTYPE getAttributeSchema() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		return m_spInputCoverage->getCoverageDefinition();
	}

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		return m_spInputCoverage->getCoverageDefinition();
	}

	//! Get a coverage value at particular index.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(const PYXIcosIndex& index,
																	int nRes,
																	int nFieldIndex = 0	) const;

protected: //CoverageBase.

	//! Create a geometry for this coverage.
	virtual void createGeometry() const;

public: //CoverageMaskProcess

	//! Constants
	static const std::string kstrScope;

	//! The modes for this filter.
	enum eFilterMode
	{
		knReturnIfControlHasData = 0,
		knReturnIfControlHasNoData,
		knReturnIfControlDataEquals,
		knReturnIfControlDataNotEqual
	};

	//! Unit test method
	static void test();

	//! Default constructor.
	CoverageMaskProcess();

protected:
	//! Destructor.
	virtual ~CoverageMaskProcess(){;}

private:

	//! Disable copy constructor
	CoverageMaskProcess(const CoverageMaskProcess&);

	//! Disable copy assignment
	void operator =(const CoverageMaskProcess&);

	//! Control input coverage.
	boost::intrusive_ptr<ICoverage> m_spControlInput; 

	//! Input Coverage.
	boost::intrusive_ptr<ICoverage> m_spInputCoverage;

	//! the mode the filter is operating in
	eFilterMode m_mode;

	//! the value used to compare in Data Equals mode
	PYXValue m_compareValue;
};

#endif	// end guard
