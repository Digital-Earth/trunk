#ifndef PYXIS__UTILITY__LOCAL_STORAGE_IMPL_H
#define PYXIS__UTILITY__LOCAL_STORAGE_IMPL_H
/******************************************************************************
local_storage_impl.h

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/local_storage.h"

///////////////////////////////////////////////////////////////////////////////
// PYXPYXLocalStorageFactory
///////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXLocalStorageFactory
{
public:
	static PYXPointer<PYXLocalStorage> createSqlite(const std::string & file);	

	static PYXPointer<PYXLocalStorage> createREST(const std::string & partition);

	static PYXPointer<PYXLocalStorage> create(const std::string & dbPath);
};

#endif // guard
