#ifndef PYXIS__PROCS__PROC_UTILS_H
#define PYXIS__PROCS__PROC_UTILS_H
/******************************************************************************
proc_utils.h

begin		: 2007-04-13
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

/*!
*/
//! PYXLIB processes and pyxcom objects.
class PYXLIB_DECL ProcUtils
{
public:

	//! For initialization.
	static void initialize();

	//! For initialization.
	static void uninitialize();

	//! Test method.
	static void test();

};

#endif // guard
