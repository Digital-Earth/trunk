#ifndef PYXIS__PROCS__VIEWPOINT_H
#define PYXIS__PROCS__VIEWPOINT_H

/******************************************************************************
viewpoint.h

begin      : August 18, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"

/*!
A public interface to be implemented by the ViewPoint process.
*/
struct PYXLIB_DECL IViewPoint : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Returns a vector of all a ViewPoint process' input pipelines that output a coverage.
	virtual std::vector<boost::intrusive_ptr<IProcess> > STDMETHODCALLTYPE getCoveragePipelines() = 0;

	//! Returns a vector of all a ViewPoint process' input pipelines that output a feature collection.
	virtual std::vector<boost::intrusive_ptr<IProcess> > STDMETHODCALLTYPE getFeatureCollectionPipelines() = 0;

	//! Returns a vector of all a ViewPoint process' input pipelines that output an elevation.
	virtual std::vector<boost::intrusive_ptr<IProcess> > STDMETHODCALLTYPE getElevationPipelines() = 0;

	//! Returns a vector of all a ViewPoint process' input pipelines.
	virtual std::vector<boost::intrusive_ptr<IProcess> > STDMETHODCALLTYPE getAllPipelines() = 0;

	//! Getter for the notifier that will raise the ViewPointChangedEvent.
	virtual Notifier& getViewPointChangedNotifier() = 0;

};

//! The event to raise when pipelines are added or removed from a ViewPoint process.
class PYXLIB_DECL ViewPointChangedEvent : public NotifierEvent
{

public:
	
	static PYXPointer<ViewPointChangedEvent> create()
	{
		return PYXNEW(ViewPointChangedEvent);
	}

	//! Default desctructor.
	~ViewPointChangedEvent() {;}

private:
	
	//! Default constructor.
	explicit ViewPointChangedEvent()
	{		
	};

};

#endif