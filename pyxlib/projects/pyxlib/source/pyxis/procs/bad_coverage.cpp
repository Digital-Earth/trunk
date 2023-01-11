/******************************************************************************
bad_coverage.cpp

begin		: 2006-04-18
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "bad_coverage.h" 

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/utility/tester.h"

// {BF12B7D4-9FC7-4640-A5C5-141C97EB4639}
PYXCOM_DEFINE_CLSID(BadCoverage,
0xbf12b7d4, 0x9fc7, 0x4640, 0xa5, 0xc5, 0x14, 0x1c, 0x97, 0xeb, 0x46, 0x39);
PYXCOM_CLASS_INTERFACES(BadCoverage, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BadCoverage, "Bad Value", "A coverage data set that will not initialize (for testing).", "Development/Diagnostic",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

// Tester class
Tester<BadCoverage> gTester;

// Test method
void BadCoverage::test()
{
	boost::intrusive_ptr<IProcess> spTestCoverageProcess(new BadCoverage);
	BadCoverage* pBadCoverage = dynamic_cast<BadCoverage*>(spTestCoverageProcess.get());
	assert(pBadCoverage != 0);

	TEST_ASSERT(spTestCoverageProcess->initProc() == knFailedToInit);
}

BadCoverage::BadCoverage() 
{
}

IProcess::eInitStatus BadCoverage::initImpl()
{
	return knFailedToInit;
}
