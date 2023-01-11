// unit_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "pyxlib_instance.h"
#include "pyxis/utility/tester.h"

int _tmain(int argc, _TCHAR* argv[])
{
	PYXLibInstance pyxLib("unit_test (C++)");

	// Run unit tests
	bool bSuccess = TestFrame::getInstance().testUnit();

	return bSuccess ? 0 : 1;
}
