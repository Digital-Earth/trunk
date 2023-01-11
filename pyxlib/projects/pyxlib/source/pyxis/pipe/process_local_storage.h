#ifndef PYXIS__PIPE__PROCESS_LOCAL_STORAGE_H
#define PYXIS__PIPE__PROCESS_LOCAL_STORAGE_H
/******************************************************************************
process_local_storage.h

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/local_storage.h"

///////////////////////////////////////////////////////////////////////////////
// PYXProcessLocalStorage
///////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXProcessLocalStorage
{
public:
	static PYXPointer<PYXLocalStorage> create(const PYXPointer<IProcess> & process);
	static PYXPointer<PYXLocalStorage> create(const std::string & identity);
};

#endif // guard
