#ifndef ZOOM_IN_PROCESS_H
#define ZOOM_IN_PROCESS_H
/******************************************************************************
zoom_in_proc.h

begin		: 2006-04-11
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

//local includes
#include "module_image_processing_procs.h"

//pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/exception.h"

/*! 
PYXZoomInProcess resamples coverage at resolution (n) to produce a coverage at resolution (n +1).

When data is requested from this coverage, the Icos Index is pinned to the resolution of the
input plus one.
*/

//! Filter to ZoomIn on a coverage going from resolution (n) to (n+1).
class MODULE_IMAGE_PROCESSING_PROCS_DECL PYXZoomInProcess : public ProcessImpl<PYXZoomInProcess>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( PYXZoomInProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the type this process outputs.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	//! Get the type this process outputs.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{		
		return getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>()->getCoverageDefinition();	
	}

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		return getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>()->getCoverageDefinition();			
	}

	//! Initialize the process.
	virtual IProcess::eInitStatus initImpl()
	{
		m_strID = "Zoom In: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

		m_spInputCoverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
		
		if (!m_spInputCoverage)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Input coverage is invalid");
			return knFailedToInit;
		}

		createGeometry();
		m_spCovDefn = m_spInputCoverage->getCoverageDefinition();
		return knInitialized;
	}

	//! Get the attributes associated with this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes associated with this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

public: // ICoverage

	//! Get a coverage value at particular index.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;
public:

	//! Constants 
	static const std::string kstrBlurringAlgor;
	static const std::string kstrIncNulls;
	static const std::string kstrYes;
	static const std::string kstrNo;
	static const std::string kstrScope;

	//! UnitTest Method
	static void test();

	//! Constructor
	PYXZoomInProcess() : m_bBlurring(false), m_bIncludeNulls(false) {;}

protected:
	//! Destructor
	virtual ~PYXZoomInProcess(){;}

protected: //CoverageBase

	//! Create the geometry for the coverage.
	virtual void createGeometry() const;
	 
private:


	//! true if we are running the blurring algorithm
	bool m_bBlurring;

	//! True if we want to weight in null values.
	bool m_bIncludeNulls;

	//! The input coverage.
	boost::intrusive_ptr<ICoverage> m_spInputCoverage;

};

#endif // end guard