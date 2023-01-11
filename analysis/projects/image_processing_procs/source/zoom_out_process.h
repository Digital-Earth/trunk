#ifndef ZOOM_OUT_PROCESS_H
#define ZOOM_OUT_PROCESS_H
/******************************************************************************
zoom_out_process.h

begin		: 2006-04-07
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
PYXZoomOutProcess resamples a resolution (n) coverage at resolution (n-1),
performing either weighted average or weighted sum as selected by client code.

When data is requested from this coverage, the Icos Index is pinned to the resolution
of the input minus one.
*/
//! Resample a resolution (n) coverage at resolution (n-1)
class MODULE_IMAGE_PROCESSING_PROCS_DECL PYXZoomOutProcess: public ProcessImpl<PYXZoomOutProcess>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public: //IUknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( PYXZoomOutProcess, IProcess);

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
	/*!
	Intialize this process, by querying the interface for the input parameter value, and storing
	that in a variable so we don't have to query the interface everytime we wish to access the input.
	If we successfully got the input then we create the geometry and pass the meta data through.
	*/
	virtual IProcess::eInitStatus initImpl()
	{
		m_strID = "Zoom Out: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

		m_spInputCoverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
		
		assert(m_spInputCoverage);

		createGeometry();
		m_spCovDefn = m_spInputCoverage->getCoverageDefinition();
		return IProcess::knInitialized;
	}

public: // ICoverage

	//! Get the coverage value at a particular index and fieldindex.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue( const PYXIcosIndex& index,
														int nFieldIndex = 0	) const;
public: //PYXZoomOutProcess.

	//! Constants 
	static const std::string kstrScope;
	static const std::string kstrAverage;
	static const std::string kstrIncNulls;
	static const std::string kstrYes;
	static const std::string kstrNo;

	//! Unit test method 
	static void test();

	//! Constructor
	PYXZoomOutProcess() : m_bAverage(true), m_bIncludeNulls(false) {;}

protected:
	//! Destructor 
	virtual ~PYXZoomOutProcess(){;}

protected: // Coverage Base.

	//! Create the geometry that corresponds to this coverage.
	virtual void createGeometry() const;

private:

	//! True if we are averaging, false if summing.
	bool m_bAverage;

	//! True if we want to weight in null values.
	bool m_bIncludeNulls;

	//! The input coverage to perform the zoom out on.
	boost::intrusive_ptr<ICoverage> m_spInputCoverage;
};

#endif // end guard