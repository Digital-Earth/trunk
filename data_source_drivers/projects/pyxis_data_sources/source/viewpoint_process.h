#ifndef VIEWPOINT_PROCESS_H
#define VIEWPOINT_PROCESS_H

/******************************************************************************
viewpoint_process.h

begin      : August 18, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_pyxis_coverages.h"

// pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/procs/viewpoint.h"

/*!
The ViewPoint process enables a pipeline (or a group of pipelines) to be 
visualized in WorldView.  It is an amalgamation of, and encapsulates, all 
the processes required to enable pipelines to be visualized.

The ViewPoint Process accepts the following inputs:
- 0 to many coverages
- 0 to many feature collections
- 0 to many elevations (eventually)
- 0 to 1 camera location
*/
class MODULE_PYXIS_COVERAGES_DECL ViewPointProcess : 
	public ProcessImpl<ViewPointProcess>, 
	public CoverageBase, 
	public IViewPoint
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(ICoverage)
		IUNKNOWN_QI_CASE(IViewPoint)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(ViewPointProcess, IProcess);

public:

	//! Constructor
	ViewPointProcess();

	//! Destructor
	~ViewPointProcess();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return m_spVisualizationPipeline;
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return m_spVisualizationPipeline;
	}
	
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;
	
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(
		const PYXIcosIndex& index, 
		int nFieldIndex = 0) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(
		const PYXIcosIndex& index, 
		int nRes, 
		int nFieldIndex = 0) const;

private: // CoverageBase

	virtual void createGeometry() const;

public: // IViewPoint

	virtual std::vector<boost::intrusive_ptr<IProcess> > STDMETHODCALLTYPE getCoveragePipelines();

	virtual std::vector<boost::intrusive_ptr<IProcess> > STDMETHODCALLTYPE getFeatureCollectionPipelines();

	virtual std::vector<boost::intrusive_ptr<IProcess> > STDMETHODCALLTYPE getElevationPipelines();

	virtual std::vector<boost::intrusive_ptr<IProcess> > STDMETHODCALLTYPE getAllPipelines();

	Notifier& getViewPointChangedNotifier()
	{
		return m_viewPointChangedNotifier;
	}

private: // ViewPointProcess

	//! Creates the visualization pipeline.
	void createVisualizationPipeline(
		std::vector<boost::intrusive_ptr<IProcess> > vecInputCoveragePipelines, 
		std::vector<boost::intrusive_ptr<IProcess> > vecInputFCPipelines);

	//! Handles the ParameterEvent raised when pipelines are added to or removed from the ViewPoint process.
	void parameterChanged(PYXPointer<NotifierEvent> spEvent);

	//! check if a pipeline need to be treated as elevation
	bool isElevation(const boost::intrusive_ptr<IProcess> & process);

	//! make sure an coverage that is trated as RGB has RGB values (add colourizer process if needed)
	boost::intrusive_ptr<IProcess> convertCoverageToRgbIfNeeded(const boost::intrusive_ptr<IProcess> & spPipeline);

public:

	//! Unit testing.
	static void test();

private:

	//! The actual visualization pipeline.
	boost::intrusive_ptr<IProcess> m_spVisualizationPipeline;
	
	//! A notifier to raise the ViewPointChangedEvent with.
	Notifier m_viewPointChangedNotifier;
};

#endif