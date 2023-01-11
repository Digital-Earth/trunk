/******************************************************************************
addr.cpp

begin		: 2007-10-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "addr.h"

namespace
{

struct AutoTest
{
	AutoTest()
	{
		Addr a1("0123");
		Addr a2("0123456");
		Addr a3("01234560123456");

		assert(a1.toString() == "0123");
		assert(a2.toString() == "0123456");
		assert(a3.toString() == "01234560123456");

		assert(a1 == a1 && !(a1 != a1));
		assert(a2 == a2 && !(a2 != a2));
		assert(a3 == a3 && !(a3 != a3));
		assert((a1 != a2 && a2 != a1) && !(a1 == a2 || a2 == a1));
		assert((a2 != a3 && a3 != a2) && !(a2 == a3 || a3 == a2));
		assert((a3 != a1 && a1 != a3) && !(a3 == a1 || a1 == a3));

		Addr array[2];
		int n = sizeof(Addr);
		int narray = sizeof(array);
	}
} autoTest;

}
