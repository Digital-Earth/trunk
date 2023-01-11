#ifndef REMOVE_FROM_LIBRARY_COMMAND_H
#define REMOVE_FROM_LIBRARY_COMMAND_H
/******************************************************************************
remove_from_library_command.h

begin      : 02/08/2007 11:40:38 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"
#include "library.h"

// pyxlib includes
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/utility/command.h"

/*!
Issue a command to the library to make a particular process temporary. The library
will be left to perform the actual delete of the process during normal cleanup
operations.
*/
//! Remove a command from the library.
class LIBRARY_DECL RemoveFromLibraryCommand : public Command
{
public:
	
	//! Create a removal command
	explicit RemoveFromLibraryCommand(ProcRef procref);

	//! Destructor.
	virtual ~RemoveFromLibraryCommand() {;}	

	//! Get the name of the command.
	virtual std::string getName() const {return "Remove process";}

	//! Execute the command.
	virtual bool execute();

	//! Undo the command.
	virtual bool undo();

protected:

private:

	//! The process to remove
	LibraryItem m_item;

	//! Indicates if the library item was successfully retrieved
	bool m_bInitialized;
};

#endif
