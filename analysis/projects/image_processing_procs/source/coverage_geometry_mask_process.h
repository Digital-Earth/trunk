#ifndef COVERAGE_GEOMETRY_MASK_PROCESS_H
#define COVERAGE_GEOMETRY_MASK_PROCESS_H
/******************************************************************************
coverage_geometry_mask_process.h

begin		: 2008-05-08
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_image_processing_procs.h"

//pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

/*!
Filters an image by returning data from an input based on the geometry of
the control input.  The process can either return data inside the control geometry
or outside the control geometery.
*/
class MODULE_IMAGE_PROCESSING_PROCS_DECL CoverageGeometryMaskProcess  : 
	public ProcessImpl<CoverageGeometryMaskProcess>, public CoverageBase
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

	IUNKNOWN_DEFAULT_CAST( CoverageGeometryMaskProcess, IProcess);

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

public: //CoverageGeometryMaskProcess

	//! Constants
	static const std::string kstrScope;

	//! The modes for this filter.
	enum eFilterMode
	{
		knReturnDataInside = 0,
		knReturnDataOutside,
		knReturnDataOnBorder
	};

	//! Unit test method
	static void test();

	//! Default constructor.
	CoverageGeometryMaskProcess();

protected:
	//! Destructor.
	virtual ~CoverageGeometryMaskProcess(){;}

private:

	//! Disable copy constructor
	CoverageGeometryMaskProcess(const CoverageGeometryMaskProcess&);

	//! Disable copy assignment
	void operator =(const CoverageGeometryMaskProcess&);

	//! Input Coverage.
	boost::intrusive_ptr<ICoverage> m_spInputCoverage; 

	//! The geometry to use for masking.
	PYXPointer<PYXGeometry> m_spMaskGeometry;

	//! the mode the filter is operating in
	eFilterMode m_mode;
};

#endif	// end guard
