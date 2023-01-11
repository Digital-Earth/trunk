#ifndef PYXIS__GEOMETRY__TEST_H
#define PYXIS__GEOMETRY__TEST_H
/******************************************************************************
test.h

begin		: 2004-09-09
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// forward declarations
class PYXIcosIndex;

//! Abstract base for tests performed by the PYXIcosTestTraverser.
/*!
PYXTest defines the interface for tests performed by the PYXIcosTestTraverser.
The test specifies whether or not a particular index is recorded in the result
set and whether or not a particular index's children are tested.
*/
class PYXLIB_DECL PYXTest
{
public:

	//! The test result.
	enum eTestResult
	{
		//! The test failed.
		knNo = 0,			
	
		//! The test succeeded. Need to examine children.
		knYes,				

		//! The test succeeded. All children passed the test.
		knYesComplete,		

		//! The test result was uncertain, need to examine children.
		knMaybe				
	}; 

	//! Constructor
	PYXTest() :
		m_nDataResolution(-1),
		m_nTargetResolution(-1),
		m_bAbortOnYes(false) {}

	//! Destructor
	virtual ~PYXTest() {}

	/*!
	Perform any initialization required before the test is run.
	Override to do initialization appropriate for the test.
	*/
	virtual void initialize() {}

	/*!
	Get the data resolution.
	
	\return	The data resolution.
	*/
	inline int getDataResolution() const {return m_nDataResolution;}

	/*!
	Set the data resolution.

	\param	nDataResolution	The data resolution.
	*/
	void setDataResolution(int nDataResolution) {m_nDataResolution = nDataResolution;}

	/*!
	Get the target resolution.

	\return	The target resolution.
	*/
	inline int getTargetResolution() const {return m_nTargetResolution;}

	/*!
	Set the target resolution.

	\param	nTargetResolution	The target resolution.
	*/
	void setTargetResolution(int nTargetResolution) {m_nTargetResolution = nTargetResolution;}

	/*!
	Set whether the test aborts on yes or yes complete.

	\param	bAbortOnYes Whether to abort.
	*/
	void setAbortOnYes(bool bAbortOnYes) {m_bAbortOnYes = bAbortOnYes;}

	//! Perform the test.
	eTestResult testIndex(	const PYXIcosIndex& index,
							bool* pbAbort	);

private:

	//! Perform the test.
	/*!
	Perform the test.

	\param	index			The index to test.
	\param	pbAbort			true to stop the test immediately, initialized to
							false on entry (out).

	\return	The test result.
	*/
	virtual eTestResult doTestIndex(	const PYXIcosIndex& index,
										bool* pbAbort	) = 0;

	//! Resolve maybe result into definite yes or no
	bool resolveMaybeResult(const PYXIcosIndex& index);

private:

	//! The resolution of the data.
	int m_nDataResolution;

	//! The resolution at which to record.
	int m_nTargetResolution;

	//! Whether to abort on yes or yes complete.
	bool m_bAbortOnYes;
};

#endif // guard
