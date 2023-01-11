/******************************************************************************
remove_from_library_command.cpp

begin      : 02/08/2007 11:38:52 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#define LIBRARY_SOURCE

// local includes
#include "remove_from_library_command.h"

/*!
\param procref The unique identifier of the process to remove.
*/
RemoveFromLibraryCommand::RemoveFromLibraryCommand(ProcRef procref) :
	m_bInitialized(false)
{
	m_bInitialized = Library::getItem(procref, &m_item);
	assert(m_bInitialized && "Item does not exist, this command will fail");
}

bool RemoveFromLibraryCommand::execute()
{
	if (m_bInitialized)
	{
//		return Library::setProcTemp(m_item, true);
	}

	return false;
}

bool RemoveFromLibraryCommand::undo()
{
	if (m_bInitialized)
	{
	//	return Library::setProcTemp(m_item, false);
	}

	return false;
}
