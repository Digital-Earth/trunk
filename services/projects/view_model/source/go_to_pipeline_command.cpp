/******************************************************************************
go_to_pipeline_command.cpp

begin      : 31/10/2007 3:43:18 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "go_to_pipeline_command.h"

// local includes
#include "camera.h"
#include "exceptions.h"
#include "view.h"
#include "view_open_gl_thread.h"

// pyxlib includes
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/vector_geometry.h"


inline PYXPointer<GoToPipelineCommand> GoToPipelineCommand::create(
		boost::intrusive_ptr<IProcess> spProc,
		int nViewID)
{
	return PYXNEW(GoToPipelineCommand, spProc, nViewID);
}

GoToPipelineCommand::GoToPipelineCommand(
	boost::intrusive_ptr<IProcess> spProc,
	int nViewID) : m_nViewID(nViewID)
{
	assert(spProc);	

	m_strName = "Navigate To: " + spProc->getProcName();

	View* pView = View::getView(m_nViewID);
	camToCookieStr(pView->getCamera(), m_strCameraStart);

	m_navigateGeometry = getGeometry(spProc);
}

/*!
Destructor.
*/
GoToPipelineCommand::~GoToPipelineCommand()
{
}

bool GoToPipelineCommand::execute()
{
	View* pView = View::getView(m_nViewID);
	
	if (m_navigateGeometry)
	{
		pView->goToGeometry(m_navigateGeometry);
	}

	return true;
}

bool GoToPipelineCommand::undo()
{
	View* pView = View::getView(m_nViewID);
	Camera cam;
	camFromCookieStr(cam, m_strCameraStart);
	pView->goToCamera(cam);

	return true;
}

/*!
get the geometry out of a pipeline

\param	spProc	The pipeline this command is being executed on.

\return	The index to navigate to.

*/

PYXPointer<PYXGeometry> GoToPipelineCommand::getGeometry(boost::intrusive_ptr<IProcess> spProc)
{	
	if (spProc && spProc->getOutput())
	{
		boost::intrusive_ptr<IFeature> spFeature;
		spProc->getOutput()->QueryInterface(IFeature::iid, (void**) &spFeature);
	
		if (spFeature)
		{
			const PYXPointer<PYXGeometry> spGeometry = spFeature->getGeometry();
			assert((0 != spGeometry) && "Invalid geometry");

			return spGeometry;
		}
	}
	
	return PYXPointer<PYXGeometry>();
}

bool GoToPipelineCommand::getRecordable() const
{
	return true;
}

bool GoToPipelineCommand::getUndoable() const
{
	return true;
}

bool GoToPipelineCommand::getEnabled() const
{
	return true;
}