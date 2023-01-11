#pragma once
#ifndef VIEW_MODEL__GO_TO_PIPELINE_COMMAND_H
#define VIEW_MODEL__GO_TO_PIPELINE_COMMAND_H
/******************************************************************************
go_to_pipeline_command.h

begin      : 31/10/2007 3:43:25 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// pyxlib includes
#include "pyxis/utility/command.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"

/*!
A Command that performs a 'Go To' on a pipeline being visualized.  If the 
pipeline outputs XY-based data, the View navigates to the center of the 
bounding rectangle.  If the pipeline outputs PYXIS data, the View navigates 
to the first index.
*/
class VIEW_MODEL_API GoToPipelineCommand : public Command
{
public:

	//! Destructor
	virtual ~GoToPipelineCommand();

	//! Create a new command
	static PYXPointer<GoToPipelineCommand> create(
		boost::intrusive_ptr<IProcess> spProc,
		int nViewID);

	//! Execute the command
	virtual bool execute();

	//! Negate the executed command.
	virtual bool undo();

	//! Return the display name of the command.
	virtual std::string getName() const {return m_strName;}

	virtual bool getRecordable() const;

	virtual bool getUndoable() const;

	virtual bool getEnabled() const;

public:
	
	//! get the Geometry to navigate to.
	static PYXPointer<PYXGeometry> getGeometry(boost::intrusive_ptr<IProcess> spProc);

private:

	//! construct the command object
	GoToPipelineCommand(boost::intrusive_ptr<IProcess> spProc, int nViewID);

	//! Disable default constructor
	GoToPipelineCommand();

	//! Disable copy constructor
	GoToPipelineCommand(const GoToPipelineCommand&);

	//! Disable copy assignment
	void operator =(const GoToPipelineCommand&);

private:

	//! The name of the command.
	std::string m_strName;

	//! The serialized camera representation for the starting position.
	std::string m_strCameraStart;

	//! The ending position for the camera to navigate to.
	PYXPointer<PYXGeometry> m_navigateGeometry;	

	//! The unique identifier for the View the command is being executed on.
	int m_nViewID;	
};

#endif
