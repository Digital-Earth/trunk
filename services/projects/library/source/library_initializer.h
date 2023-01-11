#ifndef LIBRARY_INITIALIZER_H
#define LIBRARY_INITIALIZER_H
/******************************************************************************
library_initializer.h

begin      : 08/03/2007 4:53:01 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "library_config.h"

// local includes

// PYXLib includes

// forward declarations

//! Initializer for the library module.
class LIBRARY_DECL LibraryInitializer
{
public:

	//! Initialize the library for use, if not already initialized
	LibraryInitializer();

	//! Clean up the library after use, if not already cleaned up
	~LibraryInitializer();

	//! Initialize the library for use, if not already initialized
	static void initialize();

	//! Clean up the library after use, if not already cleaned up.
	static void uninitialize();

private:

	//! Reference count, permitting "nifty counter" usage for the class.
	static int m_nCount;
};

#endif
