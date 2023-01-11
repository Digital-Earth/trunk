#ifndef PYXLIB_INSTANCE_H
#define PYXLIB_INSTANCE_H
/******************************************************************************
pyxlib_instance.h

begin		: 2006-11-06
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// standard includes
#include <string>

// boost includes
#include <boost/detail/atomic_count.hpp>

/*!
This class employs a reference count, ensuring that the initialization
and deinitialization only happens once.

This allows use as a "nifty counter" (also called the "Schwarz counter").
This can be done by creating a static instance in a common header 
in the library.

Note, however, that there are currently issues associated with this
approach: static globals with run-time initializers (vs. compile time)
do not get propertly initialized before the static nifty-counter class
is created.  As such, client code is required to create an instance of
this class manually, in the "main" function for example, prior to 
making calls into the library.
*/
//! Initializes static variables.
class PYXLIB_DECL PYXLibInstance
{
public:

	//! Initialize the library for use, if not already initialized
	PYXLibInstance(	const std::string& strAppName,
					bool bClearCache,
					const std::string& strWorkingDirectory = "", 
					const std::string& strAppDirectory = "",
					const std::string& strCacheDir = "");

	//! Clean up the library after use, if not already cleaned up
	~PYXLibInstance();

	//! Initialize the library for use, if not already initialized
	static void initialize(	const std::string& strAppName,
							bool bClearCache,
							const std::string& strWorkingDirectory = "", 
							const std::string& strAppDirectory = "",
							const std::string& strCacheDir = "");

	//! Clean up the library after use, if not already cleaned up
	static void uninitialize();

private:

	//! Initialize the procs module
	static void initProcs();

	//! Clean up the procs module
	static void uninitProcs();

	//! Initialize the pipe module
	static void initPipe();

	//! Clean up the pipe module
	static void uninitPipe();

	//! Initialize the geometry module
	static void initGeometry();

	//! Destroy the geometry module
	static void uninitGeometry();

	//! Initialize the derm module
	static void initDerm();

	//! Destroy the derm module
	static void uninitDerm();

	//! Initialize the utility module
	static void initUtility(	const std::string& strAppName,
								bool bClearCache,
								const std::string& strWorkingDirectory, 
								const std::string& strAppDirectory,
								const std::string& strCacheDir );

	//! Destroy the utility module
	static void uninitUtility();

private:

	//! Reference count, permitting "nifty counter" usage for the class.
	static boost::detail::atomic_count m_nCount;
};

#endif // guard
