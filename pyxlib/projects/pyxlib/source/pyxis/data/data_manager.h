#ifndef PYXIS__DATA__DATA_MANAGER_H
#define PYXIS__DATA__DATA_MANAGER_H
/******************************************************************************
data_manager.h

begin		: 2007-01-22
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/data_source.h"
#include "pyxis/utility/pointer.h"
#include "pyxis/utility/pyxcom.h"

// standard includes
#include <string>

/*!
*/
//! Data manager.
class PYXLIB_DECL DataManager
{
public:

	// TODO iterator to registered drivers
	// ability to register or deregister a driver (or just have this class take care of everything internally)
	// ability to open a data source

	static PYXPointer<PYXDataSource> open(const std::string& strURI);

private:

	static void initialize();

	static void uninitialize();

private:

	//! For initialization.
	friend class PYXLibInstance;
};

#endif // guard
