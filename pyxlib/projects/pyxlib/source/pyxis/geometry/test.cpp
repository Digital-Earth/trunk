/******************************************************************************
test.cpp

begin		: 2004-09-09
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/test.h"

// pyxlib includes
#include "pyxis/derm/exhaustive_iterator.h"

/*!
Use recursion to resolve uncertainty.

\param index	The index to process.
\return True if yes or yes-complete; false if no.
*/
bool PYXTest::resolveMaybeResult(const PYXIcosIndex& index)
{
	bool bResult = false;

	// Traverse children
	PYXExhaustiveIterator it(index, index.getResolution() + 1);
	for (; !it.end(); it.next())
	{
		// Perform the test
		bool bAbort = false;
		eTestResult nChildResult = doTestIndex(it.getIndex(), &bAbort);

		if (nChildResult == knYes ||
				nChildResult == knYesComplete)
		{
			bResult = true;
			break;
		}
		else if (nChildResult == knMaybe &&
			it.getIndex().getResolution() < getDataResolution())
		{
			bResult = resolveMaybeResult(it.getIndex());
			if (bResult)
			{
				break;
			}
		}
	}

	return bResult;
}

/*!
\param	index			The index to test.
\param	pbAbort			true to stop the test immediately, initialized to
						false on entry (out).
\return	The test result.
*/
PYXTest::eTestResult PYXTest::testIndex(	const PYXIcosIndex& index,
											bool* pbAbort	)
{
	eTestResult nResult = doTestIndex(index, pbAbort);

	if (nResult == knMaybe &&
		index.getResolution() == getTargetResolution() &&
		index.getResolution() < getDataResolution())
	{
		nResult = resolveMaybeResult(index) ? knYes : knNo;
	}

	if (m_bAbortOnYes && (nResult == knYes || nResult == knYesComplete))
	{
		*pbAbort = true;
	}

	return nResult;
}
